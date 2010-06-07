#ifndef SPECULAR_REFRACTOR_H
#define SPECULAR_REFRACTOR_H

#include "SpecularReflector.h"

class SpecularRefractor : public SpecularReflector
{
public:
	SpecularRefractor( const float & refractiveIndex, const Vector3 & kd = Vector3(1), 
		               const float & density = 1.0f, const float & percentRefraction = 1.0f );
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

protected:
	// returns true if refraction is allowed and false otherwise. 
	// if refraction is allowed, it also calculates the refraction direction.
	bool getRefractedRay( Ray& refractedRay, const Ray& ray, const HitInfo& hit, const Scene& scene ) const;

	float m_density; // in g/cm^3
	float m_percentRefraction; // in range [0,1]; specifies how much light should be refracted vs. reflected
};

#endif // SPECULAR_REFRACTOR_H