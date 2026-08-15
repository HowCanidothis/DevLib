#include "gtmeshloader.h"

GtMeshLoader::Parser::Parser()
{
    // Basic geometric positions are always initialized
    m_factory.insert("v", [this](QTextStream& s) {
        Point3F point;
        s >> point.X() >> point.Y() >> point.Z();
        m_vertices.append(point);
    });
    // 2. Track material switches in the OBJ stream to separate face allocations
    m_factory.insert("usemtl", [this](QTextStream& stream) {
        QString rawMaterialName;
        stream >> rawMaterialName;
        rawMaterialName = rawMaterialName.trimmed();

        if (!rawMaterialName.isEmpty()) {
            // Convert the parsed string seamlessly into your strong 'Name' token key type
            m_currentMaterialName = Name(rawMaterialName);
        }
    });

    // 3. Face triangulation and quad-splitting loop
    m_factory.insert("f", [this](QTextStream& stream) {
        thread_local static QRegExp regExp(R"((\d+)\/?(\d+)?\/?(\d+)?)");

        auto line = stream.readLine();
        qint32 pos = 0;

        // Safety Fallback: Use a default material if the file starts with "f" before a "usemtl"
        if (m_currentMaterialName.IsNull()) {
            m_currentMaterialName = Name();
        }

        // Temporary stack container to assemble the current face parameters
        QVector<ObjFaceVertex> localFace;

        while ((pos = regExp.indexIn(line, pos)) != -1) {
            ObjFaceVertex fv;

            // Remap 1-based OBJ string indices directly to 0-based integer positions
            fv.vIdx  = regExp.cap(1).toInt() - 1;
            fv.vtIdx = m_format & FmtTexCoords ? regExp.cap(2).toInt() - 1 : 0;

            QString normCap = regExp.cap(3);
            fv.vnIdx = m_format & FmtNormals ? normCap.toInt() - 1 : 0;

            localFace.append(fv);

            // 🏎️ THE CRITICAL FIX: Advance 'pos' past the end of the ACTUAL match found
            pos += regExp.matchedLength();
        }

        qint32 count = localFace.size();
        if (count >= 3) {
            // Fetch a direct reference to the active material's structural lane
            auto& activeTrack = m_materialFaces[m_currentMaterialName];

            // Triangle 1: Vertex 0 -> Vertex 1 -> Vertex 2
            activeTrack.append(localFace.at(0));
            activeTrack.append(localFace.at(1));
            activeTrack.append(localFace.at(2));

            // Quad Split Triangulation: Vertex 0 -> Vertex 2 -> Vertex 3
            if (count == 4) {
                activeTrack.append(localFace.at(0));
                activeTrack.append(localFace.at(2));
                activeTrack.append(localFace.at(3));
            }
        }
    });
}

GtMeshLoader::Parser& GtMeshLoader::Parser::AddTextures(bool invertedY)
{
    m_format |= FmtTexCoords;
    if(invertedY) {
        m_factory.insert("vt", [this](QTextStream& s) {
            Point3F point;
            s >> point.X() >> point.Y() >> point.Z();
            point.Y() = 1.0 - point.y();
            m_texCoords.append(point);
        });
    } else {
        m_factory.insert("vt", [this](QTextStream& s) {
            Point3F point;
            s >> point.X() >> point.Y() >> point.Z();
            m_texCoords.append(point);
        });
    }

    return *this;
}

GtMeshLoader::Parser& GtMeshLoader::Parser::AddNormals()
{
    m_format |= FmtNormals;
    m_factory.insert("vn", [this](QTextStream& s) {
        Point3F point;
        s >> point.X() >> point.Y() >> point.Z();
        m_normalCoords.append(point);
    });
    return *this;
}

inline quint64 PackTripletUnique(quint64 v, quint64 uv, quint64 n) {
    return ((v & 0x1FFFFF) << 42) | ((uv & 0x1FFFFF) << 21) | (n & 0x1FFFFF);
}

