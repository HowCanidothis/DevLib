#include "fluid.h"
#include <NvFlex.h>

#include "shadowmap.h"

#include "mesh.h"

#define glVerify(x) f->x

typedef unsigned int VertexBuffer;
typedef unsigned int IndexBuffer;
typedef unsigned int Texture;

//------------------------------------------------------------------------------
// Diffuse Shading

const char *vertexDiffuseShader = QT_STRINGIFY(

uniform float pointRadius;  // point size in world space
uniform float pointScale;   // scale to calculate size in pixels
uniform vec3 lightPos;
uniform vec3 lightDir;
uniform mat4 lightTransform;
uniform float spotMin;
uniform float spotMax;
uniform vec4 color;


void main()
{
    vec3 worldPos = gl_Vertex.xyz;// - vec3(0.0, 0.1*0.25, 0.0);	// hack move towards ground to account for anisotropy;
    vec4 eyePos = gl_ModelViewMatrix * vec4(worldPos, 1.0);

    gl_Position = gl_ProjectionMatrix * eyePos;
    //gl_Position.z -= 0.0025;	// bias above fluid surface

    // calculate window-space point size
    gl_PointSize = pointRadius * (pointScale / gl_Position.w);

    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_TexCoord[1] = vec4(worldPos, gl_Vertex.w);
    gl_TexCoord[2] = eyePos;

    gl_TexCoord[3].xyz = gl_ModelViewMatrix*vec4(gl_MultiTexCoord1.xyz, 0.0);
    gl_TexCoord[4].xyzw = color;

    // hack to color different emitters
    if (gl_MultiTexCoord1.w == 2.0)
        gl_TexCoord[4].xyzw = vec4(0.85, 0.65, 0.65, color.w);
    else if (gl_MultiTexCoord1.w == 1.0)
        gl_TexCoord[4].xyzw = vec4(0.65, 0.85, 0.65, color.w);

    // compute ndc pos for frustrum culling in GS
    vec4 ndcPos = gl_ModelViewProjectionMatrix * vec4(worldPos.xyz, 1.0);
    gl_TexCoord[5] = ndcPos / ndcPos.w;
}
);




const char *geometryDiffuseShader =
"#version 120\n"
"#extension GL_EXT_geometry_shader4 : enable\n"
QT_STRINGIFY(

uniform float pointScale;  // point size in world space
uniform float motionBlurScale;
uniform float diffusion;
uniform vec3 lightDir;

void main()
{
    vec4 ndcPos = gl_TexCoordIn[0][5];

    // frustrum culling
    const float ndcBound = 1.0;
    if (ndcPos.x < -ndcBound) return;
    if (ndcPos.x > ndcBound) return;
    if (ndcPos.y < -ndcBound) return;
    if (ndcPos.y > ndcBound) return;

    float velocityScale = 1.0;

    vec3 v = gl_TexCoordIn[0][3].xyz*velocityScale;
    vec3 p = gl_TexCoordIn[0][2].xyz;

    // billboard in eye space
    vec3 u = vec3(0.0, pointScale, 0.0);
    vec3 l = vec3(pointScale, 0.0, 0.0);

    // increase size based on life
    float lifeFade = mix(1.0f+diffusion, 1.0, min(1.0, gl_TexCoordIn[0][1].w*0.25f));
    u *= lifeFade;
    l *= lifeFade;

    //lifeFade = 1.0;

    float fade = 1.0/(lifeFade*lifeFade);
    float vlen = length(v)*motionBlurScale;

    if (vlen > 0.5)
    {
        float len = max(pointScale, vlen*0.016);
        fade = min(1.0, 2.0/(len/pointScale));

        u = normalize(v)*max(pointScale, vlen*0.016);	// assume 60hz
        l = normalize(cross(u, vec3(0.0, 0.0, -1.0)))*pointScale;
    }

    {

        gl_TexCoord[1] = gl_TexCoordIn[0][1];	// vertex world pos (life in w)
        gl_TexCoord[2] = gl_TexCoordIn[0][2];	// vertex eye pos
        gl_TexCoord[3] = gl_TexCoordIn[0][3];	// vertex velocity in view space
        gl_TexCoord[3].w = fade;
        gl_TexCoord[4] = gl_ModelViewMatrix*vec4(lightDir, 0.0);
        gl_TexCoord[4].w = gl_TexCoordIn[0][3].w; // attenuation
        gl_TexCoord[5].xyzw = gl_TexCoordIn[0][4].xyzw;	// color

        float zbias = 0.0f;//0.00125*2.0;

        gl_TexCoord[0] = vec4(0.0, 1.0, 0.0, 0.0);
        gl_Position = gl_ProjectionMatrix * vec4(p + u - l, 1.0);
        gl_Position.z -= zbias;
        EmitVertex();

        gl_TexCoord[0] = vec4(0.0, 0.0, 0.0, 0.0);
        gl_Position = gl_ProjectionMatrix * vec4(p - u - l, 1.0);
        gl_Position.z -= zbias;
        EmitVertex();

        gl_TexCoord[0] = vec4(1.0, 1.0, 0.0, 0.0);
        gl_Position = gl_ProjectionMatrix * vec4(p + u + l, 1.0);
        gl_Position.z -= zbias;
        EmitVertex();

        gl_TexCoord[0] = vec4(1.0, 0.0, 0.0, 0.0);
        gl_Position = gl_ProjectionMatrix * vec4(p - u + l, 1.0);
        gl_Position.z -= zbias;
        EmitVertex();
    }
}
);

const char *fragmentDiffuseShader = QT_STRINGIFY(

float sqr(float x) { return x*x; }
float cube(float x) { return x*x*x; }

uniform sampler2D depthTex;
uniform sampler2D noiseTex;
uniform vec2 invViewport;
uniform vec4 color;
uniform bool front;
uniform bool shadow;

//uniform sampler2DShadow shadowTex;
uniform sampler2D shadowTex;
uniform vec2 shadowTaps[12];
uniform mat4 lightTransform;
uniform vec3 lightDir;
uniform float inscatterCoefficient;
uniform float outscatterCoefficient;

void main()
{
    float attenuation = gl_TexCoord[4].w;
    float lifeFade = min(1.0, gl_TexCoord[1].w*0.125);

    // calculate normal from texture coordinates
    vec3 normal;
    normal.xy = gl_TexCoord[0].xy*vec2(2.0, 2.0) + vec2(-1.0, -1.0);
    float mag = dot(normal.xy, normal.xy);
    if (mag > 1.0) discard;   // kill pixels outside circle
    normal.z = 1.0-mag;

    float velocityFade = gl_TexCoord[3].w;
    float alpha = lifeFade*velocityFade*sqr(normal.z);

    gl_FragColor = alpha;
}
);

//-----------------------------------------------------------------------------------------------------
// vertex shader

const char *vertexPointDepthShader = QT_STRINGIFY(

uniform float pointRadius;  // point size in world space
uniform float pointScale;   // scale to calculate size in pixels

void main()
{
    // calculate window-space point size
    gl_Position = gl_ModelViewProjectionMatrix * vec4(gl_Vertex.xyz, 1.0);
    gl_PointSize = pointScale * (pointRadius / gl_Position.w);

    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_TexCoord[1] = gl_ModelViewMatrix * vec4(gl_Vertex.xyz, 1.0);
}
);

/*// pixel shader for rendering points as shaded spheres
const char *fragmentPointDepthShader = QT_STRINGIFY(

uniform float pointRadius;  // point size in world space

void main()
{
    // calculate normal from texture coordinates
    vec3 normal;
    normal.xy = gl_TexCoord[0].xy*vec2(2.0, -2.0) + vec2(-1.0, 1.0);
    float mag = dot(normal.xy, normal.xy);
    if (mag > 1.0) discard;   // kill pixels outside circle
    normal.z = sqrt(1.0-mag);

    vec3 eyePos = gl_TexCoord[1].xyz + normal*pointRadius*2.0;
    vec4 ndcPos = gl_ProjectionMatrix * vec4(eyePos, 1.0);
    ndcPos.z /= ndcPos.w;

    gl_FragColor = vec4(eyePos.z, 1.0, 1.0, 1.0);
    gl_FragDepth = ndcPos.z*0.5 + 0.5;
}
);*/


// pixel shader for rendering points density
const char *fragmentPointThicknessShader = QT_STRINGIFY(

void main()
{
    // calculate normal from texture coordinates
    vec3 normal;
    normal.xy = gl_TexCoord[0].xy*vec2(2.0, -2.0) + vec2(-1.0, 1.0);
    float mag = dot(normal.xy, normal.xy);
    if (mag > 1.0) discard;   // kill pixels outside circle
    normal.z = sqrt(1.0-mag);

    gl_FragColor = vec4(normal.z*0.005);
}
);

