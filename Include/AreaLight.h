#ifndef AREA_LIGHT_H
#define AREA_LIGHT_H

#include "PointLight.h"
#include "Scene.h"

class AreaLight : public PointLight
{
public:
	AreaLight( const Vector3& position, const Vector3& axis1, const Vector3& axis2 );
	~AreaLight();

	float getHitRatio( Vector3 hitPoint, const Scene& scene );
	bool containsPoint( Vector3 point ) const;

	virtual void preCalc(); // we'll use this to construct our light sample points

	static unsigned int NUM_SAMPLES;

protected:
	Vector3 m_axis1;
	Vector3 m_axis2;
	Vector3 * m_samples;
};

#endif // AREA_LIGHT_H