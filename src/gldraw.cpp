// Main entry pointt

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

// TODO: This is crude, for debugging only.  Need a full xyz direction for movement
float g_zvel;
float g_zveli;


// Cartesian coordinates used for sliding
float g_xvel;
float g_yvel;

double ki = 0.0000;   // integral error multiplier constant
double kp = 0.0045;    // proportional error multipler constant

double pv[2] = {0.0, 0.0};          // process variable 
double t = 1.0;                      // discrete time delta constant
double pi[2] = {0.0, 0.0};          // process integral 
double e[2] = {0.0, 0.0};           // error 
double kg = 0.1;                    // gravity constant

double dp[2] = {0.0, 0.0};    // delta p 

double clamp = 10.0;  // maximum change per iteration
double windup_clamp = 10.0;

double kv = 0.5;

//#define USE_CLAMPS

// Window scaffolding
int g_window;
static int g_updateThreadCreated = false;

// Function prototypes

void initGlobalMatrix();
void updateGlobalMatrix( float angle, float x, float y, float z );

void display();
void keyboard( unsigned char key, int x, int y );
void specialKeys( int key, int x, int y );

void initSphere();

// Simple physics, using PID control loop to simulate cumulative gravity and friction.
// TODO: This needs to be refactored into its own physics class
void updatePhysics()
{
  // Get the normalized slip-slidin' vector; discard the z component and apply
  // x and y to our motion
  glm::vec3 nv = g_pPlayfield->getNormalAt( g_xpos, g_ypos );

  // Slip-slidin' away
  double sp;
  for( int i = 0; i < 2; i++ )
  {
    sp = i ? nv.y : nv.x;

    e[i] = sp - pv[i];
    pi[i] += e[i];

    // set delta p
    dp[i] = kp * e[i] + ki * pi[i];

#if defined(USE_CLAMPS)
    // clamp delta p
    if( dp[i] > clamp ) {
      dp[i] = clamp;
      pi[i] = 0.0;     // Reset the integral term 
      printf("CLAMP");
    }
    if( dp[i] < -clamp ) {
      dp[i] = -clamp;
      pi[i] = 0.0;     // Reset the integral term 
      printf("CLAMP");
    }
#endif
    pv[i] = pv[i] + dp[i];        // Not going to use gravity constant for now... - kg;
  }

  g_xvel = pv[0];
  g_yvel = pv[1];
//  g_zvel = -sqrt( pow(g_xvel, 2), pow(g_yvel,2) ) * nv.z;


  // Apply Friction
  g_xvel = g_xvel * (1.0 - FRICTION_COEFF);
  g_yvel = g_yvel * (1.0 - FRICTION_COEFF);
  //printf(" pv:%1.3lf pi:%1.3lf e:%1.3lf dp:%1.3lf  %1.3lf", pv[0], pi[0], e[0], dp[0], nv.x);
  //printf("\n");
  //printf( "{ %2.2f %2.2f %2.2f }  { %2.3f  %2.3f }\n",nv.x, nv.y, nv.z, g_xvel, g_yvel );
}


