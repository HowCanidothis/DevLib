#include "flex.h"

void SimBuffers::map()
{
    positions.map();
    restPositions.map();
    velocities.map();
    phases.map();
    densities.map();
    anisotropy1.map();
    anisotropy2.map();
    anisotropy3.map();
    normals.map();
    diffusePositions.map();
    diffuseVelocities.map();
    diffuseCount.map();
    smoothPositions.map();
    activeIndices.map();

    // convexes
    shapeGeometry.map();
    shapePositions.map();
    shapeRotations.map();
    shapePrevPositions.map();
    shapePrevRotations.map();
    shapeFlags.map();

    rigidOffsets.map();
    rigidIndices.map();
    rigidMeshSize.map();
    rigidCoefficients.map();
    rigidPlasticThresholds.map();
    rigidPlasticCreeps.map();
    rigidRotations.map();
    rigidTranslations.map();
    rigidLocalPositions.map();
    rigidLocalNormals.map();

    springIndices.map();
    springLengths.map();
    springStiffness.map();

    // inflatables
    inflatableTriOffsets.map();
    inflatableTriCounts.map();
    inflatableVolumes.map();
    inflatableCoefficients.map();
    inflatablePressures.map();

    triangles.map();
    triangleNormals.map();
    uvs.map();
}

void SimBuffers::unmap()
{
    // particles
    positions.unmap();
    restPositions.unmap();
    velocities.unmap();
    phases.unmap();
    densities.unmap();
    anisotropy1.unmap();
    anisotropy2.unmap();
    anisotropy3.unmap();
    normals.unmap();
    diffusePositions.unmap();
    diffuseVelocities.unmap();
    diffuseCount.unmap();
    smoothPositions.unmap();
    activeIndices.unmap();

    // convexes
    shapeGeometry.unmap();
    shapePositions.unmap();
    shapeRotations.unmap();
    shapePrevPositions.unmap();
    shapePrevRotations.unmap();
    shapeFlags.unmap();

    // rigids
    rigidOffsets.unmap();
    rigidIndices.unmap();
    rigidMeshSize.unmap();
    rigidCoefficients.unmap();
    rigidPlasticThresholds.unmap();
    rigidPlasticCreeps.unmap();
    rigidRotations.unmap();
    rigidTranslations.unmap();
    rigidLocalPositions.unmap();
    rigidLocalNormals.unmap();

    // springs
    springIndices.unmap();
    springLengths.unmap();
    springStiffness.unmap();

    // inflatables
    inflatableTriOffsets.unmap();
    inflatableTriCounts.unmap();
    inflatableVolumes.unmap();
    inflatableCoefficients.unmap();
    inflatablePressures.unmap();

    // triangles
    triangles.unmap();
    triangleNormals.unmap();
    uvs.unmap();
}
