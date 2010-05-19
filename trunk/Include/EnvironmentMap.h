#ifndef ENVIRONMENT_MAP_H
#define ENVIRONMENT_MAP_H

#include "Vector3.h"

/*
 * created with help from http://www.debevec.org/Probes/:
 * If we consider the images to be normalized to have coordinates u=[-1,1], v=[-1,1], 
 * we have theta=atan2(v,u), phi=pi*sqrt(u*u+v*v). The unit vector pointing in the 
 * corresponding direction is obtained by rotating (0,0,-1) by phi degrees around the 
 * y (up) axis and then theta degrees around the -z (forward) axis. If for a direction 
 * vector in the world (Dx, Dy, Dz), the corresponding (u,v) coordinate in the light 
 * probe image is (Dx*r,Dy*r) where r=(1/pi)*acos(Dz)/sqrt(Dx^2 + Dy^2).
 */
class EnvironmentMap
{
public:
	static Vector3 lookUp( Vector3 direction, const Vector3 * map, int mapWidth, int mapHeight );

private:
	static float calculateR( Vector3 direction );
};

#endif // ENVIRONMENT_MAP_H