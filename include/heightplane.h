// heightplane.h
// Implement height plane

#pragma once

#include "gameobject.h"
#include <glm/glm.hpp>

#define HP_MAX_PARAMS 4

#define HP_PARAM_CHECKERBOARD_IDX 0

class HeightPlane : public GameObject
{
  public:
    HeightPlane();
    ~HeightPlane();

    virtual float getHeightAt( const float x, const float y );
    glm::vec3 getNormalAt( const float x, const float y );
    bool getColdetAdj( const float x, const float y, const float z, const float radius, glm::vec3 *ap);

    virtual void setParam( const int paramIdx, const int val )
    {
      m_param[ paramIdx ] = val;
    };

    virtual int getParam( const int paramIdx )
    {
      return( m_param[ paramIdx ] );
    };

    virtual float getXMid() { return HP_XMID; };
    virtual float getYMid() { return HP_YMID; };

  protected:
    bool initHeightPlane();

    float m_heightPlane[ HP_XSIZE ][ HP_YSIZE ];
    float calcZ(const glm::vec3 p1, const glm::vec3 p2, const glm::vec3 p3, const float x, const float y);

    int m_param[ HP_MAX_PARAMS ];
};

