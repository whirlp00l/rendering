#ifndef CSE168_BOUNDING_BOX_H_INCLUDED
#define CSE168_BOUNDING_BOX_H_INCLUDED

#include "BoundingVolume.h"

class BoundingBox : public BoundingVolume
{
public:
	BoundingBox( Objects * objects, bool isLeaf, Vector3 vecMin, Vector3 vecMax );
	virtual ~BoundingBox();

	enum IntersectingPlane
	{
		XMIN_PLANE,
		XMAX_PLANE,
		YMIN_PLANE,
		YMAX_PLANE,
		ZMIN_PLANE,
		ZMAX_PLANE
	};

	virtual void renderGL();
	virtual bool intersect( HitInfo& result, const Ray& ray, float tMin = 0.0f, float tMax = MIRO_TMAX );

	static float calcPotentialSurfaceArea( Vector3 min, Vector3 max );

protected:
	Vector3 m_vMin, m_vMax;
	Vector3 getPlaneNormal( BoundingBox::IntersectingPlane plane );
};

#endif // CSE168_BOUNDING_BOX_H_INCLUDED