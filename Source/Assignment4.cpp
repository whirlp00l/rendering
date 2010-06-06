#include "Assignment4.h"
#include <math.h>
#include "Miro.h"
#include "Scene.h"
#include "Camera.h"
#include "Image.h"

#include "AreaLight.h"
#include "PointLight.h"
#include "TriangleMesh.h"
#include "Triangle.h"
#include "Lambert.h"
#include "SpecularReflector.h"
#include "SpecularRefractor.h"
#include "Stone.h"
#include "DebugMem.h"

#include "AssignmentHelper.h"

Assignment4::Assignment4()
{
}

Assignment4::~Assignment4()
{
}

void
Assignment4::makeSponzaScene()
{
    g_camera = new Camera;
    g_scene = new Scene;
    g_image = new Image;

    g_image->resize(512, 512);
    
    // set up the camera
    g_camera->setBGColor(Vector3(0.0f, 0.0f, 0.2f));
    g_camera->setEye(Vector3(8, 1.5, 1));
    g_camera->setLookAt(Vector3(0, 2.5, -1));
    g_camera->setUp(Vector3(0, 1, 0));
    g_camera->setFOV(55);

    // create and place a point light source
    PointLight * light = new PointLight;
    light->setPosition(Vector3(0, 10.0, 0));
    light->setColor(Vector3(1, 1, 1));
    light->setWattage(200);
    g_scene->addLight(light);

    Material* material = new Lambert(Vector3(1.0f));
    TriangleMesh * mesh = new TriangleMesh;
    mesh->load("Resource\\sponza.obj");
    AssignmentHelper::addMeshTrianglesToScene(mesh, material);
    
    // let objects do pre-calculations if needed
    g_scene->preCalc();
}