//--------------------------------------------------------
// Ellipsoid shaders
//
const char *vertexEllipsoidDepthShader = "#version 120\n" QT_STRINGIFY(

// rotation matrix in xyz, scale in w
attribute vec4 q1;
attribute vec4 q2;
attribute vec4 q3;

// returns 1.0 for x==0.0 (unlike glsl)
float Sign(float x) { return x < 0.0 ? -1.0: 1.0; }

bool solveQuadratic(float a, float b, float c, out float minT, out float maxT)
{
    if (a == 0.0 && b == 0.0)
    {
        minT = maxT = 0.0;
        return false;
    }

    float discriminant = b*b - 4.0*a*c;

    if (discriminant < 0.0)
    {
        return false;
    }

    float t = -0.5*(b + Sign(b)*sqrt(discriminant));
    minT = t / a;
    maxT = c / t;

    if (minT > maxT)
    {
        float tmp = minT;
        minT = maxT;
        maxT = tmp;
    }

    return true;
}

float DotInvW(vec4 a, vec4 b) {	return a.x*b.x + a.y*b.y + a.z*b.z - a.w*b.w; }

void main()
{
    vec3 worldPos = gl_Vertex.xyz;// - vec3(0.0, 0.1*0.25, 0.0);	// hack move towards ground to account for anisotropy

    // construct quadric matrix
    mat4 q;
    q[0] = vec4(q1.xyz*q1.w, 0.0);
    q[1] = vec4(q2.xyz*q2.w, 0.0);
    q[2] = vec4(q3.xyz*q3.w, 0.0);
    q[3] = vec4(worldPos, 1.0);

    // transforms a normal to parameter space (inverse transpose of (q*modelview)^-T)
    mat4 invClip = transpose(gl_ModelViewProjectionMatrix*q);

    // solve for the right hand bounds in homogenous clip space
    float a1 = DotInvW(invClip[3], invClip[3]);
    float b1 = -2.0f*DotInvW(invClip[0], invClip[3]);
    float c1 = DotInvW(invClip[0], invClip[0]);

    float xmin;
    float xmax;
    solveQuadratic(a1, b1, c1, xmin, xmax);

    // solve for the right hand bounds in homogenous clip space
    float a2 = DotInvW(invClip[3], invClip[3]);
    float b2 = -2.0f*DotInvW(invClip[1], invClip[3]);
    float c2 = DotInvW(invClip[1], invClip[1]);

    float ymin;
    float ymax;
    solveQuadratic(a2, b2, c2, ymin, ymax);

    gl_Position = vec4(worldPos.xyz, 1.0);
    gl_TexCoord[0] = vec4(xmin, xmax, ymin, ymax);

    // construct inverse quadric matrix (used for ray-casting in parameter space)
    mat4 invq;
    invq[0] = vec4(q1.xyz/q1.w, 0.0);
    invq[1] = vec4(q2.xyz/q2.w, 0.0);
    invq[2] = vec4(q3.xyz/q3.w, 0.0);
    invq[3] = vec4(0.0, 0.0, 0.0, 1.0);

    invq = transpose(invq);
    invq[3] = -(invq*gl_Position);

    // transform a point from view space to parameter space
    invq = invq*gl_ModelViewMatrixInverse;

    // pass down
    gl_TexCoord[1] = invq[0];
    gl_TexCoord[2] = invq[1];
    gl_TexCoord[3] = invq[2];
    gl_TexCoord[4] = invq[3];

    // compute ndc pos for frustrum culling in GS
    vec4 ndcPos = gl_ModelViewProjectionMatrix * vec4(worldPos.xyz, 1.0);
    gl_TexCoord[5] = ndcPos / ndcPos.w;
}
);

// pixel shader for rendering points as shaded spheres
const char *fragmentEllipsoidDepthShader = "#version 120\n" QT_STRINGIFY(

uniform vec3 invViewport;
uniform vec3 invProjection;

float Sign(float x) { return x < 0.0 ? -1.0: 1.0; }

bool solveQuadratic(float a, float b, float c, out float minT, out float maxT)
{
    if (a == 0.0 && b == 0.0)
    {
        minT = maxT = 0.0;
        return true;
    }

    float discriminant = b*b - 4.0*a*c;

    if (discriminant < 0.0)
    {
        return false;
    }

    float t = -0.5*(b + Sign(b)*sqrt(discriminant));
    minT = t / a;
    maxT = c / t;

    if (minT > maxT)
    {
        float tmp = minT;
        minT = maxT;
        maxT = tmp;
    }

    return true;
}

float sqr(float x) { return x*x; }

void main()
{
    // transform from view space to parameter space
    mat4 invQuadric;
    invQuadric[0] = gl_TexCoord[0];
    invQuadric[1] = gl_TexCoord[1];
    invQuadric[2] = gl_TexCoord[2];
    invQuadric[3] = gl_TexCoord[3];

    vec4 ndcPos = vec4(gl_FragCoord.xy*invViewport.xy*vec2(2.0, 2.0) - vec2(1.0, 1.0), -1.0, 1.0);
    vec4 viewDir = gl_ProjectionMatrixInverse*ndcPos;

    // ray to parameter space
    vec4 dir = invQuadric*vec4(viewDir.xyz, 0.0);
    vec4 origin = invQuadric[3];

    // set up quadratric equation
    float a = sqr(dir.x) + sqr(dir.y) + sqr(dir.z);// - sqr(dir.w);
    float b = dir.x*origin.x + dir.y*origin.y + dir.z*origin.z - dir.w*origin.w;
    float c = sqr(origin.x) + sqr(origin.y) + sqr(origin.z) - sqr(origin.w);

    float minT;
    float maxT;

    if (solveQuadratic(a, 2.0*b, c, minT, maxT))
    {
        vec3 eyePos = viewDir.xyz*minT;
        vec4 ndcPos = gl_ProjectionMatrix * vec4(eyePos, 1.0);
        ndcPos.z /= ndcPos.w;

        gl_FragColor = vec4(eyePos.z, 1.0, 1.0, 1.0);
        gl_FragDepth = ndcPos.z*0.5 + 0.5;

        return;
    }
    else
        discard;

    gl_FragColor = vec4(0.5, 0.0, 0.0, 1.0);
}
);

const char* geometryEllipsoidDepthShader =
"#version 120\n"
"#extension GL_EXT_geometry_shader4 : enable\n"
QT_STRINGIFY(
void main()
{
    vec3 pos = gl_PositionIn[0].xyz;
    vec4 bounds = gl_TexCoordIn[0][0];
    vec4 ndcPos = gl_TexCoordIn[0][5];

    // frustrum culling
    const float ndcBound = 1.0;
    if (ndcPos.x < -ndcBound) return;
    if (ndcPos.x > ndcBound) return;
    if (ndcPos.y < -ndcBound) return;
    if (ndcPos.y > ndcBound) return;

    float xmin = bounds.x;
    float xmax = bounds.y;
    float ymin = bounds.z;
    float ymax = bounds.w;

    // inv quadric transform
    gl_TexCoord[0] = gl_TexCoordIn[0][1];
    gl_TexCoord[1] = gl_TexCoordIn[0][2];
    gl_TexCoord[2] = gl_TexCoordIn[0][3];
    gl_TexCoord[3] = gl_TexCoordIn[0][4];

    gl_Position = vec4(xmin, ymax, 0.0, 1.0);
    EmitVertex();

    gl_Position = vec4(xmin, ymin, 0.0, 1.0);
    EmitVertex();

    gl_Position = vec4(xmax, ymax, 0.0, 1.0);
    EmitVertex();

    gl_Position = vec4(xmax, ymin, 0.0, 1.0);
    EmitVertex();
}
);

//--------------------------------------------------------------------------------
// Composite shaders

const char* vertexPassThroughShader = QT_STRINGIFY(

void main()
{
    gl_Position = vec4(gl_Vertex.xyz, 1.0);
    gl_TexCoord[0] = gl_MultiTexCoord0;
}
);

