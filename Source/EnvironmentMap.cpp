#include "EnvironmentMap.h"
#include "Miro.h"
#include "DebugMem.h"
#include <math.h>
#include <assert.h>

Vector3
EnvironmentMap::lookUp( Vector3 direction, const Vector3 * map, int mapWidth, int mapHeight )
{
	assert( map );

	float r = calculateR( direction );
	float u = direction.x * r; // will be in range [-1,1]
	float v = direction.y * r; // will be in range [-1,1]

	float uNormalized = ( u + 1 ) / 2;
	float vNormalized = ( v + 1 ) / 2;

	int mapWidthIdx = int( uNormalized * mapWidth );
	int mapHeightIdx = int( vNormalized * mapHeight );

	assert( mapWidthIdx < mapWidth );
	assert( mapHeightIdx < mapHeight );

	return map[mapWidth * mapHeightIdx + mapWidthIdx];
}

float 
EnvironmentMap::calculateR( Vector3 dir )
{
	return (1/PI) * acos(dir.z) / sqrt(dir.x * dir.x + dir.y * dir.y );
}