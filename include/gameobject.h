// gameObject
// Declaration for gameObject, a common ancestor 

#pragma once

class GameObject
{
  public:
    GameObject();
    virtual ~GameObject();

    virtual bool init() = 0;
    virtual void draw() = 0;
  protected:
    
};
