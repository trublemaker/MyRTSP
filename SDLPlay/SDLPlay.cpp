// SDLPlay.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

//作者：持盾不给力
//链接：https ://www.zhihu.com/question/51040587/answer/123823358
//来源：知乎
//著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。

//
//FFMPEG+SDL音频解码程序
//雷霄骅
//中国传媒大学/数字电视技术
//leixiaohua1020@126.com
//
//

extern "C"
{

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/pixfmt.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/time.h"
#include "libavutil/mathematics.h"

//SDL
#include "SDL.h"
#include "SDL_thread.h"
};

#undef main
#include <Windows.h>

//#include "decoder.h"
//#include "wave.h"

//#define _WAVE_

//全局变量---------------------
 Uint8  *audio_chunk;
 Uint32  audio_len=0;
 Uint8  *audio_pos;

int iCount = 0;

//-----------------
/*  The audio function callback takes the following parameters:
stream: A pointer to the audio buffer to be filled
len: The length (in bytes) of the audio buffer (这是固定的4096？)
回调函数
注意：mp3为什么播放不顺畅？
len=4096;audio_len=4608;两个相差512！为了这512，还得再调用一次回调函数。。。
m4a,aac就不存在此问题(都是4096)！
*/
void  fill_audio(void *udata, Uint8 *stream, int len) {
	/*  Only  play  if  we  have  data  left  */
	if (audio_len == 0)
		return;
	/*  Mix  as  much  data  as  possible  */
	len = (len>audio_len ? audio_len : len);
	//printf("len = %d \t", len ); 
	//SDL_memset(stream, 0, len);
	//memset(stream, 0, len);
	//stream[len] = 0;
	//SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
	SDL_MixAudioFormat(stream, (uint8_t *)audio_pos, AUDIO_S16SYS, len, SDL_MIX_MAXVOLUME);

	audio_pos += len;
	audio_len -= len;
	printf("  audio_len:%8d len:%8d, %d\n",   audio_len,len, iCount++);
	//::Sleep(10);
}
//-----------------
const char program_name[] = "ffplay";
const int program_birth_year = 2003;

#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define MIN_FRAMES 25
#define EXTERNAL_CLOCK_MIN_FRAMES 2
#define EXTERNAL_CLOCK_MAX_FRAMES 10

/* Minimum SDL audio buffer size, in samples. */
#define SDL_AUDIO_MIN_BUFFER_SIZE 512
/* Calculate actual buffer size keeping in mind not cause too frequent audio callbacks */
#define SDL_AUDIO_MAX_CALLBACKS_PER_SEC 30

/* Step size for volume control in dB */
#define SDL_VOLUME_STEP (0.75)

/* no AV sync correction is done if below the minimum AV sync threshold */
#define AV_SYNC_THRESHOLD_MIN 0.04
/* AV sync correction is done if above the maximum AV sync threshold */
#define AV_SYNC_THRESHOLD_MAX 0.1
/* If a frame duration is longer than this, it will not be duplicated to compensate AV sync */
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1
/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0

/* maximum audio speed change to get correct sync */
#define SAMPLE_CORRECTION_PERCENT_MAX 10

/* external clock speed adjustment constants for realtime sources based on buffer fullness */
#define EXTERNAL_CLOCK_SPEED_MIN  0.900
#define EXTERNAL_CLOCK_SPEED_MAX  1.010
#define EXTERNAL_CLOCK_SPEED_STEP 0.001

/* we use about AUDIO_DIFF_AVG_NB A-V differences to make the average */
#define AUDIO_DIFF_AVG_NB   20

/* polls for possible required screen refresh at least this often, should be less than 1/fps */
#define REFRESH_RATE 0.01

/* NOTE: the size must be big enough to compensate the hardware audio buffersize size */
/* TODO: We assume that a decoded and resampled frame fits into this buffer */
#define SAMPLE_ARRAY_SIZE (8 * 65536)

#define CURSOR_HIDE_DELAY 1000000

#define USE_ONEPASS_SUBTITLE_RENDER 1

static unsigned sws_flags = SWS_BICUBIC;

typedef struct MyAVPacketList {
	AVPacket *pkt;
	int serial;
} MyAVPacketList;

