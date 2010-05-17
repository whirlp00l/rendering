#include "SpecularRefractor.h"
#include "Ray.h"
#include "Scene.h"
#include "DebugMem.h"

SpecularRefractor::SpecularRefractor( float refractiveIndex ) :
Material()
{
	m_refractive_index = refractiveIndex;
}

SpecularRefractor::~SpecularRefractor()
{
}

float 
SpecularRefractor::getRefractiveIndex( RefractiveMaterial material )
{
	// these indeces come from http://www.robinwood.com/Catalog/Technical/Gen3DTuts/Gen3DPages/RefractionIndexList.html
	float index = 1.0f;

	switch( material )
	{
	case WATER_100_C:
		index = 1.31766;
		break;
	case WATER_0_C:
		index = 1.33346;
		break;
	case WATER_20_C:
		index = 1.33283;
		break;
	case DIAMOND:
		index = 2.417;
		break;
	case MILK:
		index = 1.35;
		break;
	case ICE:
		index = 1.309;
		break;
	case GLASS_COMMON:
		index = 1.52;
		break;
	case GLASS_PYREX:
		index = 1.474;
		break;
	}
	
	return index;
}

Vector3 
SpecularRefractor::shade(const Ray& ray, const HitInfo& hit,const Scene& scene) const
{
	return Vector3(0,0,0);
}