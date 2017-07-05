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
#include "anyoption.h"

#define FFMPEG_DEC		0
#define NV_INTEL_DEC	0
#define VIDEO_DEC		1

#if FFMPEG_DEC
#include "jmgl_ff_player.h"
#endif

#if NV_INTEL_DEC
#include "jmgl_nv_intel_dec.h"
#endif

#if VIDEO_DEC
#include "jmgl_video_dec.h"
#endif



#include <stdio.h>
#include <Windows.h>

//#pragma comment(linker,"/subsystem:\"Windows\" /entry:\"mainCRTStartup\"")




int main(int argc, char **argv)
{
	int dec_type = 0;
	char *input = NULL;

	/***********************************************************/
	/*	Handle Option										   */
	/***********************************************************/
	AnyOption *opt = new AnyOption();
	opt->addUsage("Simple Panorama player, implemented by Justin Mo(mojing1999@gmail.com).");
	opt->addUsage("\tBase on OpenGL, NV CUDA decode, Intel Media SDK and FFmpeg.");
	opt->addUsage("Support view mode:");
	opt->addUsage("\tF1. Rectilinear");
	opt->addUsage("\tF2. Mirror Ball");
	opt->addUsage("\tF3. Fisheye");	
	opt->addUsage("\tF4. Little Planet");
	opt->addUsage("\t\'a\' Auto rotation");
	opt->addUsage("\t\'t\' Stop auto rotation");
	opt->addUsage("");
	opt->addUsage("Usage: jmgl_pano [options] input_file");
	opt->addUsage("");
	opt->addUsage(" -h	--help		Prints this help ");
	opt->addUsage(" -d  --dec_type	decode by 0:auto, 1:nv, 2:intel or 3:ffmpeg ");

	//
	opt->setFlag("help", 'h');
	opt->setOption("dec_type", 'd');

	opt->processCommandArgs(argc, argv);
	if (!opt->hasOptions()) {
		opt->printUsage();

		delete opt;
		return -1;
	}


	if (opt->getFlag("help") || opt->getFlag('h'))
		opt->printUsage();

	if (opt->getValue('d') != NULL || opt->getValue("dec_type") != NULL) {
		dec_type = atoi(opt->getValue('d'));
		printf("dec_type = %s\n", opt->getValue('d'));
	}

	if (opt->getArgc() > 1) {
		printf("param error!\n");
	}

	for (int i = 0; i < opt->getArgc(); i++) {
		//cout << "arg = " << opt->getArgv(i) << endl;
		printf("arg[%d] = %s\n", i, opt->getArgv(i));
	}
	input = strdup(opt->getArgv(0));


	if (opt) delete opt;
	/***********************************************************/
	gl_ctx *gl = jmgl_opengl_create_context();
#if FFMPEG_DEC
	player_ctx *player = jmgl_player_create_context();
#elif NV_INTEL_DEC
	dec_ctx *ctx = jmgl_dec_create_context();
#elif VIDEO_DEC
	dec_ctx *ctx = jmgl_video_dec_create_context();
#endif


#if FFMPEG_DEC
	jmgl_player_init(input, player);
	//jmgl_player_init(argv[1], player);
	//jmgl_player_init("rtmp://localhost/live/test live=1", player);
	//jmgl_player_init("F:\\EarthMap_2500x1250.jpg", player);
#elif NV_INTEL_DEC
	jmgl_dec_init(input, dec_type, ctx);
#elif VIDEO_DEC
	jmgl_video_dec_init(input, dec_type, ctx);

#endif

	jmgl_opengl_init(800, 600, "JMGL_PANO", gl);

	jmgl_image	* img = NULL;

#if FFMPEG_DEC
	img = player->img;
#elif NV_INTEL_DEC
	img = ctx->img;
#elif VIDEO_DEC
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
#elif VIDEO_DEC
	jmgl_video_dec_deinit(ctx);
#endif
	return 0;
}