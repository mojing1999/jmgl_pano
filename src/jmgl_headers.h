/***********************************************      
  *     OpenGL Panoramic View
  *     Author:     Justin Mo 
  *     Email:      mojing1999@gmail.com
  *     Date:       2017.04.15
  *     Version:   V0.01
  ***********************************************
  *     Include OpenGL headers files
  ***********************************************
  */
#ifndef _JMGL_OPENGL_HEADERS_H_
#define _JMGL_OPENGL_HEADERS_H_

// GLEW
#define GLEW_STATIC
#include "GL\glew.h"

// GLFW
#include "GLFW\glfw3.h"

// Other Libs
//#include <SOIL.h>
// GLM Mathematics
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtc\type_ptr.hpp"


#if 1
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "OpenGL32.lib")
//#pragma comment(lib, "GlU32.lib")
//#pragma comment(lib, "SOIL.lib")
#endif



#endif  //!_JMGL_OPENGL_HEADERS_H_
