#include "Stone.h"
#include "Ray.h"
#include "Scene.h"
#include "DebugMem.h"
#include "Perlin.h"
#include "Worley.h"

const float Stone::THRESHOLD = 0.02f;

Stone::Stone(const Vector3 & kd) :
Lambert(kd)
{
}

Stone::~Stone()
{
}

Vector3
Stone::shade(const Ray &ray, const HitInfo &hit, const Scene &scene) const
{
	float * F = new float[2];
	float (*delta)[3] = new float[2][3];
	unsigned long * ID = new unsigned long[2];

	float * at = new float[3];
	at[0] = hit.P.x;
	at[1] = hit.P.y;
	at[2] = hit.P.z;

	// use Worley noise function to create our texture
	WorleyNoise::noise3D( at, 2, F, delta, ID );
	float distance = F[1] - F[0];
		
	// clean up allocated memory
	delete [] F;
	F = NULL;

	delete [] delta;
	delta = NULL;

	delete [] ID;
	ID = NULL;

	delete [] at;
	at = NULL;

	// we're in between "stones"
	if( distance <= THRESHOLD )
		return Vector3(0,0,0);
	// just use diffuse lighting
	else 
	{
		Vector3 L = Vector3(0.0f, 0.0f, 0.0f);
    
		const Vector3 viewDir = -ray.d; // d is a unit vector
	    
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
			// we have a shadow!
			if( scene.trace( hitInfo, shadowRay, epsilon, magnitude ) )
			{
				continue;
			}
	    
			// get the diffuse component
			float nDotL = dot(hit.N, l);
			Vector3 result = pLight->color();
			result *= m_kd;
	        
			float perlinNoise = PerlinNoise::noise(hit.P.x, hit.P.y, hit.P.z);
			L += std::max(0.0f, nDotL/falloff * pLight->wattage() / PI) * result * (1-pow(perlinNoise,4));
		}
	    
		// add the ambient component
		L += m_ka;
	    
		return L;
	}
}