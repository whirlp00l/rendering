#include "Specular.h"

Specular::Specular( const Vector3 & ks )
{
	m_type = Material::SPECULAR;
}

Specular::~Specular()
{
}

Vector3
Specular::shade( const Ray& ray, const HitInfo& hit, const Scene& scene ) const
{
	return Vector3(1);
}