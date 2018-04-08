#ifndef CONVEX_H
#define CONVEX_H

#include <vector>
#include "GraphicsTools/gt_decl.h"

struct ConvexMeshBuilder
{
    ConvexMeshBuilder(const Vector4F* planes);

    Vector3F intersect(const Vector4F& p0, const Vector4F& p1, const Vector4F& p2);
    void operator()(uint32_t mask, float scale=1.0f);

    const Vector4F* mPlanes;
    std::vector<Vector3F> mVertices;
    std::vector<uint16_t> mIndices;
private:
    static Vector3F Cross(const Vector3F& v1, const Vector3F& v2) {
        return Vector3F::crossProduct(v1,v2);
    }
    static float Dot(const Vector3F& v1, const Vector3F& v2) {
        return Vector3F::dotProduct(v1,v2);
    }
};

const uint16_t sInvalid = uint16_t(-1);

// restriction: only supports a single patch per vertex.
struct HalfedgeMesh
{
    struct Halfedge
    {
        Halfedge(uint16_t vertex = sInvalid, uint16_t face = sInvalid,
            uint16_t next = sInvalid, uint16_t prev = sInvalid)
            : mVertex(vertex), mFace(face), mNext(next), mPrev(prev)
        {}

        uint16_t mVertex; // to
        uint16_t mFace; // left
        uint16_t mNext; // ccw
        uint16_t mPrev; // cw
    };

    HalfedgeMesh();

    uint16_t findHalfedge(uint16_t v0, uint16_t v1);

    void connect(uint16_t h0, uint16_t h1);

    void addTriangle(uint16_t v0, uint16_t v1, uint16_t v2);

    uint16_t removeTriangle(uint16_t f);

    // true if vertex v is in front of face f
    bool visible(uint16_t v, uint16_t f);

    float det(const Vector4F& v0, const Vector4F& v1, const Vector4F& v2, const Vector4F& v3);

    std::vector<Halfedge> mHalfedges;
    std::vector<uint16_t> mVertices; // vertex -> (boundary) halfedge
    std::vector<uint16_t> mFaces; // face -> halfedge
    std::vector<Vector4F> mPoints;
    uint16_t mNumTriangles;

private:
    static Vector3F Cross(const Vector3F& v1, const Vector3F& v2) {
        return Vector3F::crossProduct(v1,v2);
    }
    static float Dot(const Vector3F& v1, const Vector3F& v2) {
        return Vector3F::dotProduct(v1,v2);
    }
};

#endif // CONVEX_H