void
Assignment4::makePondScene()
{
	g_camera = new Camera;
    g_scene = new Scene;
    g_image = new Image;

    g_image->resize(512, 512);
    
    // set up the camera
    g_camera->setBGColor(Vector3(0.0f, 0.0f, 0.2f));
    g_camera->setEye(Vector3(3, 12.5, 26));
    g_camera->setLookAt(Vector3(0, 0, 0));
    g_camera->setUp(Vector3(0, 1, 0));
    g_camera->setFOV(55);
	g_camera->setFocalPlaneDistance( 26.0f );

	// create and place a point light source
	PointLight * light = new PointLight;
	Vector3 pos( 10, 35, 30 );
	light->setPosition(pos);
    light->setColor(Vector3(1, 1, 1));
    light->setWattage(1000);
    g_scene->addLight(light);

	// transform matrix to use for object loading
	Matrix4x4 xform;
	xform.setIdentity();

	// create the water floor
	Material * waterMaterial = new SpecularReflector(Vector3(36,66,44) / 255);
    TriangleMesh * water = new TriangleMesh;
    water->createSingleTriangle();
    water->setV1(Vector3(-100, 0, -50));
    water->setV2(Vector3(   0, 0,  100));
    water->setV3(Vector3( 100, 0, -50));
    water->setN1(Vector3(0, 1, 0));
    water->setN2(Vector3(0, 1, 0));
    water->setN3(Vector3(0, 1, 0));
    
    Triangle* t = new Triangle;
    t->setIndex(0);
    t->setMesh(water);
    t->setMaterial(waterMaterial); 
    g_scene->addObject(t);

	// create large lilly pad
	xform.setIdentity();
	xform *= AssignmentHelper::scale(.2f,.2f,.2f);
	xform *= AssignmentHelper::translate(-25, 0, 10);
	xform *= AssignmentHelper::rotate(45, 0, 1, 0);
	Material* largeLillyPadMaterial = new Lambert(Vector3(0.0f,1.0f,0.0f));
    TriangleMesh * largeLillyPadMesh = new TriangleMesh;
    largeLillyPadMesh->load("Resource\\large_lilly_pad.obj", xform);
    AssignmentHelper::addMeshTrianglesToScene(largeLillyPadMesh, largeLillyPadMaterial);

	// create lilly (on top of large lilly pad)
	xform.setIdentity();
	xform *= AssignmentHelper::scale(.2f,.2f,.2f);
	xform *= AssignmentHelper::translate(-25, 0, 10);
	Material* lillyMaterial = new Lambert(Vector3(1.0f,0.0f,0.5f));
    TriangleMesh * lillyMesh = new TriangleMesh;
    lillyMesh->load("Resource\\lilly.obj", xform);
    AssignmentHelper::addMeshTrianglesToScene(lillyMesh, lillyMaterial);

	// create small lilly pad (in foreground)
	xform.setIdentity();
	xform *= AssignmentHelper::scale(.2f,.2f,.2f);
	xform *= AssignmentHelper::translate(-27,0,70);
	Material* smallLillyPadMaterial = new Lambert(Vector3(0.0f,1.0f,1.0f));
    TriangleMesh * smallLillyPadMesh = new TriangleMesh;
    smallLillyPadMesh->load("Resource\\small_lilly_pad.obj", xform);
    AssignmentHelper::addMeshTrianglesToScene(smallLillyPadMesh, smallLillyPadMaterial);

	// create small lilly pad (in background near background cattails)
	xform.setIdentity();
	xform *= AssignmentHelper::scale(.15f,.15f,.15f);
	xform *= AssignmentHelper::translate(-100,0,5);
	xform *= AssignmentHelper::rotate(80, 0, 1, 0);
	Material* smallLillyPadMaterial2 = new Lambert(Vector3(0.0f,1.0f,1.0f));
    TriangleMesh * smallLillyPadMesh2 = new TriangleMesh;
    smallLillyPadMesh2->load("Resource\\small_lilly_pad.obj", xform);
    AssignmentHelper::addMeshTrianglesToScene(smallLillyPadMesh2, smallLillyPadMaterial2);

	// create foreground cattail mesh
	xform.setIdentity();
	xform *= AssignmentHelper::scale(.18f,.18f,.18f);
	xform *= AssignmentHelper::translate(35,0,95);
	xform *= AssignmentHelper::rotate(90, 0, 1, 0);
    Material* cattailMaterial = new Lambert(Vector3(1.0f,0.0f,0.0f));
    TriangleMesh * cattailMesh = new TriangleMesh;
    cattailMesh->load("Resource\\cattails.obj", xform);
    AssignmentHelper::addMeshTrianglesToScene(cattailMesh, cattailMaterial);

	// create background cattail mesh
	xform.setIdentity();
	xform *= AssignmentHelper::scale(.18f,.18f,.18f);
	xform *= AssignmentHelper::translate(-90,-5,-20);
	xform *= AssignmentHelper::rotate(225, 0, 1, 0);
    Material* cattailMaterial2 = new Lambert(Vector3(1.0f,0.0f,0.0f));
    TriangleMesh * cattailMesh2 = new TriangleMesh;
    cattailMesh2->load("Resource\\cattails.obj", xform);
    AssignmentHelper::addMeshTrianglesToScene(cattailMesh2, cattailMaterial2);
/*
	// create pond flowers mesh (the large, full one)
    xform.setIdentity();
	xform *= AssignmentHelper::scale(.3f,.3f,.3f);
	xform *= AssignmentHelper::translate(20,0,-65);
    Material* pondFlowersMaterial = new Lambert(Vector3(1.0f,1.0f,0.0f));
    TriangleMesh * pondFlowersMesh = new TriangleMesh;
    pondFlowersMesh->load("Resource\\pond_flowers.obj", xform);
    AssignmentHelper::addMeshTrianglesToScene(pondFlowersMesh, pondFlowersMaterial);

	// create small pond flowers mesh (fills in space between other pond flowers and left background weeds)
	xform.setIdentity();
	xform *= AssignmentHelper::scale(.3f,.3f,.3f);
	xform *= AssignmentHelper::translate(-30,0,-135);
    Material* smallPondFlowersMaterial = new Lambert(Vector3(1.0f,0.0f,1.0f));
    TriangleMesh * smallPondFlowersMesh = new TriangleMesh;
    smallPondFlowersMesh->load("Resource\\small_pond_flowers.obj", xform);
    AssignmentHelper::addMeshTrianglesToScene(smallPondFlowersMesh, smallPondFlowersMaterial);
*/

	// create left background weeds
	xform.setIdentity();
	xform *= AssignmentHelper::scale(.55f,.4f,.45f);
	xform *= AssignmentHelper::translate(-40,0,-60);
    Material* leftWeedsMaterial = new Lambert(Vector3(1.0f,0.0f,0.0f));
    TriangleMesh * leftWeedsMesh = new TriangleMesh;
    leftWeedsMesh->load("Resource\\thin_water_weeds.obj", xform);
    AssignmentHelper::addMeshTrianglesToScene(leftWeedsMesh, leftWeedsMaterial);

	// create middle background weeds
	xform.setIdentity();
	xform *= AssignmentHelper::scale(.75f,.35f,.45f);
	xform *= AssignmentHelper::translate(0,0,-80);
    Material* middleWeedsMaterial = new Lambert(Vector3(1.0f,0.0f,0.0f));
    TriangleMesh * middleWeedsMesh = new TriangleMesh;
    middleWeedsMesh->load("Resource\\thin_water_weeds.obj", xform);
    AssignmentHelper::addMeshTrianglesToScene(middleWeedsMesh, middleWeedsMaterial);

	// create right background weeds
	xform.setIdentity();
	xform *= AssignmentHelper::scale(.45f,.45f,.45f);
	xform *= AssignmentHelper::translate(45,0,-85);
    Material* rightWeedsMaterial = new Lambert(Vector3(1.0f,0.0f,0.0f));
    TriangleMesh * rightWeedsMesh = new TriangleMesh;
    rightWeedsMesh->load("Resource\\small_water_weeds.obj", xform);
    AssignmentHelper::addMeshTrianglesToScene(rightWeedsMesh, rightWeedsMaterial);

    // let objects do pre-calculations if needed
    g_scene->preCalc();
}