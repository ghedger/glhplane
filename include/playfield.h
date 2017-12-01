#pragma once

// Playfield.h
//
// Declares playfield

#include "heightplane.h"

class Playfield : public HeightPlane 
{
  public:
    Playfield();
    virtual ~Playfield();

    bool init();
    void draw();

  private:
    void drawLines();
    void drawSolids();
    void setPlaneMaterial();
    void setPlaneMaterial2();

};

