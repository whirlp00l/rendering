#ifndef CSE168_BL_PATCH_H_INCLUDED
#define CSE168_BL_PATCH_H_INCLUDED

#include "Vector3.h"
#include "Object.h"

#include <assert.h>

class BLPatch : public Object
{
public:
    BLPatch( Vector3 ptA, Vector3 ptB, Vector3 ptC, Vector3 ptD );
    virtual ~BLPatch();

    Vector3 & vertex(int i)             { assert( i >= 0 && i < 4 ); return m_verts[i]; }
    const Vector3 & vertex(int i) const { assert( i >= 0 && i < 4 ); return m_verts[i]; }

	void setPointA( const Vector3 ptA ) { m_verts[0] = ptA; }
	void setPointB( const Vector3 ptB ) { m_verts[1] = ptB; }
	void setPointC( const Vector3 ptC ) { m_verts[2] = ptC; }
	void setPointD( const Vector3 ptD ) { m_verts[3] = ptD; }

	const Vector3 & pointA()			{ return vertex(0); }
	const Vector3 & pointB()			{ return vertex(1); }
	const Vector3 & pointC()			{ return vertex(2); }
	const Vector3 & pointD()			{ return vertex(3); }

	Vector3 patchPoint( float u, float v );

    virtual void renderGL();
    virtual bool intersect(HitInfo& result, const Ray& ray,
                           float tMin = 0.0f, float tMax = MIRO_TMAX);

protected:
    Vector3 m_verts[4];

	const Vector3 & uvCoeff();			
	const Vector3 & uCoeff();
	const Vector3 & vCoeff();	
	const Vector3 & constCoeff();		
};

#endif // CSE168_BL_PATCH_H_INCLUDED
