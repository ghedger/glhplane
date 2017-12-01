// Playfield implementation
//

#include "common.h"
#include "playfield.h"

#include <GL/glut.h>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include <unistd.h>
#include <math.h>

//
// PUBLIC
//

Playfield::Playfield()
{
  this->init();
}

Playfield::~Playfield()
{
}

// init
//
bool Playfield::init()
{
  bool bRet = true;
  if( m_heightPlane ) {
    initHeightPlane();
  }
  return bRet;
}

// draw
void Playfield::draw()
{
  glPolygonMode( GL_FRONT, GL_FILL);
  glBegin( GL_TRIANGLES);
  setPlaneMaterial2();
  drawSolids();
  glEnd();
  glFlush();

  //glClear(GL_DEPTH_BUFFER_BIT);
  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
  glBegin( GL_QUADS);
  setPlaneMaterial();
  drawLines();
  glEnd();
}

//
// DRAWING
//

// drawHeightPlaneLines
//
void Playfield::drawLines()
{
  int x, y;
  for( x = 0; x < HP_XSIZE - 1; x++ ) {
    for( y = 0; y < HP_YSIZE - 1; y++ ) {
      glVertex3f( (double) x * HP_GRIDSIZE, (double) y * HP_GRIDSIZE, m_heightPlane[ x ][ y ] + 0.01 );
      glVertex3f( (double) ( x + 1 ) * HP_GRIDSIZE , (double) y * HP_GRIDSIZE, m_heightPlane[ x + 1 ][ y ] + 0.01 );
      glVertex3f( (double) ( x + 1) * HP_GRIDSIZE , (double) ( y + 1 ) * HP_GRIDSIZE, m_heightPlane[ x + 1 ][ y + 1 ] + 0.01 );
      glVertex3f( (double) x * HP_GRIDSIZE, (double) ( y + 1 ) * HP_GRIDSIZE, m_heightPlane[ x ][ y + 1 ] + 0.01 );
    }
  }
}

static bool g_squareWhite = 0;
// drawHeightPlane
//
void Playfield::drawSolids()
{
  int x, y;

  g_squareWhite = true;

  for( x = 0; x < HP_XSIZE - 1; x++ ) {
    for( y = 0; y < HP_YSIZE - 1; y++ ) {
      if( !g_squareWhite ) {
        glColor3f(0.0, 0.0, 0.0 );
      }  else {
        glColor3f(0.9, 0.9, 0.9 );
      }

      g_squareWhite = !g_squareWhite;

      glVertex3f( (double) x * HP_GRIDSIZE, (double) y * HP_GRIDSIZE, m_heightPlane[ x ][ y ] );
      glNormal3f( (double) x * HP_GRIDSIZE, (double) y * HP_GRIDSIZE, m_heightPlane[ x ][ y ] );
      glVertex3f( (double) ( x + 1 ) * HP_GRIDSIZE , (double) y * HP_GRIDSIZE, m_heightPlane[ x + 1 ][ y ] );
      glNormal3f( (double) ( x + 1 ) * HP_GRIDSIZE , (double) y * HP_GRIDSIZE, m_heightPlane[ x + 1 ][ y ] );
      glVertex3f( (double) x * HP_GRIDSIZE, (double) ( y + 1) * HP_GRIDSIZE, m_heightPlane[ x ][ y + 1 ] );
      glNormal3f( (double) x * HP_GRIDSIZE, (double) ( y + 1) * HP_GRIDSIZE, m_heightPlane[ x ][ y + 1 ] );

      glVertex3f( (double) ( x + 1 ) * HP_GRIDSIZE , (double) y * HP_GRIDSIZE, m_heightPlane[ x + 1 ][ y ] );
      glNormal3f( (double) ( x + 1 ) * HP_GRIDSIZE , (double) y * HP_GRIDSIZE, m_heightPlane[ x + 1 ][ y ] );
      glVertex3f( (double) ( x + 1) * HP_GRIDSIZE , (double) ( y + 1 ) * HP_GRIDSIZE, m_heightPlane[ x + 1 ][ y + 1 ] );
      glNormal3f( (double) ( x + 1) * HP_GRIDSIZE , (double) ( y + 1 ) * HP_GRIDSIZE, m_heightPlane[ x + 1 ][ y + 1 ] );
      glVertex3f( (double) x * HP_GRIDSIZE, (double) ( y + 1 ) * HP_GRIDSIZE, m_heightPlane[ x ][ y + 1 ] );
      glNormal3f( (double) x * HP_GRIDSIZE, (double) ( y + 1 ) * HP_GRIDSIZE, m_heightPlane[ x ][ y + 1 ] );
    }
  }
}


// Set up material
//
void Playfield::setPlaneMaterial()
{
  GLfloat materialEmission[] = {1.0f, 1.0f, 1.0f, 1.0f};
  GLfloat materialAmbient[] = {0.4f, 0.5f, 0.1f, 1.0f};
  //GLfloat materialSpecular[] = {0.41, 0.52, 0.11, 0.2};

  glColor3f(1.0, 1.0, 1.0 );
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, materialAmbient);
  //glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, materialEmission);
  //glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS,100.8); //The shininess parameter
}

// Set up material
//
void Playfield::setPlaneMaterial2()
{
  GLfloat material2Emission[] = {0.0f, 0.0f, 0.0f, 1.0f};
  GLfloat material2Ambient[] = {0.4f, 0.5f, 0.1f, 1.0f};

  glColor3f(0.0, 0.0, 0.0 );
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, material2Ambient);
  //glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, material2Emission);
  //glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS,100.8); //The shininess parameter
}