typedef struct PacketQueue {
	//AVFifoBuffer *pkt_list;
	int nb_packets;
	int size;
	int64_t duration;
	int abort_request;
	int serial;
	SDL_mutex *mutex;
	SDL_cond *cond;
} PacketQueue;

#define VIDEO_PICTURE_QUEUE_SIZE 3
#define SUBPICTURE_QUEUE_SIZE 16
#define SAMPLE_QUEUE_SIZE 9
#define FRAME_QUEUE_SIZE FFMAX(SAMPLE_QUEUE_SIZE, FFMAX(VIDEO_PICTURE_QUEUE_SIZE, SUBPICTURE_QUEUE_SIZE))

typedef struct AudioParams {
	int freq;
	int channels;
	int64_t channel_layout;
	enum AVSampleFormat fmt;
	int frame_size;
	int bytes_per_sec;
} AudioParams;

typedef struct Clock {
	double pts;           /* clock base */
	double pts_drift;     /* clock base minus time at which we updated the clock */
	double last_updated;
	double speed;
	int serial;           /* clock is based on a packet with this serial */
	int paused;
	int *queue_serial;    /* pointer to the current packet queue serial, used for obsolete clock detection */
} Clock;

/* Common struct for handling all types of decoded data and allocated render buffers. */
typedef struct Frame {
	AVFrame *frame;
	AVSubtitle sub;
	int serial;
	double pts;           /* presentation timestamp for the frame */
	double duration;      /* estimated duration of the frame */
	int64_t pos;          /* byte position of the frame in the input file */
	int width;
	int height;
	int format;
	AVRational sar;
	int uploaded;
	int flip_v;
} Frame;

typedef struct FrameQueue {
	Frame queue[FRAME_QUEUE_SIZE];
	int rindex;
	int windex;
	int size;
	int max_size;
	int keep_last;
	int rindex_shown;
	SDL_mutex *mutex;
	SDL_cond *cond;
	PacketQueue *pktq;
} FrameQueue;

enum {
	AV_SYNC_AUDIO_MASTER, /* default choice */
	AV_SYNC_VIDEO_MASTER,
	AV_SYNC_EXTERNAL_CLOCK, /* synchronize to an external clock */
};

typedef struct Decoder {
	AVPacket *pkt;
	PacketQueue *queue;
	AVCodecContext *avctx;
	int pkt_serial;
	int finished;
	int packet_pending;
	SDL_cond *empty_queue_cond;
	int64_t start_pts;
	AVRational start_pts_tb;
	int64_t next_pts;
	AVRational next_pts_tb;
	SDL_Thread *decoder_tid;
} Decoder;