void updatePosition()
{
  updatePhysics();

  g_vel += ( g_velTar - g_vel ) / VEL_DAMPER;

  g_xposPrev = g_xpos;
  g_yposPrev = g_ypos;
  g_zposPrev = g_zpos;

  g_xvel += (sin( g_xyDir ) * g_vel) * kv;
  g_yvel += (cos( g_xyDir ) * g_vel) * kv;

  g_xpos += g_xvel;
  g_ypos += g_yvel;
  g_zpos += g_zvel;

#ifndef DEBUG_STEPPOS
  //g_xpos += sin( g_xyDir ) * g_vel + g_xvel;
  //g_ypos += cos( g_xyDir ) * g_vel + g_yvel;
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


  // Clamp position
  if( g_xpos > ( HP_XSIZE - 2 ) * HP_GRIDSIZE ) {
    if( g_xposPrev < g_xpos ) {
      g_xpos = HP_XSIZE * HP_GRIDSIZE - 0.00001;
    }
  }
  if( g_xpos < 0 ) {
    if( g_xposPrev > g_xpos ) {
      g_xpos = 0;
    }
  }
  if( g_ypos > ( HP_YSIZE - 2 ) * HP_GRIDSIZE ) {
    if( g_yposPrev < g_ypos ) {
      g_ypos = HP_XSIZE * HP_GRIDSIZE - 0.00001;
    }
  }
  if( g_ypos < 0 ) {
    if( g_yposPrev > g_ypos ) {
      g_ypos = 0;
    }
  }

  // Only get the z position AFTER all the x/y position update
  // including clamping is completed.
  //g_zpos = g_pPlayfield->getHeightAt( g_xpos, g_ypos );
  // coldet
  glm::vec3 adj;
  if( g_pPlayfield->getColdetAdj( g_xpos, g_ypos, g_zpos, 1.2, &adj ) ) {
    printf("-ADJUSTING: { %2.2f %2.2f %2.2f }\n", g_xpos, g_ypos, g_zpos );
    g_xpos = adj.x;
    g_ypos = adj.y;
    g_zpos = adj.z - 0.001;
    g_zvel = -0.05;
    g_zveli = 0.0;
    printf("+ADJUSTING: { %2.2f %2.2f %2.2f }\n", g_xpos, g_ypos, g_zpos );
  } else {
    // Apply gravity
    g_zveli -= 0.005;
    g_zvel += g_zveli;
  }
 
  // Update direction
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

  usleep( 16667 );
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
  // Init playfield
  g_pPlayfield = new Playfield();
  if( g_pPlayfield ) {

    // Init global positioning
    g_xpos = g_pPlayfield->getXMid();
    g_ypos = g_pPlayfield->getYMid();
    g_zpos = 0.0;
    g_zposLookatTar = 0.0;
    g_xyDir = M_PI + M_PI / 4;
    g_xyDirTar = g_xyDir;
    g_vel = 0.0;
    g_velTar = 0.0;
    g_xvel = 0.0;
    g_yvel = 0.0;

    // TODO: This is crude and will need to be replaced by a proper 3D vector later on.  This
    // is just to debug collsiion
    g_zvel = 0.0;
    g_zveli = 0.0;
     

  } else {
    // TODO: Exit program with error
  }

  initSphere();

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


#define CAM_DIST 12.0
// Set up camera for this draw frame
//
void setCamera()
{
  GLint viewport[4];
  glGetIntegerv( GL_VIEWPORT, viewport );

  gluPerspective( 45, double( viewport[2])/viewport[3], 0.01, 300 );
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  float zposLookatTarDelta = ( ( g_zpos - g_zposPrev ) - g_zposLookatTar ) / 4;

  g_zposLookatTar += zposLookatTarDelta;
  //g_zposLookatTar = 0.0;    // TEMP

  double eyeX = g_xpos + sin(g_xyDir) * CAM_DIST;
  double eyeY = g_ypos + cos(g_xyDir) * CAM_DIST;
  double eyeZ = g_pPlayfield->getHeightAt( eyeX, eyeY ) + 2.0;

  gluLookAt(
      eyeX,
      eyeY,
      eyeZ,
      g_xpos - sin( g_xyDir ),
      g_ypos - cos( g_xyDir ),
      g_zpos + 2.0,
      0,
      0,
      3
      );
  glMultMatrixf( mo );
}




#define FACE_SIZE 4096 
GLuint pixels_face0[FACE_SIZE];
GLuint pixels_face1[FACE_SIZE];
GLuint pixels_face2[FACE_SIZE];
GLuint pixels_face3[FACE_SIZE];
GLuint pixels_face4[FACE_SIZE];
GLuint pixels_face5[FACE_SIZE];

#define width 64
#define height 64

GLuint g_textureID = 0;

void initSphereMapping(GLuint *pTextureID)
{
  glGenTextures(1, pTextureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, *pTextureID);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0); 
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0); 
  //Define all 6 faces
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels_face0);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels_face1);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels_face2);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels_face3);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels_face4);
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels_face5);
}


