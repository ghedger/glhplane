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

void initGlobalMatrix();
void updateGlobalMatrix( float angle, float x, float y, float z );

void display();
void keyboard( unsigned char key, int x, int y );
void specialKeys( int key, int x, int y );

int g_window;
static int g_updateThreadCreated = false;

// Main update, run as thread and updates at regular intervals
void updateVideoFrame(int val)
{
  usleep(16666);
  //glutSetWindow(g_window);
  glutPostRedisplay();   // Trigger an automatic redraw for animation
  glutTimerFunc( val, updateVideoFrame, val);
}

void updateVisibility(int state )
{
  if( !g_updateThreadCreated ) {
    g_updateThreadCreated = true;
    glutTimerFunc( 16, updateVideoFrame, 16 );
  }
}

#define HP_XSIZE 200
#define HP_YSIZE 200
#define HP_GRIDSIZE 1.0
#define HP_XMID (HP_GRIDSIZE * HP_XSIZE / 2)
#define HP_YMID (HP_GRIDSIZE * HP_YSIZE / 2)

float g_heightPlane[ HP_XSIZE ][ HP_YSIZE ];

float g_xpos;
float g_ypos;
float g_xyDir;

void initHeightPlane()
{
  int x, y;
  float theta = 0;
  for( x = 0; x < HP_XSIZE; x++ ) {
    for( y = 0; y < HP_YSIZE; y++ ) {
      double h;
      //h = sin( x * (M_PI * 2 / HP_XSIZE) * 6 ) * 4;
      h = sin( theta * (M_PI * 2 / HP_XSIZE) * 6) * 1 + sin( theta * (M_PI * 2 / HP_XSIZE) * 4.23 ) * 1.2;
      h += cos( y * (M_PI * 2 / HP_XSIZE) * 5.3) * 2.1 +  cos( y * (M_PI * 2 / HP_XSIZE) * 4.1) * 3;
      g_heightPlane[ x ][ y ] = h * 1.5;
      theta += 0.01;
    }
  }
}

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
// Entry:	p1 vector1
//				p2 vector2
//				p3 vector3
//				x on triangle
//				y on triangle
// Exit:	z at { x, y }
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
// Entry:   xposition
//          yposition
// Exit:    zposition
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
        // top/left
				// mean z
				float zm = (
					g_heightPlane[ x ][ y ] +
					g_heightPlane[ x + 1 ][ y ] +
					g_heightPlane[ x + 1 ][ y + 1 ] +
					g_heightPlane[ x ][ y + 1 ]
				) / 4;

        h1 = g_heightPlane[ x ][ y ];
        h2 = g_heightPlane[ x + 1 ][ y ];
        h3 = g_heightPlane[ x + 1 ][ y + 1 ];
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

				h = calcZ( v12, v23, v31, ix, iy );
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

				h = calcZ( v24, v43, v32, ix, iy );
				h += zm;
      }
      //h = g_heightPlane[ x ][ y ];
    }
  }
  return h;
}

void init()
{
  g_xpos = 0;
  g_ypos = 0;
  g_xyDir = M_PI + M_PI / 4;
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

  // glutEventTimer(16, update, 1);
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
GLfloat lightPos[] ={0.9, 0.7, -0.2, 0.0};
GLfloat ambientLight[] = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat diffuseLight[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat specularLight[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat materialColor1[] = {0.4f, 0.5f, 0.1f, 0.9f};
GLfloat materialEmission[] = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat materialAmbient[] = {0.4f, 0.5f, 0.1f, 1.0f};
GLfloat materialSpecular[] = {0.41, 0.52, 0.11, 0.2};

// Set up light
//
void setLight()
{
  //Diffuse (non-shiny) light component
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
  //Specular (shiny) light component
  glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
  // Global ambient
  glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);

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

  // Light position
  glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
}

// Set up material
//
void setPlaneMaterial()
{
  glColor3f(0.3,0.6,0.2);
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

  glMultMatrixf(mo);

  glGetFloatv( GL_MODELVIEW_MATRIX, mo );
  glPopMatrix();

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
  setLight();

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
// update the matrix based on world position
// Entry: angle
//				x rotation
//				y rotation
//				z rotation
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
// Entry:	key code
//				x
//				y
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
// Entry:	key code
//				x
//				y
//
void specialKeys( int key, int x, int y )
{
  switch( key ) {
    case GLUT_KEY_DOWN:
      g_xpos += sin(g_xyDir) * 0.2;
      g_ypos += cos(g_xyDir) * 0.2;
      break;
    case GLUT_KEY_UP:
      g_xpos -= sin(g_xyDir) * 0.2;
      g_ypos -= cos(g_xyDir) * 0.2;
      break;
    case GLUT_KEY_LEFT:
      g_xyDir -= 0.1;
      break;
    case GLUT_KEY_RIGHT:
      g_xyDir += 0.1;
      break;
    default:
      break;
  }
}


