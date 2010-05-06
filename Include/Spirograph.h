#ifndef CSE168_SPIROGRAPH_H_INCLUDED
#define CSE168_SPIROGRAPH_H_INCLUDED

#include "Vector3.h"

class Spirograph
{

public:
    Spirograph(float AX, float BX, float AY, float BY, float AZ, float BZ, int numSpheres);
    virtual ~Spirograph();
	Vector3 sphereCenter(float t);
	Vector3 getSphereLocation(int i);

	int getNumSpheres()                    { return m_numSpheres; }

private:
	// member functions
	float cx(float A, float B, float t)    { return A * cos(B * t); }
	float cy(float A, float B, float t)    { return A * sin(B * t); }
	// member objects
	Vector3 * m_sphereLocations;
	float m_AX;
	float m_BX;
	float m_AY;
	float m_BY;
	float m_AZ;
	float m_BZ;
	int m_numSpheres;

};

#endif // CSE168_SPIROGRAPH_H_INCLUDED