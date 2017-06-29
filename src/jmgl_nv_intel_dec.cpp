/***********************************************      
  *     OpenGL Panoramic View
  *     Author:     Justin Mo 
  *     Email:      mojing1999@gmail.com
  *     Date:       2017.04.15
  *     Version:   V0.01
  ***********************************************
  *     decode
  ***********************************************
  */
#include <Windows.h>
#include <stdio.h>
#include <memory.h>
#include "jmgl_nv_intel_dec.h"
#include "jm_intel_dec.h"
#include "jm_nv_dec.h"

#pragma comment(lib,"nv_dec.lib")
#pragma comment(lib,"intel_dec.lib")
#pragma comment(lib, "pthreadVC2.lib")

#define PIX_FMT_YUV420P AV_PIX_FMT_YUV420P




void player_sync(int64_t pts, dec_ctx *ctx)
{
#if 0
	if (ctx->base == 0) {
		ctx->base = clock();
	}

	while (!ctx->is_exit && (ctx->is_pause || (clock() - ctx->base < pts - 1e4)))
	{
		Sleep(1);
	}

	ctx->base = clock() - pts;
#endif
	Sleep(10);

	return ;
}

// return offset
int find_nalu_prefix(unsigned char *buf_start, int buf_size, int *prefix_len)
{
	int offset = 0;

	unsigned char *buf = buf_start;
	*prefix_len = 0;

	while (buf_size >= offset + 3)
	{
		buf = buf_start + offset;
		if ((0x00 == buf[0]) &&
			(0x00 == buf[1]) &&
			(0x01 == buf[2]))
		{
			*prefix_len = 3;
			return offset;
		}
		else if ((0x00 == buf[0]) &&
			(0x00 == buf[1]) &&
			(0x00 == buf[2]) &&
			(0x01 == buf[3]))
		{
			*prefix_len = 4;
			return offset;
		}
		offset += 1;
	}


	return -1;

}

unsigned char *find_nalu(unsigned char *buf, int size, int *nalu_len)
{
	unsigned char *p = buf;
	int offset1 = 0, offset2 = 0;

	int prefix1 = 0;
	int prefix2 = 0;
	offset1 = find_nalu_prefix(p, size, &prefix1);

	// offset1 should be 0
	offset2 = find_nalu_prefix(p + offset1 + prefix1, size - prefix1 - offset1, &prefix2);

	if (-1 == offset2) {
		*nalu_len = 0;
		p = NULL;
	}
	else {
		*nalu_len = offset2 + prefix1;
		p = p + offset1;

	}

	return p;
}

