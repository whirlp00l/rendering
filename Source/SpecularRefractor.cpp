#include "SpecularRefractor.h"
#include "Ray.h"
#include "Scene.h"
#include "DebugMem.h"
#include "EnvironmentMap.h"

#include <assert.h>

SpecularRefractor::SpecularRefractor( const float & refractiveIndex, const Vector3 & kd ) :
SpecularReflector(kd)
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

	Ray refractedRay;
	bool useRefraction = getRefractedRay( refractedRay, ray, hit, scene );

	Vector3 L;

	// use refraction
	if( useRefraction )
	{
		HitInfo recursiveHit;
		// trace the refracted ray
		if( scene.trace( recursiveHit, refractedRay, epsilon, MIRO_TMAX ) )
			L = m_kd * recursiveHit.material->shade( refractedRay, recursiveHit, scene );
		else
		{
			if( USE_ENVIRONMENT_MAP && scene.environmentMap() )
			{
				L = m_kd * EnvironmentMap::lookUp( refractedRay.d, scene.environmentMap(), scene.mapWidth(), scene.mapHeight() );
			}
			else
			{
				L = m_kd * Vector3(0.5f);
			}
		}
	}
	// total internal refraction: just use reflection instead
	else
	{
		// if we're INSIDE the surface. we don't need to shade it in that case.
		if( dot( ray.d, hit.N ) > 0 )
		{
			numRecursiveCalls--;
			return Vector3(0,0,0);
		}

		L = getReflectedColor( ray, hit, scene );
	}

	// add in the phong highlights (if necessary)
	if( m_phong_exp != 0 )
	{
		L += getPhongHighlightContribution( ray, hit, scene );
	}

	L += m_ka;
	
	numRecursiveCalls--;
	return L;
}

bool 
SpecularRefractor::getRefractedRay( Ray& refractedRay, const Ray& ray, const HitInfo& hit, const Scene& scene ) const
{
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
		n2 = 1.0f; // assume we're entering air
		normal = -hit.N;
		refractedRayIndex = 1.0f;
		nDotViewDir *= -1;
		flipped = true;
	}

	float refractiveIndexRatio = n1 / n2;
	float radicand = 1 - ( refractiveIndexRatio * refractiveIndexRatio ) * ( 1 - ( nDotViewDir * nDotViewDir ) );
	
	// can't take the square root of a negative number, so refraction is impossible.
	// this signifies total internal reflection.
	if( radicand < 0 )
		return false;

	Vector3 refractDir = -refractiveIndexRatio * ( viewDir - nDotViewDir*normal ) - sqrt(radicand)*normal;
	refractDir.normalize();

	refractedRay.d = refractDir;
	refractedRay.o = hit.P;
	refractedRay.refractiveIndex = refractedRayIndex;

	return true;
}