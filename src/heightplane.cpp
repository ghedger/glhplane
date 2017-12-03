
// HeightPlane implementation

#include "common.h"
#include "heightplane.h"

#include <GL/glut.h>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include <unistd.h>
#include <math.h>

#include <memory.h>

#include <stdio.h>

HeightPlane::HeightPlane()
{
  m_param[0] = 0;
  m_param[1] = 0;
  m_param[2] = 0;
  m_param[3] = 0;
}

HeightPlane::~HeightPlane()
{
}

// initHeightPlane
//
bool HeightPlane::initHeightPlane()
{
  bool bRet = true;
  int x, y;
  double theta = 0;
  double atten = 0;
  double attenPrev;
  for( x = 0; x < HP_XSIZE; x++ ) {
    for( y = 0; y < HP_YSIZE; y++ ) {
      double h;
      // Apply attenuation function over all
      attenPrev = atten;
      atten = abs(
          sqrt(
            pow( ( float ) x - ( float ) HP_XMID_GRID, 2 ) +
            pow( ( float ) y - ( float ) HP_YMID_GRID, 2 )
          )
      );
      if( atten < 1 ) {
        atten = 1;
      }

      //atten /= HP_XMID;

      //atten = 0.5 - pow ( 1 / ( 1 + pow(atten, -2 ) ), 25 );
      atten = 1 - pow ( 1 / ( 1 + pow(atten, -1 ) ), 30);
      atten -= 0.25;

      if( atten < 0 ) {
        atten = 0;
      }

      // Simple smoothing function
      atten = (atten + attenPrev) / 2;

      //atten = ( float ) (HP_XMID / pow(atten, 2) * atten);
      //atten = ( float ) HP_XMID * 1.415 - atten;

      h = sin( x * ( M_PI * 2 ) / HP_XSIZE ) * 30 +
          sin( y * ( M_PI * 2 ) / HP_YSIZE ) * 30;

      h += sin( x * ( M_PI * 2 ) / HP_XSIZE * 2 ) * 16 +
          sin( y * ( M_PI * 2 ) / HP_YSIZE * 2 ) * 16;

      h += sin( x * ( M_PI * 2 ) / HP_XSIZE * 4 ) * 8 +
          sin( y * ( M_PI * 2 ) / HP_YSIZE * 4 ) * 8;

      h += sin( x * ( M_PI * 2 ) / HP_XSIZE * 8 ) * 3 +
          sin( y * ( M_PI * 2 ) / HP_YSIZE * 8 ) * 3;



      /*
      h = sin( theta * (M_PI * 2 / HP_XSIZE) * 6) * 1 + 
        sin( theta * (M_PI * 2 / HP_XSIZE) * 4.23 ) * 1.2;
        sin( theta * (M_PI * 2 / HP_XSIZE) * 9.1 ) * 1;
      h += sin( y * (M_PI * 2 / HP_XSIZE) * 5.3) * 2.1 +
        sin( y * (M_PI * 2 / HP_XSIZE) * 4.1) * 3;
        sin( y * (M_PI * 2 / HP_XSIZE) * 11.1) * 1;
      */
      //h *= 10;
      //h *= atten;

      if( x > 0 ) {
        h += 100.0 / (HP_XSIZE - x );
        h += 100.0 / x;
      }
      if( y > 0 ) {
        h += 100.0 / (HP_YSIZE - y );
        h += 100.0 / y;
      }


      m_heightPlane[ x ][ y ] = h;
      theta += 0.011;
    }
  }
  return bRet;
}


// TODO: Move this to a vector math lib
glm::vec3 crossProduct( glm::vec3 v1,  glm::vec3 v2 )
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

/*
float HeightPlane::getCrossing( const float fx, const float fy, const float fz )
{

}
*/


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
      float ax, ay;
      ix = fmod( fx, HP_GRIDSIZE );
      iy = fmod( fy, HP_GRIDSIZE );

      //assert(ix >= 0.0 && ix < HP_GRIDSIZE);
      //assert(iy >= 0.0 && iy < HP_GRIDSIZE);

			ax = fx - ix;
			ay = fy - iy;

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

        printf("TL");
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
        printf("BR");
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

