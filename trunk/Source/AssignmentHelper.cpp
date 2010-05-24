#include "AssignmentHelper.h"

void
AssignmentHelper::addMeshTrianglesToScene(TriangleMesh * mesh, Material * material)
{
	// create all the triangles in the bunny mesh and add to the scene
	for (int i = 0; i < mesh->numTris(); ++i)
	{
		Triangle* t = new Triangle;
		t->setIndex(i);
		t->setMesh(mesh);
		
		if( t->getMesh()->materials() && t->getMesh()->materials()[i] )
		{
			Material * storedMaterial = t->getMesh()->materials()[i];
			t->setMaterial( storedMaterial );
		}
		else
			t->setMaterial(material); 

		g_scene->addObject(t);
	}
}

Matrix4x4
AssignmentHelper::translate(float x, float y, float z)
{
	Matrix4x4 m;
	m.setColumn4(Vector4(x, y, z, 1));
	return m;
}


Matrix4x4
AssignmentHelper::scale(float x, float y, float z)
{
	Matrix4x4 m;
	m.m11 = x;
	m.m22 = y;
	m.m33 = z;
	return m;
}

// angle is in degrees
Matrix4x4
AssignmentHelper::rotate(float angle, float x, float y, float z)
{
	float rad = angle*(PI/180.);
    
	float x2 = x*x;
	float y2 = y*y;
	float z2 = z*z;
	float c = cos(rad);
	float cinv = 1-c;
	float s = sin(rad);
	float xy = x*y;
	float xz = x*z;
	float yz = y*z;
	float xs = x*s;
	float ys = y*s;
	float zs = z*s;
	float xzcinv = xz*cinv;
	float xycinv = xy*cinv;
	float yzcinv = yz*cinv;
    
	Matrix4x4 m;
	m.set(x2 + c*(1-x2), xy*cinv+zs, xzcinv - ys, 0,
		  xycinv - zs, y2 + c*(1-y2), yzcinv + xs, 0,
		  xzcinv + ys, yzcinv - xs, z2 + c*(1-z2), 0,
		  0, 0, 0, 1);
	return m;
}