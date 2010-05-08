#ifndef CSE168_BOUNDING_VOLUME_H_INCLUDED
#define CSE168_BOUNDING_VOLUME_H_INCLUDED

#define VIEW_BOUNDING_VOLUMES 0

#include "Object.h"
#include "Ray.h"

class BoundingVolume : public Object
{
public: 
	BoundingVolume( Objects * objects, bool isLeaf );
	virtual ~BoundingVolume();

	virtual void renderGL() {}
	virtual bool intersect( HitInfo& result, const Ray& ray, float tMin = 0.0f, float tMax = MIRO_TMAX ) = 0;

	const Objects * getChildren() const	{ return &m_children; }
	bool isLeaf() const					{ return m_bIsLeaf; }

	void addChild( Object * child );
	void calcNumNodesAndLeaves( int * numNodesPtr, int * numLeavesPtr );

protected:
	bool m_bIsLeaf;
	Objects m_children; // if it's a leaf, children will be primitives. Otherwise they'll be bounding volumes.
};

#endif // CSE168_BOUNDING_VOLUME_H_INCLUDED