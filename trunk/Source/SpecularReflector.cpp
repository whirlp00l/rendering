#include "SpecularReflector.h"
#include "Ray.h"
#include "Scene.h"
#include "DebugMem.h"

#include <assert.h>

const int SpecularReflector::RECURSION_DEPTH = 20;

SpecularReflector::SpecularReflector( const Vector3 & kd ) :
Lambert(kd)
{
	m_type = Material::SPECULAR;
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
		reflectedRay.refractiveIndex = 1.0f;
		hitSomething = scene.trace( recursiveHit, reflectedRay, epsilon, MIRO_TMAX );

		numRecursiveCalls++;
	}
	while( hitSomething && recursiveHit.material->getType() == Material::SPECULAR && numRecursiveCalls < RECURSION_DEPTH );

	// we maxed out our recursion by hitting a specular surface or we simply didn't hit anything
	if( !hitSomething || recursiveHit.material->getType() == Material::SPECULAR )
	{
		return m_kd;
	}
	else
	{
		return m_kd * recursiveHit.material->shade( reflectedRay, recursiveHit, scene );
	}
}