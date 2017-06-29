/***********************************************      
  *     OpenGL Panoramic View
  *     Author:     Justin Mo 
  *     Email:      mojing1999@gmail.com
  *     Date:       2017.04.15
  *     Version:   V0.01
  ***********************************************
  *     intel decode
  ***********************************************
  */
#ifndef _JMGL_INTEL_DEC_H_
#define _JMGL_INTEL_DEC_H_
#include "pthread.h"

// OpenGL
#include "jmgl_image.h"

typedef struct jmgl_dec
{
	char		*url;
	void		* user_data;
	jmgl_image	* img;

	bool		is_exit;
	bool		is_pause;
	int64_t		cur_time;
	int64_t		duration;
	int64_t		base;
	pthread_t	play_thread;

	bool		is_nv_dec;
	void		*dec_handle;


}dec_ctx;

dec_ctx *jmgl_dec_create_context();

int jmgl_dec_init(char *url, bool nv_dec, dec_ctx *ctx);
int jmgl_dec_deinit(dec_ctx *ctx);

int jmgl_dec_set_exit(int is_exit, dec_ctx *ctx);



#endif // !_JMGL_INTEL_DEC_H_
