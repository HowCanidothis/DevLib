#include "gtmeshloader.h"

GtMeshPtr GtMeshLoader::LoadObj(const QString& fileName){
    QVector<Point3F> vertices;
    QVector<Point3F> texCoords;
    QVector<Vertex3f3f> vertexes;

    QVector<qint32> faceIndices;
    QVector<qint32> texIndices;

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
        { "vn", [&texCoords](QTextStream& stream) {
            Point3F point;
            stream >> point.X() >> point.Y() >> point.Z();
            texCoords.append(point);
        }},
        { "f", [&faceIndices, &texIndices](QTextStream& stream) {
            auto line = stream.readLine();
            qint32 pos = 0;
            qint32 count = 0;
            while((pos = regExp.indexIn(line, pos)) != -1) {
                faceIndices.append(regExp.cap(1).toInt() - 1);
                texIndices.append(regExp.cap(3).toInt() - 1);
                pos += regExp.matchedLength();
                count++;
            }
            if(count > 3) {
                auto firstIndex = faceIndices.at(faceIndices.size() - 4);
                auto thirdIndex = faceIndices.at(faceIndices.size() - 2);
                faceIndices.append(firstIndex);
                faceIndices.append(thirdIndex);
                firstIndex = texIndices.at(texIndices.size() - 4);
                thirdIndex = texIndices.at(texIndices.size() - 2);
                texIndices.append(firstIndex);
                texIndices.append(thirdIndex);
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

    for(qint32 i(0); i < faceIndices.size(); i++) {
        Vertex3f3f vertex;
        vertex.Position = vertices.at(faceIndices.at(i));
        vertex.Normal = texCoords.at(texIndices.at(i));
        vertexes.append(vertex);
    }

    auto meshBuffer = ::make_shared<GtMeshBuffer>(GtMeshBuffer::VertexType_Vertex3f3f, QOpenGLBuffer::StaticDraw);
    meshBuffer->UpdateVertexArray(vertexes);
    auto mesh = ::make_shared<GtMesh>(meshBuffer);
    return mesh;
}
