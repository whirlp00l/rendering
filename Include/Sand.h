#ifndef SAND_H
#define SAND_H

#include "Lambert.h"
#include "CustomizablePerlinNoise.h"

class Sand : public Lambert 
{
public:
	Sand( float noiseMultiplier = 1.0f, const Vector3 & kd = getStandardSandColor() );
	~Sand();

	Vector3 shade(const Ray& ray, const HitInfo& hit, const Scene& scene) const;

	static Vector3 getStandardSandColor() { return Vector3( 255, 204, 153 ) / 255; }

protected:
	float mNoiseMultiplier;
	CustomizablePerlinNoise * mNoiseMaker;
};

#endif // SAND_H