const char* fragmentBlurDepthShader =
"#extension GL_ARB_texture_rectangle : enable\n"
QT_STRINGIFY(

uniform sampler2DRect depthTex;
uniform sampler2D thicknessTex;
uniform float blurRadiusWorld;
uniform float blurScale;
uniform float blurFalloff;
uniform vec2 invTexScale;

uniform bool debug;

float sqr(float x) { return x*x; }

void main()
{
    // eye-space depth of center sample
    float depth = texture2DRect(depthTex, gl_FragCoord.xy).x;
    float thickness = texture2D(thicknessTex, gl_TexCoord[0].xy).x;

    // hack: ENABLE_SIMPLE_FLUID
    //thickness = 0.0f;

    if (debug)
    {
        // do not blur
        gl_FragColor.x = depth;
        return;
    }

    // threshold on thickness to create nice smooth silhouettes
    if (depth == 0.0)//|| thickness < 0.02f)
    {
        gl_FragColor.x = 0.0;
        return;
    }

    /*
    float dzdx = dFdx(depth);
    float dzdy = dFdy(depth);

    // handle edge case
    if (max(abs(dzdx), abs(dzdy)) > 0.05)
    {
        dzdx = 0.0;
        dzdy = 0.0;

        gl_FragColor.x = depth;
        return;
    }
    */

    float blurDepthFalloff = 5.5;//blurFalloff*mix(4.0, 1.0, thickness)/blurRadiusWorld*0.0375;	// these constants are just a re-scaling from some known good values

    float maxBlurRadius = 5.0;
    //float taps = min(maxBlurRadius, blurScale * (blurRadiusWorld / -depth));
    //vec2 blurRadius = min(mix(0.25, 2.0/blurFalloff, thickness) * blurScale * (blurRadiusWorld / -depth) / taps, 0.15)*invTexScale;

    //discontinuities between different tap counts are visible. to avoid this we
    //use fractional contributions between #taps = ceil(radius) and floor(radius)
    float radius = min(maxBlurRadius, blurScale * (blurRadiusWorld / -depth));
    float radiusInv = 1.0/radius;
    float taps = ceil(radius);
    float frac = taps - radius;

    float sum = 0.0;
    float wsum = 0.0;
    float count = 0.0;

    for(float y=-taps; y <= taps; y += 1.0)
    {
        for(float x=-taps; x <= taps; x += 1.0)
        {
            vec2 offset = vec2(x, y);

            float sample = texture2DRect(depthTex, gl_FragCoord.xy + offset).x;

            if (sample < -10000.0*0.5)
                continue;

            // spatial domain
            float r1 = length(vec2(x, y))*radiusInv;
            float w = exp(-(r1*r1));

            //float expectedDepth = depth + dot(vec2(dzdx, dzdy), offset);

            // range domain (based on depth difference)
            float r2 = (sample - depth) * blurDepthFalloff;
            float g = exp(-(r2*r2));

            //fractional radius contributions
            float wBoundary = step(radius, max(abs(x), abs(y)));
            float wFrac = 1.0 - wBoundary*frac;

            sum += sample * w * g * wFrac;
            wsum += w * g * wFrac;
            count += g * wFrac;
        }
    }

    if (wsum > 0.0) {
        sum /= wsum;
    }

    float blend = count/sqr(2.0*radius+1.0);
    gl_FragColor.x = mix(depth, sum, blend);
}
);

const char* fragmentCompositeShader = QT_STRINGIFY(

uniform sampler2D tex;
uniform vec2 invTexScale;
uniform vec3 lightPos;
uniform vec3 lightDir;
uniform float spotMin;
uniform float spotMax;
uniform vec4 color;
uniform float ior;

uniform vec2 clipPosToEye;

uniform sampler2D reflectTex;
uniform sampler2DShadow shadowTex;
uniform vec2 shadowTaps[12];
uniform mat4 lightTransform;

uniform sampler2D thicknessTex;
uniform sampler2D sceneTex;

uniform bool debug;

// sample shadow map
float shadowSample(vec3 worldPos, out float attenuation)
{
    // hack: ENABLE_SIMPLE_FLUID
    //attenuation = 0.0f;
    //return 0.5;

    vec4 pos = lightTransform*vec4(worldPos+lightDir*0.15, 1.0);
    pos /= pos.w;
    vec3 uvw = (pos.xyz*0.5)+vec3(0.5);

    attenuation = max(smoothstep(spotMax, spotMin, dot(pos.xy, pos.xy)), 0.05);

    // user clip
    if (uvw.x  < 0.0 || uvw.x > 1.0)
        return 1.0;
    if (uvw.y < 0.0 || uvw.y > 1.0)
        return 1.0;

    float s = 0.0;
    float radius = 0.002;

    for (int i=0; i < 8; i++)
    {
        s += shadow2D(shadowTex, vec3(uvw.xy + shadowTaps[i]*radius, uvw.z)).r;
    }

    s /= 8.0;
    return s;
}

vec3 viewportToEyeSpace(vec2 coord, float eyeZ)
{
    // find position at z=1 plane
    vec2 uv = (coord*2.0 - vec2(1.0))*clipPosToEye;

    return vec3(-uv*eyeZ, eyeZ);
}

vec3 srgbToLinear(vec3 c) { return pow(c, vec3(2.2)); }
vec3 linearToSrgb(vec3 c) { return pow(c, vec3(1.0/2.2)); }

float sqr(float x) { return x*x; }
float cube(float x) { return x*x*x; }

void main()
{
    float eyeZ = texture2D(tex, gl_TexCoord[0].xy).x;

    if (eyeZ == 0.0)
        discard;

    // reconstruct eye space pos from depth
    vec3 eyePos = viewportToEyeSpace(gl_TexCoord[0].xy, eyeZ);

    // finite difference approx for normals, can't take dFdx because
    // the one-sided difference is incorrect at shape boundaries
    vec3 zl = eyePos - viewportToEyeSpace(gl_TexCoord[0].xy - vec2(invTexScale.x, 0.0), texture2D(tex, gl_TexCoord[0].xy - vec2(invTexScale.x, 0.0)).x);
    vec3 zr = viewportToEyeSpace(gl_TexCoord[0].xy + vec2(invTexScale.x, 0.0), texture2D(tex, gl_TexCoord[0].xy + vec2(invTexScale.x, 0.0)).x) - eyePos;
    vec3 zt = viewportToEyeSpace(gl_TexCoord[0].xy + vec2(0.0, invTexScale.y), texture2D(tex, gl_TexCoord[0].xy + vec2(0.0, invTexScale.y)).x) - eyePos;
    vec3 zb = eyePos - viewportToEyeSpace(gl_TexCoord[0].xy - vec2(0.0, invTexScale.y), texture2D(tex, gl_TexCoord[0].xy - vec2(0.0, invTexScale.y)).x);

    vec3 dx = zl;
    vec3 dy = zt;

    if (abs(zr.z) < abs(zl.z))
        dx = zr;

    if (abs(zb.z) < abs(zt.z))
        dy = zb;

    //vec3 dx = dFdx(eyePos.xyz);
    //vec3 dy = dFdy(eyePos.xyz);

    vec4 worldPos = gl_ModelViewMatrixInverse*vec4(eyePos, 1.0);

    float attenuation;
    float shadow = shadowSample(worldPos.xyz, attenuation);

    vec3 l = (gl_ModelViewMatrix*vec4(lightDir, 0.0)).xyz;
    vec3 v = -normalize(eyePos);

    vec3 n = normalize(cross(dx, dy));
    vec3 h = normalize(v + l);

    vec3 skyColor = vec3(0.1, 0.2, 0.4)*1.2;
    vec3 groundColor = vec3(0.1, 0.1, 0.2);

    float fresnel = 0.1 + (1.0 - 0.1)*cube(1.0-max(dot(n, v), 0.0));

    vec3 lVec = normalize(worldPos.xyz-lightPos);

    float ln = dot(l, n)*attenuation;

    vec3 rEye = reflect(-v, n).xyz;
    vec3 rWorld = (gl_ModelViewMatrixInverse*vec4(rEye, 0.0)).xyz;

    vec2 texScale = vec2(0.75, 1.0);	// to account for backbuffer aspect ratio (todo: pass in)

    float refractScale = ior*0.025;
    float reflectScale = ior*0.1;

    // attenuate refraction near ground (hack)
    refractScale *= smoothstep(0.1, 0.4, worldPos.y);

    vec2 refractCoord = gl_TexCoord[0].xy + n.xy*refractScale*texScale;
    //vec2 refractCoord = gl_TexCoord[0].xy + refract(-v, n, 1.0/1.33)*refractScale*texScale;

    // read thickness from refracted coordinate otherwise we get halos around objectsw
    float thickness = max(texture2D(thicknessTex, refractCoord).x, 0.3);

    //vec3 transmission = exp(-(vec3(1.0)-color.xyz)*thickness);
    vec3 transmission = (1.0-(1.0-color.xyz)*thickness*0.8)*color.w;
    vec3 refract = texture2D(sceneTex, refractCoord).xyz*transmission;

    vec2 sceneReflectCoord = gl_TexCoord[0].xy - rEye.xy*texScale*reflectScale/eyePos.z;
    vec3 sceneReflect = (texture2D(sceneTex, sceneReflectCoord).xyz)*shadow;

    vec3 planarReflect = texture2D(reflectTex, gl_TexCoord[0].xy).xyz;
    planarReflect = vec3(0.0);

    // fade out planar reflections above the ground
    vec3 reflect = mix(planarReflect, sceneReflect, smoothstep(0.05, 0.3, worldPos.y)) + mix(groundColor, skyColor, smoothstep(0.15, 0.25, rWorld.y)*shadow);

    // lighting
    vec3 diffuse = color.xyz*mix(vec3(0.29, 0.379, 0.59), vec3(1.0), (ln*0.5 + 0.5)*max(shadow, 0.4))*(1.0-color.w);
    vec3 specular = vec3(1.2*pow(max(dot(h, n), 0.0), 400.0));

    gl_FragColor.xyz = diffuse + (mix(refract, reflect, fresnel) + specular)*color.w;
    gl_FragColor.w = 1.0;

    if (debug)
        gl_FragColor = vec4(n*0.5 + vec3(0.5), 1.0);

    // write valid z
    vec4 clipPos = gl_ProjectionMatrix*vec4(0.0, 0.0, eyeZ, 1.0);
    clipPos.z /= clipPos.w;

    gl_FragDepth = clipPos.z*0.5 + 0.5;
}
);

