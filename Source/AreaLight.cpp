#include "AreaLight.h"
#include <assert.h>
#include <stdlib.h>
#include <time.h>

AreaLight::AreaLight( const Vector3 & min, const Vector3 & max, unsigned int numSamples ) :
PointLight(), m_min(min), m_max(max), m_num_samples(numSamples), m_samples(NULL)
{
	m_type = PointLight::AREA_LIGHT;
	m_position = (m_min + m_max) / 2;
}

AreaLight::~AreaLight()
{
	if( m_samples )
	{
		delete [] m_samples;
		m_samples = NULL;
	}
}

void 
AreaLight::setPosition(const Vector3& v)
{
	m_position = v;

	float oldRangeX = m_max.x - m_min.x;
	float oldRangeY = m_max.y - m_min.y;
	float oldRangeZ = m_max.z - m_min.z;

	m_min.x = m_position.x - oldRangeX/2;
	m_max.x = m_position.x + oldRangeX/2;
	m_min.y = m_position.y - oldRangeY/2;
	m_max.y = m_position.y + oldRangeY/2;
	m_min.z = m_position.z - oldRangeZ/2;
	m_max.z = m_position.z + oldRangeZ/2;
}

float 
AreaLight::getHitRatio( Vector3 hitPoint, const Scene& scene )
{
	// make sure the sample points have been initialized
	assert( m_samples );

	int numHits = 0;
	for( unsigned int i = 0; i < m_num_samples; i++ )
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

	return ( m_num_samples - numHits ) / ( float )m_num_samples;
}

void
AreaLight::preCalc()
{
	m_samples = new Vector3[m_num_samples];

	float xRange = m_max.x - m_min.x;
	float yRange = m_max.y - m_min.y;
	float zRange = m_max.z - m_min.z;

	srand(time(0));
	for( unsigned int i = 0; i < m_num_samples; i++ )
	{
		float x, y, z, random;
		
		if( xRange == 0 )
			x = m_min.x;
		else
		{
			random = rand() / static_cast<double>(RAND_MAX) + 1.0;
			x = random * xRange + m_min.x;
		}


		if( yRange == 0 )
			y = m_min.y;
		else
		{
			random = rand() / static_cast<double>(RAND_MAX) + 1.0;
			y = random * yRange + m_min.y;
		}


		if( zRange == 0 )
			z = m_min.z;
		else
		{
			random = rand() / static_cast<double>(RAND_MAX) + 1.0;
			z = random * zRange + m_min.z;
		}

		m_samples[i] = Vector3(x,y,z);
	}
}