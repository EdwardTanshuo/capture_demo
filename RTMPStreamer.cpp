#include "RTMPStreamer.h"
#include "stdafx.h"
#include "AVException.h"

extern "C"
{
#include "include\jpeglib.h"
#include "include\turbojpeg.h"
}

#define COLOR_COMPONENTS	3
#define OUTPUT_CODEC		AV_CODEC_ID_H264
#define OUTPUT_PIX_FMT		AV_PIX_FMT_YUV420P
#define INPUT_PIX_FMT		AV_PIX_FMT_RGB24

#define DEFAULT_WIDTH	960
#define DEFAULT_HEIGHT	640
#define DEFAULT_RATE	80 * 1024
#define DEFAULT_URL		"rtmp://127.0.0.1:1935/live/test"

static tjhandle _jpegDecompressor = nullptr;

Encoder* RTMPStreamer::encoder = nullptr;

bool RTMPStreamer::jpeg_decompress_inited = false;
bool RTMPStreamer::av_utils_inited = false;

RTMPStreamer::RTMPStreamer() {

}


RTMPStreamer::~RTMPStreamer() {

}

void RTMPStreamer::push_buffer(unsigned char* pbyteImage, EdsUInt64 size) {
	if (!av_utils_inited || !jpeg_decompress_inited) {
		return;
	}
	// alloc image
	HGLOBAL hMem = ::GlobalAlloc(GHND, size);
	LPVOID pBuff = ::GlobalLock(hMem);
	memcpy(pBuff, pbyteImage, size);
	::GlobalUnlock(hMem);

	// decompress jpeg to RGB buffer
	EdsUInt64 w, h;
	unsigned char* buffer = decode_image(static_cast<unsigned char *>(pBuff), size, w, h);


	// obtain a frame
	auto frame = encoder->write_frame(buffer, w, h);
	encoder->write(frame);

	// release resource
	free(buffer);
	GlobalFree(hMem);
}

// init components
bool RTMPStreamer::init_componets() {
	return init_componets(DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_URL, DEFAULT_RATE);
}

bool RTMPStreamer::init_componets(const char* url) {
	return init_componets(DEFAULT_WIDTH, DEFAULT_HEIGHT, url, DEFAULT_RATE);
}


bool RTMPStreamer::init_componets(int w, int h, const char* url, long bitrate) {
	if (!av_utils_inited) {
		bool result = init_av_utils(w, h, url, bitrate);
		if (!result) {
			return false;
		}
	}
	else {
		OutputDebugStringW(L"[WARNING] - av utils has been inited, skip.");
	}

	if (!jpeg_decompress_inited) {
		bool result = init_jpeg_decompressor();
		if (!result) {
			return false;
		}
	}
	else {
		OutputDebugStringW(L"[WARNING] - jpeg decompressor has been inited, skip.");
	}

	return true;
}

// destroy components
void RTMPStreamer::destroy_componets() {
	if (av_utils_inited) {
		destroy_av_utils();
	}
	if (jpeg_decompress_inited) {
		destroy_jpeg_decompressor();
	}
}

bool RTMPStreamer::init_jpeg_decompressor() {
	// jpeg decompressor
	_jpegDecompressor = tjInitDecompress();
	jpeg_decompress_inited = true;
	return true;
}

void RTMPStreamer::destroy_jpeg_decompressor() {
	tjDestroy(_jpegDecompressor);

	jpeg_decompress_inited = false;
}

bool RTMPStreamer::init_av_utils(int width, int height, const char* url, long bitrate) {
	try {
		encoder = new Encoder(width, height, url, bitrate);
		av_utils_inited = true;
	}
	catch (AVException e) {
		av_utils_inited = false;
		return false;
	}
	OutputDebugStringW(L"[DEBUG] - AV components inited.");
	return true;
}

void RTMPStreamer::destroy_av_utils() {
	delete encoder;
	encoder = nullptr;
	av_utils_inited = false;
}

unsigned char* RTMPStreamer::decode_image(unsigned char* pcBuffer, EdsUInt64 size, EdsUInt64& width, EdsUInt64& height) {
	// read jpeg header
	tjDecompressHeader2(_jpegDecompressor, pcBuffer, size, &frame_width, &frame_height, &jpegSubsamp);
	width = frame_width;
	height = frame_height;

	// init buffer
	int buffer_size = frame_width * frame_height * COLOR_COMPONENTS;
	unsigned char* buffer = static_cast<unsigned char*>(malloc(buffer_size));

	// decompress the image
	tjDecompress2(_jpegDecompressor, pcBuffer, size, buffer, width, 0, height, TJPF_RGB, TJFLAG_FASTDCT);

	return buffer;
}

