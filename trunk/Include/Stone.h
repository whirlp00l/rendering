#ifndef STONE_H
#define STONE_H

#include "Lambert.h"
#include "Worley.h"

class Stone : public Lambert 
{
public:
	Stone();
	~Stone();

	Vector3 shade(const Ray& ray, const HitInfo& hit, const Scene& scene) const;

protected:
	static const float THRESHOLD;
};

#endif // STONE_H