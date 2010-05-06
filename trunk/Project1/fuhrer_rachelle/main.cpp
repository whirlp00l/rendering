#include <math.h>
#include "Miro.h"
#include "Scene.h"
#include "Camera.h"
#include "Image.h"
#include "Console.h"

#include "PointLight.h"
#include "Sphere.h"
#include "Spirograph.h"
#include "TriangleMesh.h"
#include "Triangle.h"
#include "Lambert.h"
#include "MiroWindow.h"

#include "Matrix3x3.h"
#include <stdio.h>

#include "Assignment0.h"
#include "Assignment1.h"

int
main(int argc, char*argv[])
{
    // create a scene

	/*
	// ASSIGNMENT 0
	Assignment0 * assn0 = new Assignment0();
    assn0->makeSpiralScene();
	//assn0->makeSpirographScene();
	*/

	// ASSIGNMENT 1
	Assignment1 * assn1 = new Assignment1();
	assn1->makeTeapotScene();

    MiroWindow miro(&argc, argv);
    miro.mainLoop();

    return 0; // never executed
}

