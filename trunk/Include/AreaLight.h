#ifndef AREA_LIGHT_H
#define AREA_LIGHT_H

#include "PointLight.h"
#include "Scene.h"

class AreaLight : public PointLight
{
public:
	AreaLight( const Vector3& min, const Vector3& max, unsigned int numSamples );
	~AreaLight();

	virtual void setPosition(const Vector3& v);
	float getHitRatio( Vector3 hitPoint, const Scene& scene );
	virtual void preCalc(); // we'll use this to construct our light sample points

protected:
	Vector3 m_min;
	Vector3 m_max;
	unsigned int m_num_samples;
	Vector3 * m_samples;
};

#endif // AREA_LIGHT_H