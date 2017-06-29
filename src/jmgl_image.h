/***********************************************      
  *     OpenGL Panoramic View
  *     Author:     Justin Mo 
  *     Email:      mojing1999@gmail.com
  *     Date:       2017.04.15
  *     Version:   V0.01
  ***********************************************
  *     Define YUV420 frame struct
  ***********************************************
  */

#ifndef _JMGL_IMAGE_H_
#define _JMGL_IMAGE_H_

#include <stdint.h>

typedef struct jmgl_image {
	uint8_t *data;
	uint8_t *y;
	uint8_t *u;
	uint8_t *v;
	int width;
	int height;
	int64_t pts;
}jmgl_image;


#endif // !_JMGL_IMAGE_H_

