#ifndef AREA_LIGHT_H
#define AREA_LIGHT_H

#include "PointLight.h"
#include "Scene.h"

class AreaLight : public PointLight
{
public:
	AreaLight( const Vector3& min, const Vector3& max );
	~AreaLight();

	virtual void setPosition(const Vector3& v);
	float getHitRatio( Vector3 hitPoint, const Scene& scene );
	virtual void preCalc(); // we'll use this to construct our light sample points

	static unsigned int NUM_SAMPLES;

protected:
	Vector3 m_min;
	Vector3 m_max;
	Vector3 * m_samples;
};

#endif // AREA_LIGHT_H