#include "TriangleMesh.h"
#include "Triangle.h"
#include "Scene.h"
#include "DebugMem.h"

TriangleMesh::TriangleMesh() :
    m_normals(0),
    m_vertices(0),
    m_texCoords(0),
    m_normalIndices(0),
    m_vertexIndices(0),
    m_texCoordIndices(0)
{

}

TriangleMesh::~TriangleMesh()
{
	if( m_normals )
	{
		delete [] m_normals;
		m_normals = NULL;
	}

	if( m_vertices )
	{
		delete [] m_vertices;
		m_vertices = NULL;
	}

	if( m_texCoords )
	{
		delete [] m_texCoords;
		m_texCoords = NULL;
	}
    
	if( m_normalIndices )
	{
		delete [] m_normalIndices;
		m_normalIndices = NULL;
	}
    
	if( m_vertexIndices )
	{
		delete [] m_vertexIndices;
		m_vertexIndices = NULL;
	}

	if( m_texCoordIndices )
	{
		delete [] m_texCoordIndices;
		m_texCoordIndices = NULL;
	}
}
