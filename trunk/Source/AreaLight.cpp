#include "AreaLight.h"
#include <assert.h>
#include <stdlib.h>
#include <time.h>

unsigned int AreaLight::NUM_SAMPLES = 80;

AreaLight::AreaLight( const Vector3& position, const Vector3 & axis1, const Vector3 & axis2 ) :
PointLight(), m_axis1(axis1), m_axis2(axis2), m_samples(NULL)
{
	m_type = PointLight::AREA_LIGHT;
	m_position = position;
}

AreaLight::~AreaLight()
{
	if( m_samples )
	{
		delete [] m_samples;
		m_samples = NULL;
	}
}

float 
AreaLight::getHitRatio( Vector3 hitPoint, const Scene& scene )
{
	// make sure the sample points have been initialized
	assert( m_samples );

	int numHits = 0;
	for( unsigned int i = 0; i < NUM_SAMPLES; i++ )
	{
		Ray sampleRay;

		sampleRay.o = hitPoint;
		sampleRay.d = m_samples[i] - hitPoint;
		float magnitude = sampleRay.d.length();
		sampleRay.d /= magnitude;

		HitInfo sampleHit;
		if( scene.trace( sampleHit, sampleRay, epsilon, magnitude ) )
			numHits++;
	}

	return ( NUM_SAMPLES - numHits ) / ( float )NUM_SAMPLES;
}

void
AreaLight::preCalc()
{
	m_samples = new Vector3[NUM_SAMPLES];

	Vector3 axisOrigin = m_position - 0.5 * m_axis1 - 0.5 * m_axis2;

	srand(time(0));
	for( unsigned int i = 0; i < NUM_SAMPLES; i++ )
	{
		float u, v;
		
		u = rand() / static_cast<double>(RAND_MAX);
		v = rand() / static_cast<double>(RAND_MAX);

		m_samples[i] = axisOrigin + u * m_axis1 + v * m_axis2;
	}
}

Vector3
AreaLight::getRandomSample() const
{
	assert( m_samples );
	srand(time(0));
	int sampleIndex = rand() / AreaLight::NUM_SAMPLES;
	return m_samples[sampleIndex];
}