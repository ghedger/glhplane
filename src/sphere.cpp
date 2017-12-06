
#include <math.h>
#include "sphere.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


Spherex::Spherex()
{
	generate( 1.0, 20 );
}

Spherex::Spherex( float radius, int factor )
{
  generate( radius, factor );
}

Spherex::Spherex( double radius, int factor )
{
	generate( ( float ) radius, factor );
}

Spherex::~Spherex()
{
	if( m_vertices ) {
		delete( m_vertices );
		m_vertices = NULL;
	}
}


void Spherex::draw()
{
		glDrawArrays(GL_TRIANGLES, 0, getVerticeTot());
}

#define D3DMXVECTOR3 glm::vec3
#define D3DMX_PI M_PI


bool Spherex::generate( const float radius, const int iFactor )
{
	bool bRet = true;

	// for environment mapping
	//initSphereMapping( &g_textureID );

  float arrV[iFactor* iFactor][3];
	int iPos = 0;
	m_verticeTot = pow( iFactor, 2 ) * 2;

	m_vertices = new CUSTOMVERTEX[ m_verticeTot * 2 ];
	m_shapeTot = iFactor * iFactor * 2; // use when rendering

	for ( int j = 0; j < iFactor; j++ )
	{
		float theta = ( D3DMX_PI * j )/( iFactor );
		for( int i = 0; i < iFactor; i++ )
		{
			iPos = j * iFactor + i;
			float phi = ( 2 * D3DMX_PI * i )/( iFactor );
			arrV[ iPos ][ 0 ] = ( float )( sin( theta ) * cos( phi ));
			arrV[ iPos ][ 1 ] = ( float )( sin( theta ) * sin( phi ));
			arrV[ iPos ][ 2 ] = ( float )( cos( theta ));
		}
	}

	int iNext = 0;

	for( int j = 0; j < iFactor; j++ )
	{
		for( int i = 0; i < iFactor; i++ )
		{
			if ( i  == iFactor - 1 )
				iNext = 0;
			else iNext = i +1;

			iPos = ( j * iFactor * 6 ) + ( i * 6 );
			m_vertices[ iPos ].position = D3DMXVECTOR3( arrV[ j * iFactor + i ][ 0 ], arrV[ j * iFactor + i ][ 1 ], arrV[ j * iFactor + i ][ 2 ] );
			m_vertices[ iPos + 1 ].position = D3DMXVECTOR3( arrV[ j * iFactor + iNext ][ 0 ], arrV[ j * iFactor + iNext ][ 1 ], arrV[ j * iFactor + iNext ][ 2 ] );

			if ( j  != iFactor -1 )
				m_vertices[ iPos + 2 ].position = D3DMXVECTOR3( arrV[ ( ( j + 1 ) * iFactor ) + i ][ 0 ], arrV[ ( ( j + 1 ) * iFactor ) + i ][ 1 ], arrV[ ( ( j + 1 ) * iFactor ) + i ][ 2 ] );
			else
				m_vertices[ iPos + 2 ].position = D3DMXVECTOR3( 0, 0, -1 ); //Create a pseudo triangle fan for the last set of triangles

			m_vertices[ iPos ].normal = D3DMXVECTOR3( m_vertices[ iPos ].position.x, m_vertices[ iPos ].position.y, m_vertices[ iPos ].position.z );
			m_vertices[ iPos + 1 ].normal = D3DMXVECTOR3( m_vertices[ iPos + 1 ].position.x, m_vertices[ iPos + 1 ].position.y, m_vertices[ iPos + 1 ].position.z );
			m_vertices[ iPos + 2 ].normal = D3DMXVECTOR3( m_vertices[ iPos + 2 ].position.x, m_vertices[ iPos + 2 ].position.y, m_vertices[ iPos + 2 ].position.z );

			m_vertices[ iPos + 3 ].position = D3DMXVECTOR3( m_vertices[ iPos + 2 ].position.x, m_vertices[ iPos + 2 ].position.y, m_vertices[ iPos + 2 ].position.z );
			m_vertices[ iPos + 4 ].position = D3DMXVECTOR3( m_vertices[ iPos + 1 ].position.x, m_vertices[ iPos + 1 ].position.y, m_vertices[ iPos + 1 ].position.z );

			if ( j  != iFactor - 1 )
				m_vertices[ iPos + 5 ].position = D3DMXVECTOR3( arrV[ ( ( j + 1 ) * iFactor ) + iNext ][ 0 ], arrV[ ( ( j + 1 ) * iFactor ) + iNext ][ 1 ], arrV[ ( ( j + 1 ) * iFactor ) + iNext ][ 2 ] );
			else
				m_vertices[ iPos + 5 ].position = D3DMXVECTOR3( 0,0,-1 );

			m_vertices[ iPos + 3 ].normal = D3DMXVECTOR3( m_vertices[ iPos + 3 ].position.x, m_vertices[ iPos + 3 ].position.y, m_vertices[ iPos + 3 ].position.z );
			m_vertices[ iPos + 4 ].normal = D3DMXVECTOR3( m_vertices[ iPos + 4 ].position.x, m_vertices[ iPos + 4 ].position.y, m_vertices[ iPos + 4 ].position.z );
			m_vertices[ iPos + 5 ].normal = D3DMXVECTOR3( m_vertices[ iPos + 5 ].position.x, m_vertices[ iPos + 5 ].position.y, m_vertices[ iPos + 5 ].position.z );
		}
	}
	return bRet;
}



