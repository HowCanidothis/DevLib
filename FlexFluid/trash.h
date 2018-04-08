#ifndef TRASH_H
#define TRASH_H

#include "GraphicsTools/gt_decl.h"
#include <assert.h>

struct Matrix33
{
    Vector3F Rotate(const Quaternion& q, const Vector3F& x)
    {
        return x*(2.0f*q.scalar()*q.scalar()-1.0f) +
                Vector3F::crossProduct(Vector3F(q.x(), q.y(), q.z()), x)*q.scalar()*2.0f +
                Vector3F(q.x(),q.y(),q.z())*Vector3F::dotProduct(Vector3F(q.x(), q.y(), q.z()), x)*2.0f;
    }

    Matrix33(const Quaternion& q)
    {
        cols[0] = Rotate(q, Vector3F(1.0f, 0.0f, 0.0f));
        cols[1] = Rotate(q, Vector3F(0.0f, 1.0f, 0.0f));
        cols[2] = Rotate(q, Vector3F(0.0f, 0.0f, 1.0f));
    }

    Vector3F cols[3];
};

inline void TransformBounds(const Vector3F& localLower, const Vector3F& localUpper, const Vector3F& translation, const Quaternion& rotation, float scale, Vector3F& lower, Vector3F& upper)
{
    Matrix33 transform(rotation);

    Vector3F extents = (localUpper-localLower)*scale;

    transform.cols[0] *= extents.x();
    transform.cols[1] *= extents.y();
    transform.cols[2] *= extents.z();

    float ex = fabsf(transform.cols[0].x()) + fabsf(transform.cols[1].x()) + fabsf(transform.cols[2].x());
    float ey = fabsf(transform.cols[0].y()) + fabsf(transform.cols[1].y()) + fabsf(transform.cols[2].y());
    float ez = fabsf(transform.cols[0].z()) + fabsf(transform.cols[1].z()) + fabsf(transform.cols[2].z());

    Vector3F center = (localUpper+localLower)*0.5f*scale;

    lower = rotation*center + translation - Vector3F(ex, ey, ez)*0.5f;
    upper = rotation*center + translation + Vector3F(ex, ey, ez)*0.5f;
}


// calculates local space positions given a set of particle positions, rigid indices and centers of mass of the rigids
void CalculateRigidLocalPositions(const Vector4F* restPositions, const int* offsets, const Vector3F* translations, const int* indices, int numRigids, Vector3F* localPositions)
{
    int count = 0;

    for (int i=0; i < numRigids; ++i)
    {
        const int startIndex = offsets[i];
        const int endIndex = offsets[i+1];

        assert(endIndex-startIndex);

        for (int j=startIndex; j < endIndex; ++j)
        {
            const int r = indices[j];

            localPositions[count++] = Vector3F(restPositions[r]) - translations[i];
        }
    }
}

// calculates the center of mass of every rigid given a set of particle positions and rigid indices
void CalculateRigidCentersOfMass(const Vector4F* restPositions, int numRestPositions, const int* offsets, Vector3F* translations, const int* indices, int numRigids)
{
    // To improve the accuracy of the result, first transform the restPositions to relative coordinates (by finding the mean and subtracting that from all positions)
    // Note: If this is not done, one might see ghost forces if the mean of the restPositions is far from the origin.
    Vector3F shapeOffset;

    for (int i = 0; i < numRestPositions; i++)
    {
        shapeOffset += Vector3F(restPositions[i]);
    }

    shapeOffset /= float(numRestPositions);

    for (int i=0; i < numRigids; ++i)
    {
        const int startIndex = offsets[i];
        const int endIndex = offsets[i+1];

        const int n = endIndex-startIndex;

        assert(n);

        Vector3F com;

        for (int j=startIndex; j < endIndex; ++j)
        {
            const int r = indices[j];

            // By subtracting shapeOffset the calculation is done in relative coordinates
            com += Vector3F(restPositions[r]) - shapeOffset;
        }

        com /= float(n);

        // Add the shapeOffset to switch back to absolute coordinates
        com += shapeOffset;

        translations[i] = com;

    }
}


template <typename T>
inline T SafeNormalize(const T& v, const T& fallback=T())
{
    float l = v.lengthSquared();
    if (l > 0.0f)
    {
        return v * 1.0f / sqrtf(l);
    }
    else
        return fallback;
}

static Vector3F VMin(const Vector3F& l, const Vector3F& r) {
    return Vector3F(std::min(l.x(), r.x()), std::min(l.y(), r.y()), std::min(l.z(), r.z()));
}
static Vector3F VMax(const Vector3F& l, const Vector3F& r) {
    return Vector3F(std::max(l.x(), r.x()), std::max(l.y(), r.y()), std::max(l.z(), r.z()));
}

struct Bounds
{
    Bounds()
        : lower( FLT_MAX, FLT_MAX, FLT_MAX)
        , upper(-FLT_MAX, -FLT_MAX, -FLT_MAX)
    {}

    Bounds(const Vector3F& lower, const Vector3F& upper) : lower(lower), upper(upper) {}

    Vector3F GetCenter() const { return 0.5f*(lower+upper); }
    Vector3F GetEdges() const { return upper-lower; }

    void Expand(float r)
    {
        lower -= Vector3F(r, r, r);
        upper += Vector3F(r, r, r);
    }

    void Expand(const Vector3F& r)
    {
        lower -= r;
        upper += r;
    }

    bool Empty() const { return lower.x() >= upper.x() || lower.y() >= upper.y() || lower.z() >= upper.z(); }

    bool Overlaps(const Vector3F& p) const
    {
        if (p.x() < lower.x() ||
            p.y() < lower.y() ||
            p.z() < lower.z() ||
            p.x() > upper.x() ||
            p.y() > upper.y() ||
            p.z() > upper.z())
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    bool Overlaps(const Bounds& b) const
    {
        if (lower.x() > b.upper.x() ||
            lower.y() > b.upper.y() ||
            lower.z() > b.upper.z() ||
            upper.x() < b.lower.x() ||
            upper.y() < b.lower.y() ||
            upper.z() < b.lower.z())
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    Vector3F lower;
    Vector3F upper;
};

Bounds Union(const Bounds& a, const Bounds& b)
{
    return Bounds(VMin(a.lower, b.lower), VMax(a.upper, b.upper));
}

float SampleSDF(const float* sdf, int dim, int x, int y, int z)
{
    assert(x < dim && x >= 0);
    assert(y < dim && y >= 0);
    assert(z < dim && z >= 0);

    return sdf[z*dim*dim + y*dim + x];
}

// return normal of signed distance field
Vector3F SampleSDFGrad(const float* sdf, int dim, int x, int y, int z)
{
    int x0 = std::max(x-1, 0);
    int x1 = std::min(x+1, dim-1);

    int y0 = std::max(y-1, 0);
    int y1 = std::min(y+1, dim-1);

    int z0 = std::max(z-1, 0);
    int z1 = std::min(z+1, dim-1);

    float dx = (SampleSDF(sdf, dim, x1, y, z) - SampleSDF(sdf, dim, x0, y, z))*(dim*0.5f);
    float dy = (SampleSDF(sdf, dim, x, y1, z) - SampleSDF(sdf, dim, x, y0, z))*(dim*0.5f);
    float dz = (SampleSDF(sdf, dim, x, y, z1) - SampleSDF(sdf, dim, x, y, z0))*(dim*0.5f);

    return Vector3F(dx, dy, dz);
}

#endif // TRASH_H
