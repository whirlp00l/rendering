#ifndef SPECULAR_H
#define SPECULAR_H

#include "Material.h"

class Specular : public Material
{
public:
	Specular( const Vector3 & kd = Vector3(1) );
	~Specular();

	const Vector3 & kd() const {return m_kd;}

	void setKd(const Vector3 & kd) {m_kd = kd;}

	virtual void preCalc() {}
    
    virtual Vector3 shade(const Ray& ray, const HitInfo& hit,
                          const Scene& scene) const;

	static const int RECURSION_DEPTH; // upper limit for number of reflective bounces to trace

protected:	
	Vector3 m_kd;
};

#endif;