#define D3DMXVECTOR3 glm::vec3
#define D3DMX_PI M_PI

typedef struct CUSTOMVERTEX
{
  glm::vec3   normal;
  glm::vec3   position;
};

const unsigned int ui_VCount = 1330;
CUSTOMVERTEX *arr_Vertices;

#define FLOAT float
#define DWORD unsigned int

const int g_factor = 20;

void initSphere()
{

  // for environment mapping
  initSphereMapping( &g_textureID );


  const int iFactor = g_factor;
  int iPos = 0;

  arr_Vertices = new CUSTOMVERTEX[ui_VCount * 2];
  DWORD ui_ShapeCount = iFactor *iFactor * 2; // use when rendering

  float arrV[iFactor* iFactor][3];

  for (DWORD j= 0; j < iFactor; j ++)
  {
    FLOAT theta = (D3DMX_PI*j)/(iFactor);

    for( DWORD i=0; i<iFactor; i++ )
    {
      iPos = j*iFactor+i;
      FLOAT phi = (2*D3DMX_PI*i)/(iFactor);
      arrV[iPos][0] = (float)(sin(theta)*cos(phi));
      arrV[iPos][1] = (float)(sin(theta)*sin(phi));
      arrV[iPos][2] = (float)(cos(theta));

      /*std::cout << "[" << j <<"][" << i << "] = " << arrV[iPos][0]  
        << "," << arrV[iPos][1] << "," << arrV[iPos][2] <<std::endl;*/
    }
  }

  int iNext = 0;

  for (DWORD j= 0; j < iFactor; j ++)
  { 

    for( DWORD i=0; i<iFactor; i++ )
    {
      if (i == iFactor - 1)
        iNext = 0;
      else iNext = i +1;

      iPos = (j*iFactor*6)+(i*6);
      arr_Vertices[iPos].position = D3DMXVECTOR3( arrV[j*iFactor+i][0], arrV[j*iFactor+i][1], arrV[j*iFactor+i][2]);
      arr_Vertices[iPos + 1].position = D3DMXVECTOR3( arrV[j*iFactor+iNext][0], arrV[j*iFactor+iNext][1], arrV[j*iFactor+iNext][2]);


      if (j != iFactor -1)
        arr_Vertices[iPos + 2].position = D3DMXVECTOR3( arrV[((j+1)*iFactor)+i][0], arrV[((j+1)*iFactor)+i][1], arrV[((j+1)*iFactor)+i][2]);
      else
        arr_Vertices[iPos + 2].position = D3DMXVECTOR3( 0, 0, -1); //Create a pseudo triangle fan for the last set of triangles

      arr_Vertices[iPos].normal = D3DMXVECTOR3( arr_Vertices[iPos].position.x, arr_Vertices[iPos].position.y, arr_Vertices[iPos].position.z);
      arr_Vertices[iPos + 1].normal = D3DMXVECTOR3( arr_Vertices[iPos+1].position.x, arr_Vertices[iPos+1].position.y, arr_Vertices[iPos+1].position.z);
      arr_Vertices[iPos + 2].normal = D3DMXVECTOR3( arr_Vertices[iPos+2].position.x, arr_Vertices[iPos+2].position.y, arr_Vertices[iPos+2].position.z);

      arr_Vertices[iPos + 3].position = D3DMXVECTOR3( arr_Vertices[iPos+2].position.x, arr_Vertices[iPos+2].position.y, arr_Vertices[iPos+2].position.z);
      arr_Vertices[iPos + 4].position = D3DMXVECTOR3( arr_Vertices[iPos+1].position.x, arr_Vertices[iPos+1].position.y, arr_Vertices[iPos+1].position.z);

      if (j != iFactor - 1)
        arr_Vertices[iPos + 5].position = D3DMXVECTOR3( arrV[((j+1)*iFactor)+iNext][0], arrV[((j+1)*iFactor)+iNext][1], arrV[((j+1)*iFactor)+iNext][2]);
      else
        arr_Vertices[iPos + 5].position = D3DMXVECTOR3( 0,0,-1);

      arr_Vertices[iPos + 3].normal = D3DMXVECTOR3( arr_Vertices[iPos+3].position.x, arr_Vertices[iPos+3].position.y, arr_Vertices[iPos+3].position.z);
      arr_Vertices[iPos + 4].normal = D3DMXVECTOR3( arr_Vertices[iPos+4].position.x, arr_Vertices[iPos+4].position.y, arr_Vertices[iPos+4].position.z);
      arr_Vertices[iPos + 5].normal = D3DMXVECTOR3( arr_Vertices[iPos+5].position.x, arr_Vertices[iPos+5].position.y, arr_Vertices[iPos+5].position.z);

      //std::cout << "[" << iPos <<"] = " << arr_Vertices[iPos].position.x << 
      //  "," << arr_Vertices[iPos].position.y <<
      //  "," << arr_Vertices[iPos].position.z <<std::endl;

      //std::cout << "[" << iPos + 1 <<"] = " << arr_Vertices[iPos + 1].position.x << 
      //  "," << arr_Vertices[iPos+ 1].position.y <<
      //  "," << arr_Vertices[iPos+ 1].position.z <<std::endl;

      //std::cout << "[" << iPos + 2 <<"] = " << arr_Vertices[iPos].position.x << 
      //  "," << arr_Vertices[iPos + 2].position.y <<
      //  "," << arr_Vertices[iPos + 2].position.z <<std::endl;
    }
  }
}



