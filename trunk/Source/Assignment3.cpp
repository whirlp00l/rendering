#include "Assignment3.h"
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
#include "Sand.h"
#include "Stone.h"
#include "DebugMem.h"

#include "AssignmentHelper.h"

Assignment3::Assignment3()
{
}

Assignment3::~Assignment3()
{
}

void
Assignment3::makeSphereScene()
{
	 g_camera = new Camera;
    g_scene = new Scene;
    g_image = new Image;
    
    //g_image->resize(128, 128);
    g_image->resize(512, 512);
    
    // set up the camera
    g_camera->setBGColor(Vector3(0.0f, 0.0f, 0.2f));
    g_camera->setEye(Vector3(0, 2.5, 10));
    g_camera->setLookAt(Vector3(0, 0, 0));
    g_camera->setUp(Vector3(0, 1, 0));
    g_camera->setFOV(45);
    
    // create and place a point light source
    PointLight * light = new PointLight;
    light->setPosition(Vector3(10, 20, 10));
    light->setColor(Vector3(1, 1, 1));
    light->setWattage(1000);
    g_scene->addLight(light);
    
    TriangleMesh * mesh;  
    Matrix4x4 xform;

    // sphere 1   
	Material* material = new SpecularReflector(Vector3(0.0f, 0.0f, 1.0f));
    xform.setIdentity();
	xform *= AssignmentHelper::translate(-1.0, 1.0, -0.5);
    mesh = new TriangleMesh;
    mesh->load("Resource\\sphere_high_res.obj", xform);
	AssignmentHelper::addMeshTrianglesToScene(mesh, material);
    
    // sphere 2
	//material = new Lambert(Vector3(1,0,0));
	material = new SpecularReflector(Vector3(1.0f, 0.0f, 1.0f));
    xform.setIdentity();
	xform *= AssignmentHelper::translate(1.5, 1, 1.5);
    mesh = new TriangleMesh;
    mesh->load("Resource\\sphere_high_res.obj", xform);
    AssignmentHelper::addMeshTrianglesToScene(mesh, material);
    
    // sphere 3   
    //material = new SpecularReflector(Vector3(0.0f, 1.0f, 1.0f));
	float index = SpecularRefractor::getRefractiveIndex(SpecularRefractor::GLASS_COMMON);
	material = new SpecularRefractor(index);
    xform.setIdentity();
	xform *= AssignmentHelper::translate(-1.5, 1, 2);
    mesh = new TriangleMesh;
    mesh->load("Resource\\sphere_high_res.obj", xform);
    AssignmentHelper::addMeshTrianglesToScene(mesh, material);
    
    // create the floor triangle
	material = new Lambert(Vector3(1));
    mesh = new TriangleMesh;
    mesh->createSingleTriangle();
    mesh->setV1(Vector3(-100, 0, -100));
    mesh->setV2(Vector3(   0, 0,  100));
    mesh->setV3(Vector3( 100, 0, -100));
    mesh->setN1(Vector3(0, 1, 0));
    mesh->setN2(Vector3(0, 1, 0));
    mesh->setN3(Vector3(0, 1, 0));
    
    Triangle* t = new Triangle;
    t->setIndex(0);
    t->setMesh(mesh);
    t->setMaterial(material); 
    g_scene->addObject(t);
    
    // let objects do pre-calculations if needed
    g_scene->preCalc();
}

void
Assignment3::makeTeapotScene( Material::Type teapotMaterialType )
{
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

	/*
	PointLight * light = new AreaLight(Vector3(10,10,10), Vector3(0,0,2), Vector3(2,0,0));
    light->setColor(Vector3(1, 1, 1));
    light->setWattage(700);
    g_scene->addLight(light);
	*/

	Material * teapotMaterial;
	switch( teapotMaterialType )
	{
	case Material::DIFFUSE:
		teapotMaterial = new Lambert( Vector3(1,0,0) );
		break;
	case Material::SPECULAR_REFLECTOR:
		teapotMaterial = new SpecularReflector();
		break;
	case Material::STONE:
		teapotMaterial = new Stone(Stone::COLORFUL, 10);
		break;
	case Material::SPECULAR_REFRACTOR:
		float refractiveIndex = SpecularRefractor::getRefractiveIndex( SpecularRefractor::WATER_20_C );
		teapotMaterial = new SpecularRefractor( refractiveIndex );
		break;
	}

	teapotMaterial->setPhongExp( 50.0f );
	//teapotMaterial->setUseBumpMap( true );

    TriangleMesh * teapot = new TriangleMesh;
	Matrix4x4 transform;
	transform.setIdentity();
	//transform *= rotate(45,0,1,0);
    teapot->load("Resource\\teapot.obj", transform);
	AssignmentHelper::addMeshTrianglesToScene(teapot, teapotMaterial);

    // create the floor triangle
    TriangleMesh * floor = new TriangleMesh;
    floor->createSingleTriangle();
    floor->setV1(Vector3(-10, 0, -10));
    floor->setV2(Vector3(  0, 0,  10));
    floor->setV3(Vector3( 10, 0, -10));
    floor->setN1(Vector3(0, 1, 0));
    floor->setN2(Vector3(0, 1, 0));
    floor->setN3(Vector3(0, 1, 0));
    
	Material* floorMaterial = new Stone(Stone::COLORFUL);
	floorMaterial->setUseBumpMap( true );
	//Material * floorMaterial = new Lambert(Vector3(1.0f));
    Triangle* t = new Triangle;
    t->setIndex(0);
    t->setMesh(floor);
    t->setMaterial(floorMaterial); 
    g_scene->addObject(t);
    
    // let objects do pre-calculations if needed
    g_scene->preCalc();
}