struct FluidRenderBuffersGL
{
    FluidRenderBuffersGL(OpenGLFunctions* f, int numParticles = 0):
        f(f),
        mPositionVBO(0),
        mDensityVBO(0),
        mIndices(0),
        mPositionBuf(nullptr),
        mDensitiesBuf(nullptr),
        mIndicesBuf(nullptr)
    {
        mNumParticles = numParticles;
        for (int i = 0; i < 3; i++)
        {
            mAnisotropyVBO[i] = 0;
            mAnisotropyBuf[i] = nullptr;
        }
    }
    ~FluidRenderBuffersGL()
    {
        f->glDeleteBuffers(1, &mPositionVBO);
        f->glDeleteBuffers(3, mAnisotropyVBO);
        f->glDeleteBuffers(1, &mDensityVBO);
        f->glDeleteBuffers(1, &mIndices);

        NvFlexUnregisterOGLBuffer(mPositionBuf);
        NvFlexUnregisterOGLBuffer(mDensitiesBuf);
        NvFlexUnregisterOGLBuffer(mIndicesBuf);

        NvFlexUnregisterOGLBuffer(mAnisotropyBuf[0]);
        NvFlexUnregisterOGLBuffer(mAnisotropyBuf[1]);
        NvFlexUnregisterOGLBuffer(mAnisotropyBuf[2]);
    }

    OpenGLFunctions* f;
    int mNumParticles;
    VertexBuffer mPositionVBO;
    VertexBuffer mDensityVBO;
    VertexBuffer mAnisotropyVBO[3];
    IndexBuffer mIndices;

    // wrapper buffers that allow Flex to write directly to VBOs
    NvFlexBuffer* mPositionBuf;
    NvFlexBuffer* mDensitiesBuf;
    NvFlexBuffer* mAnisotropyBuf[3];
    NvFlexBuffer* mIndicesBuf;
};

// vertex buffers for diffuse particles
struct DiffuseRenderBuffersGL
{
    DiffuseRenderBuffersGL(OpenGLFunctions* f, int numParticles = 0):
        f(f),
        mDiffusePositionVBO(0),
        mDiffuseVelocityVBO(0),
        mDiffuseIndicesIBO(0),
        mDiffuseIndicesBuf(nullptr),
        mDiffusePositionsBuf(nullptr),
        mDiffuseVelocitiesBuf(nullptr)
    {
        mNumParticles = numParticles;
    }
    ~DiffuseRenderBuffersGL()
    {
        if (mNumParticles > 0)
        {
            f->glDeleteBuffers(1, &mDiffusePositionVBO);
            f->glDeleteBuffers(1, &mDiffuseVelocityVBO);
            f->glDeleteBuffers(1, &mDiffuseIndicesIBO);

            NvFlexUnregisterOGLBuffer(mDiffuseIndicesBuf);
            NvFlexUnregisterOGLBuffer(mDiffusePositionsBuf);
            NvFlexUnregisterOGLBuffer(mDiffuseVelocitiesBuf);
        }
    }

    OpenGLFunctions* f;

    int mNumParticles;
    VertexBuffer mDiffusePositionVBO;
    VertexBuffer mDiffuseVelocityVBO;
    IndexBuffer mDiffuseIndicesIBO;

    NvFlexBuffer* mDiffuseIndicesBuf;
    NvFlexBuffer* mDiffusePositionsBuf;
    NvFlexBuffer* mDiffuseVelocitiesBuf;
};

void GlslPrintShaderLog(OpenGLFunctions* f, GLuint obj)
{
#if !PLATFORM_IOS
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

    GLint result;
    f->glGetShaderiv(obj, GL_COMPILE_STATUS, &result);

    // only print log if compile fails
    if (result == GL_FALSE)
    {
        f->glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

        if (infologLength > 1)
        {
            infoLog = (char *)malloc(infologLength);
            f->glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
            printf("%s\n",infoLog);
            free(infoLog);
        }
    }
#endif
}

GLuint FluidRenderer::CompileProgram(OpenGLFunctions* f, const char *vsource, const char *fsource, const char* gsource)
{
    GLuint vertexShader = GLuint(-1);
    GLuint geometryShader = GLuint(-1);
    GLuint fragmentShader = GLuint(-1);

    GLuint program = f->glCreateProgram();

    if (vsource)
    {
        vertexShader = f->glCreateShader(GL_VERTEX_SHADER);
        f->glShaderSource(vertexShader, 1, &vsource, 0);
        f->glCompileShader(vertexShader);
        GlslPrintShaderLog(f, vertexShader);
        f->glAttachShader(program, vertexShader);
    }

    if (fsource)
    {
        fragmentShader = f->glCreateShader(GL_FRAGMENT_SHADER);
        f->glShaderSource(fragmentShader, 1, &fsource, 0);
        f->glCompileShader(fragmentShader);
        GlslPrintShaderLog(f, fragmentShader);
        f->glAttachShader(program, fragmentShader);
    }

    if (gsource)
    {
        geometryShader = f->glCreateShader(GL_GEOMETRY_SHADER);
        f->glShaderSource(geometryShader, 1, &gsource, 0);
        f->glCompileShader(geometryShader);
        GlslPrintShaderLog(f, geometryShader);

        // hack, force billboard gs mode
        f->glAttachShader(program, geometryShader);
        f->glProgramParameteri ( program, GL_GEOMETRY_VERTICES_OUT_EXT, 4 ) ;
        f->glProgramParameteri ( program, GL_GEOMETRY_INPUT_TYPE_EXT, GL_POINTS ) ;
        f->glProgramParameteri ( program, GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP ) ;
    }

    f->glLinkProgram(program);

    // check if program linked
    GLint success = 0;
    f->glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) {
        char temp[256];
        f->glGetProgramInfoLog(program, 256, 0, temp);
        printf("Failed to link program:\n%s\n", temp);
        f->glDeleteProgram(program);
        program = 0;
    }

    return program;
}

