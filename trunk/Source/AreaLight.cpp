#include "AreaLight.h"
#include <assert.h>
#include <stdlib.h>
#include <time.h>

unsigned int AreaLight::NUM_SAMPLES = 20;

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
		{
			bool inShadow = true;
			bool hitSomething = true;

			// if we hit a refractive material, light passes through it, so it does not obstruct light
			while( hitSomething && sampleHit.material->getType() == Material::SPECULAR_REFRACTOR )
			{
				// right now, we're not in shadow
				inShadow = false;

				// see if we hit anything else; start tracing from last hit point
				sampleRay.o = sampleHit.P;
				magnitude = ( m_samples[i] - sampleRay.o ).length();
				hitSomething = scene.trace( sampleHit, sampleRay, epsilon, magnitude );

				// if we hit something, put shadow flag back on (if it's refractive, it'll get turned off on the next loop)
				if( hitSomething )
					inShadow = true;
			}

			if( inShadow )
				numHits++;
		}
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
		
		u = rand() / static_cast<double>(RAND_MAX); // yields random value in range [0,1]
		v = rand() / static_cast<double>(RAND_MAX); // yields random value in range [0,1]

		m_samples[i] = axisOrigin + u * m_axis1 + v * m_axis2;
	}
}

bool
AreaLight::containsPoint( Vector3 point ) const 
{
	/*
	 * IDEA:
	 * For a point to fall within the area light, 3 conditions must be met:
	 * 1) The projection of the plane normal (i.e. cross( m_axis, m_axis ) and the vector formed
	 *    by ( point - axisOrigin ) must be 0. That is, the point lies in the desired plane.
	 * 2) ( point - axisOrigin ) projected onto m_axis1 must be in the range [0, magnitude(m_axis1)].
	 * 3) ( point - axisOrigin ) projected onto m_axis2 must be in the range [0, magnitude(m_axis2)].
	 *
	 * Note that we should account for floating point error in all 3 conditions.
	 */
	Vector3 axisOrigin = m_position - 0.5 * m_axis1 - 0.5 * m_axis2;
	Vector3 pointMinusAxisO = point - axisOrigin;

	// test condition 1
	Vector3 planeNormal = cross( m_axis1, m_axis2 );
	planeNormal.normalize();
	float planeNormalProj = dot( planeNormal, pointMinusAxisO );
	// anything within the range [-epsilon, epsilon] is valid to account for potential floating point error
	if(  planeNormalProj < -epsilon || planeNormalProj > epsilon )
		return false; // condition 1 was not met

	// test condition 2
	float magAxis1 = m_axis1.length();
	float axis1Proj = dot( m_axis1, pointMinusAxisO );
	// anything within the range [-epsilon, 1 + epsilon] is valid to account for potential floating point error
	if( axis1Proj < -epsilon || axis1Proj > (1 + epsilon) )
		return false; // condition 2 was not met

	// test condition 3
	float magAxis2 = m_axis2.length();
	float axis2Proj = dot( m_axis2, pointMinusAxisO );
	// anything within the range [-epsilon, 1 + epsilon] is valid to account for potential floating point error
	if( axis2Proj < -epsilon || axis2Proj > (1 + epsilon) )
		return false; // condition 3 was not met

	// all 3 conditions have been met, so the point lies within the area light
	return true;
}

Vector3
AreaLight::getRandomLightPoint() const
{
	assert( m_samples );
	int sampleIndex = rand() / NUM_SAMPLES;
	return m_samples[sampleIndex];
}