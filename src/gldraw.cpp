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

using namespace std;

float mo[16];

float g_angle = 0.0;
float g_rotX = 0.0;
float g_rotY = 1.0;
float g_rotZ = 0.0;

#define HP_XSIZE 200
#define HP_YSIZE 200
#define HP_GRIDSIZE 1.0
#define HP_XMID (HP_GRIDSIZE * HP_XSIZE / 2)
#define HP_YMID (HP_GRIDSIZE * HP_YSIZE / 2)

float g_heightPlane[ HP_XSIZE ][ HP_YSIZE ];

// Global positioning

float g_xpos;
float g_ypos;
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

#define TURN_DAMPER 12
#define VEL_DAMPER 16
#define VEL_DEFAULT 0.05
#define VEL_MAX 0.85

void updatePosition()
{
  g_vel += (g_velTar - g_vel) / VEL_DAMPER;

#ifndef DEBUG_STEPPOS
  g_xpos += sin(g_xyDir) * g_vel;
  g_ypos += cos(g_xyDir) * g_vel;
#endif

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

  g_xyDir += (g_xyDirTar - g_xyDir) / TURN_DAMPER;

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
  updatePosition();

  usleep(16666);
  //glutSetWindow(g_window);
  glutPostRedisplay();   // Trigger an automatic redraw for animation
  glutTimerFunc( val, update, val);
}

void updateVisibility(int state )
{
  if( !g_updateThreadCreated ) {
    g_updateThreadCreated = true;
    glutTimerFunc( 16, update, 16 );
  }
}