namespace {

// Fast, collisionless bit-packing for triplets (up to 2,097,152 index elements per pool)
inline quint64 PackTripletUnique(quint64 v, quint64 uv, quint64 n) {
    return ((v & 0x1FFFFF) << 42) | ((uv & 0x1FFFFF) << 21) | (n & 0x1FFFFF);
}

// 🌟 Pure C++17 Branchless Direct-to-GPU multi-material unroller for Option B
template <bool HasTex, bool HasNorm, typename VertexArrayType, typename AssignFunc>
void UnifyStructuredMaterialTracks(
    VertexArrayType& outVertexArray,
    const QHash<Name, QVector<GtMeshLoader::Parser::ObjFaceVertex>>& materialFaceTracks,
    QHash<Name, QVector<qint32>>& outUnifiedIndicesLanes, // Local raw array tracks
    AssignFunc assignUniqueVertexFunc)
{
    // Global map tracking unique vertex attribute combinations across all material zones
    QHash<quint64, quint32> uniqueVertexMap;
    quint32 uniqueVertexCounter = 0;

    // Calculate maximum absolute upper bound for initial vector reservation across all materials
    qint32 totalIndicesCount = 0;
    for (auto it = materialFaceTracks.begin(); it != materialFaceTracks.end(); ++it) {
        totalIndicesCount += it.value().size();
    }
    outVertexArray.resize(totalIndicesCount);

    // Loop through each parsed material track sequentially
    for (auto it = materialFaceTracks.begin(); it != materialFaceTracks.end(); ++it) {
        const Name& matName = it.key();
        const auto& rawFaceVerts = it.value();

        if (rawFaceVerts.isEmpty()) {
            continue;
        }

        // Pre-allocated local index buffer tracking lane (0% append check branching overhead)
        auto& targetIndicesLane = outUnifiedIndicesLanes[matName];
        targetIndicesLane.resize(rawFaceVerts.size());

        for (qint32 i = 0; i < rawFaceVerts.size(); ++i) {
            const auto& vtx = rawFaceVerts.at(i);

            // Evaluated at build-time via C++17 compile-time flag optimization
            qint32 vtIdx = HasTex  ? vtx.vtIdx : 0;
            qint32 vnIdx = HasNorm ? vtx.vnIdx : 0;

            quint64 key = PackTripletUnique(vtx.vIdx, vtIdx, vnIdx);

            if (uniqueVertexMap.contains(key)) {
                targetIndicesLane[i] = uniqueVertexMap.value(key);
            } else {
                auto& uniqueVertex = outVertexArray[uniqueVertexCounter];

                assignUniqueVertexFunc(uniqueVertex, vtx.vIdx, vtIdx, vnIdx);

                targetIndicesLane[i] = uniqueVertexCounter;
                uniqueVertexMap.insert(key, uniqueVertexCounter);
                uniqueVertexCounter++;
            }
        }
    }

    // Trim the master structural array down to the true unique global vertex count
    outVertexArray.resize(uniqueVertexCounter);
}

} // namespace


