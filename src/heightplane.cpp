
// HeightPlane implementation

#include "common.h"
#include "heightplane.h"

#include <GL/glut.h>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include <unistd.h>
#include <math.h>
#include <iostream>
#include <memory.h>
#include <stdio.h>

using namespace std;

HeightPlane::HeightPlane()
{
  m_param[0] = 0;
  m_param[1] = 0;
  m_param[2] = 0;
  m_param[3] = 0;

  m_vertices = new float[ VBO_BUFSIZE ];
  memset( m_vertices, 0, sizeof( float ) * VBO_BUFSIZE );

}

HeightPlane::~HeightPlane()
{
  m_verticeTot = 0;
  if( m_vertices ) {
    delete( m_vertices );
    m_vertices = NULL;
  }
}

double HeightPlane::heightFn( double x, double y )
{
  float h = sin( x * ( M_PI * 2 ) / HP_XSIZE ) * 30 +
    sin( y * ( M_PI * 2 ) / HP_YSIZE ) * 30;

  h += sin( x * ( M_PI * 2 ) / HP_XSIZE * 2 ) * 16 +
    sin( y * ( M_PI * 2 ) / HP_YSIZE * 2 ) * 16;

  h += sin( x * ( M_PI * 2 ) / HP_XSIZE * 4 ) * 8 +
    sin( y * ( M_PI * 2 ) / HP_YSIZE * 4 ) * 8;

  h += sin( x * ( M_PI * 2 ) / HP_XSIZE * 8 ) * 3 +
    sin( y * ( M_PI * 2 ) / HP_YSIZE * 8 ) * 3;
  return h;
}


// initHeightPlane
//
bool HeightPlane::initHeightPlane()
{
  bool bRet = true;
  int x, y;
  float texIdx = 0.0;
  int i = 0;
  double h = 0;

  for( x = 0; x < HP_XSIZE; x++ ) {
    for( y = 0; y < HP_YSIZE; y++ ) {
      h = heightFn(x, y);
      if( x > 0 ) {
        h += 100.0 / (HP_XSIZE - x );
        h += 100.0 / x;
      }
      if( y > 0 ) {
        h += 100.0 / (HP_YSIZE - y );
        h += 100.0 / y;
      }

      m_heightPlane[ x ][ y ] = h * 1.0;

      if(
          !(
            ( !x ) ||
            ( !y ) ||
            (HP_XSIZE - 1 ) == x ||
            (HP_YSIZE - 1 ) == y
           )
        ) {
        m_vertices[ i++ ] = (x * HP_GRIDSIZE) ;
        m_vertices[ i++ ] = m_heightPlane[ x - 1 ][ y - 1 ];
        m_vertices[ i++ ] = (y * HP_GRIDSIZE) ;
        m_vertices[ i++ ] = 0.0;
        m_vertices[ i++ ] = 0.0;

        m_vertices[ i++ ] = (x * HP_GRIDSIZE) ;
        m_vertices[ i++ ] = m_heightPlane[ x - 1 ][ y ];
        m_vertices[ i++ ] = (y * HP_GRIDSIZE)  + HP_GRIDSIZE;
        m_vertices[ i++ ] = 0.0;
        m_vertices[ i++ ] = texIdx;

        m_vertices[ i++ ] = (x * HP_GRIDSIZE)  + HP_GRIDSIZE;
        m_vertices[ i++ ] = m_heightPlane[ x ][ y ];
        m_vertices[ i++ ] = (y * HP_GRIDSIZE)  + HP_GRIDSIZE;
        m_vertices[ i++ ] = texIdx;
        m_vertices[ i++ ] = texIdx;

        m_vertices[ i++ ] = (x * HP_GRIDSIZE)  + HP_GRIDSIZE;
        m_vertices[ i++ ] = m_heightPlane[ x ][ y ];
        m_vertices[ i++ ] = (y * HP_GRIDSIZE)  + HP_GRIDSIZE;
        m_vertices[ i++ ] = texIdx;
        m_vertices[ i++ ] = texIdx;

        m_vertices[ i++ ] = (x * HP_GRIDSIZE) ;
        m_vertices[ i++ ] = m_heightPlane[ x - 1 ][ y - 1];
        m_vertices[ i++ ] = (y * HP_GRIDSIZE) ;
        m_vertices[ i++ ] = 0.0;
        m_vertices[ i++ ] = 0.0;

        m_vertices[ i++ ] = (x * HP_GRIDSIZE)  + HP_GRIDSIZE;
        m_vertices[ i++ ] = m_heightPlane[ x ][ y - 1 ];
        m_vertices[ i++ ] = (y * HP_GRIDSIZE) ;
        m_vertices[ i++ ] = texIdx;
        m_vertices[ i++ ] = 0.0;

        if( texIdx > 0 ) {
          texIdx = 1.0f;
        } else {
          texIdx = 1.0f;
        }
      }
    }
  }

  // Offset the height plane by 1x, 1y:
  //
  // xxxxxxxxxxxx
  // xxxxxxxxxxxx.
  // xxxxxxxxxxxx.
  //  ............
#if 0
  for( x = 0; x < HP_XSIZE - 1; x++ ) {
    for( y = 0; y < HP_YSIZE - 1 ; y++ ) {
      m_heightPlane[ x ][ y ] = m_heightPlane[ x + 1 ][ y + 1 ];
    }
  }
#endif
  for( x = HP_XSIZE - 1; x > 0; x-- ) {
    for( y = HP_YSIZE - 1; y > 0 ; y-- ) {
      m_heightPlane[ x ][ y ] = m_heightPlane[ x - 1 ][ y - 1 ];
    }
  }

  std::cout << i << " " << i / 30 << " " << sqrt( i / 30 ) <<  std::endl;

  m_verticeTot = i / 5;
  return bRet;
}


