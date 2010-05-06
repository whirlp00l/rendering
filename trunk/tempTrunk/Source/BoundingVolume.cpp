#include "BoundingVolume.h"
#include "DebugMem.h"

BoundingVolume::BoundingVolume( Objects * objects, bool isLeaf ) :
m_bIsLeaf( isLeaf )
{ 
	// now add each bounded object to this bounding volume
	for( size_t i = 0; i < objects->size(); i++ )
	{
		m_children.push_back( (*objects)[i] );
	}
}

BoundingVolume::~BoundingVolume()
{
	for( unsigned int i = 0; i < m_children.size(); i++ )
		m_children[i]->~Object();

	m_children.clear();
}

void
BoundingVolume::addChild( Object * child )
{
	m_children.push_back( child );
}

void
BoundingVolume::calcNumNodesAndLeaves( int * numNodesPtr, int * numLeavesPtr )
{
	// this is a node, increment the node count
	(*numNodesPtr)++;

	// if it's a leaf, don't loop through (primitive) children
	if( m_bIsLeaf )
	{
		// increment number of leaves
		(*numLeavesPtr)++;
	}
	// not a leaf, so loop through all children as well
	else
	{
		for( size_t i = 0; i < m_children.size(); i++ )
		{
			// since this isn't a leaf, each child is a bounding volume
			BoundingVolume * childBV = ( BoundingVolume * )m_children[i];
			childBV->calcNumNodesAndLeaves( numNodesPtr, numLeavesPtr );
		}
	}

}