GtMeshLoader::Mesh GtMeshLoader::Parser::Parse(const QString& fileName)
{
    Mesh result;

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        return result;
    }

    // 1. Reset core vectors for safe object reuse
    m_vertices.clear();
    m_texCoords.clear();
    m_normalCoords.clear();
    m_materialFaces.clear();
    m_currentMaterialName = Name();

    // Stream execution loop over the file
    QTextStream stream(&file);
    while (!stream.atEnd()) {
        QString token;
        stream >> token;

        auto it = m_factory.find(token);
        if (it != m_factory.end()) {
            it.value()(stream);
        } else {
            stream.readLine();
        }
    }

    // Intermediate tracking lane to hold optimized index arrays before allocating on GPU
    QHash<Name, QVector<qint32>> unifiedIndicesLanes;

    // 🌟 Option B Core Fix: The structural layout switch sits OUTSIDE the material tracks loop
    GtMeshBufferPtr& meshBuffer = result.Buffer;
    switch(m_format) {
    case FmtVerticesAndNormals: {
        QVector<Vertex3f3f> vertexes;

        UnifyStructuredMaterialTracks<false, true>(vertexes, m_materialFaces, unifiedIndicesLanes,
                                                   [this](Vertex3f3f& v, qint32 vIdx, qint32, qint32 vnIdx) {
            v.Position = m_vertices.at(vIdx);
            v.Normal   = m_normalCoords.at(vnIdx);
        });

        meshBuffer = ::make_shared<GtMeshBuffer>(GtMeshBuffer::VertexType_Vertex3f3f, QOpenGLBuffer::StaticDraw);
        meshBuffer->UpdateVertexArray(vertexes);
        break;
    }

    case FmtVerticesAndTextures: {
        QVector<Vertex3f3f> vertexes;

        UnifyStructuredMaterialTracks<true, false>(vertexes, m_materialFaces, unifiedIndicesLanes,
                                                   [this](Vertex3f3f& v, qint32 vIdx, qint32 vtIdx, qint32) {
            v.Position = m_vertices.at(vIdx);
            v.TexCoord = m_texCoords.at(vtIdx);
        });

        meshBuffer = ::make_shared<GtMeshBuffer>(GtMeshBuffer::VertexType_Vertex3f3f, QOpenGLBuffer::StaticDraw);
        meshBuffer->UpdateVertexArray(vertexes);
        break;
    }

    case FmtFull: {
        QVector<Vertex3f3f3f> vertexes;

        UnifyStructuredMaterialTracks<true, true>(vertexes, m_materialFaces, unifiedIndicesLanes,
                                                  [this](Vertex3f3f3f& v, qint32 vIdx, qint32 vtIdx, qint32 vnIdx) {
            v.Position = m_vertices.at(vIdx);
            if(vnIdx >= 0) {
                v.Normal   = m_normalCoords.at(vnIdx);
            }
            if(vtIdx >= 0) {
                v.TexCoord = m_texCoords.at(vtIdx);
            }
        });

        meshBuffer = ::make_shared<GtMeshBuffer>(GtMeshBuffer::VertexType_Vertex3f3f3f, QOpenGLBuffer::StaticDraw);
        meshBuffer->UpdateVertexArray(vertexes);
        break;
    }

    case FmtDefault:
    default: {
        QVector<Point3F> vertexes;

        UnifyStructuredMaterialTracks<false, false>(vertexes, m_materialFaces, unifiedIndicesLanes,
                                                    [this](Point3F& v, qint32 vIdx, qint32, qint32) {
            v = m_vertices.at(vIdx);
        });

        meshBuffer = ::make_shared<GtMeshBuffer>(GtMeshBuffer::VertexType_Point3F, QOpenGLBuffer::StaticDraw);
        meshBuffer->UpdateVertexArray(vertexes);
        break;
    }
    }

    // 🌟 3. Safely upload each material's index track to separate target hardware EBO structures
    for (auto lanesIt = unifiedIndicesLanes.cbegin(); lanesIt != unifiedIndicesLanes.cend(); ++lanesIt) {
        if (!lanesIt.value().isEmpty()) {
            auto& indicesBuffer = result.MaterialIndicesBuffer[lanesIt.key()];
            indicesBuffer = ::make_shared<GtMeshBuffer>(GtMeshBuffer::VertexType_IntIndex, QOpenGLBuffer::StaticDraw);
            indicesBuffer->UpdateIndicesArray(lanesIt.value());
        }
    }

    return result;
}

