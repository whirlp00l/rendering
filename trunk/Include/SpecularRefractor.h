#ifndef SPECULAR_REFRACTOR_H
#define SPECULAR_REFRACTOR_H

#include "SpecularReflector.h"

class SpecularRefractor : public SpecularReflector
{
public:
	SpecularRefractor( const float & refractiveIndex, const Vector3 & kd = Vector3(1), const float & density = 1.0f );
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

	// returns true if refraction is allowed and false otherwise. 
	// if refraction is allowed, it also calculates the refraction direction.
	bool getRefractedRay( Ray& refractedRay, float& reflectivity, const Ray& ray, const HitInfo& hit, const Scene& scene ) const;

	static float getRefractiveIndex( RefractiveMaterial material );

protected:
	float m_density; // in g/cm^3
};

#endif // SPECULAR_REFRACTOR_H