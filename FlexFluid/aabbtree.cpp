// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2013-2016 NVIDIA Corporation. All rights reserved.

#include "aabbtree.h"

#include <algorithm>
#include <iostream>
#include <assert.h>

static float minf(const float a, const float b) { return a < b ? a : b; }
static float maxf(const float a, const float b) { return a > b ? a : b; }

static bool IntersectRayAABBOmpf(const Vector3F& pos, const Vector3F& rcp_dir, const Vector3F& min, const Vector3F& max, float& t) {

    float
        l1	= (min.x() - pos.x()) * rcp_dir.x(),
        l2	= (max.x() - pos.x()) * rcp_dir.x(),
        lmin	= minf(l1,l2),
        lmax	= maxf(l1,l2);

    l1	= (min.y() - pos.y()) * rcp_dir.y();
    l2	= (max.y() - pos.y()) * rcp_dir.y();
    lmin	= maxf(minf(l1,l2), lmin);
    lmax	= minf(maxf(l1,l2), lmax);

    l1	= (min.z() - pos.z()) * rcp_dir.z();
    l2	= (max.z() - pos.z()) * rcp_dir.z();
    lmin	= maxf(minf(l1,l2), lmin);
    lmax	= minf(maxf(l1,l2), lmax);

    //return ((lmax > 0.f) & (lmax >= lmin));
    //return ((lmax > 0.f) & (lmax > lmin));
    bool hit = ((lmax >= 0.f) & (lmax >= lmin));
    if (hit)
        t = lmin;
    return hit;
}

// Moller and Trumbore's method
static bool IntersectRayTriTwoSided(const Vector3F& p, const Vector3F& dir, const Vector3F& a, const Vector3F& b, const Vector3F& c, float& t, float& u, float& v, float& w, float& sign)//Vec3* normal)
{
    Vector3F ab = b - a;
    Vector3F ac = c - a;
    Vector3F n = Vector3F::crossProduct(ab, ac);

    float d = Vector3F::dotProduct(-dir, n);
    float ood = 1.0f / d; // No need to check for division by zero here as infinity aritmetic will save us...
    Vector3F ap = p - a;

    t = Vector3F::dotProduct(ap, n) * ood;
    if (t < 0.0f)
        return false;

    Vector3F e = Vector3F::crossProduct(-dir, ap);
    v = Vector3F::dotProduct(ac, e) * ood;
    if (v < 0.0f || v > 1.0f) // ...here...
        return false;
    w = -Vector3F::dotProduct(ab, e) * ood;
    if (w < 0.0f || v + w > 1.0f) // ...and here
        return false;

    u = 1.0f - v - w;
    //if (normal)
        //*normal = n;
    sign = d;

    return true;
}

static bool IntersectRayAABB(const Vector3F& start, const Vector3F& dir, const Vector3F& min, const Vector3F& max, float& t, Vector3F* )
{
    //! calculate candidate plane on each axis
    float tx = -1.0f, ty = -1.0f, tz = -1.0f;
    bool inside = true;

    //! use unrolled loops

    //! x
    if (start.x() < min.x())
    {
        if (dir.x() != 0.0f)
            tx = (min.x()-start.x())/dir.x();
        inside = false;
    }
    else if (start.x() > max.x())
    {
        if (dir.x() != 0.0f)
            tx = (max.x()-start.x())/dir.x();
        inside = false;
    }

    //! y
    if (start.y() < min.y())
    {
        if (dir.y() != 0.0f)
            ty = (min.y()-start.y())/dir.y();
        inside = false;
    }
    else if (start.y() > max.y())
    {
        if (dir.y() != 0.0f)
            ty = (max.y()-start.y())/dir.y();
        inside = false;
    }

    //! z
    if (start.z() < min.z())
    {
        if (dir.z() != 0.0f)
            tz = (min.z()-start.z())/dir.z();
        inside = false;
    }
    else if (start.z() > max.z())
    {
        if (dir.z() != 0.0f)
            tz = (max.z()-start.z())/dir.z();
        inside = false;
    }

    //! if point inside all planes
    if (inside)
    {
        t = 0.0f;
        return true;
    }

    //! we now have t values for each of possible intersection planes
    //! find the maximum to get the intersection point
    float tmax = tx;
    int taxis = 0;

    if (ty > tmax)
    {
        tmax = ty;
        taxis = 1;
    }
    if (tz > tmax)
    {
        tmax = tz;
        taxis = 2;
    }

    if (tmax < 0.0f)
        return false;

    //! check that the intersection point lies on the plane we picked
    //! we don't test the axis of closest intersection for precision reasons

    //! no eps for now
    float eps = 0.0f;

    Vector3F hit = start + dir*tmax;

    if ((hit.x() < min.x()-eps || hit.x() > max.x()+eps) && taxis != 0)
        return false;
    if ((hit.y() < min.y()-eps || hit.y() > max.y()+eps) && taxis != 1)
        return false;
    if ((hit.z() < min.z()-eps || hit.z() > max.z()+eps) && taxis != 2)
        return false;

    //! output results
    t = tmax;

    return true;
}

