#ifndef SPECULAR_H
#define SPECULAR_H

#include "Material.h"

class Specular : public Material
{
public:
	Specular( const Vector3 & ks = Vector3(1) );
	~Specular();

	const Vector3 & ks() const {return m_ks;}

    void setKd(const Vector3 & ks) {m_ks = ks;}

	virtual void preCalc() {}
    
    virtual Vector3 shade(const Ray& ray, const HitInfo& hit,
                          const Scene& scene) const;

protected:
	Vector3 m_ks;
};

#endif;