QHash<Name, GtMeshLoader::Material> GtMeshLoader::LoadMaterials(const QString& fileName)
{
    QHash<Name, Material> materialPool;

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        return materialPool; // Return empty pool if file is missing
    }

    // High-efficiency tracking pointer for the active material
    Material* activeMaterial = nullptr;

    // Fulfills your exact prompt: The static factory now ONLY needs the target Material parameter
    using MtlParserFunc = std::function<void(QTextStream&, Material*)>;
    using MtlParserMap  = QHash<QString, MtlParserFunc>;

    thread_local static auto mtlFactory = []() {
        MtlParserMap mtlFactory;

        // 1. Scalar Properties (Ns, Ni, d, Tr)
        mtlFactory.insert("Ns", [](QTextStream& s, Material* active) {
            if (active) s >> active->Ns;
        });
        mtlFactory.insert("Ni", [](QTextStream& s, Material* active) {
            if (active) s >> active->Ni;
        });
        mtlFactory.insert("d", [](QTextStream& s, Material* active) {
            if (active) s >> active->D; // Dissolve / Opacity
        });
        mtlFactory.insert("Tr", [](QTextStream& s, Material* active) {
            if (active) {
                float tr = 0.0f;
                s >> tr;
                active->D = 1.0f - tr; // Tr (Transparency) is inverted relative to d
            }
        });

        // 2. Color Constants (Ka, Kd, Ks, Ke)
        mtlFactory.insert("Ka", [](QTextStream& s, Material* active) {
            if (active) {
                float r = 0.0f, g = 0.0f, b = 0.0f;
                s >> r >> g >> b;
                active->Ka = Vector3F(r, g, b);
            }
        });
        mtlFactory.insert("Kd", [](QTextStream& s, Material* active) {
            if (active) {
                float r = 0.0f, g = 0.0f, b = 0.0f;
                s >> r >> g >> b;
                active->Kd = Vector3F(r, g, b);
            }
        });
        mtlFactory.insert("Ks", [](QTextStream& s, Material* active) {
            if (active) {
                float r = 0.0f, g = 0.0f, b = 0.0f;
                s >> r >> g >> b;
                active->Ks = Vector3F(r, g, b);
            }
        });
        mtlFactory.insert("Ke", [](QTextStream& s, Material* active) {
            if (active) {
                float r = 0.0f, g = 0.0f, b = 0.0f;
                s >> r >> g >> b;
                active->Ke = Vector3F(r, g, b);
            }
        });

        // 3. Texture Maps Extraction
        auto ReadFileName = [](QTextStream& s) -> QString {
            QString path;
            s >> path;
            return path.trimmed();
        };

        mtlFactory.insert("map_Ka", [ReadFileName](QTextStream& s, Material* active) {
            if (active) active->MapKaFileName = Name(ReadFileName(s));
        });
        mtlFactory.insert("map_Kd", [ReadFileName](QTextStream& s, Material* active) {
            if (active) active->MapKdFileName = Name(ReadFileName(s));
        });
        mtlFactory.insert("map_Ks", [ReadFileName](QTextStream& s, Material* active) {
            if (active) active->MapKsFileName = Name(ReadFileName(s));
        });
        mtlFactory.insert("map_Ke", [ReadFileName](QTextStream& s, Material* active) {
            if (active) active->MapKeFileName = Name(ReadFileName(s));
        });
        mtlFactory.insert("map_d", [ReadFileName](QTextStream& s, Material* active) {
            if (active) active->MapDFileName = Name(ReadFileName(s));
        });
        mtlFactory.insert("map_Pr", [ReadFileName](QTextStream& s, Material* active) {
            if (active) active->MapPrFileName = Name(ReadFileName(s));
        });
        mtlFactory.insert("map_Pm", [ReadFileName](QTextStream& s, Material* active) {
            if (active) active->MapPmFileName = Name(ReadFileName(s));
        });
        mtlFactory.insert("norm", [ReadFileName](QTextStream& s, Material* active) {
            if (active) active->MapKnFileName = Name(ReadFileName(s));
        });
        mtlFactory.insert("map_Bump", [ReadFileName](QTextStream& s, Material* active) {
            if (active) active->MapBumpFileName = Name(ReadFileName(s));
        });
        mtlFactory.insert("bump", [ReadFileName](QTextStream& s, Material* active) {
            if (active) active->MapBumpFileName = Name(ReadFileName(s)); // Standard alias fallback
        });
                // 5. Register Modern PBR Extensions to the MTL Parsing Factory
        mtlFactory.insert("Pr", [](QTextStream& s, Material* active) {
            if (active) {
                float val = 0.5f;
                s >> val;
                active->Pr = val;
            }
        });

        mtlFactory.insert("Pm", [](QTextStream& s, Material* active) {
            if (active) {
                float val = 0.0f;
                s >> val;
                active->Pm = val;
            }
        });

        mtlFactory.insert("Ps", [](QTextStream& s, Material* active) {
            if (active) {
                float val = 0.0f;
                s >> val;
                active->Ps = val;
            }
        });

        mtlFactory.insert("Pc", [](QTextStream& s, Material* active) {
            if (active) {
                float val = 0.0f;
                s >> val;
                active->Pc = val;
            }
        });

        mtlFactory.insert("Pcr", [](QTextStream& s, Material* active) {
            if (active) {
                float val = 0.0f;
                s >> val;
                active->Pcr = val;
            }
        });

        // Optional parameters matching your material string specs
        mtlFactory.insert("aniso", [](QTextStream& s, Material* active) {
            if (active) {
                float val = 0.0f;
                s >> val;
                // Add an anisotropy property to your Material struct if you intend to map it to uniforms later
                // active->aniso = val;
            }
        });

        mtlFactory.insert("anisor", [](QTextStream& s, Material* active) {
            if (active) {
                float val = 0.0f;
                s >> val;
                // active->anisor = val;
            }
        });


        // 4. Illumination Model Mapping (illum 0, 1, 2)
        mtlFactory.insert("illum", [](QTextStream& s, Material* active) {
            if (active) {
                int illumInt = 2;
                s >> illumInt;

                switch (illumInt) {
                case 0:  active->Ellum = Material::ConstantColor; break;
                case 1:  active->Ellum = Material::DiffuseShadingOnly; break;
                case 2:
                default: active->Ellum = Material::Full; break;
                }
            }
        });

        return mtlFactory;
    }();

    // Main token processing loop over the text stream
    QTextStream stream(&file);
    while (!stream.atEnd()) {
        QString token;
        stream >> token;

        if (token.startsWith('#')) {
            stream.readLine(); // Ignore line comments completely
            continue;
        }

        // Intercept structural token explicitly to manage container allocation safely
        if (token == "newmtl") {
            QString rawName;
            stream >> rawName;
            rawName = rawName.trimmed();

            if (!rawName.isEmpty()) {
                Name matKey(rawName);

                // Safely insert new material. If QHash rehashes, it happens precisely here.
                materialPool[matKey] = Material();

                // Re-resolve activeMaterial pointer immediately after modification.
                // This address is guaranteed valid until the next "newmtl" token appears.
                activeMaterial = &materialPool[matKey];
                activeMaterial->MaterialName = matKey;
            }
            continue;
        }

        // Delegate property parsing to the static, stateless factory
        auto it = mtlFactory.find(token);
        if (it != mtlFactory.end()) {
            it.value()(stream, activeMaterial); // Native O(1) property offset speed
        } else {
            stream.readLine(); // Skip unknown parameters gracefully
        }
    }

    return materialPool;
}

