#include "BoundingBox.h"
#include "BVH.h"
#include "DebugMem.h"
#include <assert.h>

BoundingBox::BoundingBox( Objects * objects, bool isLeaf, Vector3 vecMin, Vector3 vecMax ) :
BoundingVolume(objects,isLeaf), m_vMin(vecMin), m_vMax(vecMax)
{
}

BoundingBox::~BoundingBox()
{
}

bool
BoundingBox::intersect( HitInfo& result, const Ray& ray, float tMin, float tMax )
{
	BoundingBox::IntersectingPlane plane;

	// pre-compute denominators so we don't have to perform expensive divisions
	float oneOverDX = 1/ray.d.x;
	float oneOverDY = 1/ray.d.y;

	Vector3 tIntersectMin, tIntersectMax;

	if (ray.d.x >= 0) 
	{
		tIntersectMin.x = (m_vMin.x - ray.o.x) * oneOverDX;
		tIntersectMax.x = (m_vMax.x - ray.o.x) * oneOverDX;
	}
	else {
		tIntersectMin.x = (m_vMax.x - ray.o.x) * oneOverDX;
		tIntersectMax.x = (m_vMin.x - ray.o.x) * oneOverDX;
	}
	
	if (ray.d.y >= 0) 
	{
		tIntersectMin.y = (m_vMin.y - ray.o.y) * oneOverDY;
		tIntersectMax.y = (m_vMax.y - ray.o.y) * oneOverDY;
	}
	else {
		tIntersectMin.y = (m_vMax.y - ray.o.y) * oneOverDY;
		tIntersectMax.y = (m_vMin.y - ray.o.y) * oneOverDY;
	}
	
	// x and y values for t don't overlap, so there's no intersection
	if ( (tIntersectMin.x > tIntersectMax.y) || (tIntersectMin.y > tIntersectMax.x) )
		return false;

	// use the larger of the x/y min values and the smaller of the x/y max values for general min/max
	float min, max;
	min = (tIntersectMin.x > tIntersectMin.y) ? tIntersectMin.x : tIntersectMin.y;
	max = (tIntersectMax.x < tIntersectMax.y) ? tIntersectMax.x : tIntersectMax.y;

	// pre-compute denominator so we don't have to perform expensive divisions
	float oneOverDZ = 1/ray.d.z;

	if (ray.d.z >= 0) 
	{
		tIntersectMin.z = (m_vMin.z - ray.o.z) * oneOverDZ;
		tIntersectMax.z = (m_vMax.z - ray.o.z) * oneOverDZ;
	}
	else 
	{
		tIntersectMin.z = (m_vMax.z - ray.o.z) * oneOverDZ;
		tIntersectMax.z = (m_vMin.z - ray.o.z) * oneOverDZ;
	}

	// z min/max values don't overlap with those for x and y, so no intersection
	if ( (min > tIntersectMax.z) || (tIntersectMin.z > max) )
		return false;

	// we want the largest min and the smallest max of all three components
	if (tIntersectMin.z > min)
		min = tIntersectMin.z;
	if (tIntersectMax.z < max)
		max = tIntersectMax.z;

	// calculated min and max are out of the appropriate range; not an intersection
	if( !( (min < tMax) && (max > tMin) ) )
		return false;

	// if we've reached this point, we have an intersection!
	// use first intersection point (min) for computing the actual intersection
	result.t = min;
	result.P = ray.o + (result.t * ray.d);

	// figure out which plane we intersected
	if( result.P.x > ( m_vMin.x - epsilon ) && result.P.x < ( m_vMin.x + epsilon ) )
		plane = XMIN_PLANE;
	else if( result.P.x > ( m_vMax.x - epsilon ) && result.P.x < ( m_vMax.x + epsilon ) )
		plane = XMAX_PLANE;
	else if( result.P.y > ( m_vMin.y - epsilon ) && result.P.y < ( m_vMin.y + epsilon ) )
		plane = YMIN_PLANE;
	else if( result.P.y > ( m_vMax.y - epsilon ) && result.P.y < ( m_vMax.y + epsilon ) )
		plane = YMAX_PLANE;
	else if( result.P.z > ( m_vMin.z - epsilon ) && result.P.z < ( m_vMin.z + epsilon ) )
		plane = ZMIN_PLANE;
	else
		plane = ZMAX_PLANE;

	result.N = getPlaneNormal( plane );
	result.material = this->m_material;

	BVH::intersectBoundingVolume();

	bool intersectionFound = false;
	// the bounding volume was hit; we must check intersections with all of its children
	for( size_t i = 0; i < m_children.size(); i++ )
	{
		HitInfo childResult;
		if( m_children[i]->intersect( childResult, ray, tMin, tMax ) )
		{
			if( !intersectionFound || childResult.t < result.t )
			{
				result.t = childResult.t;
				result.P = childResult.P;
				result.N = childResult.N;
				result.material = childResult.material;
				intersectionFound = true;
			}
		}
	}

	// if we wanted to render the largest bounding box, we'd just return true :)
	return intersectionFound;
}

