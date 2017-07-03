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
#include "jmgl_video_dec.h"
#include "jm_intel_dec.h"
#include "jm_nv_dec.h"

#pragma comment(lib,"nv_dec.lib")
#pragma comment(lib,"intel_dec.lib")

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

#define DEC_TYPE_AUTO	0
#define DEC_TYPE_NVIDIA 1
#define DEC_TYPE_INTEL	2
#define DEC_TYPE_FFMPEG 3

#define PIX_FMT_YUV420P AV_PIX_FMT_YUV420P

// if have Nvidia hardware, use ffmpeg to hardware acceleration
#define USE_FFMPEG_NV 1


void player_sync(int64_t pts, dec_ctx *ctx)
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

static void *video_dec_thread(void *arg)
{
	dec_ctx *ctx = (dec_ctx *)arg;

	jmgl_image	* img = ctx->img;	// TODO

	AVFormatContext *ic = NULL;
	AVCodecContext *vcc = NULL;
	AVStream *vs = NULL;
	AVCodec *vc = NULL;
	AVFrame *frame = NULL;
	AVFrame *_frame = NULL;
	AVPacket packet, *pkt = &packet;
	struct SwsContext *sws = NULL;

	AVBitStreamFilterContext *h264bsfc = NULL;

	int vi = -1;
	int ret = 0;
	int got_frame;

	//
	void *dec_handle = 0;
	unsigned char *out_buf = NULL;
	int out_len = 0;
	int yuv_len = 0;
	int width = 0, height = 0;
	float frame_rate = 0.0f;


	//
	int dec_type = ctx->dec_type;
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


	// dec_type == AUTO, check if user have NV device, us FFmpeg with NV hardware acceleration
	// else try intel decode, last try FFmpeg sofeware decode.
	if (ctx->dec_type == DEC_TYPE_AUTO) {
		if (jm_nvdec_is_hw_support()) {
			if (USE_FFMPEG_NV) {
				vcc = vs->codec;
				vc = avcodec_find_decoder(vcc->codec_id);
				avcodec_open2(vcc, vc, NULL);
				frame = av_frame_alloc();
				dec_type = DEC_TYPE_FFMPEG;
			}
			else {
				// try nv decode or ffmpeg nv hardware acceleration
				out_len = 4096 * 2048 * 3 / 2;	// max size of YUV frame, max support 4096 x 2048 YUV420
				out_buf = new unsigned char[out_len];
				yuv_len = out_len;

				// init decode
				int codec_id = (vs->codec->codec_id == AV_CODEC_ID_H264) ? 0 : 1;
				dec_handle = jm_nvdec_create_handle();
				jm_nvdec_init(codec_id, 1, NULL, 0, dec_handle);
				dec_type = DEC_TYPE_NVIDIA;
			}
		}
		else if (jm_intel_is_hw_support()) {
			// try Intel hw decode
			out_len = 4096 * 2048 * 3 / 2;	// max size of YUV frame, max support 4096 x 2048 YUV420
			out_buf = new unsigned char[out_len];
			yuv_len = out_len;

			// init decode
			int codec_id = (vs->codec->codec_id == AV_CODEC_ID_H264) ? 0 : 1;
			dec_handle = jm_intel_dec_create_handle();
			jm_intel_dec_init(codec_id, 1, dec_handle);
			dec_type = DEC_TYPE_INTEL;

		}
		else{
			// try FFmpeg decode
			vcc = vs->codec;
			vc = avcodec_find_decoder(vcc->codec_id);
			avcodec_open2(vcc, vc, NULL);
			frame = av_frame_alloc();
			dec_type = DEC_TYPE_FFMPEG;
		}
	}
	else if (ctx->dec_type == DEC_TYPE_FFMPEG) {
		vcc = vs->codec;
		vc = avcodec_find_decoder(vcc->codec_id);
		avcodec_open2(vcc, vc, NULL);
		frame = av_frame_alloc();
		dec_type = DEC_TYPE_FFMPEG;
	}
	else{
		// intel or nv decoode 
		
		out_len = 4096 * 2048 * 3 / 2;	// max size of YUV frame, max support 4096 x 2048 YUV420
		out_buf = new unsigned char[out_len];
		yuv_len = out_len;

		// init decode
		if (ctx->dec_type == DEC_TYPE_NVIDIA) {
			dec_handle = jm_nvdec_create_handle();
			jm_nvdec_init(0, 1, NULL, 0, dec_handle);
			dec_type = DEC_TYPE_NVIDIA;
		}
		else {
			//
			dec_handle = jm_intel_dec_create_handle();
			jm_intel_dec_init(0, 1, dec_handle);
			dec_type = DEC_TYPE_INTEL;

		}
	}
	ctx->duration = ic->duration;

	if (vcc->codec_id == AV_CODEC_ID_H264 || vcc->codec_id == AV_CODEC_ID_HEVC) {
		if (vcc->codec_id == AV_CODEC_ID_H264)
			h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");
		else
			h264bsfc = av_bitstream_filter_init("hevc_mp4toannexb");
	}


	while (!ctx->is_exit){

		//
		if (av_read_frame(ic, pkt) >= 0) {
			if (pkt->stream_index == vi) {

				// 
				if (pkt && pkt->size) {
					if (h264bsfc) {
						av_bitstream_filter_filter(h264bsfc, vcc, NULL, &pkt->data, &pkt->size, pkt->data, pkt->size, 0);
					}

					if (dec_type == DEC_TYPE_NVIDIA) {
						jm_nvdec_decode_frame(pkt->data, pkt->size, &got_frame, dec_handle);
						if (1 == got_frame) {
							yuv_len = out_len;
							jm_nvdec_output_frame(out_buf, &yuv_len, dec_handle);
							if (yuv_len > 0) {
								// frame_count ++;
								if (!img->data) {
									jm_nvdec_stream_info(&width, &height, dec_handle);

									img->data = out_buf;

									img->width = width;// WIDTH;
									img->height = height;// HEIGHT;
									img->y = img->data;
									img->u = img->y + img->width * img->height;
									img->v = img->u + (img->width * img->height) / 4;
								}
							}
						}
					}
					else if (dec_type == DEC_TYPE_INTEL) {
						jm_intel_dec_input_data(pkt->data, pkt->size, dec_handle);
						ret = jm_intel_dec_output_frame(out_buf, &yuv_len, dec_handle);
						if (0 == ret/* && yuv_len > 0*/) {
							// Get stream display size
							if (!img->data) {

								jm_intel_get_stream_info(&width, &height, &frame_rate, dec_handle);

								img->data = out_buf;

								img->width = width;// WIDTH;
								img->height = height;// HEIGHT;
								img->y = img->data;
								img->u = img->y + img->width * img->height;
								img->v = img->u + (img->width * img->height) / 4;
							}
						}
					}
					else /*if (dec_type == DEC_TYPE_FFMPEG)*/ {	// TODO: Using FFmpeg decode
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

	if (dec_type == DEC_TYPE_FFMPEG) {
		av_free(_frame);
		av_free(frame);
		sws_freeContext(sws);
		avcodec_close(vcc);
	}
	else {
		printf(jm_nvdec_show_dec_info(dec_handle));
		jm_nvdec_deinit(dec_handle);

		if (out_buf) delete[] out_buf;

	}

	avformat_close_input(&ic);
	avformat_network_deinit();

	return NULL;
}

dec_ctx *jmgl_video_dec_create_context()
{
	dec_ctx *ctx = NULL;

	ctx = new dec_ctx;
	memset(ctx, 0x0, sizeof(dec_ctx));

	return ctx;
}

int jmgl_video_dec_init(char *url, int dec_type, dec_ctx *ctx)
{
	ctx->url = (char*)malloc(strlen(url) + 1);
	if (ctx->url != NULL) strcpy(ctx->url, url);         // Copy string if okay

	ctx->dec_type = ctx->dec_type;

	ctx->img = new jmgl_image;

	memset(ctx->img, 0x0, sizeof(jmgl_image));

	pthread_create(&ctx->play_thread, NULL, video_dec_thread, ctx);

	return 0;
}


int jmgl_video_dec_deinit(dec_ctx *ctx)
{
	ctx->is_exit = true;
	pthread_join(ctx->play_thread, NULL);
	
	free(ctx->url);
	//free(ctx->img->data);
	delete(ctx->img);

	delete ctx;

	return 0;
}

int jmgl_video_dec_set_exit(int is_exit, dec_ctx *ctx)
{
	ctx->is_exit = is_exit;
	return 0;
}