#pragma once
#include <EDSDKTypes.h>

extern "C"
{
#include <libavcodec\avcodec.h>
#include <libavformat\avformat.h>
#include <libavutil\imgutils.h>
#include <libavformat\avio.h>
#include <libswscale\swscale.h>
#include <libavutil\time.h>
#include <libavutil\opt.h>
}

class Encoder;

class RTMPStreamer {
public:
	RTMPStreamer();
	~RTMPStreamer();

	// init components
	bool init_componets();
	bool init_componets(const char* url);
	bool init_componets(int w, int h, const char* url, long bitrate);
	// destroy components
	void destroy_componets();
	// append new frame into the stream
	void push_buffer(unsigned char* pbyteImage, EdsUInt64 size);

private:
	int	frame_width;
	int	frame_height;
	int jpegSubsamp;

	static Encoder* encoder;

	static bool jpeg_decompress_inited;
	static bool	av_utils_inited;
	
	static bool init_jpeg_decompressor();
	static void destroy_jpeg_decompressor();

	static bool init_av_utils(int width, int height, const char* url, long bitrate);
	static void destroy_av_utils();

	unsigned char* decode_image(unsigned char* pcBuffer, EdsUInt64 size, EdsUInt64& width, EdsUInt64& height);
};

class Encoder {
private:
	AVFormatContext*	fmt_ctx;
	AVCodecContext*		codec_ctx; //a shortcut to st->codec
	AVStream*			st;
	SwsContext*			sws_ctx;
	AVFrame*			frame_yuv420p;
	int					pts = 0;
	int					frame_index = 0;
	int64_t				start_time;
public:
	Encoder(int width, int height, const char* url, long bitrate);
	~Encoder();
	int write(AVFrame* frame);
	AVFrame* write_frame(unsigned char* buffer, int w, int h);
};