Vector3
BoundingBox::getPlaneNormal( BoundingBox::IntersectingPlane plane )
{
	Vector3 planeNormal;

	switch( plane )
	{
	case XMIN_PLANE:
		planeNormal = Vector3( -1, 0, 0 );
		break;
	case XMAX_PLANE:
		planeNormal = Vector3( 1, 0, 0 );
		break;
	case YMIN_PLANE:
		planeNormal = Vector3( 0, -1, 0 );
		break;
	case YMAX_PLANE:
		planeNormal = Vector3( 0, 1, 0 );
		break;
	case ZMIN_PLANE:
		planeNormal = Vector3( 0, 0, -1 );
		break;
	case ZMAX_PLANE:
		planeNormal = Vector3( 0, 0, 1 );
		break;
	}

	return planeNormal;
}

void
BoundingBox::renderGL()
{
	if( VIEW_BOUNDING_VOLUMES )
	{
		Vector3 ptA( m_vMin );
		Vector3 ptB( m_vMin.x, m_vMax.y, m_vMin.z );
		Vector3 ptC( m_vMax.x, m_vMax.y, m_vMin.z );
		Vector3 ptD( m_vMax.x, m_vMin.y, m_vMin.z );
		Vector3 ptE( m_vMax.x, m_vMin.y, m_vMax.z );
		Vector3 ptF( m_vMax );
		Vector3 ptG( m_vMin.x, m_vMax.y, m_vMax.z );
		Vector3 ptH( m_vMin.x, m_vMin.y, m_vMax.z );

		glBegin(GL_QUADS); // order: top left pt, top right pt, bottom right pt, bottom left pt
			// front face
			glVertex3f( ptG.x, ptG.y, ptG.z );
			glVertex3f( ptF.x, ptF.y, ptF.z );
			glVertex3f( ptE.x, ptE.y, ptE.z );
			glVertex3f( ptH.x, ptH.y, ptH.z );
			// back face
			glVertex3f( ptC.x, ptC.y, ptC.z );
			glVertex3f( ptB.x, ptB.y, ptB.z );
			glVertex3f( ptA.x, ptA.y, ptA.z );
			glVertex3f( ptD.x, ptD.y, ptD.z );
			// right face
			glVertex3f( ptF.x, ptF.y, ptF.z );
			glVertex3f( ptC.x, ptC.y, ptC.z );
			glVertex3f( ptD.x, ptD.y, ptD.z );
			glVertex3f( ptE.x, ptE.y, ptE.z );
			// left face
			glVertex3f( ptB.x, ptB.y, ptB.z );
			glVertex3f( ptG.x, ptG.y, ptG.z );
			glVertex3f( ptH.x, ptH.y, ptH.z );
			glVertex3f( ptA.x, ptA.y, ptA.z );
			// bottom face
			glVertex3f( ptH.x, ptH.y, ptH.z );
			glVertex3f( ptE.x, ptE.y, ptE.z );
			glVertex3f( ptD.x, ptD.y, ptD.z );
			glVertex3f( ptA.x, ptA.y, ptA.z );
			// top face
			glVertex3f( ptB.x, ptB.y, ptB.z );
			glVertex3f( ptC.x, ptC.y, ptC.z );
			glVertex3f( ptF.x, ptF.y, ptF.z );
			glVertex3f( ptG.x, ptG.y, ptG.z );
		glEnd();
	}

	for( size_t i = 0; i < m_children.size(); i++ )
	{
		m_children[i]->renderGL();
	}
}

float
BoundingBox::calcPotentialSurfaceArea( Vector3 min, Vector3 max )
{
	assert( max.x >= min.x );
	assert( max.y >= min.y );
	assert( max.z >= min.z );

	float xSpan = max.x - min.x;
	float ySpan = max.y - min.y;
	float zSpan = max.z - min.z;

	return 2*xSpan*ySpan + 2*xSpan*zSpan + 2*ySpan*zSpan;
}