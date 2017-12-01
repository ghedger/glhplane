#pragma once

class gameObject
{
  public:
    gameObject();
    virtual ~gameObject();

    virtual void draw() = 0;
  protected:
    
};
