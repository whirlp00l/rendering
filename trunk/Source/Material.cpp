#include "Material.h"
#include "DebugMem.h"
#include "Lambert.h"
#include "Console.h"

const int Material::SPECULAR_RECURSION_DEPTH = 20;

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
