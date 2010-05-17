#include "Material.h"
#include "DebugMem.h"

const int Material::SPECULAR_RECURSION_DEPTH = 5;

Material::Material()
{
	m_refractive_index = 1.0f;
	m_type = Material::UNDEFINED;
}

Material::~Material()
{
}

Vector3
Material::shade(const Ray&, const HitInfo&, const Scene&) const
{
    return Vector3(1.0f, 1.0f, 1.0f);
}
