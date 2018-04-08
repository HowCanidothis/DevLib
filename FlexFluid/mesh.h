#ifndef MESH_H
#define MESH_H

#include "GraphicsTools/gt_decl.h"

struct Mesh
{
    typedef Vector3F position_t;
    typedef Vector3F normal_t;
    typedef Vector2F texcoord_t;
    typedef Color3F color_t;
    typedef uint32_t index_t;

    void AddMesh(const Mesh& m);

    size_t GetPositionSize() const { return m_positions.size() * sizeof(position_t); }
    size_t GetNormalSize() const { return m_normals.size() * sizeof(normal_t); }
    size_t GetTexCoordSize() const { return m_texcoords[0].size() * sizeof(texcoord_t); }
    size_t GetColorSize() const { return m_colours.size() * sizeof(color_t); }
    size_t GetIndicesSize() const { return m_indices.size() * sizeof(index_t); }
    uint32_t GetNumVertices() const { return uint32_t(m_positions.size()); }
    uint32_t GetNumFaces() const { return uint32_t(m_indices.size()) / 3; }

    void DuplicateVertex(uint32_t i);

    void CalculateNormals();
    void Rotate(float angle, const Vector3F& axis);
    void Transform(const Matrix4& m);
    void Normalize(float s=1.0f);	// scale so bounds in any dimension equals s and lower bound = (0,0,0)

    void GetBounds(Vector3F& minExtents, Vector3F& maxExtents) const;

    std::vector<position_t> m_positions;
    std::vector<normal_t> m_normals;
    std::vector<texcoord_t> m_texcoords[2];
    std::vector<color_t> m_colours;

    std::vector<index_t> m_indices;

    // create mesh from file
    static Mesh* ImportMeshFromObj(const char* path);
    static Mesh* ImportMeshFromPly(const char* path);
    static Mesh* ImportMeshFromBin(const char* path);

    // just switches on filename
    static Mesh* ImportMesh(const char* path);

    // save a mesh in a flat binary format
    void ExportMeshToBin(const char* path) const;
    void ExportMeshToObj(const char* path) const;

    // create procedural primitives
    static Mesh* CreateTriMesh(float size, float y=0.0f);
    static Mesh* CreateCubeMesh();
    static Mesh* CreateQuadMesh(float size, float y=0.0f);
    static Mesh* CreateDiscMesh(float radius, uint32_t segments);
    static Mesh* CreateTetrahedron(float ground=0.0f, float height=1.0f); //fixed but not used
    static Mesh* CreateSphere(int slices, int segments, float radius = 1.0f);
    static Mesh* CreateCapsule(int slices, int segments, float radius = 1.0f, float halfHeight = 1.0f);
};

#endif // MESH_H