// TODO: Move this to a vector math lib
glm::vec3 crossProduct( const glm::vec3 v1,  const glm::vec3 v2 )
{
  glm::vec3 cpv;

  cpv.x = v1.y * v2.z - v2.y * v1.z;
  cpv.y = v2.x * v1.z - v1.x * v2.z;
  cpv.z = v1.x * v2.y - v2.x * v1.y;

  return cpv;
}

// TODO: Move this to a vector math lib
// Normalize a vec 
glm::vec3 normalizeVec( glm::vec3 v )
{
  glm::vec3 nv;
  float sum = 0.0;

  sum = abs(v.x) + abs(v.y) + abs(v.z);
  nv.x = v.x / sum;
  nv.y = v.y / sum;
  nv.z = v.z / sum;

  return nv;
}


// TODO: Move this to a vector math lib
float dotProduct( glm::vec3& u, glm::vec3& v )
{
  return( u.x * v.x + u.y * v.y + u.z * v.z );
}

#define dot(u,v)   ((u).x * (v).x + (u).y * (v).y + (u).z * (v).z)
// norm = length of vector
#define norm(v)    sqrt(dot(v,v))  
// distance = norm of difference
#define d(u,v)     norm(u-v)       

// TODO: Move this to a vector math lib
float closestPointOnPlane( const glm::vec3 v0, const glm::vec3 v1, glm::vec3& origin, glm::vec3& p, glm::vec3 *cp, glm::vec3 *pn )
{
  glm::vec3 closest;
  glm::vec3 n;
  n = normalizeVec(crossProduct( v0, v1 ));

  float     sb, sn, sd;
  glm::vec3 diff;
  diff.x = p.x - origin.x;
  diff.y = p.y - origin.y;
  diff.z = p.z - origin.z;

  sn = -(dot( n, diff ));

  sd = dot( n, n );
  sb = sn / sd;

  closest.x = p.x + (n.x * sb);
  closest.y = p.y + (n.y * sb);
  closest.z = p.z + (n.z * sb);

  cp->x = closest.x;
  cp->y = closest.y;
  cp->z = closest.z;

  diff.x = p.x - closest.x;
  diff.y = p.y - closest.y;
  diff.z = p.z - closest.z;

  float sign = dotProduct( n, diff );
  *pn = n;

  float dp = abs(dotProduct( p, diff ));

  return sqrt( dp ) * (sign > 0 ? 1 : -1);
}

