#ifndef SPECULAR_REFLECTOR_H
#define SPECULAR_REFLECTOR_H

#include "Material.h"

class SpecularReflector : public Material
{
public:
	SpecularReflector( const Vector3 & kd = Vector3(1) );
	~SpecularReflector();

	const Vector3 & kd() const {return m_kd;}

	void setKd(const Vector3 & kd) {m_kd = kd;}

	virtual void preCalc() {}
    
    virtual Vector3 shade(const Ray& ray, const HitInfo& hit,
                          const Scene& scene) const;

	static const int RECURSION_DEPTH; // upper limit for number of reflective bounces to trace

protected:	
	Vector3 m_kd;
};

#endif; // SPECULAR_REFLECTOR_H