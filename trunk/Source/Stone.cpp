#include "Stone.h"
#include "Ray.h"
#include "Scene.h"
#include "DebugMem.h"
#include "PerlinNoise.h"
#include "WorleyNoise.h"

const float Stone::THRESHOLD = 0.02f;

Stone::Stone(Stone::Coloring coloring, float noiseMultiplier, const Vector3 & kd) :
Lambert(kd), mColoring(coloring), mNoiseMultiplier(noiseMultiplier)
{
	mNoiseMaker = new CustomizablePerlinNoise(4, 4, 1, 94);
}

Stone::~Stone()
{
	delete mNoiseMaker;
	mNoiseMaker = NULL;
}

Vector3
Stone::shade(const Ray &ray, const HitInfo &hit, const Scene &scene) const
{
	float * F = new float[2];
	float (*delta)[3] = new float[2][3];
	unsigned long * ID = new unsigned long[2];

	float * at = new float[3];

	at[0] = hit.P.x * mNoiseMultiplier;
	at[1] = hit.P.y * mNoiseMultiplier;
	at[2] = hit.P.z * mNoiseMultiplier;

	// use Worley noise function to create our texture
	WorleyNoise::noise3D( at, 2, F, delta, ID );
	float distance = F[1] - F[0];
	float colorID = ID[0];

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
			float perlinNoise, red, green, blue; // have to declare all these here to make the compiler happy
			switch( mColoring )
			{
			case COLORFUL:
				red = std::max( 0.0f, (1 + sin(colorID))/2 );
				blue = std::max( 0.0f, (1 + cos(colorID))/2 );
				green = std::max( 0.0f, ( red + blue ) / 2 );
				result *= Vector3(red,green,blue);
				perlinNoise = PerlinNoise::noise(hit.P.x, hit.P.y, hit.P.z);
				L += std::max(0.0f, nDotL/falloff * pLight->wattage() / PI) * result * (1-pow(perlinNoise,2));
				break;
			case REALISTIC:
				result *= m_kd;
				perlinNoise = (1 + mNoiseMaker->Get( hit.P.x, hit.P.y, hit.P.z )) / 2;
				L += std::max(0.0f, nDotL/falloff * pLight->wattage() / PI) * result * perlinNoise;
				break;
			}

			if( m_phong_exp != 0 )
			{
				// now calculate phong highlight
				Vector3 reflectDir = 2 * dot( l, hit.N ) * hit.N - l; // direction to light reflected across normal
				reflectDir.normalize();
				float viewDirDotReflectDir = dot( viewDir, reflectDir );
				if( viewDirDotReflectDir > 0 )
					L += std::max(0.0f, pow(viewDirDotReflectDir, m_phong_exp)) * pLight->color();
			}
		}
	    
		// add the ambient component
		L += m_ka;

		return L;
	}
}