void FluidRenderer::RenderDiffuse(DiffuseRenderBuffers* buffersIn, int n, float radius, float screenWidth, float screenAspect, float fov, Vector4F& color, Vector3F& lightPos, const Vector3F& lightTarget, const Matrix4& lightTransform, ShadowMap* shadowMap, float motionBlur, float inscatter, float outscatter, bool shadow, bool front)
{
    DiffuseRenderBuffersGL* buffers = reinterpret_cast<DiffuseRenderBuffersGL*>(buffersIn);
    static int sprogram = -1;
    if (sprogram == -1)
        sprogram = CompileProgram(f, vertexDiffuseShader, fragmentDiffuseShader, geometryDiffuseShader);

    int thicknessScale = 1;

    if (sprogram)
    {
        f->glEnable(GL_POINT_SPRITE);
        f->glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
        f->glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
        f->glDepthMask(GL_FALSE);
        f->glEnable(GL_DEPTH_TEST);
        f->glEnable(GL_BLEND);
        f->glDisable(GL_CULL_FACE);
        f->glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);


        f->glUseProgram(sprogram);
        f->glUniform1f( f->glGetUniformLocation(sprogram, "motionBlurScale"), motionBlur);
        f->glUniform1f( f->glGetUniformLocation(sprogram, "diffusion"), 1.0f);
        f->glUniform1f( f->glGetUniformLocation(sprogram, "pointScale"), radius*1.0f);
        f->glUniform1f( f->glGetUniformLocation(sprogram, "pointRadius"), screenWidth / float(thicknessScale) / (2.0f*screenAspect*tanf(fov*0.5f)));
        f->glUniform2fv( f->glGetUniformLocation(sprogram, "invViewport"), 1, &Vector2F(1.0f/screenWidth, screenAspect/screenWidth)[0]);
        f->glUniform4fv( f->glGetUniformLocation(sprogram, "color"), 1, &color[0]);
        f->glUniform1i( f->glGetUniformLocation(sprogram, "tex"), 0);
        f->glUniform1f( f->glGetUniformLocation(sprogram, "inscatterCoefficient"), inscatter);
        f->glUniform1f( f->glGetUniformLocation(sprogram, "outscatterCoefficient"), outscatter);

        GLint uLightTransform = f->glGetUniformLocation(sprogram, "lightTransform");
        f->glUniformMatrix4fv(uLightTransform, 1, false, lightTransform.constData());

        GLint uLightPos = f->glGetUniformLocation(sprogram, "lightPos");
        f->glUniform3fv(uLightPos, 1, &lightPos[0]);

        GLint uLightDir = f->glGetUniformLocation(sprogram, "lightDir");
        Vector3F norm = (lightTarget-lightPos).normalized();
        f->glUniform3fv(uLightDir, 1, &norm[0]);

        f->glUniform1f( f->glGetUniformLocation(sprogram, "spotMin"), 0.5f);
        f->glUniform1f( f->glGetUniformLocation(sprogram, "spotMax"), 1.f);

        Vector2F taps[] =
        {
            Vector2F(-0.326212f,-0.40581f),Vector2F(-0.840144f,-0.07358f),
            Vector2F(-0.695914f,0.457137f),Vector2F(-0.203345f,0.620716f),
            Vector2F(0.96234f,-0.194983f),Vector2F(0.473434f,-0.480026f),
            Vector2F(0.519456f,0.767022f),Vector2F(0.185461f,-0.893124f),
            Vector2F(0.507431f,0.064425f),Vector2F(0.89642f,0.412458f),
            Vector2F(-0.32194f,-0.932615f),Vector2F(-0.791559f,-0.59771f)
        };

        glVerify(glUniform2fv(f->glGetUniformLocation(sprogram, "shadowTaps"), 12, &taps[0][0]));
        glVerify(glUniform1i(f->glGetUniformLocation(sprogram, "noiseTex"), 2));
        glVerify(glUniform1i(f->glGetUniformLocation(sprogram, "shadowTex"), 1));
        glVerify(glUniform1i(f->glGetUniformLocation(sprogram, "depthTex"), 0));
        glVerify(glUniform1i(f->glGetUniformLocation(sprogram, "front"), front));
        glVerify(glUniform1i(f->glGetUniformLocation(sprogram, "shadow"), shadow));

        // noise tex
        //glActiveTexture(GL_TEXTURE2);
        //glEnable(GL_TEXTURE_2D);
        //glBindTexture(GL_TEXTURE_2D, noiseTex);

        // shadow tex
        f->glActiveTexture(GL_TEXTURE1);
        f->glEnable(GL_TEXTURE_2D);
        f->glBindTexture(GL_TEXTURE_2D, shadowMap->texture);
        glVerify(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE));
        //glVerify(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL));


        f->glActiveTexture(GL_TEXTURE0);
        f->glEnable(GL_TEXTURE_2D);
        f->glBindTexture(GL_TEXTURE_2D, mDepthSmoothTex);

        f->glClientActiveTexture(GL_TEXTURE1);
        f->glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glVerify(glBindBuffer(GL_ARRAY_BUFFER, buffers->mDiffuseVelocityVBO));
        f->glTexCoordPointer(4, GL_FLOAT, sizeof(float)*4, 0);

        f->glEnableClientState(GL_VERTEX_ARRAY);
        f->glBindBuffer(GL_ARRAY_BUFFER, buffers->mDiffusePositionVBO);
        f->glVertexPointer(4, GL_FLOAT, sizeof(float)*4, 0);

        f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        f->glDrawArrays(GL_POINTS, 0, n);

        f->glUseProgram(0);
        f->glBindBuffer(GL_ARRAY_BUFFER, 0);
        f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        f->glDisableClientState(GL_VERTEX_ARRAY);
        f->glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        f->glDisable(GL_POINT_SPRITE);
        f->glDisable(GL_BLEND);
        f->glDepthMask(GL_TRUE);

        glVerify(glActiveTexture(GL_TEXTURE2));
        glVerify(glDisable(GL_TEXTURE_2D));
        glVerify(glActiveTexture(GL_TEXTURE1));
        glVerify(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE));
        glVerify(glDisable(GL_TEXTURE_2D));
        glVerify(glActiveTexture(GL_TEXTURE0));
        glVerify(glDisable(GL_TEXTURE_2D));
    }
}

