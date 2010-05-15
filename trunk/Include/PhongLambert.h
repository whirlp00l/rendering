#ifndef CSE168_PHONG_LAMBERT_H_INCLUDED
#define CSE168_PHONG_LAMBERT_H_INCLUDED

#include "Lambert.h"

class PhongLambert : public Lambert
{
public:
    PhongLambert(const Vector3 & kd = Vector3(1),
			     const Vector3 & ka = Vector3(0),
			     const float & phongExp = 50.0f);
    virtual ~PhongLambert();

	const float & phongExp() const {return m_phong_exp;}

	void setPhongExp(const float & phongExp) {m_phong_exp = phongExp;}

    virtual void preCalc() {}
    
    virtual Vector3 shade(const Ray& ray, const HitInfo& hit,
                          const Scene& scene) const;
protected:
	float m_phong_exp;
};

#endif // CSE168_PHONG_LAMBERT_H_INCLUDED
