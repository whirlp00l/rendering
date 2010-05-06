#include "Miro.h"
#include "Spirograph.h"
#include "DebugMem.h"
#include <assert.h>

Spirograph::Spirograph(float AX, float BX, float AY, float BY, float AZ, float BZ, int numSpheres) :
m_AX(AX), m_BX(BX), m_AY(AY), m_BY(BY), m_AZ(AZ), m_BZ(BZ), m_numSpheres(numSpheres)
{
	// we want at least 2 spheres in the spirograph
	assert( numSpheres > 1 );
	m_sphereLocations = new Vector3[numSpheres];
	for(int i = 0; i < m_numSpheres; i++ ) {
		float t = ((float)i/(m_numSpheres - 1)) * 2 * PI;
		m_sphereLocations[i] = sphereCenter(t);
	}
}

Spirograph::~Spirograph()
{
	delete [] m_sphereLocations;
}

Vector3 
Spirograph::sphereCenter(float t)
{
	float x = cx(m_AX, m_BX, t) + cy(m_AZ, m_BZ, t);
	float y = cy(m_AX, m_BX, t) + cx(m_AY, m_BY, t);
	float z = cx(m_AZ, m_BZ, t) + cy(m_AY, m_BY, t);
	return Vector3(x, y, z);
}

Vector3
Spirograph::getSphereLocation(int i)
{
	assert( i >= 0 && i < m_numSpheres );
	return m_sphereLocations[i];
}