void
Assignment3::makeMultipleTeapotsScene()
{
    g_camera = new Camera;
    g_scene = new Scene;
    g_image = new Image;

    g_image->resize(512, 512);
    
    // set up the camera
    g_camera->setBGColor(Vector3(0.0f, 0.0f, 0.2f));
    g_camera->setEye(Vector3(0, 6, 12));
    g_camera->setLookAt(Vector3(0, 0, 0));
    g_camera->setUp(Vector3(0, 1, 0));
    g_camera->setFOV(45);

    // create and place a point light source
    PointLight * light = new PointLight;
    light->setPosition(Vector3(10, 10, 10));
    light->setColor(Vector3(1, 1, 1));
    light->setWattage(700);
    g_scene->addLight(light);

	Matrix4x4 transform;
	
    Material* diffuseMaterial = new Lambert(Vector3(1,0,1));
    TriangleMesh * diffuseTeapot = new TriangleMesh;
	transform.setIdentity();
	transform *= AssignmentHelper::translate( -2, 0, 0 );
	transform *= AssignmentHelper::rotate( -45, 0, 1, 0 );
    diffuseTeapot->load("Resource\\teapot.obj", transform);
    AssignmentHelper::addMeshTrianglesToScene(diffuseTeapot, diffuseMaterial);

	Material * specularMaterial = new SpecularReflector(Vector3(1));
	TriangleMesh * specularTeapot = new TriangleMesh;
	transform.setIdentity();
	transform *= AssignmentHelper::translate( 2, 0, 0 );
	transform *= AssignmentHelper::rotate( 45, 0, 1, 0 );
	specularTeapot->load("Resource\\teapot.obj", transform);
	AssignmentHelper::addMeshTrianglesToScene(specularTeapot, specularMaterial);

    // create the floor triangle
    TriangleMesh * floor = new TriangleMesh;
    floor->createSingleTriangle();
    floor->setV1(Vector3(-10, 0, -10));
    floor->setV2(Vector3(  0, 0,  10));
    floor->setV3(Vector3( 10, 0, -10));
    floor->setN1(Vector3(0, 1, 0));
    floor->setN2(Vector3(0, 1, 0));
    floor->setN3(Vector3(0, 1, 0));
    
	Material * floorMaterial = new Lambert(Vector3(1.0f));
    Triangle* t = new Triangle;
    t->setIndex(0);
    t->setMesh(floor);
    t->setMaterial(floorMaterial); 
    g_scene->addObject(t);
    
    // let objects do pre-calculations if needed
    g_scene->preCalc();
}

void
Assignment3::makeCornellScene()
{
    g_camera = new Camera;
    g_scene = new Scene;
    g_image = new Image;
    
    g_image->resize(512, 512);
    
    // set up the camera
    g_camera->setBGColor(Vector3(0.0f, 0.0f, 0.2f));
    g_camera->setEye(Vector3(2.75, 2.75, 8.75));
    g_camera->setLookAt(Vector3(2.75, 2.75, 0));
    g_camera->setUp(Vector3(0, 1, 0));
    g_camera->setFOV(35);
    
	PointLight * light = new AreaLight(Vector3(2.75, 5.51, 2.5), Vector3(0,0,4), Vector3(4,0,0));
    light->setColor(Vector3(1, 1, 1));
    light->setWattage(50);
    g_scene->addLight(light);

	/*
	PointLight * light2 = new AreaLight(Vector3(4.75, 5.51, 5), Vector3(0,0,4), Vector3(4,0,0));
	light2->setColor(Vector3(1, 1, 1));
    light2->setWattage(50);
    g_scene->addLight(light2);
	*/
    
    Material* material = new Lambert(Vector3(0.5f));
	//Material * material = new Stone(Stone::COLORFUL, 1.2);
    TriangleMesh * box = new TriangleMesh;
    box->load("Resource\\cornell_box.obj");
    AssignmentHelper::addMeshTrianglesToScene(box, material);

    // let objects do pre-calculations if needed
    g_scene->preCalc();
}

