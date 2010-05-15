#include "Miro.h"
#include "Scene.h"
#include "Camera.h"
#include "Image.h"
#include "Console.h"
#include "DebugMem.h"

#include <windows.h>
#include <time.h>

Scene * g_scene = 0;

Scene::Scene()
{
	m_num_rays_traced = 0;
}

Scene::~Scene()
{
	m_bvh.~BVH();
	
	// objects are deleted by the bvh destructor
	m_objects.clear();

	for( unsigned int i = 0; i < m_lights.size(); i++ )
	{
		if( m_lights[i] )
		{
			delete m_lights[i];
			m_lights[i] = NULL;
		}
	}
	m_lights.clear();
}

void
Scene::openGL(Camera *cam)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    cam->drawGL();

    // draw objects
	if( VIEW_BOUNDING_VOLUMES )
		m_bvh.renderGL();
	else 
	{
		for (size_t i = 0; i < m_objects.size(); ++i)
			m_objects[i]->renderGL();
	}
	

    glutSwapBuffers();
}

void
Scene::preCalc()
{
    Objects::iterator it;
    for (it = m_objects.begin(); it != m_objects.end(); it++)
    {
        Object* pObject = *it;
        pObject->preCalc();
    }
    Lights::iterator lit;
    for (lit = m_lights.begin(); lit != m_lights.end(); lit++)
    {
        PointLight* pLight = *lit;
        pLight->preCalc();
    }

    m_bvh.build(&m_objects);
}

void
Scene::raytraceImage(Camera *cam, Image *img)
{
    Ray ray;
    HitInfo hitInfo;
    Vector3 shadeResult;

	BVH::resetIntersections();

	/*
	SYSTEMTIME locStartTime;

	GetLocalTime(&locStartTime);

	printf( "\nRendering start time: %02d:%02d:%02d:%02d\n", locStartTime.wHour, locStartTime.wMinute, 
		locStartTime.wSecond, locStartTime.wMilliseconds );
	*/    
	clock_t clockStart = clock();

	m_num_rays_traced = 0;
    // loop over all pixels in the image
    for (int j = 0; j < img->height(); ++j)
    {
        for (int i = 0; i < img->width(); ++i)
        {
            ray = cam->eyeRay(i, j, img->width(), img->height());
            if (trace(hitInfo, ray))
            {
                shadeResult = hitInfo.material->shade(ray, hitInfo, *this);
                img->setPixel(i, j, shadeResult);
            }
        }

        img->drawScanline(j);
        glFinish();
        printf("Rendering Progress: %.3f%%\r", j/float(img->height())*100.0f);
        fflush(stdout);
    }

	/*
	SYSTEMTIME locEndTime;

	GetLocalTime(&locEndTime);
	*/
	clock_t clockEnd = clock();
    
    printf("Rendering Progress: 100.000%\n");
    debug("done Raytracing!\n");
	/*
	printf( "Rendering end time: %02d:%02d:%02d:%02d\n", locEndTime.wHour, locEndTime.wMinute, 
		locEndTime.wSecond, locEndTime.wMilliseconds );
	*/

	printf("Rendering statistics:\n");	
	printf("\tTotal render time: %.4f seconds\n", ((float)(clockEnd - clockStart))/CLOCKS_PER_SEC);
	printf("\t%d BVH nodes (includes # leaves)\n", m_bvh.numNodes() );
	printf("\t\t(up to %d child(ren) per node)\n", NUM_NODE_CHILDREN );
	printf("\t%d BVH leaves\n", m_bvh.numLeaves() );
	printf("\t\t(up to %d primitive(s) per leaf)\n", NUM_LEAF_CHILDREN );
	printf("\t%d rays\n", m_num_rays_traced);
	printf("\t%d ray <=> bounding volume intersections\n", BVH::BoundingVolumeIntersections());
	printf("\t%d ray <=> primitive intersections\n", BVH::PrimitiveIntersections());
	printf("\t%.4f average triangle intersections per ray\n", BVH::PrimitiveIntersections()/(float)m_num_rays_traced);
	printf("\t%.4f average bounding volume intersections per ray\n", BVH::BoundingVolumeIntersections()/(float)m_num_rays_traced);
	printf("\n");
}

bool
Scene::trace(HitInfo& minHit, const Ray& ray, float tMin, float tMax)
{
	m_num_rays_traced++; // one more ray has been traced
    return m_bvh.intersect(minHit, ray, tMin, tMax);
}
