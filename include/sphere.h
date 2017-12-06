#pragma once

#include "gameobject.h"
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

struct CUSTOMVERTEX
{
  glm::vec3   normal;
  glm::vec3   position;
};


class Spherex : virtual public GameObject
{
	public:
		Spherex();
		Spherex( float radius, int factor );
		Spherex( double radius, int factor );
		virtual ~Spherex();
		void draw();
		bool init()
		{
			return generate( ( float ) 1.0, 20 );
		}


	protected:
		bool generate( const float radius, const int myFactor );
		int getVerticeTot()
		{
			return m_verticeTot;
		}

	private:

		unsigned int		m_shapeTot;
		CUSTOMVERTEX   *m_vertices;
		unsigned int		m_verticeTot;
};