// initHeightPlane
//
void initHeightPlane()
{
  int x, y;
  float theta = 0;
  float atten;
  for( x = 0; x < HP_XSIZE; x++ ) {
    for( y = 0; y < HP_YSIZE; y++ ) {
      double h;
      // Apply attenuation function over all
      atten = abs(
          sqrt(
            pow( ( float ) x - ( float ) HP_XMID, 2 ) +
            pow( ( float ) y - ( float ) HP_YMID, 2 )
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
      //atten = ( float ) (HP_XMID / pow(atten, 2) * atten);
      //atten = ( float ) HP_XMID * 1.415 - atten;
      if( HP_XMID == x  ) {
        printf( "%d %d %2.2f\n", x, y, atten );
      }

      //h = sin( x * (M_PI * 2 / HP_XSIZE) * 6 ) * 4;
      h = sin( theta * (M_PI * 2 / HP_XSIZE) * 6) * 1 + 
        sin( theta * (M_PI * 2 / HP_XSIZE) * 4.23 ) * 1.2;
        sin( theta * (M_PI * 2 / HP_XSIZE) * 9.1 ) * 1 +
        sin( theta * (M_PI * 2 / HP_XSIZE) *19.1 ) * 0.7;
      h += sin( y * (M_PI * 2 / HP_XSIZE) * 5.3) * 2.1 +
        sin( y * (M_PI * 2 / HP_XSIZE) * 4.1) * 3;
        sin( y * (M_PI * 2 / HP_XSIZE) * 11.1) * 1;
        
      h *= 5;
      h *= atten;
      g_heightPlane[ x ][ y ] = h;
      theta += 0.01;
    }
  }
}

// drawHeightPlane
//
void drawHeightPlane()
{
  int x, y;
  for( x = 0; x < HP_XSIZE - 1; x++ ) {
    for( y = 0; y < HP_YSIZE - 1; y++ ) {
      glVertex3f( (double) x * HP_GRIDSIZE, (double) y * HP_GRIDSIZE, g_heightPlane[ x ][ y ] );
      glNormal3f( (double) x * HP_GRIDSIZE, (double) y * HP_GRIDSIZE, g_heightPlane[ x ][ y ] );
      glVertex3f( (double) ( x + 1 ) * HP_GRIDSIZE , (double) y * HP_GRIDSIZE, g_heightPlane[ x + 1 ][ y ] );
      glNormal3f( (double) ( x + 1 ) * HP_GRIDSIZE , (double) y * HP_GRIDSIZE, g_heightPlane[ x + 1 ][ y ] );
      glVertex3f( (double) x * HP_GRIDSIZE, (double) ( y + 1) * HP_GRIDSIZE, g_heightPlane[ x ][ y + 1 ] );
      glNormal3f( (double) x * HP_GRIDSIZE, (double) ( y + 1) * HP_GRIDSIZE, g_heightPlane[ x ][ y + 1 ] );

      glVertex3f( (double) ( x + 1 ) * HP_GRIDSIZE , (double) y * HP_GRIDSIZE, g_heightPlane[ x + 1 ][ y ] );
      glNormal3f( (double) ( x + 1 ) * HP_GRIDSIZE , (double) y * HP_GRIDSIZE, g_heightPlane[ x + 1 ][ y ] );
      glVertex3f( (double) ( x + 1) * HP_GRIDSIZE , (double) ( y + 1 ) * HP_GRIDSIZE, g_heightPlane[ x + 1 ][ y + 1 ] );
      glNormal3f( (double) ( x + 1) * HP_GRIDSIZE , (double) ( y + 1 ) * HP_GRIDSIZE, g_heightPlane[ x + 1 ][ y + 1 ] );
      glVertex3f( (double) x * HP_GRIDSIZE, (double) ( y + 1 ) * HP_GRIDSIZE, g_heightPlane[ x ][ y + 1 ] );
      glNormal3f( (double) x * HP_GRIDSIZE, (double) ( y + 1 ) * HP_GRIDSIZE, g_heightPlane[ x ][ y + 1 ] );
    }
  }
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
float calcZ(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float x, float y)
{
  float det = (p2.y - p3.y) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.y - p3.y);

  float l1 = ((p2.y - p3.y) * (x - p3.x) + (p3.x - p2.x) * (y - p3.y)) / det;
  float l2 = ((p3.y - p1.y) * (x - p3.x) + (p1.x - p3.x) * (y - p3.y)) / det;
  float l3 = 1.0f - l1 - l2;

  return l1 * p1.z + l2 * p2.z + l3 * p3.z;
}

// getHeightAt
// Get precise height on height plane at position { x, y }
// This facilitate smooth movement along the surfiace.
// (Nasty little function chock full of linear algeraic formulae)
// Entry:  x position
//        y position
// Exit:  z position
//
float getHeightAt( float fx, float fy )
{
  float h = 0.0;
  int x = (int) (fx / HP_GRIDSIZE);
  int y = (int) (fy / HP_GRIDSIZE);
  if( x >= 0 && x < HP_XSIZE - 1 ) {
    if( y >= 0 && y < HP_YSIZE - 1 ) {
      // Find which triangular half of the grid square { fx, fy } is in (top/right or bottom/left)
      float ix, iy;
      float h1, h2, h3;
      ix = fmod( fx, HP_GRIDSIZE );
      iy = fmod( fy, HP_GRIDSIZE );

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
        float zm = (
            g_heightPlane[ x ][ y ] +
            g_heightPlane[ x + 1 ][ y ] +
            g_heightPlane[ x + 1 ][ y + 1 ] +
            g_heightPlane[ x ][ y + 1 ]
            ) / 4;
        glm::vec3 v12 = {
          (fx + HP_GRIDSIZE) - fx,
          fy - fy,
          g_heightPlane[ x + 1 ][ y ] - g_heightPlane[ x ][ y ]
        };
        glm::vec3 v23 = {
          fx - (fx + HP_GRIDSIZE),
          (fy + HP_GRIDSIZE) - fy,
          g_heightPlane[ x ][ y + 1 ] - g_heightPlane[ x + 1 ][ y ]
        };
        glm::vec3 v31 = {
          fx - fx,
          fy - (fy + HP_GRIDSIZE),
          g_heightPlane[ x ][ y ] - g_heightPlane[ x ][ y + 1 ]
        };
        //v12 = normalize( v12 );
        //v23 = normalize( v23 );
        //v31 = normalize( v31 );
        h = calcZ( v12, v23, v31, ix, iy );
#ifdef DEBUG_HP        
        printf( "TL %2.2f  %2.2f  %2.2f  { %2.2f %2.2f %2.2f } { %2.2f %2.2f %2.2f } { %2.2f %2.2f %2.2f } \n",
            zm, h, zm + h,
            v12.x, v12.y, v12.x,
            v23.x, v23.y, v23.z,
            v31.x, v31.y, v31.z
            );
#endif
        h += zm;
      } else {
        // bottom/right
        // mean z
        float zm = (
            g_heightPlane[ x ][ y ] +
            g_heightPlane[ x + 1 ][ y ] +
            g_heightPlane[ x + 1 ][ y + 1 ] +
            g_heightPlane[ x ][ y + 1 ]
            ) / 4;
        glm::vec3 v24 = {
          (fx + HP_GRIDSIZE) - (fx + HP_GRIDSIZE),
          (fy + HP_GRIDSIZE) - fy,
          g_heightPlane[ x + 1 ][ y + 1 ] - g_heightPlane[ x + 1 ][ y ]
        };
        glm::vec3 v43 = {
          fx - (fx + HP_GRIDSIZE),
          (fy + HP_GRIDSIZE) - (fy + HP_GRIDSIZE),
          g_heightPlane[ x ][ y + 1 ] - g_heightPlane[ x + 1 ][ y + 1 ]
        };
        glm::vec3 v32 = {
          (fx + HP_GRIDSIZE) - fx,
          fy - (fy + HP_GRIDSIZE),
          g_heightPlane[ x + 1 ][ y ] - g_heightPlane[ x ][ y + 1 ]
        };
        //v24 = normalize( v24 );
        //v43 = normalize( v43 );
        //v32 = normalize( v32 );
        h = calcZ( v24, v43, v32, ix, iy );
#ifdef DEBUG_HP        
        printf( "BR %2.2f  %2.2f  %2.2f  { %2.2f %2.2f %2.2f } { %2.2f %2.2f %2.2f } { %2.2f %2.2f %2.2f } \n",
            zm, h, zm + h,
            v24.x, v24.y, v24.x,
            v43.x, v43.y, v43.z,
            v32.x, v32.y, v32.z
            );
#endif
        h += zm;
      }
    }
  }
  return h;
}

void init()
{
  g_xpos = 3.0;
  g_ypos = 3.0;
  g_xyDir = M_PI + M_PI / 4;
  g_xyDirTar = g_xyDir;
  g_vel = 0.0;
  g_velTar = 0.0;
}

int main( int argc, char **argv )
{

  init();

  initHeightPlane();

  //
  glutInit(&argc,argv);

  // We're going to animate it, so double buffer
  glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
  glutInitWindowSize( 512, 512 );

  g_window = glutCreateWindow( "Sunny Meadow" );
  glutDisplayFunc( display );
  glutKeyboardFunc( keyboard );
  glutSpecialFunc( specialKeys );
  glutVisibilityFunc( updateVisibility );

  initGlobalMatrix();

  glutTimerFunc(16, update, 1);
  glutMainLoop();

  return 0;
}

// Set up fog effect
//
void setFog()
{
  GLuint filter;                                      // Which Filter To Use
  GLuint fogMode[]= { GL_EXP, GL_EXP2, GL_LINEAR };   // Storage For Three Types Of Fog
  GLuint fogfilter= 0;                                // Which Fog To Use
  GLfloat fogColor[4]= {0.1f, 0.5f, 0.75f, 1.0f};     // Fog Color


  glFogi(GL_FOG_MODE, fogMode[fogfilter]);  // Fog Mode
  glFogfv(GL_FOG_COLOR, fogColor);          // Set Fog Color
  glFogf(GL_FOG_DENSITY, 0.0025f);          // How Dense Will The Fog Be
  glHint(GL_FOG_HINT, GL_NICEST);           // Fog Hint Value
  glFogf(GL_FOG_START, 0.1f);               // Fog Start Depth
  glFogf(GL_FOG_END, 300.0f);               // Fog End Depth
  glEnable(GL_FOG);                         // Enables GL_FOG
}

// Set up camera
//
void setCamera()
{
  GLint viewport[4];
  glGetIntegerv( GL_VIEWPORT, viewport );


  gluPerspective( 45, double(viewport[2])/viewport[3], 0.01, 300 );
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt( g_xpos,
      g_ypos,
      getHeightAt( g_xpos, g_ypos ) + 2,
      g_xpos - sin(g_xyDir),
      g_ypos - cos(g_xyDir),
      getHeightAt( g_xpos, g_ypos ) + 2,
      0,
      0,
      3 );
  glMultMatrixf(mo);
}

GLfloat ambientLightGlobal[] = {0.2f, 0.2f, 0.2f, 1.0f};
GLfloat lightPos[] ={100.0, 150.0, -10000, 0.0};
GLfloat ambientLight[] = {0.0f, 0.3f, 0.0f, 1.0f};
GLfloat diffuseLight[] = {0.8f, 0.8f, 0.8f, 1.0f};
GLfloat specularLight[] = {0.9f, 0.9f, 0.9f, 1.0f};
GLfloat materialColor1[] = {0.4f, 0.5f, 0.1f, 0.9f};
GLfloat materialEmission[] = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat materialAmbient[] = {0.4f, 0.5f, 0.1f, 1.0f};
GLfloat materialSpecular[] = {0.41, 0.52, 0.11, 0.2};



// Set up light
//
void setLight()
{

  //glMatrixMode( GL_MODELVIEW );         // Current matrix affects objects positions
  //glLoadIdentity();                  // Initialize to the identity

  //Diffuse (non-shiny) light component
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
  //Specular (shiny) light component
  glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
  // Global ambient
  glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);

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
  //glTranslatef( HP_XMID, HP_YMID, 100.0 );               // Translate rotation center from origin
  //glUniform3f(GL_LIGHT0, HP_XMID, HP_YMID, 100.0);
  // Light position
  glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

  //glPopMatrix();
}

