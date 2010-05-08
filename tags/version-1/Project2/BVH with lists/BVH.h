#ifndef CSE168_BVH_H_INCLUDED
#define CSE168_BVH_H_INCLUDED

#define USE_BVH 1
#define NUM_NODE_CHILDREN 2 // don't change this
#define NUM_LEAF_CHILDREN 1 // this can be varied for best performance

#include "Miro.h"
#include "Object.h"
#include "BoundingVolume.h"
#include <list>

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

	std::list<MidPointMap> * getTriangleMinMaxAndMidpoints( Objects *objs, Vector3 &min, Vector3 &max, bool setMidPoints );
	SplitStats findBestSplit( Objects * objs, std::list<MidPointMap> * sortedMidPointMap, int numMidPoints, float parentSurfaceArea );

	// the following 4 structs are used for sorting our lists 
	typedef struct SortByXComponent {
		bool operator()( MidPointMap midPointMap1, MidPointMap midPointMap2 )
		{
			return midPointMap1.midPoint.x < midPointMap2.midPoint.x;
		}
	} SortByXComponent;

	typedef struct SortByYComponent {
		bool operator()( MidPointMap midPointMap1, MidPointMap midPointMap2 )
		{
			return midPointMap1.midPoint.y < midPointMap2.midPoint.y;
		}
	} SortByYComponent;

	typedef struct SortByZComponent {
		bool operator()( MidPointMap midPointMap1, MidPointMap midPointMap2 )
		{
			return midPointMap1.midPoint.z < midPointMap2.midPoint.z;
		}
	} SortByZComponent;

	typedef struct SortByCost {
		bool operator()( SplitStats splitStats1, SplitStats splitStats2 )
		{
			static float totalCostSplit1, totalCostSplit2; // static to save stack space

			totalCostSplit1 = splitStats1.leftBVCost + splitStats1.rightBVCost;
			totalCostSplit2 = splitStats2.leftBVCost + splitStats2.rightBVCost;

			return totalCostSplit1 < totalCostSplit2;
		}
	} SortByCost;

private:
	static int BVIntersections;
	static int PrimIntersections;
};

#endif // CSE168_BVH_H_INCLUDED
