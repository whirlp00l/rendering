#include "SpecularReflector.h"
#include "Ray.h"
#include "Scene.h"
#include "DebugMem.h"

#include <assert.h>

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
	bool hitSomething;
	HitInfo recursiveHit = hit;
	Ray reflectedRay = ray;
	int numRecursiveCalls = 0;
	do
	{
		// direction to last "eye" point reflected across hit surface normal
		Vector3 reflectDir = -2 * dot(reflectedRay.d, recursiveHit.N) * recursiveHit.N + reflectedRay.d;
		reflectDir.normalize();
		
		reflectedRay.o = recursiveHit.P;
		reflectedRay.d = reflectDir;
		// the refractiveIndex of this ray remains unchanged

		hitSomething = scene.trace( recursiveHit, reflectedRay, epsilon, MIRO_TMAX );

		numRecursiveCalls++;
	}
	while( hitSomething && recursiveHit.material->isSpecular() && numRecursiveCalls < Material::SPECULAR_RECURSION_DEPTH );

	// we maxed out our recursion by hitting a specular surface or we simply didn't hit anything
	if( !hitSomething || recursiveHit.material->isSpecular() )
	{
		return m_kd;
	}
	else
	{
		return m_kd * recursiveHit.material->shade( reflectedRay, recursiveHit, scene );
	}
}