GtMeshBufferPtr GtMeshLoader::LoadObj(const QString& fileName){
    QVector<Point3F> vertices;
    QVector<Point3F> normalCoords;
    QVector<Point3F> texCoords;
    QVector<Vertex3f3f> vertexes;

    QVector<qint32> vertexIndices;
    QVector<qint32> normalIndices;
    QVector<qint32> textureIndices;

    thread_local static QRegExp regExp(R"((\d+)\/(\d+)\/?(\d+)?)");
    QFile file(fileName);
    if(!file.open(QFile::ReadOnly)) {
        return nullptr;
    }

    QHash<Name, std::function<void (QTextStream&)>> factory = {
    { "v", [&vertices](QTextStream& stream) {
        Point3F point;
        stream >> point.X() >> point.Y() >> point.Z();
        vertices.append(point);
    }},
    { "vt", [&texCoords](QTextStream& stream) {
        Point3F point;
        stream >> point.X() >> point.Y() >> point.Z();
        texCoords.append(point);
    }},
    { "vn", [&normalCoords](QTextStream& stream) {
        Point3F point;
        stream >> point.X() >> point.Y() >> point.Z();
        normalCoords.append(point);
    }},
    { "f", [&vertexIndices, &normalIndices, &textureIndices](QTextStream& stream) {
        auto line = stream.readLine();
        qint32 pos = 0;
        qint32 count = 0;
        while((pos = regExp.indexIn(line, pos)) != -1) {
            vertexIndices.append(regExp.cap(1).toInt() - 1);
            textureIndices.append(regExp.cap(2).toInt() - 1);
            normalIndices.append(regExp.cap(3).toInt() - 1);
            pos += regExp.matchedLength();
            count++;
        }
        if(count > 3) {
            auto firstIndex = vertexIndices.at(vertexIndices.size() - 4);
            auto thirdIndex = vertexIndices.at(vertexIndices.size() - 2);
            vertexIndices.append(firstIndex);
            vertexIndices.append(thirdIndex);
            if(!normalIndices.isEmpty()) {
                firstIndex = normalIndices.at(normalIndices.size() - 4);
                thirdIndex = normalIndices.at(normalIndices.size() - 2);
                normalIndices.append(firstIndex);
                normalIndices.append(thirdIndex);
            }
            if(!textureIndices.isEmpty()) {
                firstIndex = textureIndices.at(textureIndices.size() - 4);
                thirdIndex = textureIndices.at(textureIndices.size() - 2);
                textureIndices.append(firstIndex);
                textureIndices.append(thirdIndex);
            }
        }
    }},
    };

    QTextStream stream(&file);
    while(!stream.atEnd()) {
        QString entry;
        stream >> entry;
        Name entryName(entry);

        auto foundIt = factory.find(entryName);
        if(foundIt != factory.end()) {
            foundIt.value()(stream);
        } else {
            stream.readLine();
        }
    }

    for(qint32 i(0); i < vertexIndices.size(); i++) {
        Vertex3f3f vertex;
        vertex.Position = vertices.at(vertexIndices.at(i));
        vertex.Normal = normalCoords.at(normalIndices.at(i));
        vertexes.append(vertex);
    }

    auto meshBuffer = ::make_shared<GtMeshBuffer>(GtMeshBuffer::VertexType_Vertex3f3f, QOpenGLBuffer::StaticDraw);
    meshBuffer->UpdateVertexArray(vertexes);
    return meshBuffer;
}
