#include "Miro.h"
#include "Scene.h"
#include "Camera.h"
#include "Image.h"
#include "Console.h"
#include "PFMLoader.h"
#include "DebugMem.h"
#include "EnvironmentMap.h"
#include "AreaLight.h"
#include "SpecularReflector.h"
#include "SpecularRefractor.h"

#include <windows.h>
#include <time.h>
#include <stdlib.h>

Scene * g_scene = 0;

Scene::Scene() : m_environment_map(0), m_map_width(0), m_map_height(0), m_photon_map(0)
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

	if( m_photon_map )
	{
		delete m_photon_map;
		m_photon_map = NULL;
	}
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

	// seed randomizer for photon mapping, bump mapping, path tracing and/or depth of field 
	// (always seed it just in case we're doing bump mapping)
	srand((unsigned)time(0));

	// create the photon map first (don't do this if we've already done it once!)
	if( USE_PHOTON_MAPPING && !m_photon_map )
	{
		printf( "Beginning photon mapping calculations...\n" );

		// divide total number of photons up evenly amongst all lights in the scene
		const Lights *lightlist = this->lights();
		int numPhotonsPerLight = ( int )( NUM_PHOTONS / lightlist->size() );

		// just in case the number of photons wasn't evenly divisible by the number of lights
		int totalNumPhotons = numPhotonsPerLight * lightlist->size();
		m_photon_map = new PhotonMap( totalNumPhotons );
	    
		// loop over all of the lights
		Lights::const_iterator lightIter;
		for (lightIter = lightlist->begin(); lightIter != lightlist->end(); lightIter++)
		{
			PointLight* pLight = *lightIter;
			for( int i = 0; i < numPhotonsPerLight; i++ )
			{
				// generate random direction for this photon
				float x = rand() / static_cast<double>(RAND_MAX); // yields random value in range [0,1]
				float y = rand() / static_cast<double>(RAND_MAX); // yields random value in range [0,1]
				float z = rand() / static_cast<double>(RAND_MAX); // yields random value in range [0,1]

				// rand() only returns positive numbers; randomize whether each component is positive or negative
				float posOrNeg = rand() / static_cast<double>(RAND_MAX); // yields random value in range [0,1]
				if( posOrNeg < 0.5 )
					x *= -1;
				posOrNeg = rand() / static_cast<double>(RAND_MAX); // yields random value in range [0,1]
				if( posOrNeg < 0.5 )
					y *= -1;
				posOrNeg = rand() / static_cast<double>(RAND_MAX); // yields random value in range [0,1]
				if( posOrNeg < 0.5 )
					z *= -1;

				Vector3 photonDir( x, y, z );
				photonDir.normalize();

				// now we must trace the scene to find out where to store this photon
				ray.d = photonDir;
				// if it's an area light, randomize a place in the light where this photon originates
				if( pLight->isAreaLight() )
					ray.o = ( ( AreaLight * )pLight )->getRandomLightPoint();
				// otherwise it's a point light. originate the photon ray from the light's position
				else
					ray.o = pLight->position();

				Vector3 photonPower = pLight->color() * ( pLight->wattage() / numPhotonsPerLight );
				bool keepTracing = true;
				float traceMinDistance = 0.0f;
				int numBounces = 0;
				int specularRecursionCount = 0;
				// the photon hit something!
				while( keepTracing && trace( hitInfo, ray, traceMinDistance ) )
				{
					// if this wasn't a diffuse material, we need to reflect/refract appropriately and keep tracing
					if( !hitInfo.material->isDiffuse() )
					{
						if( specularRecursionCount >= SpecularReflector::SPECULAR_RECURSION_DEPTH )
						{
							keepTracing = false;
						}
						else
						{
							specularRecursionCount++;
							ray.o = hitInfo.P;

							// it's either reflective or refractive; get the new direction accordingly
							if( hitInfo.material->getType() == Material::SPECULAR_REFLECTOR ) // reflective
							{
								ray.d = ( ( SpecularReflector * )hitInfo.material )->getReflectedDir( ray, hitInfo );
							}
							else // refractive
							{
								Ray refractedRay;
								float reflectivity;
								if( ( ( SpecularRefractor * )hitInfo.material )->getRefractedRay( refractedRay, reflectivity, ray, hitInfo, *this ) )
								{
									ray.d = refractedRay.d;
								}
								else
								{
									ray.d = ( ( SpecularRefractor * )hitInfo.material )->getReflectedDir( ray, hitInfo );
								}
							}
						}

						continue;
					}

					float power[3];
					float pos[3];
					float dir[3];

					power[0] = photonPower.x;
					power[1] = photonPower.y;
					power[2] = photonPower.z;

					pos[0] = hitInfo.P.x;
					pos[1] = hitInfo.P.y;
					pos[2] = hitInfo.P.z;

					dir[0] = ray.d.x;
					dir[1] = ray.d.y;
					dir[2] = ray.d.z;

					// store it in the photon map
					m_photon_map->store( power, pos, dir );

					// we've bounced this photon around enough
					if( numBounces == MAX_PHOTON_BOUNCES )
						keepTracing = false;
					else 
					{
						// use Russian Roulette to determine whether or not to terminate this photon
						float russianRoulette = rand() / static_cast<double>(RAND_MAX); // yields random value in range [0,1]

						// arbitrarily using probabily 0.5 to bounce this photon
						if( russianRoulette < 0.5 )
						{
							// we're gonna bounce this photon again
							numBounces++;

							// generate random direction for this photon
							x = rand() / static_cast<double>(RAND_MAX); // yields random value in range [0,1]
							y = rand() / static_cast<double>(RAND_MAX); // yields random value in range [0,1]
							z = rand() / static_cast<double>(RAND_MAX); // yields random value in range [0,1]

							// rand() only returns positive numbers; randomize whether each component is positive or negative
							posOrNeg = rand() / static_cast<double>(RAND_MAX); // yields random value in range [0,1]
							if( posOrNeg < 0.5 )
								x *= -1;
							posOrNeg = rand() / static_cast<double>(RAND_MAX); // yields random value in range [0,1]
							if( posOrNeg < 0.5 )
								y *= -1;
							posOrNeg = rand() / static_cast<double>(RAND_MAX); // yields random value in range [0,1]
							if( posOrNeg < 0.5 )
								z *= -1;

							photonDir = Vector3( x, y, z ).normalize();

							// set up the ray for tracing again
							ray.d = photonDir;
							ray.o = hitInfo.P;
							traceMinDistance = epsilon;
						}
						else
							keepTracing = false;
					}
				}
			}
		}

		// now that we're done creating the photon map, balance the kd tree
		m_photon_map->balance();

		printf( "Done with photon map calculations!\n\n" );
	} // end if( USE_PHOTON_MAPPING )

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
				if( USE_DEPTH_OF_FIELD )
				{
					Vector3 focalPlanePt;
					Ray depthOfFieldRay;
					HitInfo depthOfFieldHitInfo;
					Vector3 depthOfFieldShadeResult(0,0,0);
					for( int k = 0; k < NUM_DEPTH_OF_FIELD_SAMPLES; k++ )
					{
						bool foundPt = cam->getFocalPlaneIntersection( focalPlanePt, hitInfo.P );
						// if we didn't find the focal plane point, do nothing
						if( foundPt )
						{
							depthOfFieldRay.o = cam->getRandomApertureSample();
							depthOfFieldRay.d = focalPlanePt - depthOfFieldRay.o;
							depthOfFieldRay.d.normalize();
							
							if( trace( depthOfFieldHitInfo, depthOfFieldRay ) )
							{
								depthOfFieldShadeResult += depthOfFieldHitInfo.material->shade( depthOfFieldRay, depthOfFieldHitInfo, *this );
							}
						}
					}

					shadeResult = depthOfFieldShadeResult / NUM_DEPTH_OF_FIELD_SAMPLES;
				} // end if( USE_DEPTH_OF_FIELD )
				// don't use depth of field
				else
				{
					shadeResult = hitInfo.material->shade(ray, hitInfo, *this);
				} // end don't use depth of field
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

		clock_t rowEndTime = clock();

		float timeSoFar = rowEndTime - clockStart;
		int numRowsDone = j + 1;
		int numRowsLeft = img->height() - numRowsDone;
		float avgSecondsPerRow = ( timeSoFar/CLOCKS_PER_SEC ) / numRowsDone;

        img->drawScanline(j);
        glFinish();
		//printf("Rendering Progress: %.3f%%\r", j/float(img->height())*100.0f);
        printf("\rProgress: %.3f%%, Time elapsed: %.4f sec, Est. time left: %.4f sec\r", 
			j/float(img->height())*100.0f, timeSoFar/CLOCKS_PER_SEC, numRowsLeft * (timeSoFar/CLOCKS_PER_SEC) / numRowsDone);
		
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



