#include "convex.h"

#include "assert.h"

ConvexMeshBuilder::ConvexMeshBuilder(const Vector4F* planes)
    : mPlanes(planes)
{}

Vector3F ConvexMeshBuilder::intersect(const Vector4F& p0, const Vector4F& p1, const Vector4F& p2)
{
    const Vector3F& d0 = reinterpret_cast<const Vector3F&>(p0);
    const Vector3F& d1 = reinterpret_cast<const Vector3F&>(p1);
    const Vector3F& d2 = reinterpret_cast<const Vector3F&>(p2);

    Vector3F r =  (p0.w() * Cross(d1, d2)
                   + p1.w() * Cross(d2, d0)
                   + p2.w() * Cross(d0, d1))
            / Dot(d0, Cross(d2,d1));

    return Vector3F(r.x(), r.y(), r.z());
}

void ConvexMeshBuilder::operator()(uint32_t numPlanes, float scale)
{

    if(numPlanes < 4)
        return; // todo: handle degenerate cases

    HalfedgeMesh mesh;

    // gather points (planes, that is)
    mesh.mPoints.reserve(numPlanes);
    for(uint32_t i=0; i < numPlanes; ++i)
        mesh.mPoints.push_back(Vector4F(mPlanes[i].x(), mPlanes[i].y(), mPlanes[i].z(), mPlanes[i].w()));

    // initialize to tetrahedron
    mesh.addTriangle(0, 1, 2);
    mesh.addTriangle(0, 3, 1);
    mesh.addTriangle(1, 3, 2);
    mesh.addTriangle(2, 3, 0);

    // flip if inside-out
    if(mesh.visible(3, 0))
        std::swap(mesh.mPoints[0], mesh.mPoints[1]);

    // iterate through remaining points
    for(uint16_t i=4; i<mesh.mPoints.size(); ++i)
    {
        // remove any visible triangle
        uint16_t v0 = sInvalid;
        for(uint16_t j=0; j<mesh.mFaces.size(); ++j)
        {
            if(mesh.visible(i, j))
                v0 = std::min(v0, mesh.removeTriangle(j));
        }

        if(v0 == sInvalid)
            continue; // no triangle removed

        if(!mesh.mNumTriangles)
            return; // empty mesh

        // find non-deleted boundary vertex
        for(uint16_t h=0; mesh.mVertices[v0] == sInvalid; h+=2)
        {
            if ((mesh.mHalfedges[h  ].mFace == sInvalid) ^
                (mesh.mHalfedges[h+1].mFace == sInvalid))
            {
                v0 = mesh.mHalfedges[h].mVertex;
            }
        }

        // tesselate hole
        uint16_t start = v0;
        do {
            uint16_t h = mesh.mVertices[v0];
            uint16_t v1 = mesh.mHalfedges[h].mVertex;
            mesh.addTriangle(v0, v1, i);
            v0 = v1;
        } while(v0 != start && mesh.mNumTriangles < 200);

        if (mesh.mNumTriangles == 200)
        {
            return;
        }
    }

    // convert triangles to vertices (intersection of 3 planes)
    std::vector<uint32_t> face2Vertex(mesh.mFaces.size());
    for(uint32_t i=0; i<mesh.mFaces.size(); ++i)
    {
        face2Vertex[i] = uint32_t(mVertices.size());

        uint16_t h = mesh.mFaces[i];
        if(h == sInvalid)
            continue;

        uint16_t v0 = mesh.mHalfedges[h].mVertex;
        h = mesh.mHalfedges[h].mNext;
        uint16_t v1 = mesh.mHalfedges[h].mVertex;
        h = mesh.mHalfedges[h].mNext;
        uint16_t v2 = mesh.mHalfedges[h].mVertex;

        mVertices.push_back(intersect(mesh.mPoints[v0], mesh.mPoints[v1], mesh.mPoints[v2]));
    }

    // convert vertices to polygons (face one-ring)
    for(uint32_t i=0; i<mesh.mVertices.size(); ++i)
    {
        uint16_t h = mesh.mVertices[i];
        if(h == sInvalid || mesh.mHalfedges[h].mFace == sInvalid)
            continue;

        uint16_t v0 = face2Vertex[mesh.mHalfedges[h].mFace];
        h = mesh.mHalfedges[h].mPrev^1;
        if(h == sInvalid || mesh.mHalfedges[h].mFace == sInvalid)
            continue;

        uint16_t v1 = face2Vertex[mesh.mHalfedges[h].mFace];

        while(mIndices.size() < 1000)
        {
            h = mesh.mHalfedges[h].mPrev^1;
            if(h == sInvalid || mesh.mHalfedges[h].mFace == sInvalid)
                continue;

            uint16_t v2 = face2Vertex[mesh.mHalfedges[h].mFace];

            if(v0 == v2)
                break;

            mIndices.push_back(v0);
            mIndices.push_back(v2);
            mIndices.push_back(v1);

            v1 = v2;
        }

    }
}

HalfedgeMesh::HalfedgeMesh() : mNumTriangles(0) {}

