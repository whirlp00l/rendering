#ifndef SPECULAR_REFLECTOR_H
#define SPECULAR_REFLECTOR_H

#include "Lambert.h"

class SpecularReflector : public Lambert
{
public:
	SpecularReflector( const Vector3 & kd = Vector3(1) );
	~SpecularReflector();

	virtual void preCalc() {}
    
    virtual Vector3 shade(const Ray& ray, const HitInfo& hit,
                          const Scene& scene) const;

protected:
	Vector3 getReflectedColor( const Ray& ray, const HitInfo& hit, const Scene& scene ) const;
	static const int SPECULAR_RECURSION_DEPTH;
};

#endif; // SPECULAR_REFLECTOR_H