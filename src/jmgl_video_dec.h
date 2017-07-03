/***********************************************      
  *     OpenGL Panoramic View
  *     Author:     Justin Mo 
  *     Email:      mojing1999@gmail.com
  *     Date:       2017.04.15
  *     Version:   V0.01
  ***********************************************
  *     video decode, basic ffmpeg, 
  *		intel video decode or 
  *		nvidia cuda video dec library
  ***********************************************
  */
#ifndef _JMGL_VIDEO_DEC_H_
#define _JMGL_VIDEO_DEC_H_
#include "pthread.h"

// OpenGL
#include "jmgl_image.h"


typedef struct jmgl_video_dec_ctx
{
	char		*url;
	void		*user_data;
	jmgl_image	*img;

	bool		is_exit;
	bool		is_pause;
	int64_t		cur_time;
	int64_t		duration;
	int64_t		base;
	pthread_t	play_thread;

	int			dec_type;

}dec_ctx;

dec_ctx *jmgl_video_dec_create_context();

/*
 *	@desc: 
 */
int jmgl_video_dec_init(char *url, int dec_type, dec_ctx *ctx);
int jmgl_video_dec_deinit(dec_ctx *ctx);

int jmgl_video_dec_set_exit(int is_exit, dec_ctx *ctx);



#endif // !_JMGL_VIDEO_DEC_H_
