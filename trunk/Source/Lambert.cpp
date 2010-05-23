#include "Lambert.h"
#include "Ray.h"
#include "Scene.h"
#include "DebugMem.h"
#include "AreaLight.h"

Lambert::Lambert(const Vector3 & kd, const Vector3 & ka, const float & phongExp) : 
m_kd(kd), m_ka(ka)
{
	m_type = Material::DIFFUSE;
	m_phong_exp = phongExp;
}

Lambert::~Lambert()
{
}

Vector3
Lambert::shade(const Ray& ray, const HitInfo& hit, const Scene& scene) const
{
    Vector3 L = getDiffuseColor( ray, hit, scene );

    // add the ambient component
    L += m_ka;
    
    return L;
}

Vector3
Lambert::getDiffuseColor( const Ray& ray, const HitInfo& hit, const Scene& scene ) const
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
		HitInfo hitInfo;

		// we have a (hard) shadow!
		if( !pLight->isAreaLight() && scene.trace( hitInfo, shadowRay, epsilon, magnitude ) )
		{
			continue;
		}

		float hitRatio = 1;
		if( pLight->isAreaLight() )
			hitRatio = (( AreaLight * )pLight)->getHitRatio( hit.P, scene );
    
        // get the diffuse component
        float nDotL = dot(hit.N, l);
        Vector3 result = pLight->color();
        result *= m_kd;
        
        L += std::max(0.0f, nDotL/falloff * pLight->wattage() / PI) * result * hitRatio;

		if( m_phong_exp != 0 )
		{
			L += getPhongHighlightContribution( pLight, ray, hit );
		}
    }
    
    return L;
}