using namespace std;

uint32_t AABBTree::s_traceDepth;

AABBTree::AABBTree(const Vector3F* vertices, uint32_t numVerts, const uint32_t* indices, uint32_t numFaces)
    : m_vertices(vertices)
    , m_numVerts(numVerts)
    , m_indices(indices)
    , m_numFaces(numFaces)
{
    // build stats
    m_treeDepth = 0;
    m_innerNodes = 0;
    m_leafNodes = 0;

    Build();
}

namespace
{

	struct FaceSorter
	{
        FaceSorter(const Vector3F* positions, const uint32_t* indices, uint32_t n, uint32_t axis)
			: m_vertices(positions)
			, m_indices(indices)
			, m_numIndices(n)
			, m_axis(axis)
		{        
		}

		inline bool operator()(uint32_t lhs, uint32_t rhs) const
		{
			float a = GetCentroid(lhs);
			float b = GetCentroid(rhs);

			if (a == b)
				return lhs < rhs;
			else
				return a < b;
		}

		inline float GetCentroid(uint32_t face) const
		{
            const Vector3F& a = m_vertices[m_indices[face*3+0]];
            const Vector3F& b = m_vertices[m_indices[face*3+1]];
            const Vector3F& c = m_vertices[m_indices[face*3+2]];

			return (a[m_axis] + b[m_axis] + c[m_axis])/3.0f;
		}

        const Vector3F* m_vertices;
		const uint32_t* m_indices;
		uint32_t m_numIndices;
		uint32_t m_axis;
	};
	
    inline uint32_t LongestAxis(const Vector3F& v)
	{    
        if (v.x() > v.y() && v.x() > v.z())
			return 0;
		else
            return (v.y() > v.z()) ? 1 : 2;
	}

} // anonymous namespace

