#include "Stone.h"
#include "Ray.h"
#include "Scene.h"
#include "DebugMem.h"
#include "AreaLight.h"
#include "PerlinNoise.h"
#include "WorleyNoise.h"

const float Stone::THRESHOLD = 0.02f;

Stone::Stone(Stone::Coloring coloring, float noiseMultiplier, const Vector3 & kd) :
Lambert(kd), mColoring(coloring), mNoiseMultiplier(noiseMultiplier)
{
	m_type = Material::STONE;
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

	Vector3 L(0.0f, 0.0f, 0.0f);
	Vector3 diffuseComponent(0.0f, 0.0f, 0.0f);
	// use diffuse lighting (if distance <= threshold, leave it black)
	if( distance > THRESHOLD ) 
	{
		float perlinNoise, red, green, blue; // have to declare all these here to make the compiler happy
		switch( mColoring )
		{
		case COLORFUL:
			red = std::max( 0.0f, (1 + sin(colorID))/2 );
			blue = std::max( 0.0f, (1 + cos(colorID))/2 );
			green = std::max( 0.0f, ( red + blue ) / 2 );
			diffuseComponent = Vector3( red, green, blue );
			perlinNoise = PerlinNoise::noise(hit.P.x, hit.P.y, hit.P.z);
			L += diffuseComponent * getDiffuseColor( ray, hit, scene );
			break;
		case REALISTIC:
			diffuseComponent = m_kd;
			perlinNoise = (1 + mNoiseMaker->Get( hit.P.x, hit.P.y, hit.P.z )) / 2;
			L += perlinNoise * getDiffuseColor( ray, hit, scene );
			break;
		}
	}

	// incorporate indirect lighting
	if( USE_PATH_TRACING )
	{	
		// add in the indirect lighting result
		L += getIndirectLight( hit, scene ) * diffuseComponent;
	} // end indirect lighting
		    
	// add the ambient component
	L += m_ka;

	// make sure all components of the shading color are greater than 0
	L.x = std::max( 0.0f, L.x );
	L.y = std::max( 0.0f, L.y );
	L.z = std::max( 0.0f, L.z );

	return L;
}