Encoder::Encoder(int width, int height, const char* url, long bitrate) {
	// reset frame counter
	frame_index = 0;
	start_time = av_gettime();

	int err = 0;

	AVCodec*			codec = nullptr;
	AVDictionary*		fmt_opts = nullptr;
	AVOutputFormat*		fmt = nullptr;

	// regist all av utils 
	av_register_all();

	// regist network components
	avformat_network_init();

	// init a sws context
	sws_ctx = sws_getContext(
		width, height,
		INPUT_PIX_FMT,
		width, height,
		OUTPUT_PIX_FMT,
		SWS_BILINEAR, nullptr, nullptr, nullptr);

	// alloc a av format context
	this->fmt_ctx = avformat_alloc_context();
	if (this->fmt_ctx == nullptr) {
		throw AVException(ENOMEM, "can not alloc av format context");
	}

	//init encoding format
	avformat_alloc_output_context2(&fmt_ctx, NULL, "flv", url);
	if (!fmt_ctx) {
		throw AVException(ENOMEM, "can not open output format context.");
	}
	fmt = fmt_ctx->oformat;
	if (!fmt) {
		throw AVException(ENOMEM, "can not get the output format.");
	}


	// set format header infos
	snprintf(fmt_ctx->filename, sizeof(fmt_ctx->filename), "%s", url);

	// set format's privater options, to be passed to avformat_write_header()
	err = av_dict_set(&fmt_opts, "movflags", "faststart", 0);
	if (err < 0) {
		std::cerr << "Error : " << AVException(err, "av_dict_set movflags").what() << std::endl;
	}

	// default brand is "isom", which fails on some devices
	av_dict_set(&fmt_opts, "brand", "mp42", 0);
	if (err < 0) {
		std::cerr << "Error : " << AVException(err, "av_dict_set brand").what() << std::endl;
	}

	// looking for a codec
	codec = avcodec_find_encoder(OUTPUT_CODEC);
	if (!codec) {
		throw AVException(EPERM, "can't find encoder");
	}

	// create a new stream use the codec
	if (!(st = avformat_new_stream(fmt_ctx, codec))) {
		throw AVException(EPERM, "can't create new stream");
	}

	// set stream time_base, assume that camera frame rate is 50 fps 
	st->time_base = AVRational{ 1, 50 };

	// set codec_ctx to stream's codec structure
	codec_ctx = st->codec;


	// set sample parameters
	codec_ctx->sample_fmt = codec->sample_fmts ? codec->sample_fmts[0] : AV_SAMPLE_FMT_S16;
	codec_ctx->width = width;
	codec_ctx->height = height;
	codec_ctx->bit_rate = bitrate;
	codec_ctx->time_base = st->time_base;
	codec_ctx->pix_fmt = OUTPUT_PIX_FMT;

	//H.264 specific options
	codec_ctx->gop_size = 1;
	codec_ctx->level = 31;
	err = av_opt_set(codec_ctx->priv_data, "crf", "12", 0);
	if (err < 0) {
		std::cerr << "Error : " << AVException(err, "av_opt_set crf").what() << std::endl;
	}
	err = av_opt_set(codec_ctx->priv_data, "profile", "main", 0);
	if (err < 0) {
		std::cerr << "Error : " << AVException(err, "av_opt_set profile").what() << std::endl;
	}
	err = av_opt_set(codec_ctx->priv_data, "preset", "fast", 0);
	if (err < 0) {
		std::cerr << "Error : " << AVException(err, "av_opt_set preset").what() << std::endl;
	}

	// disable b-pyramid. support ios player
	err = av_opt_set(codec_ctx->priv_data, "b-pyramid", "0", 0);
	if (err < 0) {
		std::cerr << "Error : " << AVException(err, "av_opt_set b-pyramid").what() << std::endl;
	}

	// open the codec and link to the context
	err = avcodec_open2(codec_ctx, codec, NULL);
	if (err < 0) {
		throw AVException(err, "avcodec_open2");
	}

	// dump av format informations (debug)
	av_dump_format(fmt_ctx, 0, url, 1);


	// open the url
	if (!(fmt->flags & AVFMT_NOFILE)) {
		int ret = avio_open(&fmt_ctx->pb, url, AVIO_FLAG_WRITE);
		if (ret < 0) {
			throw AVException(ret, "can not open the url.");
		}
	}

	// write file header if necessary
	err = avformat_write_header(fmt_ctx, &fmt_opts);
	if (err < 0) {
		throw AVException(err, "avformat_write_header");
	}

	// init a frame
	frame_yuv420p = av_frame_alloc();

	// link a buffer to the frame
	int numBytes = avpicture_get_size(OUTPUT_PIX_FMT, width, height);
	uint8_t* yuv_buffer = static_cast<uint8_t*>(av_malloc(numBytes * sizeof(uint8_t)));
	avpicture_fill((AVPicture *)frame_yuv420p, yuv_buffer, OUTPUT_PIX_FMT, width, height);
}