// Set up material
//
void setMaterial()
{
  GLfloat materialEmission[] = {0.01f, 0.01f, 0.01f, 1.0f};
  GLfloat materialAmbient[] = {0.4f, 0.1f, 0.1f, 1.0f};
  GLfloat materialSpecular[] = {0.81, 0.81, 0.89, 0.2};

  glColor3f(1.0, 0.2, 0.2 );
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, materialAmbient);
  glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
  glMaterialfv(GL_FRONT, GL_EMISSION, materialEmission);
  glMaterialf(GL_FRONT, GL_SHININESS,100.8); //The shininess parameter

  glShadeModel( GL_SMOOTH);
}


void drawSphere()
{
  const int iFactor = g_factor;
  int iPos = 0;

  DWORD ui_ShapeCount = iFactor *iFactor * 2; // use when rendering
#if 1
// ENVIRONMENT MAPPING DOESN'T WORK!!!



#if 0
  glPushMatrix();

  // Rotate the image
  glMatrixMode( GL_MODELVIEW );             // Current matrix affects objects positions
  glLoadIdentity();                         // Initialize to the identity

  glTranslatef( HP_XMID, HP_YMID, 0.0 );               // Translate rotation center from origin
  glTranslatef( -HP_XMID, -HP_YMID, 0.0 );            // Translate rotation center to origin


  glRotatef( 1.0, 1.0, 0.0, 0.0 );
  glMultMatrixf( mo );
  glGetFloatv( GL_MODELVIEW_MATRIX, mo );

  glPopMatrix();
#endif



//  glRotatef( 1.1, 1.0, 0.0, 0.0 );       // Rotate the whole world...
  glTranslatef( g_xpos, g_ypos, g_zpos /*g_pPlayfield->getHeightAt( g_xpos, g_ypos ) + 1.0*/ );

  // glMatrixMode( GL_MODELVIEW);

  glMatrixMode(GL_TEXTURE);
  glEnable(GL_AUTO_NORMAL);

  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);


  glDisable (GL_COLOR_MATERIAL);
  glDisable( GL_TEXTURE_2D );
  glEnable (GL_TEXTURE_CUBE_MAP);


  glBindTexture(GL_TEXTURE_2D, g_textureID);

  glEnable( GL_T );
  glEnable( GL_S );
  glEnable( GL_R );
  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);
  glEnable(GL_TEXTURE_GEN_R);

  glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
  glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );
  glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP );

//  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

