#ifndef CSE168_MATERIAL_H_INCLUDED
#define CSE168_MATERIAL_H_INCLUDED

#include "Miro.h"
#include "Vector3.h"

class Material
{
public:
    Material();
    virtual ~Material();

	enum Type
	{
		DIFFUSE,
		SPECULAR_REFLECTOR,
		SPECULAR_REFRACTOR,
		STONE,
		UNDEFINED
	};

    virtual void preCalc() {}

	const float & phongExp() const {return m_phong_exp;}

	void setPhongExp(const float & phongExp) {m_phong_exp = phongExp;}
    
    virtual Vector3 shade(const Ray& ray, const HitInfo& hit,
                          const Scene& scene) const;

	Type getType() const { return m_type; }
	float getRefractiveIndex() const { return m_refractive_index; }
	bool isSpecular() const { return m_type == SPECULAR_REFLECTOR || m_type == SPECULAR_REFRACTOR; }

	static const int SPECULAR_RECURSION_DEPTH; // upper limit for number of reflective bounces to trace

	static Material * loadMaterial( char * fileName );

protected:
	Type m_type;
	float m_refractive_index;
	float m_phong_exp;
};

#endif // CSE168_MATERIAL_H_INCLUDED
