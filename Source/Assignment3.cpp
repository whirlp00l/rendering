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
#include "Specular.h"

// local helper function declarations
namespace
{
void addMeshTrianglesToScene(TriangleMesh * mesh, Material * material);
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
	case Material::SPECULAR:
		teapotMaterial = new Specular();
		break;
	}

    TriangleMesh * teapot = new TriangleMesh;
    teapot->load("Resource\\teapot.obj");
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

} // namespace

