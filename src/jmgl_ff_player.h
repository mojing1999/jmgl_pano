/***********************************************      
  *     OpenGL Panoramic View
  *     Author:     Justin Mo 
  *     Email:      mojing1999@gmail.com
  *     Date:       2017.04.15
  *     Version:   V0.01
  ***********************************************
  *     player, basic ffmpeg library
  ***********************************************
  */
#ifndef _JMGL_FF_PLAYER_H_
#define _JMGL_FF_PLAYER_H_
#include "pthread.h"

// OpenGL
#include "jmgl_image.h"

typedef struct jmgl_player
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

}player_ctx;

player_ctx *jmgl_player_create_context();
int jmgl_player_init(char *url, player_ctx *ctx);
int jmgl_player_deinit(player_ctx *ctx);

int jmgl_player_set_exit(int is_exit, player_ctx *ctx);



#endif // !_JMGL_FF_PLAYER_H_S
