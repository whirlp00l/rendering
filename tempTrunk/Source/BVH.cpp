#include "BVH.h"
#include "BoundingBox.h"
#include "Ray.h"
#include "Triangle.h"
#include "TriangleMesh.h"
#include "Console.h"
#include "DebugMem.h"

#include <assert.h>

int BVH::BVIntersections = 0;
int BVH::PrimIntersections = 0;

BVH::BVH() :
m_numLeaves(0), m_numNodes(0), m_objects(NULL), m_BVHRoot(NULL)
{
}

BVH::~BVH() 
{
}

void
BVH::build(Objects * objs)
{
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

	// did we have any memory leaks?
	_CrtDumpMemoryLeaks();
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
	static std::list<MidPointMap> * midPointMap; // static to save stack space
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
		static SortByXComponent xComponentSorter;
		static SortByYComponent yComponentSorter;
		static SortByZComponent zComponentSorter;
		static SortByCost costSorter;

		// find best splitting option along the x axis
		midPointMap->sort( xComponentSorter );
		bestXSplit = findBestSplit( objs, midPointMap, objs->size(), thisBVSurfaceArea );

		// find best splitting option along the y axis
		midPointMap->sort( yComponentSorter );
		bestYSplit = findBestSplit( objs, midPointMap, objs->size(), thisBVSurfaceArea );

		// find best splitting option along the z axis
		midPointMap->sort( zComponentSorter );
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
			midPointMap->sort( xComponentSorter );
			bestSplitLastLeftNodeIdx = bestXSplit.lastLeftNodeIndex;
		}
		else if( bestYTotalCost <= bestXTotalCost && bestYTotalCost <= bestZTotalCost )
		{
			// use y split; put objects back in y component order
			midPointMap->sort( yComponentSorter );
			bestSplitLastLeftNodeIdx = bestYSplit.lastLeftNodeIndex;
		}
		else
		{
			// use z split; put objects back in z component order
			midPointMap->sort( zComponentSorter );
			bestSplitLastLeftNodeIdx = bestZSplit.lastLeftNodeIndex;
		}

		Objects * leftChildObjs = new Objects();
		Objects * rightChildObjs = new Objects();
		static unsigned int i; // static iterator to save stack space
		i = 0;
		// create the child object vectors
		for( std::list<MidPointMap>::iterator iter = midPointMap->begin(); iter != midPointMap->end(); iter++ )
		{
			// put this triangle in the left child
			if( i <= bestSplitLastLeftNodeIdx )
				leftChildObjs->push_back( (*objs)[(*iter).origIndex] );
			// put this triangle in the right child
			else 
				rightChildObjs->push_back( (*objs)[(*iter).origIndex] );

			i++;
		}

		// we're done with our midpointMap; free the midPointMap memory
		midPointMap->clear();
		delete midPointMap;
		midPointMap = NULL;

		// now recursively build the hierarchy for each node
		Objects * childObjs = new Objects();

		// build the left child
		BoundingVolume * leftBV = buildBVH( leftChildObjs );
		if( leftBV ) // only add it to this bounding volume if it's non-null
			childObjs->push_back( leftBV );

		// build the right child
		BoundingVolume * rightBV = buildBVH( rightChildObjs );
		if( rightBV ) // only add it to this bounding volume if it's non-null
			childObjs->push_back( rightBV );

		// finally, construct this bounding volume and return it
		return new BoundingBox( childObjs, false, min, max );		
	}
}

BVH::SplitStats
BVH::findBestSplit( Objects * objs, std::list<BVH::MidPointMap> * sortedMidPointMap, int numMidPoints, float parentSurfaceArea )
{
	// make these variables static to save stack space
	static std::list<SplitStats> * allSplitStats;
	static SplitStats bestSplit, thisSplit; 
	static int i, j, numTris;
	static Vector3 min, max;
	static Triangle * tri; 		
	static Vector3 triVerts[3]; 
	static TriangleMesh::TupleI3 ti3; 
	static bool minAndMaxSet;
	static std::list<MidPointMap>::iterator midpointMapIter;
	static std::list<SplitStats>::iterator splitStatsIter;
	static SortByCost costSorter;
		
	// we're going to have numMidPoints - 1 possibilities for a split
	allSplitStats = new std::list<SplitStats>;

	minAndMaxSet = false;
	midpointMapIter = sortedMidPointMap->begin();  
	// determine the cost of the left child for each split
	for( i = 0, numTris = 1; i < (numMidPoints - 1); i++, numTris++ )
	{
		tri = ( Triangle * )(*objs)[(*midpointMapIter).origIndex];
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

		thisSplit.lastLeftNodeIndex = i;
		thisSplit.leftBVCost = computeCost( parentSurfaceArea, BoundingBox::calcPotentialSurfaceArea( min, max ), numTris );
		allSplitStats->push_back( thisSplit );

		midpointMapIter++;
	}

	minAndMaxSet = false;
	midpointMapIter = sortedMidPointMap->end(); 
	splitStatsIter = allSplitStats->end();
	
	// determine the cost of the right child for each split
	for( i = ( numMidPoints - 1 ), numTris = 1; i > 0; i--,numTris++ )
	{
		midpointMapIter--;
		splitStatsIter--;

		tri = ( Triangle * )(*objs)[(*midpointMapIter).origIndex];
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

		(*splitStatsIter).rightBVCost = computeCost( parentSurfaceArea, BoundingBox::calcPotentialSurfaceArea( min, max ), numTris );
	}

	// sort all the split stats by cost
	allSplitStats->sort( costSorter );

	// choose the lowest cost (i.e. the first element)
	splitStatsIter = allSplitStats->begin();
	bestSplit.leftBVCost = splitStatsIter->leftBVCost;
	bestSplit.rightBVCost = splitStatsIter->rightBVCost;
	bestSplit.lastLeftNodeIndex = splitStatsIter->lastLeftNodeIndex;

	// free the split stats memory
	allSplitStats->clear();
	delete allSplitStats;
	allSplitStats = NULL;

	// return the best choice
	return bestSplit;
}

std::list<BVH::MidPointMap> *
BVH::getTriangleMinMaxAndMidpoints( Objects *objs, Vector3 &min, Vector3 &max, bool setMidPoints )
{
	// make variable static to save stack space
	static bool minAndMaxSet;
	static MidPointMap thisMap;

	std::list<MidPointMap> * midPointMap = setMidPoints ? new std::list<MidPointMap> : NULL;
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
			thisMap.midPoint = tri->getMidPoint();
			thisMap.origIndex = i;
			midPointMap->push_back( thisMap );
		}
	}

	return midPointMap;
}