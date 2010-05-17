#ifndef SPECULAR_REFRACTOR_H
#define SPECULAR_REFRACTOR_H

#include "Lambert.h"

class SpecularRefractor : public Lambert
{
public:
	SpecularRefractor( const float & refractiveIndex, const Vector3 & kd );
	~SpecularRefractor();

	virtual void preCalc() {}
    
    virtual Vector3 shade(const Ray& ray, const HitInfo& hit,
                          const Scene& scene) const;

	enum RefractiveMaterial
	{
		WATER_100_C,
		WATER_0_C,
		WATER_20_C,
		DIAMOND,
		MILK,
		ICE,
		GLASS_COMMON,
		GLASS_PYREX
	};

	static float getRefractiveIndex( RefractiveMaterial material );
};

#endif // SPECULAR_REFRACTOR_H