static void *nv_dec_thread(void *arg)
{
	dec_ctx *ctx = (dec_ctx *)arg;

	jmgl_image	* img = ctx->img;	// TODO

	FILE *ifile = NULL;
	ifile = fopen(ctx->url, "rb");

	unsigned char *in_buf = NULL;
	unsigned char *out_buf = NULL;
	int in_len = 0, out_len = 0;

	in_len = (10 << 20);	// 10 MB
	out_len = (20 << 20);	// 20 MB
	in_buf = new unsigned char[in_len];
	out_buf = new unsigned char[out_len];



	int is_eof = 0;
	int yuv_len = 0;
	int remaining_len = 0;
	int rd_len = 0, wt_len = 0;
	int ret = 0;


	int width = 0, height = 0;
	float frame_rate = 0.0;
	int duration = 0;

	unsigned char *buf = NULL;
	unsigned char *nalu = NULL;
	int nalu_len = 0;
	int offset = 0;
	int prefix = 0;

	int got_frame = 0;
	long nalu_count = 0;
	unsigned long frame_count = 0;


	//
	rd_len = fread(in_buf, 1, in_len, ifile);

	// find the first prefix
	offset = find_nalu_prefix(in_buf, rd_len, &prefix);

	int buf_len = rd_len - offset;
	buf = in_buf + offset;


	ctx->dec_handle = jm_nvdec_create_handle();

	jm_nvdec_init(0, 1, NULL, 0, ctx->dec_handle);




	while (!ctx->is_exit) {

		if (0 == is_eof) {
			nalu = find_nalu(buf, buf_len, &nalu_len);

			if (!nalu) {
				// need more data
				//if (0 == is_eof) {
				memmove(in_buf, buf, buf_len);
				rd_len = fread(in_buf + buf_len, 1, in_len - buf_len, ifile);

				buf_len += rd_len;
				buf = in_buf;

				if (0 == rd_len) {
					is_eof = 1;
					nalu = buf;
					nalu_len = buf_len;
					//jm_nvdec_set_eof(1, dec_handle);
				}
				//}
				continue;
			}

			buf += nalu_len;
			buf_len -= nalu_len;

		}



		if ((nalu)/* ||(1 == is_eof)*/) {
			// decode nalu
			nalu_count += 1;
			jm_nvdec_decode_frame(nalu, nalu_len, &got_frame, ctx->dec_handle);
			if (1 == got_frame) {
				yuv_len = out_len;
				jm_nvdec_output_frame(out_buf, &yuv_len, ctx->dec_handle);
				if (yuv_len > 0) {
					frame_count += 1;
					//write_len = fwrite(out_buf, 1, yuv_len, ofile);
					if (!img->data) {

						jm_nvdec_stream_info(&width, &height, ctx->dec_handle);

						//duration = 1000 / frame_rate;

						img->data = out_buf;

						img->width = width;// WIDTH;
						img->height = height;// HEIGHT;
						img->y = img->data;
						img->u = img->y + img->width * img->height;
						img->v = img->u + (img->width * img->height) / 4;
					}
					player_sync(0, ctx);
				}
			}

			if (1 == is_eof) {
				// last nalu
				nalu = 0;
				nalu_len = 0;
				//jm_nvdec_set_eof(1, dec_handle);
			}
			//}

		}
		else if (1 == is_eof) {
			// decode cached frame
			nalu_count += 1;
			jm_nvdec_decode_frame(NULL, 0, &got_frame, ctx->dec_handle);
			if (1 == got_frame) {
				yuv_len = out_len;
				jm_nvdec_output_frame(out_buf, &yuv_len, ctx->dec_handle);
				if (yuv_len > 0) {
					frame_count += 1;
					//write_len = fwrite(out_buf, 1, yuv_len, ofile);
					player_sync(0, ctx);
				}
			}

		}



	}


	printf(jm_nvdec_show_dec_info(ctx->dec_handle));

	//jm_intel_dec_set_eof(1, ctx->dec_handle);

	jm_nvdec_deinit(ctx->dec_handle);

	if (in_buf) delete[] in_buf;
	if (out_buf) delete[] out_buf;

	if (ifile) fclose(ifile);

	return NULL;
}
static void *intel_dec_thread(void *arg)
{
	dec_ctx *ctx = (dec_ctx *)arg;

	jmgl_image	* img = ctx->img;	// TODO

	ctx->dec_handle = jm_intel_dec_create_handle();

	jm_intel_dec_init(0, 1, ctx->dec_handle);

	FILE *ifile = NULL;
	ifile = fopen(ctx->url, "rb");

	unsigned char *in_buf = NULL;
	unsigned char *out_buf = NULL;
	int in_len = 0, out_len = 0;

	in_len = (10 << 20);	// 10 MB
	out_len = (20 << 20);	// 20 MB
	in_buf = new unsigned char[in_len];
	out_buf = new unsigned char[out_len];


	int is_eof = 0;
	int yuv_len = 0;
	int remaining_len = 0;
	int rd_len = 0, wt_len = 0;
	int ret = 0;

#define WIDTH 3840//4096
#define HEIGHT 1920//2048

	int width = 0, height = 0;
	float frame_rate = 0.0;
	int duration = 0;


	while (!ctx->is_exit) {
		if (jm_intel_dec_need_more_data(ctx->dec_handle) && 0 == is_eof) {
			remaining_len = jm_intel_dec_free_buf_len(ctx->dec_handle);
			rd_len = fread(in_buf, 1, remaining_len, ifile);
			if (0 == rd_len) {
				// no more data
				is_eof = 1;
				jm_intel_dec_set_eof(1, ctx->dec_handle);
			}
			else {
				jm_intel_dec_input_data(in_buf, rd_len, ctx->dec_handle);
			}
		}

		ret = jm_intel_dec_output_frame(out_buf, &yuv_len, ctx->dec_handle);
		if (0 == ret/* && yuv_len > 0*/) {
			// Get stream display size
			if (!img->data) {

				jm_intel_get_stream_info(&width, &height, &frame_rate, ctx->dec_handle);

				duration = 1000 / frame_rate;

				img->data = out_buf;

				img->width = width;// WIDTH;
				img->height = height;// HEIGHT;
				img->y = img->data;
				img->u = img->y + img->width * img->height;
				img->v = img->u + (img->width * img->height) / 4;
			}

			// play sync
			player_sync(duration, ctx);
			//write_len = fwrite(out_buf, 1, yuv_len, ofile);
		}



	}


	printf(jm_intel_dec_info(ctx->dec_handle));

	//jm_intel_dec_set_eof(1, ctx->dec_handle);

	jm_intel_dec_deinit(ctx->dec_handle);

	if (in_buf) delete[] in_buf;
	if (out_buf) delete[] out_buf;

	if (ifile) fclose(ifile);

	return NULL;
}

dec_ctx *jmgl_dec_create_context()
{
	dec_ctx *ctx = NULL;

	ctx = new dec_ctx;
	memset(ctx, 0x0, sizeof(dec_ctx));

	return ctx;
}

int jmgl_dec_init(char *url, bool nv_dec, dec_ctx *ctx)
{
	ctx->url = (char*)malloc(strlen(url) + 1);
	if (ctx->url != NULL) strcpy(ctx->url, url);         // Copy string if okay

	ctx->is_nv_dec = nv_dec;

	ctx->img = new jmgl_image;

	memset(ctx->img, 0x0, sizeof(jmgl_image));

	if (nv_dec) {
		pthread_create(&ctx->play_thread, NULL, nv_dec_thread, ctx);
	}
	else {
		pthread_create(&ctx->play_thread, NULL, intel_dec_thread, ctx);
	}
	return 0;
}


int jmgl_dec_deinit(dec_ctx *ctx)
{
	ctx->is_exit = true;
	pthread_join(ctx->play_thread, NULL);
	
	free(ctx->url);
	//free(ctx->img->data);
	free(ctx->img);

	delete ctx;

	return 0;
}

int jmgl_dec_set_exit(int is_exit, dec_ctx *ctx)
{
	ctx->is_exit = is_exit;
	return 0;
}