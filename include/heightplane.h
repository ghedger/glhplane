#pragma once

#include "gameobject.h"
#include <glm/glm.hpp>

class HeightPlane : public GameObject
{
  public:
    HeightPlane();
    ~HeightPlane();

    virtual float getHeightAt( const float x, const float y );

  protected:
    bool initHeightPlane();

    float m_heightPlane[ HP_XSIZE ][ HP_YSIZE ];
    float calcZ(const glm::vec3 p1, const glm::vec3 p2, const glm::vec3 p3, const float x, const float y);
};

