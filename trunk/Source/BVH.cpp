#include "BVH.h"
#include "BoundingBox.h"
#include "Ray.h"
#include "Triangle.h"
#include "TriangleMesh.h"
#include "Console.h"
#include "DebugMem.h"

#include <assert.h>
#include <time.h>

int BVH::BVIntersections = 0;
int BVH::PrimIntersections = 0;

BVH::BVH() :
m_numLeaves(0), m_numNodes(0), m_objects(NULL), m_BVHRoot(NULL)
{
}

BVH::~BVH() 
{
	delete m_BVHRoot;
	m_BVHRoot = NULL;
}

void
BVH::build(Objects * objs)
{
	clock_t clockStart = clock();

	if( USE_BVH )
	{
		// don't build anything if the scene is empty
		if( !objs->empty() )
		{
			// construct the bounding volume hierarchy
			m_BVHRoot = buildBVH( objs );
			m_BVHRoot->calcNumNodesAndLeaves( &m_numNodes, &m_numLeaves );
		}
	}
	else
	{
		// just use a vector of objects
		m_objects = objs;
	}

	clock_t clockEnd = clock();

	printf("\nTotal build time: %.4f seconds\n\n", ((float)(clockEnd - clockStart))/1000.0f);
}

void
BVH::renderGL()
{
	if( m_BVHRoot )
		m_BVHRoot->renderGL();
}

bool
BVH::intersect(HitInfo& minHit, const Ray& ray, float tMin, float tMax) const
{
	if( USE_BVH )
	{
		// traverse the BVH to perform ray-intersection
		if( m_BVHRoot )
			return m_BVHRoot->intersect( minHit, ray, tMin, tMax );
		else
			return false;
	}
	else
	{
		// Just intersect every object.
		bool hit = false;
		HitInfo tempMinHit;
		minHit.t = MIRO_TMAX;
	    
		for (size_t i = 0; i < m_objects->size(); ++i)
		{
			if ((*m_objects)[i]->intersect(tempMinHit, ray, tMin, tMax))
			{
				hit = true;
				if (tempMinHit.t < minHit.t)
					minHit = tempMinHit;
			}
		}
	    
		return hit;
	}
}

float
BVH::computeCost( float parentSurfaceArea, float childSurfaceArea, unsigned int childNumObjs )
{
	static float multiplier; // static to save stack space
	multiplier = 1 / parentSurfaceArea;
	return childSurfaceArea * multiplier * childNumObjs;
}

