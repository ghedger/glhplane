// Main entry point

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <GL/glut.h>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <assert.h>

#include "common.h"
#include "playfield.h"

using namespace std;

float mo[16];

float g_angle = 0.0;
float g_rotX = 0.0;
float g_rotY = 1.0;
float g_rotZ = 0.0;

Playfield *g_pPlayfield = NULL;

// Global positioning
float g_xpos;
float g_ypos;
float g_zpos;
float g_xposPrev;
float g_yposPrev;
float g_zposPrev;

float g_zposLookatTar;

float g_xyDir;
float g_xyDirTar;
float g_vel;
float g_velTar;

// Window scaffolding
int g_window;
static int g_updateThreadCreated = false;

// Function prototypes

void initGlobalMatrix();
void updateGlobalMatrix( float angle, float x, float y, float z );

void display();
void keyboard( unsigned char key, int x, int y );
void specialKeys( int key, int x, int y );

void updatePosition()
{
  g_vel += ( g_velTar - g_vel ) / VEL_DAMPER;

	g_xposPrev = g_xpos;
	g_yposPrev = g_ypos;
	g_zposPrev = g_zpos;

#ifndef DEBUG_STEPPOS
  g_xpos += sin( g_xyDir ) * g_vel;
  g_ypos += cos( g_xyDir ) * g_vel;
#endif
	g_zpos = g_pPlayfield->getHeightAt( g_xpos, g_ypos );

  if( abs( g_xyDir - g_xyDirTar ) > M_PI )
  {
    if( g_xyDir < M_PI )
    {
      if( g_xyDirTar > M_PI )
      {
        g_xyDirTar -= M_PI * 2;
      }
    } else {
      if( g_xyDirTar < M_PI )
      {
        g_xyDirTar += M_PI * 2;
      }
    }
  }

  g_xyDir += ( g_xyDirTar - g_xyDir ) / TURN_DAMPER;

  if( g_xyDir < 0 ) {
    g_xyDir += M_PI * 2;
  }
  if( g_xyDirTar < 0 ) {
    g_xyDirTar += M_PI * 2;
  }
  if( g_xyDir > M_PI * 2 ) {
    g_xyDir -= M_PI * 2;
  }
  if( g_xyDirTar > M_PI * 2 ) {
    g_xyDirTar -= M_PI * 2;
  }

  g_xyDir = fmod( g_xyDir, M_PI * 2 );
  g_xyDirTar = fmod( g_xyDirTar, M_PI * 2 );
}

// Main update loop, run as thread and updates at regular intervals
//
void update( int val )
{

  usleep( 16666 );
  updatePosition();
  //glutSetWindow( g_window );
  glutPostRedisplay();   // Trigger an automatic redraw for animation

  glutTimerFunc( val, update, val );
}

void updateVisibility( int state )
{
  if( !g_updateThreadCreated ) {
    g_updateThreadCreated = true;
    glutTimerFunc( 16, update, 16 );
  }
}

void init()
{
	// Init global positioning
  g_xpos = 3.0;
  g_ypos = 3.0;
  g_zpos = 0.0;
  g_zposLookatTar = 0.0;
  g_xyDir = M_PI + M_PI / 4;
  g_xyDirTar = g_xyDir;
  g_vel = 0.0;
  g_velTar = 0.0;

	// Init playfield
	g_pPlayfield = new Playfield();
}

int main( int argc, char **argv )
{

  init();

  // initHeightPlane();

  //
  glutInit(&argc,argv );

  // We're going to animate it, so double buffer
  glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
  glutInitWindowSize( 512, 512 );

  g_window = glutCreateWindow( "Sunny Meadow" );
  glutDisplayFunc( display );
  glutKeyboardFunc( keyboard );
  glutSpecialFunc( specialKeys );
  glutVisibilityFunc( updateVisibility );

  initGlobalMatrix();

  glutTimerFunc( 16, update, 1 );
  glutMainLoop();

  return 0;
}

// Set up fog effect
//
void setFog()
{
  GLuint fogMode[]= { GL_EXP, GL_EXP2, GL_LINEAR };   // Storage For Three Types Of Fog
  GLuint fogfilter= 0;                                // Which Fog To Use
  GLfloat fogColor[ 4 ]= { 0.2f, 0.4f, 0.55f, 1.0f };     // Fog Color

  glFogi( GL_FOG_MODE, fogMode[ fogfilter ] );
  glFogfv( GL_FOG_COLOR, fogColor );
  glFogf( GL_FOG_DENSITY, 0.0125f );
  glHint( GL_FOG_HINT, GL_NICEST );
  glFogf( GL_FOG_START, 0.1f );
  glFogf( GL_FOG_END, 300.0f );
  glEnable( GL_FOG );
}

// Set up camera
//
void setCamera()
{
  GLint viewport[4];
  glGetIntegerv( GL_VIEWPORT, viewport );

  gluPerspective( 45, double( viewport[2])/viewport[3], 0.01, 300 );
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

	g_zposLookatTar += ( ( g_zpos - g_zposPrev ) - g_zposLookatTar ) / 4;

  gluLookAt(
			g_xpos,
      g_ypos,
      g_zpos + 2.0 - g_zposLookatTar,
      g_xpos - sin( g_xyDir ),
      g_ypos - cos( g_xyDir ),
      g_zpos + 2.0,
      0,
      0,
      3
	);
  glMultMatrixf( mo );
}

GLfloat ambientLightGlobal[] = {0.2f, 0.2f, 0.2f, 1.0f};
GLfloat lightPos[] ={100.0, 150.0, -10000, 0.0};
GLfloat ambientLight[] = {0.0f, 0.3f, 0.0f, 1.0f};
GLfloat diffuseLight[] = {0.8f, 0.8f, 0.8f, 1.0f};
GLfloat specularLight[] = {0.9f, 0.9f, 0.9f, 1.0f};


