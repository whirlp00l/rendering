#ifndef ASSIGNMENT_HELPER_H
#define ASSIGNMENT_HELPER_H

#include "Triangle.h"
#include "TriangleMesh.h"
#include "Material.h"
#include "Matrix4x4.h"
#include "Scene.h"

class AssignmentHelper
{
public:
	static void addMeshTrianglesToScene(TriangleMesh * mesh, Material * material);
	static Matrix4x4 translate(float x, float y, float z);
	static Matrix4x4 scale(float x, float y, float z);
	static Matrix4x4 rotate(float angle, float x, float y, float z);
};

#endif // ASSIGNMENT_HELPER_H