BoundingVolume *
BVH::buildBVH( Objects * objs )
{
	if( objs->empty() )
		return NULL;

	// keep track of the triangles' midpoints now so we don't have to iterate through them again
	static MidPointMap * midPointMap; // static to save stack space
	Vector3 min, max; // can't be static; must be preserved through recursion

	// base case: we've reached the desired number of primitives!
	if( objs->size() <= NUM_LEAF_CHILDREN )
	{
		// we don't need the midpoints if we know this is a leaf node
		midPointMap = getTriangleMinMaxAndMidpoints( objs, min, max, false );
		// create a leaf node containing the primitives
		return new BoundingBox( objs, true, min, max );
	}	
	// recursive case: we need to subdivide this bounding box
	else
	{
		// we need to calculate the midpoints in this case; we'll free the memory later
		midPointMap = getTriangleMinMaxAndMidpoints( objs, min, max, true );

		static float thisBVSurfaceArea;
		thisBVSurfaceArea = BoundingBox::calcPotentialSurfaceArea( min, max );

		static SplitStats bestXSplit, bestYSplit, bestZSplit;

		// find best splitting option along the x axis
		qsort( midPointMap, objs->size(), sizeof( MidPointMap ), BVH::sortByXComponent );
		bestXSplit = findBestSplit( objs, midPointMap, objs->size(), thisBVSurfaceArea );

		// find best splitting option along the y axis
		qsort( midPointMap, objs->size(), sizeof( MidPointMap ), BVH::sortByYComponent );
		bestYSplit = findBestSplit( objs, midPointMap, objs->size(), thisBVSurfaceArea );

		// find best splitting option along the z axis
		qsort( midPointMap, objs->size(), sizeof( MidPointMap ), BVH::sortByZComponent );
		bestZSplit = findBestSplit( objs, midPointMap, objs->size(), thisBVSurfaceArea );
	
		static float bestXTotalCost, bestYTotalCost, bestZTotalCost;
		static unsigned int bestSplitLastLeftNodeIdx;
		bestXTotalCost = bestXSplit.leftBVCost + bestXSplit.rightBVCost;
		bestYTotalCost = bestYSplit.leftBVCost + bestYSplit.rightBVCost;
		bestZTotalCost = bestZSplit.leftBVCost + bestZSplit.rightBVCost;

		// use the lowest cost split for our final choice
		if( bestXTotalCost <= bestYTotalCost && bestXTotalCost <= bestZTotalCost )
		{
			// use x split; put objects back in x component order
			qsort( midPointMap, objs->size(), sizeof( MidPointMap ), BVH::sortByXComponent );
			bestSplitLastLeftNodeIdx = bestXSplit.lastLeftNodeIndex;
		}
		else if( bestYTotalCost <= bestXTotalCost && bestYTotalCost <= bestZTotalCost )
		{
			// use y split; put objects back in y component order
			qsort( midPointMap, objs->size(), sizeof( MidPointMap ), BVH::sortByYComponent );
			bestSplitLastLeftNodeIdx = bestYSplit.lastLeftNodeIndex;
		}
		else
		{
			// use z split; put objects back in z component order
			qsort( midPointMap, objs->size(), sizeof( MidPointMap ), BVH::sortByZComponent );
			bestSplitLastLeftNodeIdx = bestZSplit.lastLeftNodeIndex;
		}

		Objects * leftChildObjs = new Objects();
		Objects * rightChildObjs = new Objects();
		static unsigned int i; // static iterator to save stack space
		// create the child object vectors
		for( i = 0; i < objs->size(); i++ )
		{
			// put this triangle in the left child
			if( i <= bestSplitLastLeftNodeIdx )
				leftChildObjs->push_back( (*objs)[midPointMap[i].origIndex] );
			// put this triangle in the right child
			else 
				rightChildObjs->push_back( (*objs)[midPointMap[i].origIndex] );
		}

		// we're done with our midpointMap; free the midPointMap memory
		delete [] midPointMap;
		midPointMap = NULL;

		// now recursively build the hierarchy for each node
		Objects * childObjs = new Objects();

		// build the left child
		BoundingVolume * leftBV = buildBVH( leftChildObjs );
		// now we're done with our local leftChildObjs; delete it
        delete leftChildObjs;
        leftChildObjs = NULL;
        if( leftBV ) // only add it to this bounding volume if it's non-null
                childObjs->push_back( leftBV );

		// build the right child
		BoundingVolume * rightBV = buildBVH( rightChildObjs );
		// now we're done with our local rightChildObjs; delete it
        delete rightChildObjs;
        rightChildObjs = NULL;
        if( rightBV ) // only add it to this bounding volume if it's non-null
                childObjs->push_back( rightBV );

		// finally, construct this bounding volume and return it
        static BoundingVolume * bv;
        bv = new BoundingBox( childObjs, false, min, max );             
        // now we're done with childObjs; delete it
        delete childObjs;
        childObjs = NULL;

        return bv;
	}
}

