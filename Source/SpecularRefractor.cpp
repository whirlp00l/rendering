#include "SpecularRefractor.h"
#include "Ray.h"
#include "Scene.h"
#include "DebugMem.h"
#include "EnvironmentMap.h"

#include <assert.h>

SpecularRefractor::SpecularRefractor( const float & refractiveIndex, const Vector3 & kd, const float & density ) :
SpecularReflector(kd), m_density(density)
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
	if( numRecursiveCalls == SpecularReflector::SPECULAR_RECURSION_DEPTH )
	{
		return Vector3(0,0,0);
	}

	numRecursiveCalls++;

	Ray refractedRay;
	float reflectivity;
	bool useRefraction = getRefractedRay( refractedRay, reflectivity, ray, hit, scene );

	Vector3 L;

	// use refraction
	if( useRefraction )
	{
		HitInfo recursiveHit;
		// trace the refracted ray
		if( scene.trace( recursiveHit, refractedRay, epsilon, MIRO_TMAX ) )
		{
			// use Beer's law: http://www.flipcode.com/archives/Raytracing_Topics_Techniques-Part_3_Refractions_and_Beers_Law.shtml
			float rayLength = ( recursiveHit.P - refractedRay.o ).length();
			Vector3 absorbance = m_kd * m_density * -rayLength;
			Vector3 transparency( expf( absorbance.x ), expf( absorbance.y ), expf( absorbance.z ) );
			Vector3 refractedColor = transparency * recursiveHit.material->shade( refractedRay, recursiveHit, scene );
			Vector3 reflectedColor = getReflectedColor( ray, hit, scene );
			L = ( 1 - reflectivity ) * refractedColor + reflectivity * reflectedColor;
		}
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
		const Lights *lightlist = scene.lights();
		 // loop over all of the lights
		Lights::const_iterator lightIter;
		for (lightIter = lightlist->begin(); lightIter != lightlist->end(); lightIter++)
		{
			PointLight* pLight = *lightIter;
			L += getPhongHighlightContribution( pLight, ray, hit );
		}
	}

	L += m_ka;
	
	numRecursiveCalls--;

	// make sure all components of the shading color are greater than 0
	L.x = std::max( 0.0f, L.x );
	L.y = std::max( 0.0f, L.y );
	L.z = std::max( 0.0f, L.z );

	return L;
}

bool 
SpecularRefractor::getRefractedRay( Ray& refractedRay, float& reflectivity, const Ray& ray, const HitInfo& hit, const Scene& scene ) const
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

	// since we're using refraction, calculate the reflectivity based on the incident angle
	float cosTheta = dot( viewDir, normal );
	float r0 = ( n2 - 1 ) / ( n2 + 1 );
	r0 *= r0; // square it
	// use the Schlick approximation to approximate the Fresnel equations
	reflectivity = r0 + ( 1 - r0 ) * pow( 1 - cosTheta, 5 );  

	return true;
}