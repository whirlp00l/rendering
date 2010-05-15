#include "Triangle.h"
#include "TriangleMesh.h"
#include "Matrix3x3.h"
#include "Ray.h"
#include "BVH.h"
#include "DebugMem.h"

Triangle::Triangle(TriangleMesh * m, unsigned int i) :
    m_mesh(m), m_index(i)
{

}


Triangle::~Triangle()
{
	if( m_mesh )
	{
		// for some reason this doesn't work!!!	
		//delete m_mesh;
		//m_mesh = NULL;
	}
}


void
Triangle::renderGL()
{
    TriangleMesh::TupleI3 ti3 = m_mesh->vIndices()[m_index];
    const Vector3 & v0 = m_mesh->vertices()[ti3.x]; //vertex a of triangle
    const Vector3 & v1 = m_mesh->vertices()[ti3.y]; //vertex b of triangle
    const Vector3 & v2 = m_mesh->vertices()[ti3.z]; //vertex c of triangle

    glBegin(GL_TRIANGLES);
        glVertex3f(v0.x, v0.y, v0.z);
        glVertex3f(v1.x, v1.y, v1.z);
        glVertex3f(v2.x, v2.y, v2.z);
    glEnd();
}



bool
Triangle::intersect(HitInfo& result, const Ray& r,float tMin, float tMax)
{
	BVH::intersectPrimitive();

	TriangleMesh::TupleI3 ti3Vertices = m_mesh->vIndices()[m_index];
	const Vector3 & ptA = m_mesh->vertices()[ti3Vertices.x]; //vertex a of triangle
	const Vector3 & ptB = m_mesh->vertices()[ti3Vertices.y]; //vertex b of triangle
	const Vector3 & ptC = m_mesh->vertices()[ti3Vertices.z]; //vertex c of triangle

	TriangleMesh::TupleI3 ti3Normals = m_mesh->nIndices()[m_index];
	const Vector3 & normalPtA = m_mesh->normals()[ti3Normals.x]; //vertex a normal of triangle
	const Vector3 & normalPtB = m_mesh->normals()[ti3Normals.y]; //vertex b normal of triangle
	const Vector3 & normalPtC = m_mesh->normals()[ti3Normals.z]; //vertex c normal of triangle

	// compute intersection with Plucker coordinates, with help from http://pelopas.uop.gr/~nplatis/files/PlatisTheoharisRayTetra.pdf
	if( USE_PLUCKER_COORDS ) 
	{
		Vector3 pluckerRay[2];
		calcPluckerCoords(r.o, r.d, pluckerRay);

		Vector3 pluckerEdge0[2]; // edge0: (ptB, ptC)
		Vector3 pluckerEdge1[2]; // edge1: (ptC, ptA)
		Vector3 pluckerEdge2[2]; // edge2: (ptA, ptB)
		
		Vector3 edge0Dir = ptC - ptB;
		Vector3 edge1Dir = ptA - ptC;
		Vector3 edge2Dir = ptB - ptA;
		calcPluckerCoords( ptB, edge0Dir, pluckerEdge0 );
		calcPluckerCoords( ptC, edge1Dir, pluckerEdge1 );
		calcPluckerCoords( ptA, edge2Dir, pluckerEdge2 );

		// PIP <=> permuted inner product
		float edge0PIP = permutedInnerProduct(pluckerRay, pluckerEdge0);
		float edge1PIP = permutedInnerProduct(pluckerRay, pluckerEdge1);
		float edge2PIP = permutedInnerProduct(pluckerRay, pluckerEdge2);

		// if all permuted inner products are 0, the ray is coplaner, so no intersection
		if( edge0PIP == 0 && edge1PIP == 0 && edge2PIP == 0 )
			return false;
		// if all permuted inner products have same sign (or 1 or 2 are zero), we have an intersection
		else if( ( edge0PIP >= 0 && edge1PIP >=0 && edge2PIP >= 0 ) ||
				 ( edge0PIP <= 0 && edge1PIP <=0 && edge2PIP <= 0 ) )
		{
			float sumPIP = edge0PIP + edge1PIP + edge2PIP;
			float alpha = edge0PIP / sumPIP;
			float beta = edge1PIP / sumPIP;
			float gamma = edge2PIP / sumPIP;

			result.P = alpha*ptA + beta*ptB + gamma*ptC;

			// calculate t using largest direction component
			if( fabs(r.d.x) >= fabs(r.d.y) && fabs(r.d.x) >= fabs(r.d.z) )
				result.t = (result.P.x - r.o.x)/r.d.x;
			else if( fabs(r.d.y) >= fabs(r.d.x) && fabs(r.d.y) >= fabs(r.d.z) )
				result.t = (result.P.y - r.o.y)/r.d.y;
			else
				result.t = (result.P.z - r.o.z)/r.d.z;

			// make sure the resulting t value is okay
			if( result.t < tMin || result.t > tMax )
				return false;

			result.N = alpha*normalPtA + beta*normalPtB + gamma*normalPtC;
			// normalize the result's normal vector
			result.N.normalize();
			result.material = this->m_material;

			return true;
		}
		else
			return false;
	} // end if( USE_PLUCKER_COORDS )
	// don't use Plucker coords
	else
	{
		const float denom = dot(-r.d, cross(ptB - ptA, ptC - ptA));
		// avoid dividing by zero
		if( 0 == denom )
			return false;

		const float t = dot((r.o - ptA), cross(ptB - ptA, ptC - ptA)) / denom;
		if( t >= tMin && t <= tMax )
		{
			const float beta = (Matrix3x3(-r.d, r.o - ptA, ptC - ptA)).det() / 
						 (Matrix3x3(-r.d, ptB - ptA, ptC - ptA)).det();
			const float gamma = (Matrix3x3(-r.d, ptB - ptA, r.o - ptA)).det() /
						  (Matrix3x3(-r.d, ptB - ptA, ptC - ptA)).det();	
			const float alpha = 1 - beta - gamma;

			// make sure barycentric coordinates are within the appropriate range
			if( alpha < 0 || alpha > 1 || beta < 0 || beta > 1 || gamma < 0 || gamma > 1 || (beta + gamma) > 1 )
				return false;

			result.t = t;
			result.P = r.o + (t * r.d);
			result.N = (alpha * normalPtA) + (beta * normalPtB) + (gamma * normalPtC);
			// normalize the result's normal vector
			result.N.normalize();
			result.material = this->m_material;
			
			return true;
		}
		else
			return false;
	}
}

void 
Triangle::calcPluckerCoords(Vector3 linePt, Vector3 lineDir, Vector3 pluckerCoordsOut[2])
{
	// make sure line direction is normalized
	//lineDir.normalize();
	pluckerCoordsOut[0] = lineDir;
	pluckerCoordsOut[1] = cross(lineDir, linePt);
}

float 
Triangle::permutedInnerProduct(Vector3 pluckerR[2], Vector3 pluckerS[2])
{
	return dot(pluckerR[0], pluckerS[1]) + dot(pluckerR[1], pluckerS[0]);
}

Vector3
Triangle::getMidPoint()
{
	TriangleMesh::TupleI3 ti3Vertices = m_mesh->vIndices()[m_index];
	const Vector3 & ptA = m_mesh->vertices()[ti3Vertices.x]; //vertex a of triangle
	const Vector3 & ptB = m_mesh->vertices()[ti3Vertices.y]; //vertex b of triangle
	const Vector3 & ptC = m_mesh->vertices()[ti3Vertices.z]; //vertex c of triangle
	return ( ptA + ptB + ptC ) / 3;
}