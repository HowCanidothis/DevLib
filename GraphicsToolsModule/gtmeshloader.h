#ifndef GTMESHLOADER_H
#define GTMESHLOADER_H

#include <SharedModule/internal.hpp>

#include "gtmeshbase.h"

class GtMeshLoader
{
public:
    struct Mesh
    {
        // Buffer reference
        GtMeshBufferPtr Buffer;
        QHash<Name, GtMeshBufferPtr> MaterialIndicesBuffer;
    };

    struct Material
    {
        enum EllumType {
            ConstantColor = 0,       // illum 0
            DiffuseShadingOnly = 1,  // illum 1
            Full = 2                 // illum 2
        };

        // Identification
        Name MaterialName;        // Matches 'newmtl' name token

        // Scalar properties
        float Ns = 64.0f;            // MODIFIED: Changed from 0.0f to 64.0f. 0.0f causes infinite specular spread or divide-by-zero.
        float Ni = 1.0f;             // Index of Refraction (Air default is 1.0)
        float D = 1.0f;              // Dissolve / Opacity (1.0 = fully opaque)

        // ADDED: PBR Extension Scalar Constants
        float Pr = 0.5f;             // Roughness constant fallback (Pr 0.500000)
        float Pm = 0.0f;             // Metalness constant fallback (Pm 0.000000)
        float Ps = 0.0f;             // Sheen constant fallback     (Ps 0.000000)
        float Pc = 0.0f;             // Clearcoat factor fallback  (Pc 0.000000)
        float Pcr = 0.1f;            // MODIFIED: Changed from 0.0f to 0.1f. Clearcoat roughness must never be 0.0f to prevent microfacet division crashing.

        // Color properties
        Vector3F Ka{1.0f, 1.0f, 1.0f}; // MODIFIED: Changed from 0,0,0 to 1,1,1. Ambient scaling must pass full environment light value by default.
        Vector3F Kd{1.0f, 1.0f, 1.0f}; // Diffuse color RGB (Default white)
        Vector3F Ks{1.0f, 1.0f, 1.0f}; // MODIFIED: Changed from 0,0,0 to 1,1,1. Specular colors must be white by default so PBR equations control reflections.
        Vector3F Ke{0.0f, 0.0f, 0.0f}; // Emissive color RGB

        // Texture filenames
        Name MapKaFileName;       // map_Ka
        Name MapKdFileName;       // map_Kd
        Name MapKsFileName;       // map_Ks
        Name MapKeFileName;       // map_Ke
        Name MapDFileName;        // map_d (alpha map)
        Name MapBumpFileName;     // map_Bump or bump
        Name MapPmFileName;
        Name MapPrFileName;
        Name MapKnFileName;

        EllumType Ellum = ConstantColor;      // Default to full shading
    };


    class Parser
    {
    public:
        struct ObjFaceVertex {
            qint32 vIdx  = 0;
            qint32 vtIdx = 0;
            qint32 vnIdx = 0;

            qint64 Packed() const {
                qint64 v = vIdx, uv = vtIdx, n = vnIdx;
                return ((v & 0x1FFFFF) << 42) | ((uv & 0x1FFFFF) << 21) | (n & 0x1FFFFF);
            }
        };

        enum Format {
            FmtDefault,
            FmtNormals = 0x1,
            FmtTexCoords = 0x2,

            FmtVerticesAndNormals = FmtNormals,
            FmtVerticesAndTextures = FmtTexCoords,
            FmtFull = FmtNormals | FmtTexCoords
        };

        using FTokenHandler = std::function<void(QTextStream&)>;

        Parser();

        Parser& AddNormals();
        Parser& AddTextures(bool invertedY = true);

        Mesh Parse(const QString& fileName);

    private:
        qint32 m_format = FmtDefault;

        // Core internal storage vectors
        QVector<Point3F> m_vertices;
        QVector<Point3F> m_texCoords;
        QVector<Point3F> m_normalCoords;        

        QHash<Name, QVector<ObjFaceVertex>> m_materialFaces;
        Name m_currentMaterialName;

        // The persistent Factory mapping
        QHash<QString, FTokenHandler> m_factory;
    };

    static QHash<Name, GtMeshLoader::Material> LoadMaterials(const QString& fileName);
    static GtMeshBufferPtr LoadObj(const QString& fileName);
};

#endif // GTMESHLOADER_H
