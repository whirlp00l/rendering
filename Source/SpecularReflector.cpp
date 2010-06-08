#include "SpecularReflector.h"
#include "Ray.h"
#include "Scene.h"
#include "DebugMem.h"
#include "EnvironmentMap.h"

#include <assert.h>

const int SpecularReflector::SPECULAR_RECURSION_DEPTH = 20;

SpecularReflector::SpecularReflector( const Vector3 & kd ) :
Lambert(kd)
{
	m_type = Material::SPECULAR_REFLECTOR;
}

SpecularReflector::~SpecularReflector()
{
}

Vector3
SpecularReflector::shade( const Ray& ray, const HitInfo& hit, const Scene& scene ) const
{
	static int numRecursiveCalls = 0;
	// we've maxed out our recursion
	if( numRecursiveCalls == SpecularReflector::SPECULAR_RECURSION_DEPTH )
	{
		return Vector3(0,0,0);
	}

	numRecursiveCalls++;

	Vector3 L = m_kd * getReflectedColor( ray, hit, scene );

	// add in the phong highlights (if necessary)
	if( m_phong_exp != 0 )
	{
		const Lights *lightlist = scene.lights();
		 // loop over all of the lights
		Lights::const_iterator lightIter;
		for (lightIter = lightlist->begin(); lightIter != lightlist->end(); lightIter++)
		{
			PointLight* pLight = *lightIter;
			L += getPhongHighlightContribution( pLight, ray, hit );
		}
	}

	// add ambient color
	L += m_ka;

	// we're about to pop a (potentially) recursive call off the stack
	numRecursiveCalls--;

	// make sure all components of the shading color are greater than 0
	L.x = std::max( 0.0f, L.x );
	L.y = std::max( 0.0f, L.y );
	L.z = std::max( 0.0f, L.z );

	return L;
}

Vector3 
SpecularReflector::getReflectedColor( const Ray& ray, const HitInfo& hit, const Scene& scene ) const
{
	// direction to last "eye" point reflected across hit surface normal
	Vector3 reflectDir = getReflectedDir( ray, hit );
	
	Ray reflectedRay;
	reflectedRay.o = hit.P;
	reflectedRay.d = reflectDir;
	reflectedRay.refractiveIndex = ray.refractiveIndex;

	HitInfo recursiveHit;
	Vector3 reflectedLight(0,0,0);
	if( scene.trace( recursiveHit, reflectedRay, epsilon, MIRO_TMAX ) )
		reflectedLight = m_kd * recursiveHit.material->shade( reflectedRay, recursiveHit, scene );
	else
	{
		if( USE_ENVIRONMENT_MAP && scene.environmentMap() )
		{
			reflectedLight = EnvironmentMap::lookUp( reflectedRay.d, scene.environmentMap(), scene.mapWidth(), scene.mapHeight() );
		}
		else
		{
			reflectedLight = m_kd * Vector3(0.5f);
		}
	}

	return reflectedLight;
}

Vector3
SpecularReflector::getReflectedDir( const Ray& ray, const HitInfo& hit ) const
{
	// direction to last "eye" point reflected across hit surface normal
	Vector3 normal = m_use_bump_map ? calcBumpMappedNormal( hit.P, hit.N ) : hit.N;
	Vector3 reflectDir = -2 * dot(ray.d, hit.N) * normal + ray.d;
	reflectDir.normalize();

	return reflectDir;
}