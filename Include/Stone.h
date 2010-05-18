#ifndef STONE_H
#define STONE_H

#include "Lambert.h"

class Stone : public Lambert 
{
public:
	enum Coloring
	{
		REALISTIC,
		COLORFUL
	};

	Stone( Stone::Coloring coloring, const Vector3 & kd = Vector3(1) );
	~Stone();

	Vector3 shade(const Ray& ray, const HitInfo& hit, const Scene& scene) const;

protected:
	static const float THRESHOLD;
	Coloring mColoring;
};

#endif // STONE_H