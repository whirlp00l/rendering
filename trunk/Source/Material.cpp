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

	Material * material = NULL;
	Vector3 kd(1), ka(0);
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
			// else ignore line
		}
		// else ignore line
	}

	( void )fclose( fp );

	return new Lambert( kd, ka );
}

void
Material::setUseBumpMap( bool useBumpMap )
{
	// use a bump map for this material
	if( useBumpMap )
	{
		// if we don't already have one, we need to create a noise maker
		if( !m_bump_map_noise_maker )
		{
			m_bump_map_noise_maker = new CustomizablePerlinNoise(4, 4, 1, 14);
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

	// let's make some noise!
	float noiseCoefX = m_bump_map_noise_maker->Get(hitPoint.x + 0.1, hitPoint.y + 0.1, hitPoint.z + 0.1);
	float noiseCoefY = m_bump_map_noise_maker->Get(hitPoint.y + 0.1, hitPoint.z + 0.1, hitPoint.x + 0.1);
	float noiseCoefZ = m_bump_map_noise_maker->Get(hitPoint.z + 0.1, hitPoint.x + 0.1, hitPoint.y + 0.1);

	//Vector3 perturbedNormal( ( origNormal.x + 0.1 )* noiseCoefX, ( origNormal.y + 0.1 ) * noiseCoefY, ( origNormal.z + 0.1 ) * noiseCoefZ );
	//perturbedNormal *= perturbedNormal;
	Vector3 perturbedNormal( noiseCoefX, noiseCoefY, noiseCoefZ );
	perturbedNormal += origNormal;
	perturbedNormal.normalize();

	// we don't want to completely FLIP any normals
	if( dot( perturbedNormal, origNormal ) < 0 )
		perturbedNormal *= -1;

	return perturbedNormal;
}