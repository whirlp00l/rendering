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
		PHONG_DIFFUSE,
		SPECULAR,
		UNDEFINED
	};

    virtual void preCalc() {}
    
    virtual Vector3 shade(const Ray& ray, const HitInfo& hit,
                          const Scene& scene) const;

	Type getType() const { return m_type; }
protected:
	Type m_type;
};

#endif // CSE168_MATERIAL_H_INCLUDED
