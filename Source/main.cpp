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

#include "DebugMem.h"
#include "Assignment0.h"
#include "Assignment1.h"
#include "Assignment2.h"
#include "Assignment3.h"

#include <stdio.h>

#define ASSIGNMENT_NUMBER 3

int
main(int argc, char*argv[])
{
    // create a scene

	Assignment0 *assn0;
	Assignment1 *assn1;
	Assignment2 *assn2;
	Assignment3 *assn3;

	switch( ASSIGNMENT_NUMBER )
	{
	case 0:
		// ASSIGNMENT 0
		assn0 = new Assignment0();
		assn0->makeSpiralScene();
		//assn0->makeSpirographScene();
		delete assn0;
		assn0 = NULL;
		break;

	case 1:
		// ASSIGNMENT 1
		assn1 = new Assignment1();
		assn1->makeSimpleTriangleScene();
		delete assn1;
		assn1 = NULL;
		break;

	case 2:
		// ASSIGNMENT 2
		assn2 = new Assignment2();
		assn2->makeSponzaScene();
		delete assn2;
		assn2 = NULL;
		break;

	case 3:
		// ASSIGNMENT 3
		assn3 = new Assignment3();
		assn3->make3TeapotsScene();
		//assn3->makeTeapotScene(Material::DIFFUSE);
		delete assn3;
		assn3 = NULL;
		break;
		
	default:
		printf( "ERROR: Don't know how to build assignment %d!!!\n\n", ASSIGNMENT_NUMBER );
		// make an empty scene so we don't get a seg fault
		g_camera = new Camera;
		g_scene = new Scene;
		g_image = new Image;

		g_image->resize(512, 512);
	    
		// set up the camera
		g_camera->setBGColor(Vector3(0.0f, 0.0f, 0.2f));
		g_camera->setEye(Vector3(0, 3, 6));
		g_camera->setLookAt(Vector3(0, 0, 0));
		g_camera->setUp(Vector3(0, 1, 0));
		g_camera->setFOV(45);

		// create and place a point light source
		PointLight * light = new PointLight;
		light->setPosition(Vector3(10, 10, 10));
		light->setColor(Vector3(1, 1, 1));
		light->setWattage(700);
		g_scene->addLight(light);

		// let objects do pre-calculations if needed
		g_scene->preCalc();
	}

    MiroWindow miro(&argc, argv);
    miro.mainLoop();

    return 0; // never executed
}

