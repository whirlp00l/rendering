#ifndef CSE168_LAMBERT_H_INCLUDED
#define CSE168_LAMBERT_H_INCLUDED

#include "Material.h"

class Lambert : public Material
{
public:
    Lambert(const Vector3 & kd = Vector3(1),
            const Vector3 & ka = Vector3(0),
			const float & phongExp = 50.0f);
    virtual ~Lambert();

    const Vector3 & kd() const {return m_kd;}
    const Vector3 & ka() const {return m_ka;}
	const float & phongExp() const {return m_phong_exp;}

    void setKd(const Vector3 & kd) {m_kd = kd;}
    void setKa(const Vector3 & ka) {m_ka = ka;}
	void setPhongExp(const float & phongExp) {m_phong_exp = phongExp;}
	void setUseHighlights(const bool & useHighlights) {m_use_highlights = useHighlights;}

    virtual void preCalc() {}
    
    virtual Vector3 shade(const Ray& ray, const HitInfo& hit,
                          const Scene& scene) const;
protected:
    Vector3 m_kd;
    Vector3 m_ka;
	float m_phong_exp;
	bool m_use_highlights;
};

#endif // CSE168_LAMBERT_H_INCLUDED
