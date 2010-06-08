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
	
	Vector3 getReflectedDir( const Ray& ray, const HitInfo& hit ) const;

	static const int SPECULAR_RECURSION_DEPTH;

protected:
	Vector3 getReflectedColor( const Ray& ray, const HitInfo& hit, const Scene& scene ) const;
};

#endif; // SPECULAR_REFLECTOR_H