BVH::SplitStats
BVH::findBestSplit( Objects * objs, BVH::MidPointMap * sortedMidPointMap, int numMidPoints, float parentSurfaceArea )
{
	// make these variables static to save stack space
	static SplitStats * allSplitStats;
	static SplitStats bestSplit; 
	static int i, j, numTris;
	static Vector3 min, max;
	static Triangle * tri; 		
	static Vector3 triVerts[3]; 
	static TriangleMesh::TupleI3 ti3; 
	static bool minAndMaxSet;
		
	// we're going to have numMidPoints - 1 possibilities for a split
	allSplitStats = new SplitStats[numMidPoints - 1];

	minAndMaxSet = false;
	// determine the cost of the left child for each split
	for( i = 0, numTris = 1; i < (numMidPoints - 1); i++, numTris++ )
	{
		tri = ( Triangle * )(*objs)[sortedMidPointMap[i].origIndex];
		ti3 = tri->getMesh()->vIndices()[tri->getIndex()];
		triVerts[0] = tri->getMesh()->vertices()[ti3.x]; //vertex a of triangle
		triVerts[1] = tri->getMesh()->vertices()[ti3.y]; //vertex b of triangle
		triVerts[2] = tri->getMesh()->vertices()[ti3.z]; //vertex c of triangle
		for( j = 0; j < 3; j++ )
		{
			if( !minAndMaxSet || triVerts[j].x < min.x )
				min.x = triVerts[j].x;
			if( !minAndMaxSet || triVerts[j].y < min.y )
				min.y = triVerts[j].y;
			if( !minAndMaxSet || triVerts[j].z < min.z )
				min.z = triVerts[j].z;
			if( !minAndMaxSet || triVerts[j].x > max.x )
				max.x = triVerts[j].x;
			if( !minAndMaxSet || triVerts[j].y > max.y )
				max.y = triVerts[j].y;
			if( !minAndMaxSet || triVerts[j].z > max.z )
				max.z = triVerts[j].z;

			minAndMaxSet = true;
		}

		allSplitStats[i].lastLeftNodeIndex = i;
		allSplitStats[i].leftBVCost = computeCost( parentSurfaceArea, BoundingBox::calcPotentialSurfaceArea( min, max ), numTris );
	}

	minAndMaxSet = false;
	// determine the cost of the right child for each split
	for( i = ( numMidPoints - 1 ), numTris = 1; i > 0; i--,numTris++ )
	{
		tri = ( Triangle * )(*objs)[sortedMidPointMap[i].origIndex];
		ti3 = tri->getMesh()->vIndices()[tri->getIndex()];
		triVerts[0] = tri->getMesh()->vertices()[ti3.x]; //vertex a of triangle
		triVerts[1] = tri->getMesh()->vertices()[ti3.y]; //vertex b of triangle
		triVerts[2] = tri->getMesh()->vertices()[ti3.z]; //vertex c of triangle
		for( j = 0; j < 3; j++ )
		{
			if( !minAndMaxSet || triVerts[j].x < min.x )
				min.x = triVerts[j].x;
			if( !minAndMaxSet || triVerts[j].y < min.y )
				min.y = triVerts[j].y;
			if( !minAndMaxSet || triVerts[j].z < min.z )
				min.z = triVerts[j].z;
			if( !minAndMaxSet || triVerts[j].x > max.x )
				max.x = triVerts[j].x;
			if( !minAndMaxSet || triVerts[j].y > max.y )
				max.y = triVerts[j].y;
			if( !minAndMaxSet || triVerts[j].z > max.z )
				max.z = triVerts[j].z;

			minAndMaxSet = true;
		}

		allSplitStats[i-1].rightBVCost = computeCost( parentSurfaceArea, BoundingBox::calcPotentialSurfaceArea( min, max ), numTris );
	}

	// sort all the split stats by cost
	qsort( allSplitStats, numMidPoints - 1, sizeof( SplitStats ), BVH::sortByCost );

	// choose the lowest cost (i.e. the first element)
	bestSplit.leftBVCost = allSplitStats[0].leftBVCost;
	bestSplit.rightBVCost = allSplitStats[0].rightBVCost;
	bestSplit.lastLeftNodeIndex = allSplitStats[0].lastLeftNodeIndex;

	// free the split stats memory
	delete [] allSplitStats;
	allSplitStats = NULL;

	// return the best choice
	return bestSplit;
}

