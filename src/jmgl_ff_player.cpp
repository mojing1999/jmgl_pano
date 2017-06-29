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
#include <Windows.h>
#include <stdio.h>
#include <memory.h>
#include "jmgl_ff_player.h"

extern "C" {
#include "libavformat\avformat.h"
#include "libavcodec\avcodec.h"
#include "libavutil\avutil.h"
#include "libavutil\time.h"
#include "libswscale\swscale.h"

#pragma comment(lib, "pthreadVC2.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")

}
#define PIX_FMT_YUV420P AV_PIX_FMT_YUV420P




void player_sync(int64_t pts, player_ctx *ctx)
{
	if (ctx->base == 0) {
		ctx->base = av_gettime();
	}

	while (!ctx->is_exit && (ctx->is_pause || (av_gettime() - ctx->base < pts - 1e4)))
	{
		av_usleep(1000);
	}

	ctx->base = av_gettime() - pts;
	ctx->cur_time = pts;

	return ;
}

static void *ffplayer_thread(void *arg)
{
	player_ctx *ctx = (player_ctx *)arg;

	jmgl_image	* img = ctx->img;	// TODO

	AVFormatContext *ic = NULL;
	AVCodecContext *vcc = NULL;
	AVStream *vs = NULL;
	AVCodec *vc = NULL;
	AVFrame *frame = NULL;
	AVFrame *_frame = NULL;
	AVPacket packet, *pkt = &packet;
	struct SwsContext *sws = NULL;
	int vi = -1;
	int ret = 0;
	int got_frame;

	//
	av_register_all();
	avformat_network_init();

	ic = avformat_alloc_context();
	//ic->interrupt_callback.opaque = timer;
	//ic->interrupt_callback.callback = ffmpeg_interrupt_cp;

	avformat_open_input(&ic, ctx->url, NULL, NULL);
	av_dump_format(ic, 0, ctx->url, 0);
	avformat_find_stream_info(ic, NULL);

	for (int i = 0; i < ic->nb_streams; i++) {
		if (ic->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			vs = ic->streams[i];
			vi = i;
			break;
		}
	}

	vcc = vs->codec;
	vc = avcodec_find_decoder(vcc->codec_id);
	avcodec_open2(vcc, vc, NULL);
	frame = av_frame_alloc();

	ctx->duration = ic->duration;

	while (!ctx->is_exit){
#if 0
		if (timer->seek >= 0) {
			int64_t target = av_rescale_q(timer->seek, AV_TIME_BASE_Q, vs->time_base);
			ret = avformat_seek_file(ic, vi, INT64_MIN, target, INT64_MAX, -AVSEEK_FLAG_BYTE);
			if (ret < 0) {
				fprintf(stderr, "avformat_seek_file() error %d\n", ret);
			}
			else {
				avcodec_flush_buffers(vcc);
			}
			timer->seek = TIMER_SEEK_RESET;
		}
#endif
		//
		if (av_read_frame(ic, pkt) >= 0) {
			if (pkt->stream_index == vi) {
				ret = avcodec_decode_video2(vcc, frame, &got_frame, pkt);
				if (ret < 0) {
					// error
					fprintf(stderr, "avcodec_decode_video2() error %d\n", ret);
				}
				else if (got_frame) {
					if (img->width != frame->width || img->height != frame->height) {
						if (_frame) av_free(_frame);
						if (img->data) free(img->data);
						_frame = av_frame_alloc();
						img->data = (uint8_t*)av_malloc(avpicture_get_size(PIX_FMT_YUV420P, frame->width, frame->height) * sizeof(uint8_t));
						avpicture_fill((AVPicture *)_frame, img->data, PIX_FMT_YUV420P, frame->width, frame->height);
						img->y = _frame->data[0];
						img->u = _frame->data[1];
						img->v = _frame->data[2];
						img->width = frame->width;
						img->height = frame->height;
						sws = sws_getCachedContext(sws, frame->width, frame->height, vcc->pix_fmt, frame->width, frame->height, PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);

					}
					sws_scale(sws, (const uint8_t * const *)frame->data, frame->linesize, 0, frame->height, _frame->data, _frame->linesize);

					img->pts = av_frame_get_best_effort_timestamp(frame);
					if (img->pts == AV_NOPTS_VALUE) {
						img->pts = 0;
					}
					img->pts = (img->pts - ic->start_time) * av_q2d(vs->time_base) * 1000000;
					player_sync(img->pts, ctx);
				}
			}
			av_free_packet(pkt);
		}
		else
		{
			//nanosleep(&TIMER_TEN_MILLI, NULL);
			av_usleep(1);
		}
	}

	av_free(_frame);
	av_free(frame);
	sws_freeContext(sws);
	avcodec_close(vcc);
	avformat_close_input(&ic);
	avformat_network_deinit();

	return NULL;
}

player_ctx *jmgl_player_create_context()
{
	player_ctx *ctx = NULL;

	ctx = new player_ctx;
	memset(ctx, 0x0, sizeof(player_ctx));

	return ctx;
}

int jmgl_player_init(char *url, player_ctx *ctx)
{
	ctx->url = (char*)malloc(strlen(url) + 1);
	if (ctx->url != NULL) strcpy(ctx->url, url);         // Copy string if okay


	ctx->img = new jmgl_image;

	memset(ctx->img, 0x0, sizeof(jmgl_image));

	pthread_create(&ctx->play_thread, NULL, ffplayer_thread, ctx);

	return 0;
}


int jmgl_player_deinit(player_ctx *ctx)
{
	ctx->is_exit = true;
	pthread_join(ctx->play_thread, NULL);
	
	free(ctx->url);
	//free(ctx->img->data);
	free(ctx->img);

	delete ctx;

	return 0;
}

int jmgl_player_set_exit(int is_exit, player_ctx *ctx)
{
	ctx->is_exit = is_exit;
	return 0;
}