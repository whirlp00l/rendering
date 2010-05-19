#ifndef ENVIRONMENT_MAP_H
#define ENVIRONMENT_MAP_H

#include "Vector3.h"

class EnvironmentMap
{
public:
	static Vector3 lookUp( Vector3 direction, const Vector3 * map, int mapWidth, int mapHeight );

private:
	static float calculateR( Vector3 direction );
};

#endif // ENVIRONMENT_MAP_H