BVH::MidPointMap *
BVH::getTriangleMinMaxAndMidpoints( Objects *objs, Vector3 &min, Vector3 &max, bool setMidPoints )
{
	MidPointMap * midPointMap = setMidPoints ? new MidPointMap[objs->size()] : NULL;

	// make variable static to save stack space
	static bool minAndMaxSet;
	minAndMaxSet = false;

	static size_t i, j; // static to save stack space
	for( i = 0; i < objs->size(); i++ )
	{
		static Triangle * tri; // static to save stack space		
		static Vector3 triVerts[3]; // static to save stack space
		static TriangleMesh::TupleI3 ti3; // static to save stack space

		tri = ( Triangle * )(*objs)[i];
		ti3 = tri->getMesh()->vIndices()[tri->getIndex()];
		triVerts[0] = tri->getMesh()->vertices()[ti3.x]; //vertex a of triangle
		triVerts[1] = tri->getMesh()->vertices()[ti3.y]; //vertex b of triangle
		triVerts[2] = tri->getMesh()->vertices()[ti3.z]; //vertex c of triangle
		for( j = 0; j < 3; j++ )
		{
			if( !minAndMaxSet || triVerts[j].x < min.x )
				min.x = triVerts[j].x;
			if( !minAndMaxSet || triVerts[j].y < min.y )
				min.y = triVerts[j].y;
			if( !minAndMaxSet || triVerts[j].z < min.z )
				min.z = triVerts[j].z;
			if( !minAndMaxSet || triVerts[j].x > max.x )
				max.x = triVerts[j].x;
			if( !minAndMaxSet || triVerts[j].y > max.y )
				max.y = triVerts[j].y;
			if( !minAndMaxSet || triVerts[j].z > max.z )
				max.z = triVerts[j].z;

			minAndMaxSet = true;
		}

		// add this triangle's midpoint to the midpoint vector
		if( setMidPoints )
		{
			midPointMap[i].midPoint = tri->getMidPoint();
			midPointMap[i].origIndex = i;
		}
	}

	return midPointMap;
}

int
BVH::sortByXComponent( const void * p1, const void * p2 )
{
	MidPointMap * midPointMap1 = ( MidPointMap * )p1;
	MidPointMap * midPointMap2 = ( MidPointMap * )p2;

	// 1st x value is smaller
	if( midPointMap1->midPoint.x < midPointMap2->midPoint.x )
		return -1;
	// 2nd x value is smaller
	else if( midPointMap1->midPoint.x > midPointMap2->midPoint.x )
		return 1;
	// the x values are equal
	else
		return 0;
}

int
BVH::sortByYComponent( const void * p1, const void * p2 )
{
	MidPointMap * midPointMap1 = ( MidPointMap * )p1;
	MidPointMap * midPointMap2 = ( MidPointMap * )p2;

	// 1st y value is smaller
	if( midPointMap1->midPoint.y < midPointMap2->midPoint.y )
		return -1;
	// 2nd y value is smaller
	else if( midPointMap1->midPoint.y > midPointMap2->midPoint.y )
		return 1;
	// the y values are equal
	else
		return 0;
}

int
BVH::sortByZComponent( const void * p1, const void * p2 )
{
	MidPointMap * midPointMap1 = ( MidPointMap * )p1;
	MidPointMap * midPointMap2 = ( MidPointMap * )p2;

	// 1st z value is smaller
	if( midPointMap1->midPoint.z < midPointMap2->midPoint.z )
		return -1;
	// 2nd z value is smaller
	else if( midPointMap1->midPoint.z > midPointMap2->midPoint.z )
		return 1;
	// the z values are equal
	else
		return 0;
}

int
BVH::sortByCost( const void * p1, const void * p2 )
{
	static float totalCostSplit1, totalCostSplit2; // static to save stack space
	SplitStats * splitStats1 = ( SplitStats * )p1;
	SplitStats * splitStats2 = ( SplitStats * )p2;

	totalCostSplit1 = splitStats1->leftBVCost + splitStats1->rightBVCost;
	totalCostSplit2 = splitStats2->leftBVCost + splitStats2->rightBVCost;

	// 1st cost is smaller
	if( totalCostSplit1 < totalCostSplit2 )
		return -1;
	// 2nd cost is smaller
	else if( totalCostSplit1 > totalCostSplit2 )
		return 1;
	// the costs are equal
	else
		return 0;
}

