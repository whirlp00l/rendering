#ifndef STONE_H
#define STONE_H

#include "Material.h"

class Stone : public Material 
{
public:
	Stone();
	~Stone();

	Vector3 shade(const Ray& ray, const HitInfo& hit, const Scene& scene) const;
};

#endif // STONE_H