#include "Lambert.h"
#include "Ray.h"
#include "Scene.h"
#include "DebugMem.h"
#include "AreaLight.h"

const int Lambert::PATH_TRACING_RECURSION_DEPTH = 2;

Lambert::Lambert(const Vector3 & kd, const Vector3 & ka, const float & phongExp) : 
m_kd(kd), m_ka(ka)
{
	m_type = Material::DIFFUSE;
	m_phong_exp = phongExp;
}

Lambert::~Lambert()
{
}

Vector3
Lambert::shade(const Ray& ray, const HitInfo& hit, const Scene& scene) const
{
	static int numRecursiveCalls = 0;
	// we've maxed out our recursion
	if( numRecursiveCalls == Lambert::PATH_TRACING_RECURSION_DEPTH )
	{
		return Vector3(0,0,0);
	}

	numRecursiveCalls++;

    Vector3 L = getDiffuseColor( ray, hit, scene );

	// incorporate indirect lighting
	if( USE_PATH_TRACING )
	{	
		// add in the indirect lighting result
		L += getIndirectLight( hit, scene ) * m_kd;
	} // end indirect lighting

    // add the ambient component
    L += m_ka;

	// we're about to pop a (potentially) recursive call off the stack
	numRecursiveCalls--;
    
    return L;
}

Vector3
Lambert::getDiffuseColor( const Ray& ray, const HitInfo& hit, const Scene& scene ) const
{
	Vector3 L = Vector3(0.0f, 0.0f, 0.0f);
    
    const Vector3 viewDir = -ray.d; // d is a unit vector
   
	// BEGIN CODE FOR DIRECT LIGHTING
    const Lights *lightlist = scene.lights();
    
    // loop over all of the lights
    Lights::const_iterator lightIter;
    for (lightIter = lightlist->begin(); lightIter != lightlist->end(); lightIter++)
    {
        PointLight* pLight = *lightIter;

		Vector3 l = pLight->position() - hit.P;

		// the inverse-squared falloff
        float falloff = l.length2();
        float magnitude = sqrt(falloff);
        // normalize the light direction
        l /= magnitude;

		// we only need to add this light's contribution if we're not in shadow
		Ray shadowRay;
		shadowRay.d = l;
		shadowRay.o = hit.P;
		HitInfo hitInfo;

		// we have a (hard) shadow!
		if( !pLight->isAreaLight() && scene.trace( hitInfo, shadowRay, epsilon, magnitude ) )
		{
			continue;
		}

		float hitRatio = 1;
		if( pLight->isAreaLight() )
			hitRatio = (( AreaLight * )pLight)->getHitRatio( hit.P, scene );
    
        // get the diffuse component
		Vector3 normal = m_use_bump_map ? calcBumpMappedNormal( hit.P, hit.N ) : hit.N;
        float nDotL = dot(normal, l);
        Vector3 result = pLight->color();
        result *= m_kd;
        
        L += std::max(0.0f, nDotL/falloff * pLight->wattage() / PI) * result * hitRatio;

		if( m_phong_exp != 0 )
		{
			L += getPhongHighlightContribution( pLight, ray, hit );
		}
    }
	// END CODE FOR DIRECT LIGHTING
    
    return L;
}

Vector3
Lambert::getIndirectLight( const HitInfo hitInfo, const Scene& scene ) const
{
	Vector3 indirectLighting(0,0,0);
	Ray indirectLightingRay;
	HitInfo indirectLightingHit; 
	
	for( int k = 0; k < NUM_PATH_TRACING_SAMPLES; k++ )
	{
		// sample indirect lighting here
		float x = rand() / static_cast<double>(RAND_MAX);
		float y = rand() / static_cast<double>(RAND_MAX);
		float z = rand() / static_cast<double>(RAND_MAX);

		// since rand() only generates values between 0 adn RAND_MAX,
		// we must randomize whether or not this value is negative
		float posOrNeg = rand() / static_cast<double>(RAND_MAX);
		if( posOrNeg < 0.5 )
			x *= -1;
		posOrNeg = rand() / static_cast<double>(RAND_MAX);
		if( posOrNeg < 0.5 )
			y *= -1;
		posOrNeg = rand() / static_cast<double>(RAND_MAX);
		if( posOrNeg < 0.5 )
			z *= -1;

		Vector3 randomDir(x,y,z);
		randomDir.normalize();

		// make sure the random direction isn't pointing INTO the material
		if( dot( randomDir, hitInfo.N ) < 0 )
			randomDir *= -1;

		indirectLightingRay.o = hitInfo.P;
		indirectLightingRay.d = randomDir;
		if( scene.trace( indirectLightingHit, indirectLightingRay, epsilon, MIRO_TMAX ) )
		{
			bool hitAreaLight = false;

			// loop over all of the lights to see if we hit an area light
			const Lights *lightlist = scene.lights();
			Lights::const_iterator lightIter;
			for (lightIter = lightlist->begin(); lightIter != lightlist->end(); lightIter++)
			{
				PointLight* pLight = *lightIter;
				if( pLight->isAreaLight() && (( AreaLight * )pLight)->containsPoint( indirectLightingHit.P ) )
				{
					hitAreaLight = true;
					break;
				}
			}

			// only add this indirect lighting contribution if we didn't hit an area light
			if( !hitAreaLight )
				indirectLighting += indirectLightingHit.material->shade(indirectLightingRay, indirectLightingHit, scene);
		}
	}

	// average the result and add it to the shade result here
	indirectLighting /= NUM_PATH_TRACING_SAMPLES;

	return indirectLighting;
}
