/***********************************************
*     OpenGL Panoramic View
*     Author:     Justin Mo
*     Email:      mojing1999@gmail.com
*     Date:       2017.04.15
*     Version:   V0.01
***********************************************
*     Shader program
***********************************************
*/
#include "jmgl_opengl.h"

#include "jmgl_image.h"

#define FFMPEG_DEC 1
#define NV_INTEL_DEC 0

#if FFMPEG_DEC
#include "jmgl_ff_player.h"
#endif

#if NV_INTEL_DEC
#include "jmgl_nv_intel_dec.h"
#endif



#include <stdio.h>
#include <Windows.h>

#pragma comment(linker,"/subsystem:\"Windows\" /entry:\"mainCRTStartup\"")




int main(int argc, char **argv)
{
	gl_ctx *gl = jmgl_opengl_create_context();
#if FFMPEG_DEC
	player_ctx *player = jmgl_player_create_context();
#elif NV_INTEL_DEC
	dec_ctx *ctx = jmgl_dec_create_context();
#endif


#if FFMPEG_DEC
	jmgl_player_init("F:\\star_war_2.h264", player);
	//jmgl_player_init(argv[1], player);
	//jmgl_player_init("rtmp://localhost/live/test live=1", player);
	//jmgl_player_init("F:\\EarthMap_2500x1250.jpg", player);
#elif NV_INTEL_DEC
	jmgl_dec_init("F:\\star_war_2_new.h264", 0, ctx);

#endif

	jmgl_opengl_init(800, 600, "JMGL_PANO", gl);

	jmgl_image	* img = NULL;

#if FFMPEG_DEC
	img = player->img;
#elif NV_INTEL_DEC
	img = ctx->img;
#endif

	while (!jmgl_window_is_close(gl))
	{
		jmgl_opengl_render(img, gl);
	}



	jmgl_opengl_deinit(gl);
#if FFMPEG_DEC
	jmgl_player_deinit(player);
#elif NV_INTEL_DEC
	jmgl_dec_deinit(ctx);
#endif
	return 0;
}