void
Assignment3::makeCornellSceneWithSpheres()
{
    g_camera = new Camera;
    g_scene = new Scene;
    g_image = new Image;
    
    g_image->resize(512, 512);
    
    // set up the camera
    g_camera->setBGColor(Vector3(0.0f, 0.0f, 0.2f));
    g_camera->setEye(Vector3(2.75, 2.75, 8.75));
    g_camera->setLookAt(Vector3(2.75, 2.75, 0));
    g_camera->setUp(Vector3(0, 1, 0));
    g_camera->setFOV(35);
    
	PointLight * light = new AreaLight(Vector3(2.75, 5.51, 2.5), Vector3(0,0,4), Vector3(4,0,0));
    light->setColor(Vector3(1, 1, 1));
    light->setWattage(50);
    g_scene->addLight(light);

	PointLight * light2 = new AreaLight(Vector3(4.75, 5.51, 5), Vector3(0,0,4), Vector3(4,0,0));
	light2->setColor(Vector3(1, 1, 1));
    light2->setWattage(50);
    g_scene->addLight(light2);
    
    Material* material = new Lambert(Vector3(0.5f));
    TriangleMesh * box = new TriangleMesh;
    box->load("Resource\\empty_cornell_box.obj");
    AssignmentHelper::addMeshTrianglesToScene(box, material);

	TriangleMesh * mesh;
	Matrix4x4 xform;

	// sphere 1   
	material = new SpecularReflector(Vector3(1.0f));
    xform.setIdentity();
    xform *= AssignmentHelper::translate(1.5, 1.0, -3.5);
    mesh = new TriangleMesh;
    mesh->load("Resource\\sphere_high_res.obj", xform);
    AssignmentHelper::addMeshTrianglesToScene(mesh, material);
    
    // sphere 2
	material = new Lambert(Vector3(0.75,0,1.0));
    xform.setIdentity();
    xform *= AssignmentHelper::translate(4.0, 1.0, -1.5);
    mesh = new TriangleMesh;
    mesh->load("Resource\\sphere_high_res.obj", xform);
    AssignmentHelper::addMeshTrianglesToScene(mesh, material);

    // let objects do pre-calculations if needed
    g_scene->preCalc();
}

void
Assignment3::make3TeapotsScene()
{
    g_camera = new Camera;
    g_scene = new Scene;
    g_image = new Image;
    
    //g_image->resize(128, 128);
    g_image->resize(512, 512);
    
    // set up the camera
    g_camera->setBGColor(Vector3(0.0f, 0.0f, 0.2f));
    g_camera->setEye(Vector3(0, 5, 15));
    g_camera->setLookAt(Vector3(0, 0, 0));
    g_camera->setUp(Vector3(0, 1, 0));
    g_camera->setFOV(45);
    
    // create and place a point light source
    PointLight * light = new PointLight;
    light->setPosition(Vector3(10, 20, 10));
    light->setColor(Vector3(1, 1, 1));
    light->setWattage(1000);
    g_scene->addLight(light);
    
    TriangleMesh * mesh;  
    Matrix4x4 xform;

    // teapot 1   
	Material* material = new SpecularReflector(Vector3(0.0f, 0.0f, 1.0f));
    xform.setIdentity();
    xform *= AssignmentHelper::translate(-1, 0, -1);
    xform *= AssignmentHelper::rotate(25, 0, 0, 0);
    mesh = new TriangleMesh;
    mesh->load("Resource\\teapot.obj", xform);
    AssignmentHelper::addMeshTrianglesToScene(mesh, material);
    
    // teapot 2
	material = new SpecularReflector();
    xform.setIdentity();
    xform *= AssignmentHelper::translate(2.0, 0, 3);
    mesh = new TriangleMesh;
    mesh->load("Resource\\teapot.obj", xform);
    AssignmentHelper::addMeshTrianglesToScene(mesh, material);
    
    // teapot 3   
    //material = new SpecularReflector(Vector3(0.0f, 1.0f, 1.0f));
	float index = SpecularRefractor::getRefractiveIndex( SpecularRefractor::GLASS_COMMON );
	material = new SpecularRefractor(index);
	material->setPhongExp( 50 );
    xform.setIdentity();
    xform *= AssignmentHelper::translate(-2, 0, 4);
    xform *= AssignmentHelper::rotate(45, 0, 1, 0);
    mesh = new TriangleMesh;
    mesh->load("Resource\\teapot.obj", xform);
    AssignmentHelper::addMeshTrianglesToScene(mesh, material);
    
    // create the floor triangle
	material = new Stone(Stone::COLORFUL, 2);
    mesh = new TriangleMesh;
    mesh->createSingleTriangle();
    mesh->setV1(Vector3(-100, 0, -100));
    mesh->setV2(Vector3(   0, 0,  100));
    mesh->setV3(Vector3( 100, 0, -100));
    mesh->setN1(Vector3(0, 1, 0));
    mesh->setN2(Vector3(0, 1, 0));
    mesh->setN3(Vector3(0, 1, 0));
    
    Triangle* t = new Triangle;
    t->setIndex(0);
    t->setMesh(mesh);
    t->setMaterial(material); 
    g_scene->addObject(t);
    
    // let objects do pre-calculations if needed
    g_scene->preCalc();
}