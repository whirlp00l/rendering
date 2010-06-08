#ifndef CSE168_MATERIAL_H_INCLUDED
#define CSE168_MATERIAL_H_INCLUDED

#include "Miro.h"
#include "Vector3.h"
#include "CustomizablePerlinNoise.h"

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
		SAND,
		UNDEFINED
	};

    virtual void preCalc() {}

	const float & phongExp() const {return m_phong_exp;}

	void setPhongExp(const float & phongExp) {m_phong_exp = phongExp;}
    
    virtual Vector3 shade(const Ray& ray, const HitInfo& hit,
                          const Scene& scene) const;

	Type getType() const { return m_type; }
	float getRefractiveIndex() const { return m_refractive_index; }
	bool useBumpMap() const { return m_use_bump_map; }
	void setUseBumpMap( bool useBumpMap, int octaves = 4, float freq = 6, float amp = 1, int seed = 14 ); 
	bool isSpecular() const { return m_type == SPECULAR_REFLECTOR || m_type == SPECULAR_REFRACTOR; }
	bool isDiffuse() const { return m_type == DIFFUSE || m_type == STONE || m_type == SAND; }

	static Material * loadMaterial( char * fileName );

	Vector3 calcBumpMappedNormal( Vector3 hitPoint, Vector3 origNormal ) const;

protected:
	Type m_type;
	float m_refractive_index;
	float m_phong_exp;
	bool m_use_bump_map;
	CustomizablePerlinNoise * m_bump_map_noise_maker;

	Vector3 getPhongHighlightContribution( const PointLight * pLight, const Ray& ray, const HitInfo& hit ) const;
};

#endif // CSE168_MATERIAL_H_INCLUDED