// Set up material
//
void setPlaneMaterial()
{
  glColor3f(0.5,0.7,0.2);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, materialAmbient);
  glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
  glMaterialfv(GL_FRONT, GL_EMISSION, materialEmission);
  glMaterialf(GL_FRONT, GL_SHININESS,100.8); //The shininess parameter
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

  //  glMultMatrixf(mo);

  //  glGetFloatv( GL_MODELVIEW_MATRIX, mo );

  glPopMatrix();

  setLight();
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLightGlobal);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_COLOR_MATERIAL);

  // Clear to sky color / draw sky
  glClearColor( 0.1, 0.5, 0.75, 1.0 );

  setFog();

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  setCamera();
  // Clear the rendering window
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glPolygonMode( GL_FRONT, GL_FILL);

  glBegin( GL_TRIANGLES );
  setPlaneMaterial();
  drawHeightPlane();
  glEnd();

  // Flush the pipeline, swap the buffers
  glFlush();
  glutSwapBuffers();
}

// initGlobalMatrix
//
void initGlobalMatrix()
{
  memset( mo, 0, sizeof(mo) );
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
  if( g_angle > 360) {
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
// Handle non-visible keys like arrows (OpenGL idiosyncrasy)
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
      g_xpos += sin(g_xyDir) * VEL_DEFAULT;
      g_ypos += cos(g_xyDir) * VEL_DEFAULT;
#endif
      break;
    case GLUT_KEY_UP:

      g_velTar = -VEL_DEFAULT;
      if( g_velTar < -VEL_MAX )
      {
        g_velTar = -VEL_MAX;
      }
#ifdef DEBUG_STEPPOS
      // TEMP CODE; REMOVE
      g_xpos -= sin(g_xyDir) * VEL_DEFAULT;
      g_ypos -= cos(g_xyDir) * VEL_DEFAULT;
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


