#include "Sand.h"
#include "Ray.h"
#include "Scene.h"
#include "DebugMem.h"
#include "AreaLight.h"
#include "PerlinNoise.h"
#include "WorleyNoise.h"

Sand::Sand(float noiseMultiplier, const Vector3 & kd) :
Lambert(kd), mNoiseMultiplier(noiseMultiplier)
{
	mNoiseMaker = new CustomizablePerlinNoise(8, 8, 0.2, 36);
	// we always want to use a bump map with our sand
	setUseBumpMap( true, 1, 1, 0.4, 23 );
}

Sand::~Sand()
{
	delete mNoiseMaker;
	mNoiseMaker = NULL;
}

Vector3
Sand::shade(const Ray &ray, const HitInfo &hit, const Scene &scene) const
{
	Vector3 L(0.0f, 0.0f, 0.0f);
	// use diffuse lighting
	float perlinNoise = (1 + mNoiseMaker->Get( hit.P.x, hit.P.y, hit.P.z )) / 2;
	L += getDiffuseColor( ray, hit, scene ) * perlinNoise;

	// incorporate indirect lighting
	if( USE_PATH_TRACING )
	{	
		// add in the indirect lighting result
		L += getIndirectLight( hit, scene ) * m_kd;
	} // end indirect lighting
		    
	// add the ambient component
	L += m_ka;

	// make sure all components of the shading color are greater than 0
	L.x = std::max( 0.0f, L.x );
	L.y = std::max( 0.0f, L.y );
	L.z = std::max( 0.0f, L.z );

	return L;
}