void FluidRenderer::DrawMesh(const Mesh* m, Color3F& color)
{
    if (m)
    {
        glVerify(glColor3fv(&color[0]));
        glVerify(glSecondaryColor3fv(&color[0]));

        glVerify(glBindBuffer(GL_ARRAY_BUFFER, 0));
        glVerify(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

        glVerify(glEnableClientState(GL_NORMAL_ARRAY));
        glVerify(glEnableClientState(GL_VERTEX_ARRAY));

        glVerify(glNormalPointer(GL_FLOAT, sizeof(float) * 3, &m->m_normals[0]));
        glVerify(glVertexPointer(3, GL_FLOAT, sizeof(float) * 3, &m->m_positions[0]));

        if (m->m_colours.size())
        {
            glVerify(glEnableClientState(GL_COLOR_ARRAY));
            glVerify(glColorPointer(4, GL_FLOAT, 0, &m->m_colours[0]));
        }

        glVerify(glDrawElements(GL_TRIANGLES, m->GetNumFaces() * 3, GL_UNSIGNED_INT, &m->m_indices[0]));

        glVerify(glDisableClientState(GL_VERTEX_ARRAY));
        glVerify(glDisableClientState(GL_NORMAL_ARRAY));

        if (m->m_colours.size())
            glVerify(glDisableClientState(GL_COLOR_ARRAY));
    }
}

void FluidRenderer::RenderFluid(FluidRenderBuffers* buffersIn,
                                int n,
                                int offset,
                                float radius,
                                float screenWidth,
                                float screenAspect,
                                float fov,
                                Vector3F& lightPos,
                                const Vector3F& lightTarget,
                                const Matrix4& lightTransform,
                                ShadowMap* shadowMap,
                                Vector4F& color,
                                float blur,
                                float ior,
                                GLuint msaaFbo,
                                Mesh* mesh // TODO. EBANUTCA
                                )
{
    FluidRenderBuffersGL* buffers = reinterpret_cast<FluidRenderBuffersGL*>(buffersIn);

    // Simple fluid
    // resolve msaa back buffer to texture
    glVerify(glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, msaaFbo));
    glVerify(glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, mSceneFbo));
    glVerify(glBlitFramebuffer(0, 0, GLsizei(screenWidth), GLsizei(screenWidth/screenAspect), 0, 0, GLsizei(screenWidth), GLsizei(screenWidth/screenAspect), GL_COLOR_BUFFER_BIT, GL_LINEAR));

    //thickness texture
    glVerify(glBindFramebuffer(GL_FRAMEBUFFER, mThicknessFbo));
    glVerify(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mThicknessTex, 0));
    glVerify(glDrawBuffer(GL_COLOR_ATTACHMENT0));

    f->glViewport(0, 0, GLsizei(screenWidth), GLsizei(screenWidth/screenAspect));
    f->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    f->glClear(GL_DEPTH_BUFFER_BIT);

    f->glDepthMask(GL_TRUE);
    f->glDisable(GL_CULL_FACE);

    if (mesh)
        DrawMesh(mesh, Vector3F(1.f, 1.f, 1.f));

    f->glClear(GL_COLOR_BUFFER_BIT);

    f->glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
    f->glEnable(GL_POINT_SPRITE);
    f->glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    f->glEnable(GL_DEPTH_TEST);
    f->glEnable(GL_BLEND);
    f->glBlendFunc(GL_ONE, GL_ONE);
    f->glDepthMask(GL_FALSE);

    // make sprites larger to get smoother thickness texture
    const float thicknessScale = 4.0f;

    f->glUseProgram(mPointThicknessProgram);
    f->glUniform1f( f->glGetUniformLocation(mPointThicknessProgram, "pointRadius"), thicknessScale*radius);
    f->glUniform1f( f->glGetUniformLocation(mPointThicknessProgram, "pointScale"), screenWidth/screenAspect * (1.0f / (tanf(fov*0.5f))));

    f->glEnableClientState(GL_VERTEX_ARRAY);
    f->glBindBuffer(GL_ARRAY_BUFFER, buffers->mPositionVBO);
    f->glVertexPointer(3, GL_FLOAT, sizeof(float)*4, (void*)(offset*sizeof(float)*4));

    f->glDrawArrays(GL_POINTS, 0, n);

    f->glUseProgram(0);
    f->glDisableClientState(GL_VERTEX_ARRAY);
    f->glDisable(GL_POINT_SPRITE);
    f->glDisable(GL_BLEND);
    //

    // depth texture
    glVerify(glBindFramebuffer(GL_FRAMEBUFFER, mDepthFbo));
    glVerify(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE_ARB, mDepthTex, 0));
    glVerify(glDrawBuffer(GL_COLOR_ATTACHMENT0));

    // draw points
    //glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
    f->glDisable(GL_POINT_SPRITE);
    f->glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
    f->glEnable(GL_DEPTH_TEST);
    f->glDepthMask(GL_TRUE);

    f->glViewport(0, 0, int(screenWidth), int(screenWidth/screenAspect));
    f->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float viewHeight = tanf(fov/2.0f);

    f->glUseProgram(mEllipsoidDepthProgram);
    f->glUniform3fv( f->glGetUniformLocation(mEllipsoidDepthProgram, "invViewport"), 1, &Vector3F(1.0f/screenWidth, screenAspect/screenWidth, 1.0f)[0]);
    f->glUniform3fv( f->glGetUniformLocation(mEllipsoidDepthProgram, "invProjection"), 1, &Vector3F(screenAspect*viewHeight, viewHeight, 1.0f)[0]);

    f->glEnableClientState(GL_VERTEX_ARRAY);
    f->glBindBuffer(GL_ARRAY_BUFFER, buffers->mPositionVBO);
    f->glVertexPointer(3, GL_FLOAT, sizeof(float)*4, 0);//(void*)(offset*sizeof(float)*4));

    // ellipsoid eigenvectors
    int s1 = f->glGetAttribLocation(mEllipsoidDepthProgram, "q1");
    f->glEnableVertexAttribArray(s1);
    f->glBindBuffer(GL_ARRAY_BUFFER, buffers->mAnisotropyVBO[0]);
    f->glVertexAttribPointer(s1, 4, GL_FLOAT, GL_FALSE, 0, 0);// (void*)(offset*sizeof(float)*4));

    int s2 = f->glGetAttribLocation(mEllipsoidDepthProgram, "q2");
    f->glEnableVertexAttribArray(s2);
    f->glBindBuffer(GL_ARRAY_BUFFER, buffers->mAnisotropyVBO[1]);
    f->glVertexAttribPointer(s2, 4, GL_FLOAT, GL_FALSE, 0, 0);//(void*)(offset*sizeof(float)*4));

    int s3 = f->glGetAttribLocation(mEllipsoidDepthProgram, "q3");
    f->glEnableVertexAttribArray(s3);
    f->glBindBuffer(GL_ARRAY_BUFFER, buffers->mAnisotropyVBO[2]);
    f->glVertexAttribPointer(s3, 4, GL_FLOAT, GL_FALSE, 0, 0);// (void*)(offset*sizeof(float)*4));

    glVerify(glDrawArrays(GL_POINTS, offset, n));

    f->glUseProgram(0);
    f->glBindBuffer(GL_ARRAY_BUFFER, 0);
    f->glDisableClientState(GL_VERTEX_ARRAY);
    f->glDisableVertexAttribArray(s1);
    f->glDisableVertexAttribArray(s2);
    f->glDisableVertexAttribArray(s3);

    f->glDisable(GL_POINT_SPRITE);

    //---------------------------------------------------------------
    // blur

    f->glDisable(GL_DEPTH_TEST);
    f->glDepthMask(GL_FALSE);

    glVerify(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mDepthSmoothTex, 0));
    f->glUseProgram(mDepthBlurProgram);

    f->glActiveTexture(GL_TEXTURE0);
    f->glEnable(GL_TEXTURE_RECTANGLE_ARB);
    f->glBindTexture(GL_TEXTURE_RECTANGLE_ARB, mDepthTex);

    f->glActiveTexture(GL_TEXTURE1);
    f->glEnable(GL_TEXTURE_2D);
    f->glBindTexture(GL_TEXTURE_2D, mThicknessTex);

    glVerify(glUniform1f( f->glGetUniformLocation(mDepthBlurProgram, "blurRadiusWorld"), radius*0.5f));	// blur half the radius by default
    glVerify(glUniform1f( f->glGetUniformLocation(mDepthBlurProgram, "blurScale"), screenWidth/screenAspect * (1.0f / (tanf(fov*0.5f)))));
    glVerify(glUniform2fv( f->glGetUniformLocation(mDepthBlurProgram, "invTexScale"), 1, &Vector2F(1.0f/screenAspect, 1.0f)[0]));
    glVerify(glUniform1f( f->glGetUniformLocation(mDepthBlurProgram, "blurFalloff"),  blur));
    glVerify(glUniform1i( f->glGetUniformLocation(mDepthBlurProgram, "depthTex"), 0));
    glVerify(glUniform1i( f->glGetUniformLocation(mDepthBlurProgram, "thicknessTex"), 1));
    glVerify(glUniform1i( f->glGetUniformLocation(mDepthBlurProgram, "debug"), false));

    { //Render fullscreen quad
        f->glColor3f(1.0f, 1.0f, 1.0f);
        f->glBegin(GL_QUADS);

        f->glTexCoord2f(0.0f, 0.0f);
        f->glVertex2f(-1.0f, -1.0f);

        f->glTexCoord2f(1.0f, 0.0f);
        f->glVertex2f(1.0f, -1.0f);

        f->glTexCoord2f(1.0f, 1.0f);
        f->glVertex2f(1.0f, 1.0f);

        f->glTexCoord2f(0.0f, 1.0f);
        f->glVertex2f(-1.0f, 1.0f);

        f->glEnd();
    }

    f->glActiveTexture(GL_TEXTURE0);
    f->glDisable(GL_TEXTURE_RECTANGLE_ARB);

    //---------------------------------------------------------------
    // composite with scene

    glVerify(glBindFramebuffer(GL_FRAMEBUFFER, msaaFbo));
    glVerify(glEnable(GL_DEPTH_TEST));
    glVerify(glDepthMask(GL_TRUE));
    glVerify(glDisable(GL_BLEND));
    glVerify(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));

    glVerify(glUseProgram(mCompositeProgram));

    glVerify(glUniform2fv(f->glGetUniformLocation(mCompositeProgram, "invTexScale"), 1, &Vector2F(1.0f/screenWidth, screenAspect/screenWidth)[0]));
    glVerify(glUniform2fv(f->glGetUniformLocation(mCompositeProgram, "clipPosToEye"), 1, &Vector2F(tanf(fov*0.5f)*screenAspect, tanf(fov*0.5f))[0]));
    glVerify(glUniform4fv(f->glGetUniformLocation(mCompositeProgram, "color"), 1, &color[0]));
    glVerify(glUniform1f(f->glGetUniformLocation(mCompositeProgram, "ior"),  ior));
    glVerify(glUniform1f(f->glGetUniformLocation(mCompositeProgram, "spotMin"), 0.5f));
    glVerify(glUniform1f(f->glGetUniformLocation(mCompositeProgram, "spotMax"), 1.f));
    glVerify(glUniform1i(f->glGetUniformLocation(mCompositeProgram, "debug"), false));

    glVerify(glUniform3fv(f->glGetUniformLocation(mCompositeProgram, "lightPos"), 1, &lightPos[0]));
    Vector3F norm = -(lightTarget-lightPos).normalized();
    glVerify(glUniform3fv(f->glGetUniformLocation(mCompositeProgram, "lightDir"), 1, &norm[0]));
    glVerify(glUniformMatrix4fv(f->glGetUniformLocation(mCompositeProgram, "lightTransform"), 1, false, lightTransform.constData()));

    Vector2F taps[] =
    {
        Vector2F(-0.326212f,-0.40581f),Vector2F(-0.840144f,-0.07358f),
        Vector2F(-0.695914f,0.457137f),Vector2F(-0.203345f,0.620716f),
        Vector2F(0.96234f,-0.194983f),Vector2F(0.473434f,-0.480026f),
        Vector2F(0.519456f,0.767022f),Vector2F(0.185461f,-0.893124f),
        Vector2F(0.507431f,0.064425f),Vector2F(0.89642f,0.412458f),
        Vector2F(-0.32194f,-0.932615f),Vector2F(-0.791559f,-0.59771f)
    };

    glVerify(glUniform2fv(f->glGetUniformLocation(mCompositeProgram, "shadowTaps"), 12, &taps[0][0]));

    // smoothed depth tex
    glVerify(glActiveTexture(GL_TEXTURE0));
    glVerify(glEnable(GL_TEXTURE_2D));
    glVerify(glBindTexture(GL_TEXTURE_2D, mDepthSmoothTex));

    // shadow tex
    glVerify(glActiveTexture(GL_TEXTURE1));
    glVerify(glEnable(GL_TEXTURE_2D));
    glVerify(glBindTexture(GL_TEXTURE_2D, shadowMap->texture));

    // thickness tex
    glVerify(glActiveTexture(GL_TEXTURE2));
    glVerify(glEnable(GL_TEXTURE_2D));
    glVerify(glBindTexture(GL_TEXTURE_2D, mThicknessTex));

    // scene tex
    glVerify(glActiveTexture(GL_TEXTURE3));
    glVerify(glEnable(GL_TEXTURE_2D));
    glVerify(glBindTexture(GL_TEXTURE_2D, mSceneTex));

    /*
    // reflection tex
    glVerify(glActiveTexture(GL_TEXTURE5));
    glVerify(glEnable(GL_TEXTURE_2D));
    glVerify(glBindTexture(GL_TEXTURE_2D, reflectMap->texture));
    */

    glVerify(glUniform1i(f->glGetUniformLocation(mCompositeProgram, "tex"), 0));
    glVerify(glUniform1i(f->glGetUniformLocation(mCompositeProgram, "shadowTex"), 1));
    glVerify(glUniform1i(f->glGetUniformLocation(mCompositeProgram, "thicknessTex"), 2));
    glVerify(glUniform1i(f->glGetUniformLocation(mCompositeProgram, "sceneTex"), 3));
    glVerify(glUniform1i(f->glGetUniformLocation(mCompositeProgram, "reflectTex"), 5));

    // -- end shadowing

    // ignores projection matrices
    { //Render fullscreen quad
        f->glColor3f(1.0f, 1.0f, 1.0f);
        f->glBegin(GL_QUADS);

        f->glTexCoord2f(0.0f, 0.0f);
        f->glVertex2f(-1.0f, -1.0f);

        f->glTexCoord2f(1.0f, 0.0f);
        f->glVertex2f(1.0f, -1.0f);

        f->glTexCoord2f(1.0f, 1.0f);
        f->glVertex2f(1.0f, 1.0f);

        f->glTexCoord2f(0.0f, 1.0f);
        f->glVertex2f(-1.0f, 1.0f);

        f->glEnd();
    }

    // reset state
    f->glActiveTexture(GL_TEXTURE5);
    f->glDisable(GL_TEXTURE_2D);
    f->glActiveTexture(GL_TEXTURE3);
    f->glDisable(GL_TEXTURE_2D);
    f->glActiveTexture(GL_TEXTURE2);
    f->glDisable(GL_TEXTURE_2D);
    f->glActiveTexture(GL_TEXTURE1);
    f->glDisable(GL_TEXTURE_2D);
    f->glActiveTexture(GL_TEXTURE0);
    f->glDisable(GL_TEXTURE_2D);

    f->glEnable(GL_DEPTH_TEST);
    f->glDepthMask(GL_TRUE);
    f->glDisable(GL_BLEND);
}

