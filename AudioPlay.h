#pragma once
extern "C"
{
///#define DECLSPEC   // __declspec(dllimport)
#include "SDL2-2.0.10/include/SDL.h"
#include "SDL2-2.0.10/include/SDL_thread.h"
}

extern "C"
{
# include <libavcodec\avcodec.h>
# include <libavformat\avformat.h>
# include <libswscale\swscale.h>
# include <libswresample\swresample.h>
}

using namespace std;

class AudioPlay
{
public:
	AudioPlay();
	~AudioPlay();
	void InitAudio();
};


typedef struct PacketQueue
{
	AVPacketList *first_pkt; // 队头
	AVPacketList *last_pkt; // 队尾

	int nb_packets; //包的个数
	int size; // 占用空间的字节数
	SDL_mutex* mutext; // 互斥信号量
	SDL_cond* cond; // 条件变量
}PacketQueue;

extern  PacketQueue audioq;
extern int quit ;

extern AVFrame wanted_frame;
void audio_callback(void* userdata, Uint8* stream, int len);
int audio_decode_frame(AVCodecContext* aCodecCtx, uint8_t* audio_buf, int buf_size);
static int packet_queue_get(PacketQueue* q, AVPacket* pkt, bool block);
void packet_queue_init(PacketQueue* q);
int packet_queue_put(PacketQueue*q, AVPacket *pkt);

#define SDL_AUDIO_BUFFER_SIZE 1024 
#define MAX_AUDIO_FRAME_SIZE 192000