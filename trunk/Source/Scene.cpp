#include "Miro.h"
#include "Scene.h"
#include "Camera.h"
#include "Image.h"
#include "Console.h"
#include "PFMLoader.h"
#include "DebugMem.h"
#include "EnvironmentMap.h"
#include "AreaLight.h"

#include <windows.h>
#include <time.h>
#include <stdlib.h>

Scene * g_scene = 0;

Scene::Scene() : m_environment_map(0), m_map_width(0), m_map_height(0)
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
	// we are using an environment map!
	if( USE_ENVIRONMENT_MAP )
	{
		m_environment_map = PFMLoader::readPFMImage( ENVIRONMENT_MAP_FILE_NAME, &m_map_width, &m_map_height );
	}

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

	// seed randomizer for path tracing
	if( USE_PATH_TRACING )
		srand((unsigned)time(0));

	clock_t clockStart = clock();

	g_scene->m_num_rays_traced = 0;
    // loop over all pixels in the image
    for (int j = 0; j < img->height(); ++j)
    {
        for (int i = 0; i < img->width(); ++i)
        {		
			ray = cam->eyeRay(i, j, img->width(), img->height());
			if (trace(hitInfo, ray))
			{
				shadeResult = hitInfo.material->shade(ray, hitInfo, *this);

				// incorporate indirect lighting
				if( hitInfo.material->isDiffuse() && USE_PATH_TRACING )
				{	
					// add in the indirect lighting result (only use a fraction of it in the final color)
					shadeResult += getIndirectLight( hitInfo ) * Vector3(0.5f);
				} // end indirect lighting
			}
			else
			{
				if( USE_ENVIRONMENT_MAP && this->environmentMap() )
				{
					shadeResult = EnvironmentMap::lookUp( ray.d, this->environmentMap(), this->mapWidth(), this->mapHeight() );
				}
			}

			

			// now actually set the pixel color
			img->setPixel(i, j, shadeResult);
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
	printf("\t%d rays\n", g_scene->m_num_rays_traced);
	printf("\t%d ray <=> bounding volume intersections\n", BVH::BoundingVolumeIntersections());
	printf("\t%d ray <=> primitive intersections\n", BVH::PrimitiveIntersections());
	printf("\t%.4f average triangle intersections per ray\n", BVH::PrimitiveIntersections()/(float)g_scene->m_num_rays_traced);
	printf("\t%.4f average bounding volume intersections per ray\n", BVH::BoundingVolumeIntersections()/(float)g_scene->m_num_rays_traced);
	printf("\n");
}

bool
Scene::trace(HitInfo& minHit, const Ray& ray, float tMin, float tMax) const
{
	g_scene->m_num_rays_traced++; // one more ray has been traced
    return m_bvh.intersect(minHit, ray, tMin, tMax);
}

Vector3
Scene::getIndirectLight( const HitInfo hitInfo )
{
	Vector3 indirectLighting(0,0,0);
	Ray indirectLightingRay;
	HitInfo indirectLightingHit; 
	
	for( int k = 0; k < NUM_SAMPLES_PER_PIXEL; k++ )
	{
		// sample indirect lighting here
		float x = rand() / static_cast<double>(RAND_MAX);
		float y = rand() / static_cast<double>(RAND_MAX);
		float z = rand() / static_cast<double>(RAND_MAX);

		// since rand() only generates values between 0 adn RAND_MAX,
		// we must randomize whether or not this value is negative
		float posOrNeg = rand() / static_cast<double>(RAND_MAX);
		if( posOrNeg < 0.5 )
			x *= -1;
		posOrNeg = rand() / static_cast<double>(RAND_MAX);
		if( posOrNeg < 0.5 )
			y *= -1;
		posOrNeg = rand() / static_cast<double>(RAND_MAX);
		if( posOrNeg < 0.5 )
			z *= -1;

		Vector3 randomDir(x,y,z);
		randomDir.normalize();

		// make sure the random direction isn't pointing INTO the material
		if( dot( randomDir, hitInfo.N ) < 0 )
			randomDir *= -1;

		indirectLightingRay.o = hitInfo.P;
		indirectLightingRay.d = randomDir;
		if (trace(indirectLightingHit, indirectLightingRay, epsilon, MIRO_TMAX))
		{
			bool hitAreaLight = false;

			// loop over all of the lights to see if we hit an area light
			const Lights *lightlist = this->lights();
			Lights::const_iterator lightIter;
			for (lightIter = lightlist->begin(); lightIter != lightlist->end(); lightIter++)
			{
				PointLight* pLight = *lightIter;
				if( pLight->isAreaLight() && (( AreaLight * )pLight)->containsPoint( indirectLightingHit.P ) )
				{
					hitAreaLight = true;
					break;
				}
			}

			// only add this indirect lighting contribution if we didn't hit an area light
			if( !hitAreaLight )
				indirectLighting += indirectLightingHit.material->shade(indirectLightingRay, indirectLightingHit, *this);
		}
	}

	// average the result and add it to the shade result here
	indirectLighting /= NUM_SAMPLES_PER_PIXEL;

	return indirectLighting;
}