typedef struct VideoState {
	SDL_Thread *read_tid;
	const AVInputFormat *iformat;
	int abort_request;
	int force_refresh;
	int paused;
	int last_paused;
	int queue_attachments_req;
	int seek_req;
	int seek_flags;
	int64_t seek_pos;
	int64_t seek_rel;
	int read_pause_return;
	AVFormatContext *ic;
	int realtime;

	Clock audclk;
	Clock vidclk;
	Clock extclk;

	FrameQueue pictq;
	FrameQueue subpq;
	FrameQueue sampq;

	Decoder auddec;
	Decoder viddec;
	Decoder subdec;

	int audio_stream;

	int av_sync_type;

	double audio_clock;
	int audio_clock_serial;
	double audio_diff_cum; /* used for AV difference average computation */
	double audio_diff_avg_coef;
	double audio_diff_threshold;
	int audio_diff_avg_count;
	AVStream *audio_st;
	PacketQueue audioq;
	int audio_hw_buf_size;
	uint8_t *audio_buf;
	uint8_t *audio_buf1;
	unsigned int audio_buf_size; /* in bytes */
	unsigned int audio_buf1_size;
	int audio_buf_index; /* in bytes */
	int audio_write_buf_size;
	int audio_volume;
	int muted;
	struct AudioParams audio_src;
#if CONFIG_AVFILTER
	struct AudioParams audio_filter_src;
#endif
	struct AudioParams audio_tgt;
	struct SwrContext *swr_ctx;
	int frame_drops_early;
	int frame_drops_late;

	enum ShowMode {
		SHOW_MODE_NONE = -1, SHOW_MODE_VIDEO = 0, SHOW_MODE_WAVES, SHOW_MODE_RDFT, SHOW_MODE_NB
	} show_mode;
	int16_t sample_array[SAMPLE_ARRAY_SIZE];
	int sample_array_index;
	int last_i_start;
	//RDFTContext *rdft;
	int rdft_bits;
	//FFTSample *rdft_data;
	int xpos;
	double last_vis_time;
	SDL_Texture *vis_texture;
	SDL_Texture *sub_texture;
	SDL_Texture *vid_texture;

	int subtitle_stream;
	AVStream *subtitle_st;
	PacketQueue subtitleq;

	double frame_timer;
	double frame_last_returned_time;
	double frame_last_filter_delay;
	int video_stream;
	AVStream *video_st;
	PacketQueue videoq;
	double max_frame_duration;      // maximum duration of a frame - above this, we consider the jump a timestamp discontinuity
	struct SwsContext *img_convert_ctx;
	struct SwsContext *sub_convert_ctx;
	int eof;

	char *filename;
	int width, height, xleft, ytop;
	int step;

#if CONFIG_AVFILTER
	int vfilter_idx;
	AVFilterContext *in_video_filter;   // the first filter in the video chain
	AVFilterContext *out_video_filter;  // the last filter in the video chain
	AVFilterContext *in_audio_filter;   // the first filter in the audio chain
	AVFilterContext *out_audio_filter;  // the last filter in the audio chain
	AVFilterGraph *agraph;              // audio filter graph
#endif

	int last_video_stream, last_audio_stream, last_subtitle_stream;

	SDL_cond *continue_read_thread;
} VideoState;
static int64_t audio_callback_time;
/* prepare a new audio buffer */
static void sdl_audio_callback(void *opaque, Uint8 *stream, int len)
{
	VideoState *is = (VideoState*)opaque;
	int audio_size, len1;

	audio_callback_time = av_gettime_relative();

	while (len > 0) {
		if (is->audio_buf_index >= is->audio_buf_size) {
			//audio_size = audio_decode_frame(is);
			if (audio_size < 0) {
				/* if error, just output silence */
				is->audio_buf = NULL;
				is->audio_buf_size = SDL_AUDIO_MIN_BUFFER_SIZE / is->audio_tgt.frame_size * is->audio_tgt.frame_size;
			}
			else {
				//if (is->show_mode != SHOW_MODE_VIDEO)
				//	update_sample_display(is, (int16_t *)is->audio_buf, audio_size);
				is->audio_buf_size = audio_size;
			}
			is->audio_buf_index = 0;
		}
		len1 = is->audio_buf_size - is->audio_buf_index;
		if (len1 > len)
			len1 = len;
		if (!is->muted && is->audio_buf && is->audio_volume == SDL_MIX_MAXVOLUME)
			memcpy(stream, (uint8_t *)is->audio_buf + is->audio_buf_index, len1);
		else {
			memset(stream, 0, len1);
			if (!is->muted && is->audio_buf)
				SDL_MixAudioFormat(stream, (uint8_t *)is->audio_buf + is->audio_buf_index, AUDIO_S16SYS, len1, is->audio_volume);
		}
		len -= len1;
		stream += len1;
		is->audio_buf_index += len1;
	}
	is->audio_write_buf_size = is->audio_buf_size - is->audio_buf_index;
	/* Let's assume the audio driver that is used by SDL has two periods. */
	if (!isnan(is->audio_clock)) {
		//set_clock_at(&is->audclk, is->audio_clock - (double)(2 * is->audio_hw_buf_size + is->audio_write_buf_size) / is->audio_tgt.bytes_per_sec, is->audio_clock_serial, audio_callback_time / 1000000.0);
		//sync_clock_to_slave(&is->extclk, &is->audclk);
	}
}

