#include "Assignment3.h"
#include <math.h>
#include "Miro.h"
#include "Scene.h"
#include "Camera.h"
#include "Image.h"

#include "PointLight.h"
#include "TriangleMesh.h"
#include "Triangle.h"
#include "Lambert.h"
#include "PhongLambert.h"
#include "SpecularReflector.h"
#include "SpecularRefractor.h"

// local helper function declarations
namespace
{
	void addMeshTrianglesToScene(TriangleMesh * mesh, Material * material);
	inline Matrix4x4 translate(float x, float y, float z);
	inline Matrix4x4 scale(float x, float y, float z);
	inline Matrix4x4 rotate(float angle, float x, float y, float z);
} // namespace

Assignment3::Assignment3()
{
}

Assignment3::~Assignment3()
{
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


	Material * teapotMaterial;
	switch( teapotMaterialType )
	{
	case Material::DIFFUSE:
		teapotMaterial = new Lambert( Vector3(1,0,0) );
		break;
	case Material::PHONG_DIFFUSE:
		teapotMaterial = new PhongLambert( Vector3(1,0,0) );
		break;
	case Material::SPECULAR_REFLECTOR:
		teapotMaterial = new SpecularReflector();
		break;
	case Material::SPECULAR_REFRACTOR:
		float refractiveIndex = SpecularRefractor::getRefractiveIndex( SpecularRefractor::GLASS_COMMON );
		teapotMaterial = new SpecularRefractor( refractiveIndex );
		break;
	}

    TriangleMesh * teapot = new TriangleMesh;
	Matrix4x4 transform;
	transform.setIdentity();
	//transform *= rotate(45,0,1,0);
    teapot->load("Resource\\teapot.obj", transform);
	addMeshTrianglesToScene(teapot, teapotMaterial);

    // create the floor triangle
    TriangleMesh * floor = new TriangleMesh;
    floor->createSingleTriangle();
    floor->setV1(Vector3(-10, 0, -10));
    floor->setV2(Vector3(  0, 0,  10));
    floor->setV3(Vector3( 10, 0, -10));
    floor->setN1(Vector3(0, 1, 0));
    floor->setN2(Vector3(0, 1, 0));
    floor->setN3(Vector3(0, 1, 0));
    
	Material* floorMaterial = new Lambert(Vector3(1.0f));
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
	transform *= translate( -2, 0, 0 );
	transform *= rotate( -45, 0, 1, 0 );
    diffuseTeapot->load("Resource\\teapot.obj", transform);
    addMeshTrianglesToScene(diffuseTeapot, diffuseMaterial);

	Material * specularMaterial = new SpecularReflector(Vector3(1));
	TriangleMesh * specularTeapot = new TriangleMesh;
	transform.setIdentity();
	transform *= translate( 2, 0, 0 );
	transform *= rotate( 45, 0, 1, 0 );
	specularTeapot->load("Resource\\teapot.obj", transform);
	addMeshTrianglesToScene(specularTeapot, specularMaterial);

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
    
    // TODO: replace this point light with the Area light that you implement for Task 3
    PointLight * light = new PointLight;
    light->setPosition(Vector3(2.75, 5.51, -2.75));
    light->setColor(Vector3(1, 1, 1));
    light->setWattage(50);
    g_scene->addLight(light);
    
    Material* material = new Lambert(Vector3(0.5f));
    TriangleMesh * box = new TriangleMesh;
    box->load("Resource\\cornell_box.obj");
    addMeshTrianglesToScene(box, material);

    // let objects do pre-calculations if needed
    g_scene->preCalc();
}

void
Assignment3::makeTeapot2Scene()
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
    xform *= translate(-1, 0, -1);
    xform *= rotate(25, 0, 0, 0);
    mesh = new TriangleMesh;
    mesh->load("Resource\\teapot.obj", xform);
    addMeshTrianglesToScene(mesh, material);
    
    // teapot 2
	//material = new SpecularRefractor(SpecularRefractor::getRefractiveIndex(SpecularRefractor::GLASS_COMMON));
	material = new SpecularReflector();
    xform.setIdentity();
    xform *= translate(2.0, 0, 3);
    mesh = new TriangleMesh;
    mesh->load("Resource\\teapot.obj", xform);
    addMeshTrianglesToScene(mesh, material);
    
    // teapot 3   
    material = new Lambert(Vector3(0.0f, 1.0f, 1.0f));
    xform.setIdentity();
    xform *= translate(-2, 0, 4);
    xform *= rotate(45, 0, 1, 0);
    mesh = new TriangleMesh;
    mesh->load("Resource\\teapot.obj", xform);
    addMeshTrianglesToScene(mesh, material);
    
    // create the floor triangle
	material = new Lambert(Vector3(0,1,0));
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

// local helper function definitions
namespace
{
	void
	addMeshTrianglesToScene(TriangleMesh * mesh, Material * material)
	{
		// create all the triangles in the bunny mesh and add to the scene
		for (int i = 0; i < mesh->numTris(); ++i)
		{
			Triangle* t = new Triangle;
			t->setIndex(i);
			t->setMesh(mesh);
			t->setMaterial(material); 
			g_scene->addObject(t);
		}
	}

	inline Matrix4x4
	translate(float x, float y, float z)
	{
		Matrix4x4 m;
		m.setColumn4(Vector4(x, y, z, 1));
		return m;
	}


	inline Matrix4x4
	scale(float x, float y, float z)
	{
		Matrix4x4 m;
		m.m11 = x;
		m.m22 = y;
		m.m33 = z;
		return m;
	}

	// angle is in degrees
	inline Matrix4x4
	rotate(float angle, float x, float y, float z)
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


} // namespace

