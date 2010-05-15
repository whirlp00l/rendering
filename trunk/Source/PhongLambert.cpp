#include "PhongLambert.h"
#include "Ray.h"
#include "Scene.h"
#include "DebugMem.h"

PhongLambert::PhongLambert(const Vector3 & kd, const Vector3 & ka, const float & phongExp) :
Lambert(kd,ka), m_phong_exp(phongExp)
{
	m_type = Material::PHONG_DIFFUSE;
}

PhongLambert::~PhongLambert()
{
}

Vector3
PhongLambert::shade(const Ray& ray, const HitInfo& hit, const Scene& scene) const
{
    Vector3 L = Vector3(0.0f, 0.0f, 0.0f);
    
    const Vector3 viewDir = -ray.d; // d is a unit vector
    
    const Lights *lightlist = scene.lights();
    
    // loop over all of the lights
    Lights::const_iterator lightIter;
    for (lightIter = lightlist->begin(); lightIter != lightlist->end(); lightIter++)
    {
        PointLight* pLight = *lightIter;

		Vector3 l = pLight->position() - hit.P;

		// the inverse-squared falloff
        float falloff = l.length2();
        float magnitude = sqrt(falloff);
        // normalize the light direction
        l /= magnitude;

		// we only need to add this light's contribution if we're not in shadow
		Ray shadowRay;
		shadowRay.d = l;
		shadowRay.o = hit.P;
		HitInfo hitInfo = hit;
		// we have a shadow!
		if( g_scene->trace( hitInfo, shadowRay, epsilon, magnitude ) )
		{
			continue;
		}

        // get the diffuse component
        float nDotL = dot(hit.N, l);
        Vector3 result = pLight->color();
        result *= m_kd;
        
        L += std::max(0.0f, nDotL/falloff * pLight->wattage() / PI) * result;

		// now calculate phong highlight
		Vector3 reflectDir = 2 * dot( l, hit.N ) * hit.N - l; // direction to light reflected across normal
		reflectDir.normalize();
		float viewDirDotReflectDir = dot( viewDir, reflectDir );
		if( viewDirDotReflectDir > 0 )
			L += std::max(0.0f, pow(viewDirDotReflectDir, m_phong_exp)) * pLight->color();
    }
    
    // add the ambient component
    L += m_ka;
    
    return L;
}
