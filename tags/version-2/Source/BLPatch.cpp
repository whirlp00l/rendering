#include "BLPatch.h"
#include "Lambert.h"
#include "Ray.h"
#include "Console.h"
#include "DebugMem.h"

#include "BVH.h"

BLPatch::BLPatch( Vector3 ptA, Vector3 ptB, Vector3 ptC, Vector3 ptD )
{
	setPointA( ptA );
	setPointB( ptB );
	setPointC( ptC );
	setPointD( ptD );
}

BLPatch::~BLPatch()
{
}

const Vector3
BLPatch::uvCoeff()
{
	return Vector3(pointA() - pointB() + pointC() - pointD());
}

const Vector3
BLPatch::uCoeff()
{
	return Vector3(-pointA() + pointB());
}

const Vector3
BLPatch::vCoeff()
{
	return Vector3(-pointA() + pointD());	
}

const Vector3
BLPatch::constCoeff()
{
	return Vector3(pointA());
}

Vector3
BLPatch::patchPoint( float u, float v )
{
	assert( u >= 0 && u <= 1 && v >= 0 && v <= 1 );
	return (u * v * uvCoeff()) + (u * uCoeff()) + (v * vCoeff()) + constCoeff();
}

void
BLPatch::renderGL()
{
	const Vector3 ptA = pointA();
	const Vector3 ptB = pointB();
	const Vector3 ptC = pointC();
	const Vector3 ptD = pointD();
	const Vector3 ptMid = patchPoint( 0.5, 0.5 );

	glBegin(GL_TRIANGLES);
        glVertex3f(ptA.x, ptA.y, ptA.z);
        glVertex3f(ptMid.x, ptMid.y, ptMid.z);
        glVertex3f(ptB.x, ptB.y, ptB.z);

		glVertex3f(ptB.x, ptB.y, ptB.z);
		glVertex3f(ptMid.x, ptMid.y, ptMid.z);
		glVertex3f(ptC.x, ptC.y, ptC.z);

		glVertex3f(ptC.x, ptC.y, ptC.z);
		glVertex3f(ptMid.x, ptMid.y, ptMid.z);
		glVertex3f(ptD.x, ptD.y, ptD.z);

		glVertex3f(ptD.x, ptD.y, ptD.z);
		glVertex3f(ptMid.x, ptMid.y, ptMid.z);
		glVertex3f(ptA.x, ptA.y, ptA.z);
    glEnd();
}