void AABBTree::CalculateFaceBounds(uint32_t* faces, uint32_t numFaces, Vector3F& outMinExtents, Vector3F& outMaxExtents)
{
    Vector3F minExtents(FLT_MAX, FLT_MAX, FLT_MAX);
    Vector3F maxExtents(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    // calculate face bounds
    for (uint32_t i=0; i < numFaces; ++i)
    {
        Vector3F a = Vector3F(m_vertices[m_indices[faces[i]*3+0]]);
        Vector3F b = Vector3F(m_vertices[m_indices[faces[i]*3+1]]);
        Vector3F c = Vector3F(m_vertices[m_indices[faces[i]*3+2]]);

        minExtents = VMin(a, minExtents);
        maxExtents = VMax(a, maxExtents);

        minExtents = VMin(b, minExtents);
        maxExtents = VMax(b, maxExtents);

        minExtents = VMin(c, minExtents);
        maxExtents = VMax(c, maxExtents);
    }

    outMinExtents = minExtents;
    outMaxExtents = maxExtents;
}

// track current tree depth
static uint32_t s_depth = 0;

void AABBTree::Build()
{
    assert(m_numFaces*3);

    //const double startTime = GetSeconds();

    const uint32_t numFaces = m_numFaces;

    // build initial list of faces
    m_faces.reserve(numFaces);

    // calculate bounds of each face and store
    m_faceBounds.reserve(numFaces);   
    
	for (uint32_t i=0; i < numFaces; ++i)
    {
		Bounds top;
        CalculateFaceBounds(&i, 1, top.m_min, top.m_max);
		
		m_faces.push_back(i);
		m_faceBounds.push_back(top);
    }

	m_nodes.reserve(uint32_t(numFaces*1.5f));

    // allocate space for all the nodes
	m_freeNode = 1;

    // start building
    BuildRecursive(0, &m_faces[0], numFaces);

    assert(s_depth == 0);

    FaceBoundsArray f;
    m_faceBounds.swap(f);
}

// partion faces around the median face
uint32_t AABBTree::PartitionMedian(Node& n, uint32_t* faces, uint32_t numFaces)
{
	FaceSorter predicate(&m_vertices[0], &m_indices[0], m_numFaces*3, LongestAxis(n.m_maxExtents-n.m_minExtents));
    std::nth_element(faces, faces+numFaces/2, faces+numFaces, predicate);

	return numFaces/2;
}

// partion faces based on the surface area heuristic
uint32_t AABBTree::PartitionSAH(Node& n, uint32_t* faces, uint32_t numFaces)
{
	
	uint32_t bestAxis = 0;
	uint32_t bestIndex = 0;
	float bestCost = FLT_MAX;

	for (uint32_t a=0; a < 3; ++a)	
	//uint32_t a = bestAxis;
	{
		// sort faces by centroids
		FaceSorter predicate(&m_vertices[0], &m_indices[0], m_numFaces*3, a);
		std::sort(faces, faces+numFaces, predicate);

		// two passes over data to calculate upper and lower bounds
		vector<float> cumulativeLower(numFaces);
		vector<float> cumulativeUpper(numFaces);

		Bounds lower;
		Bounds upper;

		for (uint32_t i=0; i < numFaces; ++i)
		{
			lower.Union(m_faceBounds[faces[i]]);
			upper.Union(m_faceBounds[faces[numFaces-i-1]]);

			cumulativeLower[i] = lower.GetSurfaceArea();        
			cumulativeUpper[numFaces-i-1] = upper.GetSurfaceArea();
		}

		float invTotalSA = 1.0f / cumulativeUpper[0];

		// test all split positions
		for (uint32_t i=0; i < numFaces-1; ++i)
		{
			float pBelow = cumulativeLower[i] * invTotalSA;
			float pAbove = cumulativeUpper[i] * invTotalSA;

			float cost = 0.125f + (pBelow*i + pAbove*(numFaces-i));
			if (cost <= bestCost)
			{
				bestCost = cost;
				bestIndex = i;
				bestAxis = a;
			}
		}
	}

	// re-sort by best axis
	FaceSorter predicate(&m_vertices[0], &m_indices[0], m_numFaces*3, bestAxis);
	std::sort(faces, faces+numFaces, predicate);

	return bestIndex+1;
}

void AABBTree::BuildRecursive(uint32_t nodeIndex, uint32_t* faces, uint32_t numFaces)
{
    const uint32_t kMaxFacesPerLeaf = 6;
    
    // if we've run out of nodes allocate some more
    if (nodeIndex >= m_nodes.size())
    {
		uint32_t s = std::max(uint32_t(1.5f*m_nodes.size()), 512U);

		//cout << "Resizing tree, current size: " << m_nodes.size()*sizeof(Node) << " new size: " << s*sizeof(Node) << endl;

        m_nodes.resize(s);
    }

    // a reference to the current node, need to be careful here as this reference may become invalid if array is resized
	Node& n = m_nodes[nodeIndex];

	// track max tree depth
    ++s_depth;
    m_treeDepth = max(m_treeDepth, s_depth);

	CalculateFaceBounds(faces, numFaces, n.m_minExtents, n.m_maxExtents);

	// calculate bounds of faces and add node  
    if (numFaces <= kMaxFacesPerLeaf)
    {
        n.m_faces = faces;
        n.m_numFaces = numFaces;		

        ++m_leafNodes;
    }
    else
    {
        ++m_innerNodes;        

        // face counts for each branch
        //const uint32_t leftCount = PartitionMedian(n, faces, numFaces);
        const uint32_t leftCount = PartitionSAH(n, faces, numFaces);
        const uint32_t rightCount = numFaces-leftCount;

		// alloc 2 nodes
		m_nodes[nodeIndex].m_children = m_freeNode;

		// allocate two nodes
		m_freeNode += 2;
  
        // split faces in half and build each side recursively
        BuildRecursive(m_nodes[nodeIndex].m_children+0, faces, leftCount);
        BuildRecursive(m_nodes[nodeIndex].m_children+1, faces+leftCount, rightCount);
    }

    --s_depth;
}

struct StackEntry
{
    uint32_t m_node;   
    float m_dist;
};

#define TRACE_STATS 0

bool AABBTree::TraceRay(const Vector3F& start, const Vector3F& dir, float& outT, float& u, float& v, float& w, float& faceSign, uint32_t& faceIndex) const
{   
    //s_traceDepth = 0;

    outT = FLT_MAX;
    TraceRecursive(0, start, dir, outT, u, v, w, faceSign, faceIndex);

    return (outT != FLT_MAX);
}


void AABBTree::TraceRecursive(uint32_t nodeIndex, const Vector3F& start, const Vector3F& dir, float& outT, float& outU, float& outV, float& outW, float& faceSign, uint32_t& faceIndex) const
{
	const Node& node = m_nodes[nodeIndex];

    if (node.m_faces == NULL)
    {
#if _WIN32
        ++s_traceDepth;
#endif
		
#if TRACE_STATS
        extern uint32_t g_nodesChecked;
        ++g_nodesChecked;
#endif

        // find closest node
        const Node& leftChild = m_nodes[node.m_children+0];
        const Node& rightChild = m_nodes[node.m_children+1];

        float dist[2] = {FLT_MAX, FLT_MAX};

        IntersectRayAABB(start, dir, leftChild.m_minExtents, leftChild.m_maxExtents, dist[0], NULL);
        IntersectRayAABB(start, dir, rightChild.m_minExtents, rightChild.m_maxExtents, dist[1], NULL);
        
        uint32_t closest = 0;
        uint32_t furthest = 1;
		
        if (dist[1] < dist[0])
        {
            closest = 1;
            furthest = 0;
        }		

        if (dist[closest] < outT)
            TraceRecursive(node.m_children+closest, start, dir, outT, outU, outV, outW, faceSign, faceIndex);

        if (dist[furthest] < outT)
            TraceRecursive(node.m_children+furthest, start, dir, outT, outU, outV, outW, faceSign, faceIndex);

    }
    else
    {
        float t, u, v, w, s;

        for (uint32_t i=0; i < node.m_numFaces; ++i)
        {
            uint32_t indexStart = node.m_faces[i]*3;

            const Vector3F& a = m_vertices[m_indices[indexStart+0]];
            const Vector3F& b = m_vertices[m_indices[indexStart+1]];
            const Vector3F& c = m_vertices[m_indices[indexStart+2]];

            if (IntersectRayTriTwoSided(start, dir, a, b, c, t, u, v, w, s))
            {
                if (t < outT)
                {
                    outT = t;
					outU = u;
					outV = v;
					outW = w;
					faceSign = s;
					faceIndex = node.m_faces[i];
                }
            }
        }
    }
}

bool AABBTree::TraceRaySlow(const Vector3F& start, const Vector3F& dir, float& outT, float& outU, float& outV, float& outW, float& faceSign, uint32_t& faceIndex) const
{    
    const uint32_t numFaces = GetNumFaces();

    float minT, minU, minV, minW, minS;
	minT = minU = minV = minW = minS = FLT_MAX;

    Vector3F minNormal(0.0f, 1.0f, 0.0f);

    Vector3F n(0.0f, 1.0f, 0.0f);
    float t, u, v, w, s;
    bool hit = false;
	uint32_t minIndex = 0;

    for (uint32_t i=0; i < numFaces; ++i)
    {
        const Vector3F& a = m_vertices[m_indices[i*3+0]];
        const Vector3F& b = m_vertices[m_indices[i*3+1]];
        const Vector3F& c = m_vertices[m_indices[i*3+2]];

        if (IntersectRayTriTwoSided(start, dir, a, b, c, t, u, v, w, s))
        {
            if (t < minT)
            {
                minT = t;
				minU = u;
				minV = v;
				minW = w;
				minS = s;
                minNormal = n;
				minIndex = i;
                hit = true;
            }
        }
    }

    outT = minT;
	outU = minU;
	outV = minV;
	outW = minW;
	faceSign = minS;
	faceIndex = minIndex;

    return hit;
}

void AABBTree::DebugDraw()
{
	/*
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    DebugDrawRecursive(0, 0);

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	*/
}

void AABBTree::DebugDrawRecursive(uint32_t nodeIndex, uint32_t depth)
{
    static uint32_t kMaxDepth = 3;

    if (depth > kMaxDepth)
        return;
}
