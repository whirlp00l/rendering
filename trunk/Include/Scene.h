#ifndef CSE168_SCENE_H_INCLUDED
#define CSE168_SCENE_H_INCLUDED

#include "Miro.h"
#include "Object.h"
#include "PointLight.h"
#include "BVH.h"

class Camera;
class Image;

#define USE_PATH_TRACING 0
#define NUM_SAMPLES_PER_PIXEL 30
#define USE_ENVIRONMENT_MAP 1
#define ENVIRONMENT_MAP_FILE_NAME "Resource\\stpeters_probe.pfm"

class Scene
{
public:
	Scene();
	~Scene();

    void addObject(Object* pObj)        {m_objects.push_back(pObj);}
    const Objects* objects() const      {return &m_objects;}

    void addLight(PointLight* pObj)     {m_lights.push_back(pObj);}
    const Lights* lights() const        {return &m_lights;}

	const Vector3* environmentMap() const {return m_environment_map;}
	const int mapWidth() const {return m_map_width;}
	const int mapHeight() const {return m_map_height;}

    void preCalc();
    void openGL(Camera *cam);

    void raytraceImage(Camera *cam, Image *img);
    bool trace(HitInfo& minHit, const Ray& ray,
               float tMin = 0.0f, float tMax = MIRO_TMAX) const;

protected:
    Objects m_objects;
    BVH m_bvh;
    Lights m_lights;
	int m_num_rays_traced;
	Vector3 * m_environment_map;
	int m_map_width;
	int m_map_height;
};

extern Scene * g_scene;

#endif // CSE168_SCENE_H_INCLUDED
