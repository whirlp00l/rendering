#ifndef CSE168_BVH_H_INCLUDED
#define CSE168_BVH_H_INCLUDED

#define USE_BVH 1
#define NUM_NODE_CHILDREN 2 // don't change this
#define NUM_LEAF_CHILDREN 1 // this can be varied for best performance

#include "Miro.h"
#include "Object.h"
#include "BoundingVolume.h"

class BVH
{
public:
	BVH();
	virtual ~BVH();

    void build(Objects * objs);

	void renderGL();
    bool intersect(HitInfo& result, const Ray& ray,
                   float tMin = 0.0f, float tMax = MIRO_TMAX) const;

	int numNodes()	{ return m_numNodes; }
	int numLeaves()	{ return m_numLeaves; }

	static void intersectBoundingVolume()	{ BVIntersections++; }
	static void intersectPrimitive()		{ PrimIntersections++; }
	static void resetIntersections()		{ BVIntersections = 0; PrimIntersections = 0; }

	static int BoundingVolumeIntersections()	{ return BVIntersections; }
	static int PrimitiveIntersections()			{ return PrimIntersections; }

protected:
    Objects * m_objects;
	BoundingVolume * m_BVHRoot;
	int m_numNodes;
	int m_numLeaves;

	float computeCost( float parentSurfaceArea, float childSurfaceArea, unsigned int childNumObjs );
	BoundingVolume * buildBVH( Objects * objs );

	typedef struct MidPointMap {
		Vector3 midPoint;
		unsigned int origIndex;
	} MidPointMap;

	typedef struct SplitStats {
		unsigned int lastLeftNodeIndex;
		float leftBVCost;
		float rightBVCost;
	} SplitStats;

	MidPointMap * getTriangleMinMaxAndMidpoints( Objects *objs, Vector3 &min, Vector3 &max, bool setMidPoints );
	SplitStats findBestSplit( Objects * objs, MidPointMap * sortedMidPointMap, int numMidPoints, float parentSurfaceArea );

	// functions to use with qsort
	static int sortByXComponent( const void * p1, const void * p2 );
	static int sortByYComponent( const void * p1, const void * p2 );
	static int sortByZComponent( const void * p1, const void * p2 );

private:
	static int BVIntersections;
	static int PrimIntersections;
};

#endif // CSE168_BVH_H_INCLUDED