Encoder::~Encoder() {
	int err;
	std::cout << "cleaning Encoder" << std::endl;

	// flush pending packets
	while ((err = write(static_cast<AVFrame*>(nullptr))) == 1) {};
	if (err < 0) {
		std::cout << "error writing delayed frame" << std::endl;
	}

	// write file trailer before exit
	av_write_trailer(this->fmt_ctx);

	// close file
	avio_close(fmt_ctx->pb);

	// close av format context
	avformat_free_context(this->fmt_ctx);

	// close codec context
	avcodec_flush_buffers(codec_ctx);
	avcodec_free_context(&codec_ctx);

	// clean the frame
	av_frame_free(&frame_yuv420p);

	// free sws context
	sws_freeContext(sws_ctx);
}

// return 1 if a packet was written. 0 if nothing was done.
// return error_code < 0 if there was an error.
int Encoder::write(AVFrame* frame) {
	int err;
	int got_output = 1;
	AVPacket pkt = { 0 };
	av_init_packet(&pkt);

	//Set frame pts, monotonically increasing, starting from 0
	if (frame != NULL) frame->pts = pts++;
	err = avcodec_encode_video2(this->codec_ctx, &pkt, frame, &got_output);
	if (err < 0) {
		std::cout << AVException(err, "encode frame").what() << std::endl;
		return err;
	}
	if (got_output) {
		av_packet_rescale_ts(&pkt, this->codec_ctx->time_base, this->st->time_base);
		pkt.stream_index = this->st->index;

		// convert from camara rate
		//pkt.pts = av_rescale_q_rnd(pkt.pts, AVRational { 1, 10 }, st->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		//pkt.dts = av_rescale_q_rnd(pkt.dts, AVRational{ 1, 10 }, st->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		//pkt.duration = av_rescale_q(pkt.duration, AVRational{ 1, 10 }, st->time_base);
		pkt.pos = -1;

		// time base parameters
		AVRational time_base = st->time_base;
		AVRational time_base_q = { 1, AV_TIME_BASE };
		int64_t pts_gap = av_rescale_q(1, time_base, time_base_q) * av_q2d(this->codec_ctx->time_base);

		// write pts
		pkt.pts = (double)(frame_index * pts_gap);
		pkt.dts = pkt.pts;

		int64_t pts_time = av_rescale_q(pkt.dts, time_base, time_base_q);
		int64_t now_time = av_gettime() - start_time;

		// sync
		if (pts_time > now_time) {
			// wait (slow down, will result in accumulated latency)
			// av_usleep(pts_time - now_time);
		}
		else if (pts_time < now_time) {
			// jump a frame (speed up)
			frame_index++;
			frame->pts++;
			pkt.pts += pts_gap;
			pkt.dts += pts_gap;
		}

		// write_frame will take care of freeing the packet.
		err = av_interleaved_write_frame(this->fmt_ctx, &pkt);
		frame_index++;

	end:	av_free_packet(&pkt);
		if (err < 0) {
			std::cout << AVException(err, "write frame").what() << std::endl;
			return err;
		}
		return 1;
	}
	else {
		return 0;
	}
}

AVFrame* Encoder::write_frame(unsigned char* buffer, int w, int h) {
	uint8_t* inData[1] = { buffer };
	int      inLinesize[1] = { 3 * w };
	int h_slice = sws_scale(sws_ctx, inData,
		inLinesize, 0, h,
		frame_yuv420p->data, frame_yuv420p->linesize);
	return frame_yuv420p;
}