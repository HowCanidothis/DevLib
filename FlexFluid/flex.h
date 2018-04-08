#ifndef FLEX_H
#define FLEX_H

#include <NvFlexExt.h>
#include "GraphicsTools/gt_decl.h"

struct SimBuffers
{
    NvFlexVector<Vector4F> positions;
    NvFlexVector<Vector4F> restPositions;
    NvFlexVector<Vector3F> velocities;
    NvFlexVector<int> phases;
    NvFlexVector<float> densities;
    NvFlexVector<Vector4F> anisotropy1;
    NvFlexVector<Vector4F> anisotropy2;
    NvFlexVector<Vector4F> anisotropy3;
    NvFlexVector<Vector4F> normals;
    NvFlexVector<Vector4F> smoothPositions;
    NvFlexVector<Vector4F> diffusePositions;
    NvFlexVector<Vector4F> diffuseVelocities;
    NvFlexVector<int> diffuseCount;

    NvFlexVector<int> activeIndices;

    // convexes
    NvFlexVector<NvFlexCollisionGeometry> shapeGeometry;
    NvFlexVector<Vector4F> shapePositions;
    NvFlexVector<Quaternion> shapeRotations;
    NvFlexVector<Vector4F> shapePrevPositions;
    NvFlexVector<Quaternion> shapePrevRotations;
    NvFlexVector<int> shapeFlags;

    // rigids
    NvFlexVector<int> rigidOffsets;
    NvFlexVector<int> rigidIndices;
    NvFlexVector<int> rigidMeshSize;
    NvFlexVector<float> rigidCoefficients;
    NvFlexVector<float> rigidPlasticThresholds;
    NvFlexVector<float> rigidPlasticCreeps;
    NvFlexVector<Quaternion> rigidRotations;
    NvFlexVector<Vector3F> rigidTranslations;
    NvFlexVector<Vector3F> rigidLocalPositions;
    NvFlexVector<Vector4F> rigidLocalNormals;

    // inflatables
    NvFlexVector<int> inflatableTriOffsets;
    NvFlexVector<int> inflatableTriCounts;
    NvFlexVector<float> inflatableVolumes;
    NvFlexVector<float> inflatableCoefficients;
    NvFlexVector<float> inflatablePressures;

    // springs
    NvFlexVector<int> springIndices;
    NvFlexVector<float> springLengths;
    NvFlexVector<float> springStiffness;

    NvFlexVector<int> triangles;
    NvFlexVector<Vector3F> triangleNormals;
    NvFlexVector<Vector3F> uvs;

    SimBuffers(NvFlexLibrary* l) :
        positions(l), restPositions(l), velocities(l), phases(l), densities(l),
        anisotropy1(l), anisotropy2(l), anisotropy3(l), normals(l), smoothPositions(l),
        diffusePositions(l), diffuseVelocities(l), diffuseCount(l), activeIndices(l),
        shapeGeometry(l), shapePositions(l), shapeRotations(l), shapePrevPositions(l),
        shapePrevRotations(l),	shapeFlags(l), rigidOffsets(l), rigidIndices(l), rigidMeshSize(l),
        rigidCoefficients(l), rigidPlasticThresholds(l), rigidPlasticCreeps(l), rigidRotations(l), rigidTranslations(l),
        rigidLocalPositions(l), rigidLocalNormals(l), inflatableTriOffsets(l),
        inflatableTriCounts(l), inflatableVolumes(l), inflatableCoefficients(l),
        inflatablePressures(l), springIndices(l), springLengths(l),
        springStiffness(l), triangles(l), triangleNormals(l), uvs(l)
    {}

    void map();
    void unmap();
};
#endif // FLEX_H