// Set up light
//
void setLight()
{
  //glMatrixMode( GL_MODELVIEW );         // Current matrix affects objects positions
  //glLoadIdentity();                  // Initialize to the identity

  //Diffuse ( non-shiny ) light component
  glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuseLight );
  //Specular ( shiny ) light component
  glLightfv( GL_LIGHT0, GL_SPECULAR, specularLight );
  // Global ambient
  glLightfv( GL_LIGHT0, GL_AMBIENT, ambientLight );

#if 0
  // Normalize the light. Note, only needs must be done once.
  {
    float sum = 0.0;
    for( int i = 0; i < 3; i++ ) {
      sum += lightPos[i];
    }

    for( int i = 0; i < 3; i++ ) {
      lightPos[i] = lightPos[i] / sum;
    }
  }
#endif

  // Light position
  glLightfv( GL_LIGHT0, GL_POSITION, lightPos );
}



// Display
// Update the display
//
void display()
{
  glPushMatrix();
  glLoadIdentity();

  // Rotate the image
  glMatrixMode( GL_MODELVIEW );         // Current matrix affects objects positions
  glLoadIdentity();                  // Initialize to the identity

  glTranslatef( HP_XMID, HP_YMID, 0.0 );               // Translate rotation center from origin
  glRotatef( g_angle, g_rotX, g_rotY, g_rotZ );
  glTranslatef( -HP_XMID, -HP_YMID, 0.0 );            // Translate rotation center to origin

  //  glMultMatrixf( mo );
  //  glGetFloatv( GL_MODELVIEW_MATRIX, mo );

  glPopMatrix();

  //setLight();
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  glLightModelfv( GL_LIGHT_MODEL_AMBIENT, ambientLightGlobal );

  glEnable( GL_DEPTH_TEST );
  glEnable( GL_NORMALIZE );
  //glEnable( GL_LIGHTING );
  //glEnable( GL_LIGHT0 );
  glShadeModel( GL_FLAT );
  glEnable( GL_COLOR_MATERIAL );

  // Clear to sky color / draw sky
  glClearColor( 0.2, 0.4, 0.55, 1.0 );
  setFog();

  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  setCamera();
	g_pPlayfield->draw();

  // Flush the pipeline, swap the buffers
  glFlush();
  glutSwapBuffers();
}

// initGlobalMatrix
//
void initGlobalMatrix()
{
  memset( mo, 0, sizeof( mo ) );
  mo[0]=mo[5]=mo[10]=mo[15]=1;
  glutPostRedisplay();
}

// updateGlobalMatrix
// Change the matrix based on world position
// Entry: angle
//        x rotation
//        y rotation
//        z rotation
//
void updateGlobalMatrix( float angle, float x, float y, float z )
{
  g_rotX = x;
  g_rotY = y;
  g_rotZ = z;
  g_angle = angle;
  if( g_angle > 360 ) {
    g_angle -= 360;
  }
  if( g_angle < 0 ) {
    g_angle += 360;
  }

}

// keyboard
// Handle regular character keys
// Entry:  key code
//        x
//        y
//
void keyboard( unsigned char key, int x, int y )
{
  cout << "keyboard: " << key << std::endl;

  switch( key )
  {
    case 27:
      initGlobalMatrix();
      updateGlobalMatrix( 0.0, 1,0,0 );
      break;

    case 'c':
			g_pPlayfield->setParam( 0, 0 );
			break;
    case 'g':
			g_pPlayfield->setParam( 0, 1 );
			break;

    case '1':
      updateGlobalMatrix( -10.0, 1,0,0 );
      break;
    case '2':
      updateGlobalMatrix( 10.0, 1,0,0 );
      break;

    case '3':
      updateGlobalMatrix( -10.0, 0,1,0 );
      break;
    case '4':
      updateGlobalMatrix( 10.0, 0,1,0 );
      break;

    case '5':
      updateGlobalMatrix( -10.0, 0,0,1 );
      break;
    case '6':
      updateGlobalMatrix( 10.0, 0,0,1 );
      break;
    default:
      break;
  }
}

// specialKeys
// Handle non-visible keys like arrows ( OpenGL idiosyncrasy )
// Entry:  key code
//        x
//        y
//
void specialKeys( int key, int x, int y )
{
  switch( key ) {
    case GLUT_KEY_DOWN:
      g_velTar += VEL_DEFAULT;
      if( g_velTar > VEL_MAX )
      {
        g_velTar = VEL_MAX;
      }
#ifdef DEBUG_STEPPOS
      // TEMP CODE; REMOVE
      g_xpos += sin( g_xyDir ) * VEL_DEFAULT;
      g_ypos += cos( g_xyDir ) * VEL_DEFAULT;
#endif
      break;
    case GLUT_KEY_UP:
      g_velTar += -VEL_DEFAULT;
      if( g_velTar < -VEL_MAX )
      {
        g_velTar = -VEL_MAX;
      }
#ifdef DEBUG_STEPPOS
      // TEMP CODE; REMOVE
      g_xpos -= sin( g_xyDir ) * VEL_DEFAULT;
      g_ypos -= cos( g_xyDir ) * VEL_DEFAULT;
#endif
      break;
    case GLUT_KEY_LEFT:
      g_xyDirTar -= M_PI / 8;
      if( g_xyDirTar < 0 ) {
        g_xyDirTar += M_PI * 2;
      }
      break;
    case GLUT_KEY_RIGHT:
      g_xyDirTar += M_PI / 8;
      if( g_xyDirTar > M_PI * 2 ) {
        g_xyDirTar -= M_PI * 2;
      }
      break;
    default:
      break;
  }
}


