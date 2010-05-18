#include "Stone.h"

Stone::Stone()
{
}

Stone::~Stone()
{
}

Vector3
Stone::shade(const Ray &ray, const HitInfo &hit, const Scene &scene) const
{
	return Vector3(0,0,0);
}