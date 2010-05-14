#ifndef CSE168_ASSIGNMENT_3_H_INCLUDED
#define CSE168_ASSIGNMENT_3_H_INCLUDED

#include "Material.h"

class Assignment3
{
public:
	Assignment3();
	virtual ~Assignment3();

	void makeCornellScene();
	void makeTeapotScene( Material::Type teapotMaterialType );
};

#endif // CSE168_ASSIGNMENT_3_H_INCLUDED