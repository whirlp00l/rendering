#include "SpecularRefractor.h"
#include "Ray.h"
#include "Scene.h"
#include "DebugMem.h"

#include <assert.h>

SpecularRefractor::SpecularRefractor( const float & refractiveIndex, const Vector3 & kd ) :
Lambert(kd)
{
	m_refractive_index = refractiveIndex;
	m_type = Material::SPECULAR_REFRACTOR;
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
	static int numRecursiveCalls = 0;
	// we've maxed out our recursion
	if( numRecursiveCalls == Material::SPECULAR_RECURSION_DEPTH )
	{
		return Vector3(0,0,0);
	}

	numRecursiveCalls++;

	Vector3 viewDir = -ray.d; // d is a unit vector
	// set up refracted ray here
	float nDotViewDir = dot( viewDir, hit.N );

	float refractedRayIndex;
	float n1;
	float n2;
	Vector3 normal;

	bool flipped = false;
	// we're entering the refracted material
	if( nDotViewDir > 0 )
	{
		n1 = ray.refractiveIndex;
		n2 = hit.material->getRefractiveIndex();
		normal = hit.N;
		refractedRayIndex = hit.material->getRefractiveIndex();
	}
	// we're exiting the refractive material
	else
	{
		n1 = hit.material->getRefractiveIndex();
		n2 = ray.refractiveIndex;
		normal = -hit.N;
		refractedRayIndex = 1.0f;
		nDotViewDir *= -1;
		flipped = true;
	}

	float refractiveIndexRatio = n1 / n2;
	float radicand = 1 - ( refractiveIndexRatio * refractiveIndexRatio ) * ( 1 - ( nDotViewDir * nDotViewDir ) );

	Vector3 L;

	// total internal refraction: just use reflection instead
	if( radicand < 0 )
	{
		// if we've flipped the normal, we're INSIDE the surface. we don't need to shade it in that case.
		if( flipped )
			return Vector3(0,0,0);

		// direction to last "eye" point reflected across hit surface normal
		Vector3 reflectDir = -2 * dot(ray.d, hit.N) * hit.N + ray.d;
		reflectDir.normalize();
		
		Ray reflectedRay;
		reflectedRay.o = hit.P;
		reflectedRay.d = reflectDir;
		reflectedRay.refractiveIndex = ray.refractiveIndex;

		HitInfo recursiveHit;
		if( scene.trace( recursiveHit, reflectedRay, epsilon, MIRO_TMAX ) )
			L = m_kd * recursiveHit.material->shade( reflectedRay, recursiveHit, scene );
		else
			L = m_kd * Vector3(0.5f);
	}
	// use refraction
	else {
		Ray refractedRay;
		Vector3 refractDir = -refractiveIndexRatio * ( viewDir - nDotViewDir*normal ) - sqrt(radicand)*normal;
		refractDir.normalize();

		refractedRay.d = refractDir;
		refractedRay.o = hit.P;
		refractedRay.refractiveIndex = refractedRayIndex;

		HitInfo recursiveHit;
		// trace the refracted ray
		if( scene.trace( recursiveHit, refractedRay, epsilon, MIRO_TMAX ) )
			L = m_kd * recursiveHit.material->shade( refractedRay, recursiveHit, scene );
		else
			L = m_kd * Vector3(0.5f);
	}

	L += m_ka;
	
	numRecursiveCalls--;
	return L;
}