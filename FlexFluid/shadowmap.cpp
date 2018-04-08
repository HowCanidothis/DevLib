#include "shadowmap.h"

#include "fluid.h"
#define glVerify(x) f->x

// vertex shader
const char *vertexShader = "#version 130\n" QT_STRINGIFY(

uniform mat4 lightTransform;
uniform vec3 lightDir;
uniform float bias;
uniform vec4 clipPlane;
uniform float expand;

uniform mat4 objectTransform;

void main()
{
    vec3 n = normalize((objectTransform*vec4(gl_Normal, 0.0)).xyz);
    vec3 p = (objectTransform*vec4(gl_Vertex.xyz, 1.0)).xyz;

    // calculate window-space point size
    gl_Position = gl_ModelViewProjectionMatrix * vec4(p + expand*n, 1.0);

    gl_TexCoord[0].xyz = n;
    gl_TexCoord[1] = lightTransform*vec4(p + n*bias, 1.0);
    gl_TexCoord[2] = gl_ModelViewMatrix*vec4(lightDir, 0.0);
    gl_TexCoord[3].xyz = p;
    gl_TexCoord[4] = gl_Color;
    gl_TexCoord[5] = gl_MultiTexCoord0;
    gl_TexCoord[6] = gl_SecondaryColor;
    gl_TexCoord[7] = gl_ModelViewMatrix*vec4(gl_Vertex.xyz, 1.0);

    gl_ClipDistance[0] = dot(clipPlane,vec4(gl_Vertex.xyz, 1.0));
}
);

const char *passThroughShader = QT_STRINGIFY(

void main()
{
    gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);

}
);

ShadowMap* ShadowMap::ShadowCreate(OpenGLFunctions* f)
{
    GLuint texture;
    GLuint framebuffer;

    glVerify(glGenFramebuffers(1, &framebuffer));
    glVerify(glGenTextures(1, &texture));
    glVerify(glBindTexture(GL_TEXTURE_2D, texture));

    glVerify(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    glVerify(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    // This is to allow usage of shadow2DProj function in the shader
    glVerify(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE));
    glVerify(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL));
    glVerify(glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY));

    glVerify(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, kShadowResolution(), kShadowResolution(), 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL));

    glVerify(glBindFramebuffer(GL_FRAMEBUFFER, framebuffer));

    glVerify(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0));

    ShadowMap* map = new ShadowMap();
    map->texture = texture;
    map->framebuffer = framebuffer;

    return map;

}

void ShadowMap::ShadowBegin(OpenGLFunctions* f)
{
    f->glEnable(GL_POLYGON_OFFSET_FILL);
    f->glPolygonOffset(8.f, 8.f);

    glVerify(glBindFramebuffer(GL_FRAMEBUFFER, framebuffer));

    f->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    f->glClear(GL_DEPTH_BUFFER_BIT);
    f->glViewport(0, 0, kShadowResolution(), kShadowResolution());

    // draw back faces (for teapot)
    f->glDisable(GL_CULL_FACE);

    // bind shadow shader
    if (!shadowProgram) {
        shadowProgram = FluidRenderer::CompileProgram(f, vertexShader, passThroughShader);
    }

    static Matrix4 indentity;

    glVerify(glUseProgram(shadowProgram));
    glVerify(glUniformMatrix4fv(f->glGetUniformLocation(shadowProgram, "objectTransform"), 1, false, indentity.constData()));
}

void ShadowMap::ShadowEnd(OpenGLFunctions* f, quint32 msaaFbo)
{
    glVerify(glDisable(GL_POLYGON_OFFSET_FILL));

    glVerify(glBindFramebuffer(GL_FRAMEBUFFER, msaaFbo));

    glVerify(glEnable(GL_CULL_FACE));
    glVerify(glUseProgram(0));
}

void ShadowMap::ShadowApply(OpenGLFunctions* f, GLint sprogram, Vector3F& lightPos, const Vector3F& lightTarget, const Matrix4& lightTransform)
{
    GLint uLightTransform = f->glGetUniformLocation(sprogram, "lightTransform");
    f->glUniformMatrix4fv(uLightTransform, 1, false, lightTransform.constData());

    GLint uLightPos = f->glGetUniformLocation(sprogram, "lightPos");
    f->glUniform3fv(uLightPos, 1, &lightPos[0]);

    GLint uLightDir = f->glGetUniformLocation(sprogram, "lightDir");
    Vector3F n = (lightTarget-lightPos).normalized();
    f->glUniform3fv(uLightDir, 1, &n[0]);

    GLint uBias = f->glGetUniformLocation(sprogram, "bias");
    f->glUniform1f(uBias, 0.05f);

    Vector2F taps[] =
    {
        Vector2F(-0.326212f,-0.40581f),Vector2F(-0.840144f,-0.07358f),
        Vector2F(-0.695914f,0.457137f),Vector2F(-0.203345f,0.620716f),
        Vector2F(0.96234f,-0.194983f),Vector2F(0.473434f,-0.480026f),
        Vector2F(0.519456f,0.767022f),Vector2F(0.185461f,-0.893124f),
        Vector2F(0.507431f,0.064425f),Vector2F(0.89642f,0.412458f),
        Vector2F(-0.32194f,-0.932615f),Vector2F(-0.791559f,-0.59771f)
    };

    GLint uShadowTaps = f->glGetUniformLocation(sprogram, "shadowTaps");
    f->glUniform2fv(uShadowTaps, 12, &taps[0][0]);

    f->glEnable(GL_TEXTURE_2D);
    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, texture);

}
