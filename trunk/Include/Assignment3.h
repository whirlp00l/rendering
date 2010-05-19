#ifndef CSE168_ASSIGNMENT_3_H_INCLUDED
#define CSE168_ASSIGNMENT_3_H_INCLUDED

#include "Material.h"

class Assignment3
{
public:
	Assignment3();
	virtual ~Assignment3();

	void makeCornellScene();
	void makeCornellSceneWithSpheres();
	void makeTeapotScene( Material::Type teapotMaterialType );
	void makeMultipleTeapotsScene();
	void make3TeapotsScene();
	void makeSphereScene();
};

#endif // CSE168_ASSIGNMENT_3_H_INCLUDED