bool HeightPlane::getColdetAdj( const float fx, const float fy, const float fz, const float radius, glm::vec3 *ap )
{
  bool bRet = false;
  int x = (int) (fx / HP_GRIDSIZE);
  int y = (int) (fy / HP_GRIDSIZE);
  glm::vec3 cp = { fx, fy, fz };
  if( x >= 0 && x < HP_XSIZE - 1 ) {
    if( y >= 0 && y < HP_YSIZE - 1 ) {
      // Find which triangular half of the grid square { fx, fy } is in (top/right or bottom/left)
      float ix, iy;
      ix = fmod( fx, HP_GRIDSIZE );
      iy = fmod( fy, HP_GRIDSIZE );

      //assert(ix >= 0.0 && ix < HP_GRIDSIZE);
      //assert(iy >= 0.0 && iy < HP_GRIDSIZE);

      glm::vec3 n = {0,0,0};
      glm::vec3 p = { fx, fy, fz };
      glm::vec3 origin;
      glm::vec3 v0;
      glm::vec3 v1;
      if( ix > ( HP_GRIDSIZE - iy ) ) {
        // top/left
        origin = { fx - ix, fy - iy, m_heightPlane[ x ][ y ] };
        v1 = { HP_GRIDSIZE, 0.0, m_heightPlane[ x + 1 ][ y ] - m_heightPlane[ x ][ y ] };
        v0 = { 0.0, HP_GRIDSIZE, m_heightPlane[ x ][ y + 1 ] - m_heightPlane[ x ][ y ] };
      } else {
        // bottom/right
        origin = {  ( fx - ix ) + HP_GRIDSIZE, ( fy - iy ) + HP_GRIDSIZE, m_heightPlane[ x + 1 ][ y + 1 ] };
        v0 = { 0, -HP_GRIDSIZE, m_heightPlane[ x + 1 ][ y ] - m_heightPlane[ x + 1 ][ y + 1 ] };
        v1 = { -HP_GRIDSIZE, 0, m_heightPlane[ x ][ y + 1 ] - m_heightPlane[ x + 1 ][ y + 1 ] };
      }

      float dn = closestPointOnPlane( v0, v1, origin, p, &cp, &n );
      glm::vec3 surfacePoint = { 
        fx + ( n.x * radius ),
        fy + ( n.y * radius ),
        fz + ( n.z * radius )
      }; 
      // Now that we have the closest point on the plane to the center,
      // we must call again with the surface so we can an appropriate distance.
      dn = closestPointOnPlane( v0, v1, origin, surfacePoint, &cp, &n );
      if( dn > 0 && -sqrt( pow(cp.x - fx, 2) + pow(cp.y - fy, 2) + pow(cp.z - fz, 2)) < radius ) {
        ap->x = cp.x - ( n.x * radius ); 
        ap->y = cp.y - ( n.y * radius ); 
        ap->z = cp.z - ( n.z * radius ); 
        bRet = true;
        printf("b");
      }
#if 1
      printf("BR { %2.2f %2.2f %2.2f }  { %2.2f %2.2f %2.2f }  %2.2f\n", 
          cp.x, cp.y, cp.z,
          n.x, n.y, n.z,
          dn
          );
#endif
    }
  }
  return bRet;
}

//#define DEBUG_HP
// getHeightAt
// Get precise height on height plane at position { x, y }
// This facilitate smooth movement along the surfiace.
// (Nasty little function chock full of linear algeraic formulae)
// Entry:  x position
//        y position
// Exit:  z position
//
float HeightPlane::getHeightAt( const float fx, const float fy )
{
  float h = 0.0;
  int x = (int) (fx / HP_GRIDSIZE);
  int y = (int) (fy / HP_GRIDSIZE);
  if( x >= 0 && x < HP_XSIZE - 1 ) {
    if( y >= 0 && y < HP_YSIZE - 1 ) {
      // Find which triangular half of the grid square { fx, fy } is in (top/right or bottom/left)
      float ix, iy;
      float ax, ay;
      ix = fmod( fx, HP_GRIDSIZE );
      iy = fmod( fy, HP_GRIDSIZE );

      //assert(ix >= 0.0 && ix < HP_GRIDSIZE);
      //assert(iy >= 0.0 && iy < HP_GRIDSIZE);

      ax = fx - ix;
      ay = fy - iy;

      // What's the vector, Victor?
      //
      // Spatial point convention:
      //   1   2
      //   3   4
      //
      // Origin:
      //   1
      // Vect { 2, 4 } has (2) as the origin.
      if( ix > ( HP_GRIDSIZE - iy ) ) {
        //iy = HP_GRIDSIZE - iy;
        // top/left
        // mean z
        glm::vec3 v1 = {
          ax,
          ay,
          m_heightPlane[ x ][ y ]
        };
        glm::vec3 v2 = {
          ( ax + HP_GRIDSIZE ),
          ay,
          m_heightPlane[ x + 1 ][ y ]
        };
        glm::vec3 v3 = {
          ax,
          ( ay + HP_GRIDSIZE ),
          m_heightPlane[ x ][ y + 1 ]
        };
        h = calcZ( v1, v2, v3, fx, fy );
      } else {
        // bottom/right
        glm::vec3 v2 = {
          ( ax + HP_GRIDSIZE),
          ay,
          m_heightPlane[ x + 1 ][ y ]
        };
        glm::vec3 v4 = {
          ( ax + HP_GRIDSIZE ),
          ( ay + HP_GRIDSIZE ),
          m_heightPlane[ x + 1 ][ y + 1 ]
        };
        glm::vec3 v3 = {
          ax,
          (ay + HP_GRIDSIZE),
          m_heightPlane[ x ][ y + 1 ]
        };
        h = calcZ( v2, v3, v4, fx, fy );
      }
    }
  }
  return h;
}