int decode_audio(char* no_use)
{
	AVFormatContext	*pFormatCtx;
	int				i, audioStream;
	AVCodecContext	*pCodecCtx;
	AVCodec			*pCodec;

	char url[300] = { 0 };
	strcpy(url, no_use);
	//Register all available file formats and codecs
	av_register_all();

	//支持网络流输入
	avformat_network_init();
	//初始化
	pFormatCtx = avformat_alloc_context();
	//有参数avdic
	//if(avformat_open_input(&pFormatCtx,url,NULL,&avdic)!=0){
	if (avformat_open_input(&pFormatCtx, url, NULL, NULL) != 0) {
		printf("Couldn't open file.\n");
		return -1;
	}

	// Retrieve stream information
	if (avformat_find_stream_info(pFormatCtx, NULL)<0)
	{
		printf("Couldn't find stream information.\n");
		return -1;
	}
	// Dump valid information onto standard error
	av_dump_format(pFormatCtx, 0, url, false);

	// Find the first audio stream
	audioStream = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
		//原为codec_type==CODEC_TYPE_AUDIO
		if (pFormatCtx->streams[i]->codec->codec_type == 1)
		{
			audioStream = i;
			break;
		}

	if (audioStream == -1)
	{
		printf("Didn't find a audio stream.\n");
		return -1;
	}

	// Get a pointer to the codec context for the audio stream
	pCodecCtx = pFormatCtx->streams[audioStream]->codec;

	// Find the decoder for the audio stream
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL)
	{
		printf("Codec not found.\n");
		return -1;
	}

	// Open codec
	if (avcodec_open2(pCodecCtx, pCodec, NULL)<0)
	{
		printf("Could not open codec.\n");
		return -1;
	}

	/********* For output file ******************/
	FILE *pFile;
#ifdef _WAVE_
	pFile = fopen("output.wav", "wb");
	fseek(pFile, 44, SEEK_SET); //预留文件头的位置
#else
	pFile = fopen("output.pcm", "wb");