bool
BLPatch::intersect(HitInfo& result, const Ray& ray,
                   float tMin, float tMax)
{
    const Vector3 x1 = uvCoeff();
	const Vector3 x2 = uCoeff();
	const Vector3 x3 = vCoeff();
	const Vector3 x4 = constCoeff();

	float A1 = (x1.x * ray.d.z) - (x1.z * ray.d.x);
	float B1 = (x2.x * ray.d.z) - (x2.z * ray.d.x);
	float C1 = (x3.x * ray.d.z) - (x3.z * ray.d.x);
	float D1 = (x4.x - ray.o.x) * ray.d.z - (x4.z - ray.o.z) * ray.d.x;
	float A2 = (x1.y * ray.d.z) - (x1.z * ray.d.y);
	float B2 = (x2.y * ray.d.z) - (x2.z * ray.d.y);
	float C2 = (x3.y * ray.d.z) - (x3.z * ray.d.y);
	float D2 = (x4.y - ray.o.y) * ray.d.z - (x4.z - ray.o.z) * ray.d.y;

	float vSquaredCoeff = A2*C1 - A1*C2;
	float vCoeff = A2*D1 - A1*D2 + B2*C1 - B1*C2;
	float constTerm = B2*D1 - B1*D2;

	float discrim = (vCoeff * vCoeff) - (4 * vSquaredCoeff * constTerm);

	// don't take the square root of a negative number
	if( discrim < 0 )
		return false;

	// don't divid by 0
	if( 0 == vSquaredCoeff )
		return false;

	float sqrtDiscrim = sqrt(discrim);

	float v[2];

	v[0] = (-vCoeff + sqrtDiscrim) / (2 * vSquaredCoeff);
	v[1]= (-vCoeff - sqrtDiscrim) / (2 * vSquaredCoeff);

	bool useOption[2];
	useOption[0] = useOption[1] = true;
	
	// determine which option(s) are viable
	for( int i = 0; i < 2; i++ ) {
		// v must range from 0 to 1
		if( v[i] < 0 || v[i] > 1 )
			useOption[i] = false;
	}

	// we have no viable options, so we have no intersection
	if( !useOption[0] && !useOption[1] )
		return false;

	// we want to use the largest component of the ray's direction vector
	// to determine how we will compute t; calculate ahead of time so we
	// only do it once
	int largestComponent;
	// x is the largest component
	if( fabs(ray.d.x) >= fabs(ray.d.y) && fabs(ray.d.x) >= fabs(ray.d.z) )
		largestComponent = 1;
	// y is the largest component
	else if( fabs(ray.d.y) >= fabs(ray.d.x) && fabs(ray.d.y) >= fabs(ray.d.z) )
		largestComponent = 2;
	// z is the largest component
	else
		largestComponent = 3;

	float x1Comp, x2Comp, x3Comp, x4Comp, dComp, oComp;

	switch( largestComponent )
	{
	case 1: // use x component
		x1Comp = x1.x;
		x2Comp = x2.x;
		x3Comp = x3.x;
		x4Comp = x4.x;
		dComp = ray.d.x;
		oComp = ray.o.x;
		break;
	case 2: // use y component
		x1Comp = x1.y;
		x2Comp = x2.y;
		x3Comp = x3.y;
		x4Comp = x4.y;
		dComp = ray.d.y;
		oComp = ray.o.y;
		break;
	case 3: // use z component
		x1Comp = x1.z;
		x2Comp = x2.z;
		x3Comp = x3.z;
		x4Comp = x4.z;
		dComp = ray.d.z;
		oComp = ray.o.z;
		break;
	}

	// just in case - watch for dividing by 0!!!
	if( 0 == dComp )
		return false;

	float u[2];
	float t[2];
	
	for( int i = 0; i < 2; i++ ) {
		if( useOption[i] ) {
			// avoid dividing by 0; use alternate formula for u
			if( 0 == ( A2*v[i] + B2 ) )
				u[i] = (v[i] * (C1 - C2) + D1 - D2) / (v[i] * (A2 - A1) + B2 - B1);
			else
				u[i] = -1 * (C2*v[i] + D2) / (A2*v[i] + B2);

			// u must range from 0 to 1
			if( u[i] < 0 || u[i] > 1 )
				useOption[i] = false;

			// we only want to render the edge of the bilinear patch
			if( !( u[i] < 0.1 || u[i] > 0.9 || v[i] < 0.1 || v[i] > 0.9 ) )
				useOption[i] = false;
			// these u and v coordinates are candidates for intersection!
			else {
				// find t
				t[i] = (u[i]*v[i]*x1Comp + u[i]*x2Comp + v[i]*x3Comp + x4Comp - oComp) / dComp;
				// calculated t value must be within desired range
				if( ( t[i] < tMin ) || ( t[i] > tMax ) )
					useOption[i] = false;	
			}
		}
	}

	// neither option is viable now
	if( !useOption[0] && !useOption[1] )
		return false;

	int useIndex;
	// don't use first option; must use second
	if( !useOption[0] )
		useIndex = 1;
	// don't use second option; must use first
	else if( !useOption[1] )
		useIndex = 0;
	// both options are still valid; choose the smaller of the t values
	else {
		if( t[0] < t[1] )
			useIndex = 0;
		else
			useIndex = 1;
	}

	// we now know that there is an intersection!!
	Vector3 tanU = (1 - v[useIndex])*pointB() + v[useIndex]*pointC() - (1 - v[useIndex])*pointA() - v[useIndex]*pointD();
	Vector3 tanV = (1 - u[useIndex])*pointD() + u[useIndex]*pointC() - (1 - u[useIndex])*pointA() - u[useIndex]*pointB();

	result.t = t[useIndex];
	result.P = ray.o + result.t*ray.d;
	result.N = cross(tanV, tanU);
	// normalize the normal
	result.N.normalize();
	//result.material = new Lambert(Vector3(fabs(result.N.x), fabs(result.N.y), fabs(result.N.z)));
	result.material = new Lambert(Vector3(u[useIndex],v[useIndex],1-v[useIndex]));

	BVH::intersectPrimitive();
	return true;
}