glm::vec3 HeightPlane::getNormalAt( const float fx, const float fy )
{
  int x = (int) (fx / HP_GRIDSIZE);
  int y = (int) (fy / HP_GRIDSIZE);
  glm::vec3 cp = { 0, 0, 0 };
  if( x >= 0 && x < HP_XSIZE - 1 ) {
    if( y >= 0 && y < HP_YSIZE - 1 ) {
      // Find which triangular half of the grid square { fx, fy } is in (top/right or bottom/left)
      float ix, iy;
      ix = fmod( fx, HP_GRIDSIZE );
      iy = fmod( fy, HP_GRIDSIZE );

      //assert(ix >= 0.0 && ix < HP_GRIDSIZE);
      //assert(iy >= 0.0 && iy < HP_GRIDSIZE);

      if( ix > ( HP_GRIDSIZE - iy ) ) {
        //iy = HP_GRIDSIZE - iy;
        // top/left
        // mean z
        glm::vec3 v12 = {
          HP_GRIDSIZE,
          0.0,
          m_heightPlane[ x + 1 ][ y ] - m_heightPlane[ x ][ y ]
        };
        glm::vec3 v13 = {
          0.0,
          HP_GRIDSIZE,
          m_heightPlane[ x ][ y + 1 ] - m_heightPlane[ x ][ y ]
        };
        cp = normalizeVec(crossProduct( v12, v13 ));

      } else {
        // bottom/right
        glm::vec3 v42 = {
          0,
          -HP_GRIDSIZE,
          m_heightPlane[ x + 1 ][ y ] - m_heightPlane[ x + 1 ][ y + 1 ]
        };
        glm::vec3 v43 = {
          -HP_GRIDSIZE,
          0,
          m_heightPlane[ x ][ y + 1 ] - m_heightPlane[ x + 1 ][ y + 1 ]
        };
        cp = normalizeVec( crossProduct( v43, v42 ) );
      }
    }
  }
  return cp;
}




// calcZ
// Use Barycentric coordinate algorithm to derive z from y on heightplane triangle
// Entry:  p1 vector1
//        p2 vector2
//        p3 vector3
//        x on triangle
//        y on triangle
// Exit:  z at { x, y }
//
float HeightPlane::calcZ(const glm::vec3 p1, const glm::vec3 p2, const glm::vec3 p3, const float x, const float y)
{
  float det = (p2.y - p3.y) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.y - p3.y);

  float l1 = ((p2.y - p3.y) * (x - p3.x) + (p3.x - p2.x) * (y - p3.y)) / det;
  float l2 = ((p3.y - p1.y) * (x - p3.x) + (p1.x - p3.x) * (y - p3.y)) / det;
  float l3 = 1.0f - l1 - l2;


  float z = l1 * p1.z + l2 * p2.z + l3 * p3.z;
#ifdef DEBUG_BARYCENTRIC
  printf("{%2.2f %2.2f %2.2f} {%2.2f %2.2f %2.2f} {%2.2f %2.2f %2.2f} [%2.2f %2.2f %2.2f] %2.2f %2.2f  %2.4f\n", 
      p1.x, p1.y, p1.z,
      p2.x, p2.y, p2.z,
      p3.x, p3.y, p3.z,
      l1, l2, l3,
      x, y, z 
      );
#endif
  return z;

  //return l1 * p1.z + l2 * p2.z + l3 * p3.z;
}