#endif

	// Open the time stamp file
	FILE *pTSFile;
	pTSFile = fopen("audio_time_stamp.txt", "wb");
	if (pTSFile == NULL)
	{
		printf("Could not open output file.\n");
		return -1;
	}
	fprintf(pTSFile, "Time Base: %d/%d\n", pCodecCtx->time_base.num, pCodecCtx->time_base.den);

	/*** Write audio into file ******/
	//把结构体改为指针
	AVPacket *packet = (AVPacket *)malloc(sizeof(AVPacket));
	av_init_packet(packet);

	//音频和视频解码更加统一！
	//新加
	AVFrame	*pFrame;
	pFrame = av_frame_alloc();

	//---------SDL--------------------------------------
	//初始化
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		printf("Could not initialize SDL - %s\n", SDL_GetError());
		exit(1);
	}

	//结构体，包含PCM数据的相关信息
	SDL_AudioSpec wanted_spec;
	wanted_spec.freq =  pCodecCtx->sample_rate;
	//wanted_spec.freq = 48000;
	wanted_spec.format = AUDIO_S16SYS; AUDIO_F32SYS; AUDIO_U16SYS; AUDIO_S32SYS; AUDIO_S16SYS;
	wanted_spec.channels =  pCodecCtx->channels;
	wanted_spec.silence = 0;
	wanted_spec.samples = 1024; //播放AAC，M4a，缓冲区的大小 
								//wanted_spec.samples = 1152; //播放MP3，WMA时候用
	wanted_spec.callback = fill_audio;
	wanted_spec.userdata = pCodecCtx;

	if (SDL_OpenAudio(&wanted_spec, NULL)<0)//步骤（2）打开音频设备
	{
		printf("error : %s\n", SDL_GetError());
		return 0;
	}
	//-----------------------------------------------------
	printf("bit rate %3d\n", pFormatCtx->bit_rate);
	printf("name of decoder %s\n", pCodecCtx->codec->long_name);
	printf("time_base  %d \n", pCodecCtx->time_base);
	printf("number of channels  %d \n", pCodecCtx->channels);
	printf("sample per second  %d \n", pCodecCtx->sample_rate);
	//新版不再需要
	//	short decompressed_audio_buf[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
	//	int decompressed_audio_buf_size;
	uint32_t ret, len = 0;
	int got_picture;
	int index = 0;
	while (av_read_frame(pFormatCtx, packet) >= 0&& audio_len==0)
	{
		if (packet->stream_index == audioStream)
		{
			//decompressed_audio_buf_size = (AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2;
			//原为avcodec_decode_audio2
			//ret = avcodec_decode_audio4( pCodecCtx, decompressed_audio_buf,
			//&decompressed_audio_buf_size, packet.data, packet.size );
			//改为
			ret = avcodec_decode_audio4(pCodecCtx, pFrame,
				&got_picture, packet);
			if (ret < 0) // if error len = -1
			{
				printf("Error in decoding audio frame.\n");
				exit(0);
			}
			if (got_picture > 0)
			{
#if 0
				printf("index %3d\n", index);
				printf("pts %5d\n", packet->pts);
				printf("dts %5d\n", packet->dts);
				printf("packet_size %5d\n", packet->size);

				//printf("test %s\n", rtmp->m_inChunkSize);
#endif
				//直接写入
				//注意：数据是data【0】，长度是linesize【0】
#if 0
				fwrite(pFrame->data[0], 1, pFrame->linesize[0], pFile);
				//fwrite(pFrame, 1, got_picture, pFile);
				//len+=got_picture;
				index++;
				//fprintf(pTSFile, "%4d,%5d,%8d\n", index, decompressed_audio_buf_size, packet.pts);
#endif
			}
#if 1
			//---------------------------------------
			//printf("begin....\n");
			//设置音频数据缓冲,PCM数据
			audio_chunk = (Uint8*)pFrame->data[0];
			//设置音频数据长度
			audio_len = pFrame->linesize[0];
			//audio_len = 4096;
			//播放mp3的时候改为audio_len = 4096
			//则会比较流畅，但是声音会变调！MP3一帧长度4608
			//使用一次回调函数（4096字节缓冲）播放不完，所以还要使用一次回调函数，导致播放缓慢。。。
			//设置初始播放位置
			audio_pos = audio_chunk;
			//回放音频数据
			SDL_PauseAudio(0);
			//printf("don't close, audio playing...\n");
			while (audio_len>0)//等待直到音频数据播放完毕!
				SDL_Delay(0);
			//---------------------------------------
#endif
		}
		// Free the packet that was allocated by av_read_frame
		//已改
		av_free_packet(packet);
	}
	//printf("The length of PCM data is %d bytes.\n", len);

#ifdef _WAVE_
	fseek(pFile, 0, SEEK_SET);
	struct WAVE_HEADER wh;

	memcpy(wh.header.RiffID, "RIFF", 4);
	wh.header.RiffSize = 36 + len;
	memcpy(wh.header.RiffFormat, "WAVE", 4);

	memcpy(wh.format.FmtID, "fmt ", 4);
	wh.format.FmtSize = 16;
	wh.format.wavFormat.FormatTag = 1;
	wh.format.wavFormat.Channels = pCodecCtx->channels;
	wh.format.wavFormat.SamplesRate = pCodecCtx->sample_rate;
	wh.format.wavFormat.BitsPerSample = 16;
	calformat(wh.format.wavFormat); //Calculate AvgBytesRate and BlockAlign

	memcpy(wh.data.DataID, "data", 4);
	wh.data.DataSize = len;

	fwrite(&wh, 1, sizeof(wh), pFile);
#endif
	SDL_CloseAudio();//关闭音频设备
					 // Close file
	fclose(pFile);
	// Close the codec
	avcodec_close(pCodecCtx);
	// Close the video file
	//av_close_input_file(pFormatCtx);

	return 0;
}

int main(int argc, char* argv[])
{
	//if (argc < 1) 
	{
		printf("please enter your music file name\n");
		//return 0;
	}

	//char * filename = "F:\\Music\\买辣椒也用券 - 起风了.mp3";// argv[1];
	//char * filename = "F:\\Music\\深情MV《大地》致敬弗格森 永远的红魔主帅！.flv";
	char * filename = "C:\\Qt\\RTSP\\zsyf.mp3";

	//scanf("%s", filename);
	if (decode_audio(filename) == 0)
		printf("Decode audio successfully.\n");

	return 0;
}