#if 1
  glBindTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X, g_textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, g_textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, g_textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, g_textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, g_textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, g_textureID);
#endif

  glEnable( GL_TEXTURE_CUBE_MAP);
#endif


  



  setMaterial();

  glPushMatrix();

  glBegin( GL_TRIANGLE_STRIP);


  //glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  //glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  for( int i = 0; i < ui_ShapeCount * 3; i++ ) {
    glVertex3f(
        arr_Vertices[ i ].position.x,
        arr_Vertices[ i ].position.y,
        arr_Vertices[ i ].position.z
    );
    glNormal3f(
        arr_Vertices[ i ].position.x,
        arr_Vertices[ i ].position.y,
        arr_Vertices[ i ].position.z
    );
  }


  glPopMatrix();

#if 0
  glm::vec3 e = normalize( vec3( modelViewMatrix * p ) );
  glm::vec3 n = normalize( normalMatrix * normal );

  glm::vec3 r = reflect( e, n );
#endif 
  glEnd();

  glDisable(GL_TEXTURE_GEN_S);
  glDisable(GL_TEXTURE_GEN_T);
  glDisable(GL_TEXTURE_GEN_R);

  glFlush();


}











GLfloat ambientLightGlobal[] = {0.2f, 0.2f, 0.2f, 1.0f};
GLfloat lightPos[] ={100.0, 150.0, 100.0, 0.0};
GLfloat ambientLight[] = {0.3f, 0.3f, 0.3f, 1.0f};
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

  // Normalize the light. Note, only needs must be done once.
  {
    float sum = 0.0;
    for( int i = 0; i < 3; i++ ) {
      sum += abs(lightPos[i]);
    }

    for( int i = 0; i < 3; i++ ) {
      lightPos[i] = lightPos[i] / sum;
    }
  }

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
  glRotatef( g_angle, g_rotX, g_rotY, g_rotZ );       // Rotate the whole world...
  glTranslatef( -HP_XMID, -HP_YMID, 0.0 );            // Translate rotation center to origin

  //  glMultMatrixf( mo );
  //  glGetFloatv( GL_MODELVIEW_MATRIX, mo );

  glPopMatrix();

  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  glLightModelfv( GL_LIGHT_MODEL_AMBIENT, ambientLightGlobal );

  glEnable( GL_DEPTH_TEST );
  glEnable( GL_NORMALIZE );
  glEnable( GL_LIGHTING );
  glEnable( GL_LIGHT0 );
  glShadeModel( GL_FLAT );
  glEnable( GL_COLOR_MATERIAL );

  // Clear to sky color / draw sky
  glClearColor( 0.2, 0.4, 0.55, 1.0 );
  setFog();

  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  setCamera();
  setLight();
  g_pPlayfield->draw();

  //glTranslatef( g_xpos - 1.0, g_ypos - 1.0, 4.0 );
  // TEMP CODE; REFACTOR AND MOVE
  {
#if 0
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( g_xpos, g_ypos, g_pPlayfield->getHeightAt( HP_XMID, HP_YMID ) + 1 );
    glPopMatrix();
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    setCamera();
#endif

    drawSphere();
  }

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

    case ' ':
      g_velTar = 0;
      //g_xvel = 0;
      //g_yvel = 0;

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
      g_velTar += VEL_INC;
      if( g_velTar > VEL_MAX )
      {
        g_velTar = VEL_MAX;
      }
#ifdef DEBUG_STEPPOS
      // TEMP CODE; REMOVE
      g_xpos += sin( g_xyDir ) * VEL_INC;
      g_ypos += cos( g_xyDir ) * VEL_INC;
#endif
      break;
    case GLUT_KEY_UP:
      g_velTar += -VEL_INC;
      if( g_velTar < -VEL_MAX )
      {
        g_velTar = -VEL_MAX;
      }
#ifdef DEBUG_STEPPOS
      // TEMP CODE; REMOVE
      g_xpos -= sin( g_xyDir ) * VEL_INC;
      g_ypos -= cos( g_xyDir ) * VEL_INC;
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


