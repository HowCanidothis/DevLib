#ifndef FLUID_H
#define FLUID_H

#include "GraphicsTools/gt_decl.h"

struct FluidRenderBuffers;
struct DiffuseRenderBuffers;

struct NvFlexSolver;
struct NvFlexLibrary;

struct ShadowMap;
struct Mesh;

struct FluidRenderer : protected OpenGLFunctions
{
    FluidRenderer(NvFlexLibrary* flexLib, OpenGLFunctions* f) : flexLib(flexLib), f(f)
    {}

    NvFlexLibrary* flexLib;
    OpenGLFunctions* f;

    GLuint mDepthFbo;
    GLuint mDepthTex;
    GLuint mDepthSmoothTex;
    GLuint mSceneFbo;
    GLuint mSceneTex;
    GLuint mReflectTex;

    GLuint mThicknessFbo;
    GLuint mThicknessTex;

    GLuint mPointThicknessProgram;
    //GLuint mPointDepthProgram;

    GLuint mEllipsoidThicknessProgram;
    GLuint mEllipsoidDepthProgram;

    GLuint mCompositeProgram=0;
    GLuint mDepthBlurProgram=0;

    int mSceneWidth;
    int mSceneHeight;


    void RenderDiffuse(DiffuseRenderBuffers* buffersIn,
                            int n,
                            float radius,
                            float screenWidth,
                            float screenAspect,
                            float fov,
                            Vector4F& color,
                            Vector3F& lightPos,
                            const Vector3F& lightTarget,
                            const Matrix4& lightTransform,
                            ShadowMap* shadowMap,
                            float motionBlur,
                            float inscatter,
                            float outscatter,
                            bool shadow,
                            bool front);
    void RenderFluid(FluidRenderBuffers* buffersIn,
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
                     Mesh* mesh);

    static GLuint CompileProgram(OpenGLFunctions* f, const char *vsource, const char *fsource, const char* gsource=0);

    static FluidRenderer* CreateFluidRenderer(NvFlexLibrary* flexLib, OpenGLFunctions* f, uint32_t width, uint32_t height);

    void DestroyFluidRenderer();

    FluidRenderBuffers* CreateFluidRenderBuffers(int numFluidParticles, bool enableInterop);

    void DestroyFluidRenderBuffers(FluidRenderBuffers* buffers);

    void UpdateFluidRenderBuffers(FluidRenderBuffers* buffersIn, NvFlexSolver* solver, bool anisotropy, bool density);

    void UpdateFluidRenderBuffers(FluidRenderBuffers* buffersIn, Vector4F* particles, float* densities, Vector4F* anisotropy1, Vector4F* anisotropy2, Vector4F* anisotropy3, int numParticles, int* indices, int numIndices);

    DiffuseRenderBuffers* CreateDiffuseRenderBuffers(int numDiffuseParticles, bool& enableInterop);

    void DestroyDiffuseRenderBuffers(DiffuseRenderBuffers* buffersIn);

    void UpdateDiffuseRenderBuffers(DiffuseRenderBuffers* buffersIn, NvFlexSolver* solver);

    void UpdateDiffuseRenderBuffers(DiffuseRenderBuffers* buffersIn, Vector4F* diffusePositions, Vector4F* diffuseVelocities, int numDiffuseParticles);

private:
    void DrawMesh(const Mesh* m, Color3F& color);
};

#endif // FLUID_H