FluidRenderer* FluidRenderer::CreateFluidRenderer(NvFlexLibrary* flexLib, OpenGLFunctions* f, uint32_t width, uint32_t height)
{
    FluidRenderer* renderer = new FluidRenderer(flexLib, f);

    renderer->mSceneWidth = width;
    renderer->mSceneHeight = height;

    // scene depth texture
    glVerify(glGenTextures(1, &renderer->mDepthTex));
    glVerify(glBindTexture(GL_TEXTURE_RECTANGLE_ARB, renderer->mDepthTex));

    glVerify(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    glVerify(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    glVerify(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    glVerify(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    glVerify(glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_LUMINANCE32F_ARB, width, height, 0, GL_LUMINANCE, GL_FLOAT, NULL));

    // smoothed depth texture
    glVerify(glGenTextures(1, &renderer->mDepthSmoothTex));
    glVerify(glBindTexture(GL_TEXTURE_2D, renderer->mDepthSmoothTex));

    glVerify(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    glVerify(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    glVerify(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    glVerify(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    glVerify(glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE32F_ARB, width, height, 0, GL_LUMINANCE, GL_FLOAT, NULL));

    // scene copy
    glVerify(glGenTextures(1, &renderer->mSceneTex));
    glVerify(glBindTexture(GL_TEXTURE_2D, renderer->mSceneTex));

    glVerify(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    glVerify(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    glVerify(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    glVerify(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    glVerify(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));

    glVerify(glGenFramebuffers(1, &renderer->mSceneFbo));
    glVerify(glBindFramebuffer(GL_FRAMEBUFFER, renderer->mSceneFbo));
    glVerify(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer->mSceneTex, 0));

    // frame buffer
    glVerify(glGenFramebuffers(1, &renderer->mDepthFbo));
    glVerify(glBindFramebuffer(GL_FRAMEBUFFER, renderer->mDepthFbo));
    glVerify(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE_ARB, renderer->mDepthTex, 0));

    GLuint zbuffer;
    glVerify(glGenRenderbuffers(1, &zbuffer));
    glVerify(glBindRenderbuffer(GL_RENDERBUFFER, zbuffer));
    glVerify(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height));
    glVerify(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, zbuffer));

    glVerify(glDrawBuffer(GL_COLOR_ATTACHMENT0));
    glVerify(glReadBuffer(GL_COLOR_ATTACHMENT0));

    glVerify(glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // reflect texture
    glVerify(glGenTextures(1, &renderer->mReflectTex));
    glVerify(glBindTexture(GL_TEXTURE_2D, renderer->mReflectTex));

    glVerify(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    glVerify(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    glVerify(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    glVerify(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    glVerify(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));

    // thickness texture
    const int thicknessWidth = width;
    const int thicknessHeight = height;

    glVerify(glGenTextures(1, &renderer->mThicknessTex));
    glVerify(glBindTexture(GL_TEXTURE_2D, renderer->mThicknessTex));

    glVerify(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    glVerify(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    glVerify(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    glVerify(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

#if USE_HDR_DIFFUSE_BLEND
    glVerify(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, thicknessWidth, thicknessHeight, 0, GL_RGBA, GL_FLOAT, NULL));
#else
    glVerify(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, thicknessWidth, thicknessHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
#endif

    // thickness buffer
    glVerify(glGenFramebuffers(1, &renderer->mThicknessFbo));
    glVerify(glBindFramebuffer(GL_FRAMEBUFFER, renderer->mThicknessFbo));
    glVerify(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer->mThicknessTex, 0));

    GLuint thickz;
    glVerify(glGenRenderbuffers(1, &thickz));
    glVerify(glBindRenderbuffer(GL_RENDERBUFFER, thickz));
    glVerify(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, thicknessWidth, thicknessHeight));
    glVerify(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, thickz));

    glVerify(glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // compile shaders
    //renderer->mPointDepthProgram = CompileProgram(vertexPointDepthShader, fragmentPointDepthShader);
    renderer->mPointThicknessProgram = CompileProgram(f, vertexPointDepthShader, fragmentPointThicknessShader);

    //renderer->mEllipsoidThicknessProgram = CompileProgram(vertexEllipsoidDepthShader, fragmentEllipsoidThicknessShader);
    renderer->mEllipsoidDepthProgram = CompileProgram(f, vertexEllipsoidDepthShader, fragmentEllipsoidDepthShader, geometryEllipsoidDepthShader);

    renderer->mCompositeProgram = CompileProgram(f, vertexPassThroughShader, fragmentCompositeShader);
    renderer->mDepthBlurProgram = CompileProgram(f, vertexPassThroughShader, fragmentBlurDepthShader);

    return renderer;
}

void FluidRenderer::DestroyFluidRenderer()
{
    FluidRenderer* renderer = this;

    glVerify(glDeleteFramebuffers(1, &renderer->mSceneFbo));
    glVerify(glDeleteFramebuffers(1, &renderer->mDepthFbo));
    glVerify(glDeleteTextures(1, &renderer->mDepthTex));
    glVerify(glDeleteTextures(1, &renderer->mDepthSmoothTex));
    glVerify(glDeleteTextures(1, &renderer->mSceneTex));

    glVerify(glDeleteFramebuffers(1, &renderer->mThicknessFbo));
    glVerify(glDeleteTextures(1, &renderer->mThicknessTex));
}

FluidRenderBuffers*FluidRenderer::CreateFluidRenderBuffers(int numFluidParticles, bool enableInterop)
{
    FluidRenderBuffersGL* buffers = new FluidRenderBuffersGL(f,numFluidParticles);

    // vbos
    glVerify(glGenBuffers(1, &buffers->mPositionVBO));
    glVerify(glBindBuffer(GL_ARRAY_BUFFER, buffers->mPositionVBO));
    glVerify(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * numFluidParticles, 0, GL_DYNAMIC_DRAW));

    // density
    glVerify(glGenBuffers(1, &buffers->mDensityVBO));
    glVerify(glBindBuffer(GL_ARRAY_BUFFER, buffers->mDensityVBO));
    glVerify(glBufferData(GL_ARRAY_BUFFER, sizeof(int)*numFluidParticles, 0, GL_DYNAMIC_DRAW));

    for (int i = 0; i < 3; ++i)
    {
        glVerify(glGenBuffers(1, &buffers->mAnisotropyVBO[i]));
        glVerify(glBindBuffer(GL_ARRAY_BUFFER, buffers->mAnisotropyVBO[i]));
        glVerify(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * numFluidParticles, 0, GL_DYNAMIC_DRAW));
    }

    glVerify(glGenBuffers(1, &buffers->mIndices));
    glVerify(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers->mIndices));
    glVerify(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*numFluidParticles, 0, GL_DYNAMIC_DRAW));

    if (enableInterop)
    {
        buffers->mPositionBuf = NvFlexRegisterOGLBuffer(flexLib, buffers->mPositionVBO, numFluidParticles, sizeof(Vector4F));
        buffers->mDensitiesBuf = NvFlexRegisterOGLBuffer(flexLib, buffers->mDensityVBO, numFluidParticles, sizeof(float));
        buffers->mIndicesBuf = NvFlexRegisterOGLBuffer(flexLib, buffers->mIndices, numFluidParticles, sizeof(int));

        buffers->mAnisotropyBuf[0] = NvFlexRegisterOGLBuffer(flexLib, buffers->mAnisotropyVBO[0], numFluidParticles, sizeof(Vector4F));
        buffers->mAnisotropyBuf[1] = NvFlexRegisterOGLBuffer(flexLib, buffers->mAnisotropyVBO[1], numFluidParticles, sizeof(Vector4F));
        buffers->mAnisotropyBuf[2] = NvFlexRegisterOGLBuffer(flexLib, buffers->mAnisotropyVBO[2], numFluidParticles, sizeof(Vector4F));
    }

    return reinterpret_cast<FluidRenderBuffers*>(buffers);
}

void FluidRenderer::DestroyFluidRenderBuffers(FluidRenderBuffers* buffers)
{
    delete reinterpret_cast<FluidRenderBuffersGL*>(buffers);
}

void FluidRenderer::UpdateFluidRenderBuffers(FluidRenderBuffers* buffersIn, NvFlexSolver* solver, bool anisotropy, bool density)
{
    FluidRenderBuffersGL* buffers = reinterpret_cast<FluidRenderBuffersGL*>(buffersIn);
    // use VBO buffer wrappers to allow Flex to write directly to the OpenGL buffers
    // Flex will take care of any CUDA interop mapping/unmapping during the get() operations
    if (!anisotropy)
    {
        // regular particles
        NvFlexGetParticles(solver, buffers->mPositionBuf, NULL);
    }
    else
    {
        // fluid buffers
        NvFlexGetSmoothParticles(solver, buffers->mPositionBuf, NULL);
        NvFlexGetAnisotropy(solver, buffers->mAnisotropyBuf[0], buffers->mAnisotropyBuf[1], buffers->mAnisotropyBuf[2], NULL);
    }

    if (density)
    {
        NvFlexGetDensities(solver, buffers->mDensitiesBuf, NULL);
    }
    else
    {
        NvFlexGetPhases(solver, buffers->mDensitiesBuf, NULL);
    }

    NvFlexGetActive(solver, buffers->mIndicesBuf, NULL);
}

void FluidRenderer::UpdateFluidRenderBuffers(FluidRenderBuffers* buffersIn, Vector4F* particles, float* densities, Vector4F* anisotropy1, Vector4F* anisotropy2, Vector4F* anisotropy3, int numParticles, int* indices, int numIndices)
{
    FluidRenderBuffersGL* buffers = reinterpret_cast<FluidRenderBuffersGL*>(buffersIn);
    // regular particles
    glVerify(glBindBuffer(GL_ARRAY_BUFFER, buffers->mPositionVBO));
    glVerify(glBufferSubData(GL_ARRAY_BUFFER, 0, buffers->mNumParticles*sizeof(Vector4F), particles));

    Vector4F *const anisotropies[] =
    {
        anisotropy1,
        anisotropy2,
        anisotropy3,
    };

    for (int i = 0; i < 3; i++)
    {
        Vector4F* anisotropy = anisotropies[i];
        if (anisotropy)
        {
            glVerify(glBindBuffer(GL_ARRAY_BUFFER, buffers->mAnisotropyVBO[i]));
            glVerify(glBufferSubData(GL_ARRAY_BUFFER, 0, buffers->mNumParticles * sizeof(Vector4F), anisotropy));
        }
    }

    // density /phase buffer
    if (densities)
    {
        glVerify(glBindBuffer(GL_ARRAY_BUFFER, buffers->mDensityVBO));
        glVerify(glBufferSubData(GL_ARRAY_BUFFER, 0, buffers->mNumParticles*sizeof(float), densities));
    }

    if (indices)
    {
        glVerify(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers->mIndices));
        glVerify(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, numIndices*sizeof(int), indices));
    }

    // reset
    glVerify(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    glVerify(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

DiffuseRenderBuffers*FluidRenderer::CreateDiffuseRenderBuffers(int numDiffuseParticles, bool& enableInterop)
{
    DiffuseRenderBuffersGL* buffers = new DiffuseRenderBuffersGL(f,numDiffuseParticles);

    if (numDiffuseParticles > 0)
    {
        glVerify(glGenBuffers(1, &buffers->mDiffusePositionVBO));
        glVerify(glBindBuffer(GL_ARRAY_BUFFER, buffers->mDiffusePositionVBO));
        glVerify(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * numDiffuseParticles, 0, GL_DYNAMIC_DRAW));
        glVerify(glBindBuffer(GL_ARRAY_BUFFER, 0));

        glVerify(glGenBuffers(1, &buffers->mDiffuseVelocityVBO));
        glVerify(glBindBuffer(GL_ARRAY_BUFFER, buffers->mDiffuseVelocityVBO));
        glVerify(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * numDiffuseParticles, 0, GL_DYNAMIC_DRAW));
        glVerify(glBindBuffer(GL_ARRAY_BUFFER, 0));

        if (enableInterop)
        {
            buffers->mDiffusePositionsBuf = NvFlexRegisterOGLBuffer(flexLib, buffers->mDiffusePositionVBO, numDiffuseParticles, sizeof(Vector4F));
            buffers->mDiffuseVelocitiesBuf = NvFlexRegisterOGLBuffer(flexLib, buffers->mDiffuseVelocityVBO, numDiffuseParticles, sizeof(Vector4F));
        }
    }

    return reinterpret_cast<DiffuseRenderBuffers*>(buffers);
}

void FluidRenderer::DestroyDiffuseRenderBuffers(DiffuseRenderBuffers* buffersIn)
{
    DiffuseRenderBuffersGL* buffers = reinterpret_cast<DiffuseRenderBuffersGL*>(buffersIn);
    if (buffers->mNumParticles > 0)
    {
        glDeleteBuffers(1, &buffers->mDiffusePositionVBO);
        glDeleteBuffers(1, &buffers->mDiffuseVelocityVBO);

        NvFlexUnregisterOGLBuffer(buffers->mDiffusePositionsBuf);
        NvFlexUnregisterOGLBuffer(buffers->mDiffuseVelocitiesBuf);
    }
}

void FluidRenderer::UpdateDiffuseRenderBuffers(DiffuseRenderBuffers* buffersIn, NvFlexSolver* solver)
{
    DiffuseRenderBuffersGL* buffers = reinterpret_cast<DiffuseRenderBuffersGL*>(buffersIn);
    // diffuse particles
    if (buffers->mNumParticles)
    {
        NvFlexGetDiffuseParticles(solver, buffers->mDiffusePositionsBuf, buffers->mDiffuseVelocitiesBuf, NULL);
    }
}

void FluidRenderer::UpdateDiffuseRenderBuffers(DiffuseRenderBuffers* buffersIn, Vector4F* diffusePositions, Vector4F* diffuseVelocities, int numDiffuseParticles)
{
    DiffuseRenderBuffersGL* buffers = reinterpret_cast<DiffuseRenderBuffersGL*>(buffersIn);
    // diffuse particles
    if (buffers->mNumParticles)
    {
        glVerify(glBindBuffer(GL_ARRAY_BUFFER, buffers->mDiffusePositionVBO));
        glVerify(glBufferSubData(GL_ARRAY_BUFFER, 0, buffers->mNumParticles*sizeof(Vector4F), diffusePositions));

        glVerify(glBindBuffer(GL_ARRAY_BUFFER, buffers->mDiffuseVelocityVBO));
        glVerify(glBufferSubData(GL_ARRAY_BUFFER, 0, buffers->mNumParticles*sizeof(Vector4F), diffuseVelocities));
    }
}
