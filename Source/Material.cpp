#include "Material.h"
#include "DebugMem.h"
#include "Lambert.h"
#include "Console.h"
#include "PointLight.h"
#include "Scene.h"
#include "Ray.h"
#include "WorleyNoise.h"

Material::Material()
{
	m_phong_exp = 0.0f;
	m_refractive_index = 1.0f;
	m_use_bump_map = false;
	m_bump_map_noise_maker = NULL;
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

Vector3 
Material::getPhongHighlightContribution( const PointLight * pLight, const Ray& ray, const HitInfo& hit ) const
{
	Vector3 contribution(0,0,0);

	Vector3 l = pLight->position() - hit.P;
	l.normalize();

	Vector3 viewDir = -ray.d;
	Vector3 normal = m_use_bump_map ? calcBumpMappedNormal( hit.P, hit.N ) : hit.N;
	Vector3 lightReflectDir = 2 * dot( l, normal ) * normal - l; // direction to light reflected across normal
	float viewDirDotReflectDir = dot( viewDir, lightReflectDir );
	if( viewDirDotReflectDir > 0 )
		contribution += std::max(0.0f, pow(viewDirDotReflectDir, m_phong_exp)) * pLight->color();

	return contribution;
}

Material *
Material::loadMaterial( char * fileName )
{
	char fileNameWithExt[80];
	strncpy_s( fileNameWithExt, 80, fileName, _TRUNCATE );
	char * endStr = strchr( fileNameWithExt, '\0' );

	// add .mtl suffix
	strncpy_s( endStr, 80 - strlen(fileNameWithExt) - 1, ".mtl", _TRUNCATE );

	FILE * fp;
	fopen_s( &fp, fileNameWithExt, "rb" );
	if( !fp )
	{
		debug( "Error: could not open %s for reading.\n", fileNameWithExt );
		return NULL;
	}

	Vector3 kd(1), ka(0);
	bool useBumpMap;
	int octaves, seed; // for bump map noise maker
	float freq, amp; // for bump map noise maker
	char buf[80];
	while( fgets( buf, 80, fp ) != 0 )
	{
		// this is a comment; skip it
		if( buf[0] == '#' )
			continue; 

		char * space = strchr( buf, ' ' );
		if( space )
		{
			*space = '\0';

			// ambient term
			if( strcmp( buf, "Ka" ) == 0 )
			{
				float x, y, z;
				sscanf_s(space + 1, "%f %f %f\n", &x, &y, &z);
				ka = Vector3(x,y,z);
			}
			// diffuse term
			else if( strcmp( buf, "Kd" ) == 0 )
			{
				float x, y, z;
				sscanf_s(space + 1, "%f %f %f\n", &x, &y, &z);
				kd = Vector3(x,y,z);
			}
			// bump mapping information
			else if( strcmp( buf, "use_bump_map" ) == 0 )
			{
				sscanf_s(space + 1, "%d %f %f %d\n", &octaves, &freq, &amp, &seed);
				useBumpMap = true;
			}
			// else ignore line
		}
		// else ignore line
	}

	( void )fclose( fp );

	Material * material = new Lambert( kd, ka );
	if( useBumpMap )
		material->setUseBumpMap( true, octaves, freq, amp, seed );

	return material;
}

void
Material::setUseBumpMap( bool useBumpMap, int octaves, float freq, float amp, int seed )
{
	// use a bump map for this material
	if( useBumpMap )
	{
		// if we don't already have one, we need to create a noise maker
		if( !m_bump_map_noise_maker )
		{
			m_bump_map_noise_maker = new CustomizablePerlinNoise( octaves, freq, amp, seed );
		}
	}
	// DON'T use a bump map for this material
	else
	{
		if( m_use_bump_map )
		{
			delete m_bump_map_noise_maker;
			m_bump_map_noise_maker = NULL;
		}
	}

	m_use_bump_map = useBumpMap;
}

Vector3 
Material::calcBumpMappedNormal( Vector3 hitPoint, Vector3 origNormal ) const
{
	// if we don't want to use bumpmapping, just return the original normal
	if( !m_use_bump_map || !m_bump_map_noise_maker )
		return origNormal;

	/*
	 * IDEA:
	 * We want to approximate changing the height of each point that we render on the surface.
	 * If the original point is P and its normal is N, then the new point can be defined as:
	 *		P' = P + h * N
	 * where h is a randomly generated height (given by a noise function).
	 * We can define 2 vectors U and V that describe the plane in which point P sits (using its normal N):
	 *		U = R x N, V = N x U
	 * where R is some randomly generated vector that is NOT parallel to N.
	 * We can then find dP'/dU and dP'/dV, which define the surface at P', as follows (uses the derivative of the 1st eqn):
	 *		dP'/dU = dP/dU + dh/dU * N + h * dN/dU 
	 *		dP'/dV = dP/dV + dh/dV * N + h * dN/dV
	 * Note that in both of these partial derivatives, the 3rd term in the sum can be ignored 
	 * since N does not change with respec to U or V. In other words,
	 *		dP'/dU = dP/dU + dh/dU * N 
	 *		dP'/dV = dP/dV + dh/dV * N
	 * Then our desired bump-mapped normal, N', can be found by taking the cross product of 2 derivates:
	 *		N' = dP'/dU x dP'/dV
	 */

	// calculate random bump-mapped height with noise function
	float bumpHeight = m_bump_map_noise_maker->Get( hitPoint.x, hitPoint.y, hitPoint.z );

	// find a random vector that we can use to find U (random direction can't be parallel to original normal) 
	Vector3 randomDir;
	float magCrossProd;
	float epsilonSquared = epsilon * epsilon; // square epsilon to avoid having to take the sqrt to get the cross product's magnitude
	do
	{
		randomDir.x = rand() / static_cast<double>(RAND_MAX); // yields random value in range [0,1]
		randomDir.y = rand() / static_cast<double>(RAND_MAX); // yields random value in range [0,1]
		randomDir.z = rand() / static_cast<double>(RAND_MAX); // yields random value in range [0,1]
		randomDir.normalize();
		Vector3 crossProd = cross( randomDir, origNormal );
		magCrossProd = crossProd.length2();
	}
	while( magCrossProd < epsilonSquared );

	// now we can calculate U and V
	Vector3 uDir = cross( randomDir, origNormal );
	uDir.normalize();
	Vector3 vDir = cross( origNormal, uDir );
	vDir.normalize();

	// now calculate the partial derivates of P' with respect to U and V
	Vector3 dPdU = ( ( hitPoint + epsilon * uDir ) - ( hitPoint - epsilon * uDir ) ) / ( 2 * epsilon );
	Vector3 dPdV = ( ( hitPoint + epsilon * vDir ) - ( hitPoint - epsilon * vDir ) ) / ( 2 * epsilon );

	// calculate the partial derivates of h with respect to U and V
	Vector3 PUPrime = hitPoint + epsilon * uDir;
	Vector3 PVPrime = hitPoint + epsilon * vDir;
	Vector3 dHdU = ( m_bump_map_noise_maker->Get( PUPrime.x, PUPrime.y, PUPrime.z ) - bumpHeight ) / epsilon;
	Vector3 dHdV = ( m_bump_map_noise_maker->Get( PVPrime.x, PVPrime.y, PVPrime.z ) - bumpHeight ) / epsilon;

	// now we can calculate dP'/dU and dP'/dV
	Vector3 dPPrime_dU = dPdU + dHdU * origNormal;
	Vector3 dPPrime_dV = dPdV + dHdV * origNormal;

	// finally, calcualte the perturbed normal
	Vector3 perturbedNormal = cross( dPPrime_dU, dPPrime_dV );
	perturbedNormal.normalize();

	return perturbedNormal;
}