uint16_t HalfedgeMesh::findHalfedge(uint16_t v0, uint16_t v1)
{
    uint16_t h = mVertices[v0], start = h;
    while(h != sInvalid && mHalfedges[h].mVertex != v1)
    {
        h = mHalfedges[h ^ 1].mNext;
        if(h == start)
            return sInvalid;
    }
    return h;
}



void HalfedgeMesh::connect(uint16_t h0, uint16_t h1)
{
    mHalfedges[h0].mNext = h1;
    mHalfedges[h1].mPrev = h0;
}



void HalfedgeMesh::addTriangle(uint16_t v0, uint16_t v1, uint16_t v2)
{
    // add new vertices
    uint16_t n = std::max(v0, std::max(v1, v2))+1;
    if(mVertices.size() < n)
        mVertices.resize(n, sInvalid);

    // collect halfedges, prev and next of triangle
    uint16_t verts[] = { v0, v1, v2 };
    uint16_t handles[3], prev[3], next[3];
    for(uint16_t i=0; i<3; ++i)
    {
        uint16_t j = (i+1)%3;
        uint16_t h = findHalfedge(verts[i], verts[j]);
        if(h == sInvalid)
        {
            // add new edge
            h = uint16_t(mHalfedges.size());
            mHalfedges.push_back(Halfedge(verts[j]));
            mHalfedges.push_back(Halfedge(verts[i]));
        }
        handles[i] = h;
        prev[i] = mHalfedges[h].mPrev;
        next[i] = mHalfedges[h].mNext;
    }

    // patch connectivity
    for(uint16_t i=0; i<3; ++i)
    {
        uint16_t j = (i+1)%3;

        mHalfedges[handles[i]].mFace = uint16_t(mFaces.size());

        // connect prev and next
        connect(handles[i], handles[j]);

        if(next[j] == sInvalid) // new next edge, connect opposite
            connect(handles[j]^1, next[i]!=sInvalid ? next[i] : handles[i]^1);

        if(prev[i] == sInvalid) // new prev edge, connect opposite
            connect(prev[j]!=sInvalid ? prev[j] : handles[j]^1, handles[i]^1);

        // prev is boundary, update middle vertex
        if(mHalfedges[handles[i]^1].mFace == sInvalid)
            mVertices[verts[j]] = handles[i]^1;
    }

    assert(mNumTriangles < 0xffff);
    mFaces.push_back(handles[2]);
    ++mNumTriangles;
}



uint16_t HalfedgeMesh::removeTriangle(uint16_t f)
{
    uint16_t result = sInvalid;

    for(uint16_t i=0, h = mFaces[f]; i<3; ++i)
    {
        uint16_t v0 = mHalfedges[h^1].mVertex;
        uint16_t v1 = mHalfedges[h].mVertex;

        mHalfedges[h].mFace = sInvalid;

        if(mHalfedges[h^1].mFace == sInvalid) // was boundary edge, remove
        {
            uint16_t v0Prev = mHalfedges[h  ].mPrev;
            uint16_t v0Next = mHalfedges[h^1].mNext;
            uint16_t v1Prev = mHalfedges[h^1].mPrev;
            uint16_t v1Next = mHalfedges[h  ].mNext;

            // update halfedge connectivity
            connect(v0Prev, v0Next);
            connect(v1Prev, v1Next);

            // update vertex boundary or delete
            mVertices[v0] = (v0Prev^1) == v0Next ? sInvalid : v0Next;
            mVertices[v1] = (v1Prev^1) == v1Next ? sInvalid : v1Next;
        }
        else
        {
            mVertices[v0] = h; // update vertex boundary
            result = v1;
        }

        h = mHalfedges[h].mNext;
    }

    mFaces[f] = sInvalid;
    --mNumTriangles;

    return result;
}



bool HalfedgeMesh::visible(uint16_t v, uint16_t f)
{
    uint16_t h = mFaces[f];
    if(h == sInvalid)
        return false;

    uint16_t v0 = mHalfedges[h].mVertex;
    h = mHalfedges[h].mNext;
    uint16_t v1 = mHalfedges[h].mVertex;
    h = mHalfedges[h].mNext;
    uint16_t v2 = mHalfedges[h].mVertex;
    h = mHalfedges[h].mNext;

    return det(mPoints[v], mPoints[v0], mPoints[v1], mPoints[v2]) < -1e-3f;
}

float HalfedgeMesh::det(const Vector4F& v0, const Vector4F& v1, const Vector4F& v2, const Vector4F& v3)
{
    const Vector3F& d0 = reinterpret_cast<const Vector3F&>(v0);
    const Vector3F& d1 = reinterpret_cast<const Vector3F&>(v1);
    const Vector3F& d2 = reinterpret_cast<const Vector3F&>(v2);
    const Vector3F& d3 = reinterpret_cast<const Vector3F&>(v3);

    return v0.w() * Dot(Cross(d1,d2), d3)
            - v1.w() * Dot(Cross(d0, d2), d3)
            + v2.w() * Dot(Cross(d0, d1), d3)
            - v3.w() * Dot(Cross(d0, d1), d2);
}
