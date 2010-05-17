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
	bool hitSomething = false;
	HitInfo recursiveHit = hit;
	Ray refractedRay = ray;
	int numRecursiveCalls = 0;
	do
	{
		bool stop = false;
		Vector3 viewDir = -refractedRay.d; // d is a unit vector
		// set up refracted ray here
		float nDotViewDir = dot( viewDir, recursiveHit.N );
		float radicand;
		float refractiveIndexRatio;
		Vector3 refractDir;
		// positive dot product means we're entering the refractive material
		if( nDotViewDir > 0 )
		{
			refractiveIndexRatio = refractedRay.refractiveIndex / m_refractive_index;
			radicand = 1 - ( refractiveIndexRatio * refractiveIndexRatio ) * ( 1 - ( nDotViewDir * nDotViewDir ) );
			// can't take the square root of a negative number
			if( radicand < 0 ) 
			{
				// could perform reflection here instead. for now, just stop the refraction process.
				stop = true;
			}

			refractDir = -refractiveIndexRatio * ( viewDir - nDotViewDir*recursiveHit.N ) - sqrt(radicand)*recursiveHit.N;
			refractDir.normalize();

			refractedRay.d = refractDir;
			refractedRay.o = recursiveHit.P;
			refractedRay.refractiveIndex = m_refractive_index;
		}
		// negative dot product means we're exiting the refractive material
		else 
		{
			refractiveIndexRatio = m_refractive_index / refractedRay.refractiveIndex;
			// don't need to negate dot product since we're squaring it
			radicand = 1 - ( refractiveIndexRatio * refractiveIndexRatio ) * ( 1 - ( nDotViewDir * nDotViewDir ) );
			// can't take the square root of a negative number
			if( radicand < 0 ) 
			{
				// could perform reflection here instead. for now, just stop the refraction process.
				stop = true;
			}

			refractDir = -refractiveIndexRatio * ( viewDir + nDotViewDir*(-recursiveHit.N) ) - sqrt(radicand)*(-recursiveHit.N);
			refractDir.normalize();

			refractedRay.d = refractDir;
			refractedRay.o = recursiveHit.P;
			refractedRay.refractiveIndex = 1.0f;
		}

		// make sure we haven't set a halting flag
		if( stop )
			hitSomething = false;
		else
			hitSomething = scene.trace( recursiveHit, refractedRay, epsilon, MIRO_TMAX ); // trace the refracted ray

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
		return m_kd * recursiveHit.material->shade( refractedRay, recursiveHit, scene );
	}
}