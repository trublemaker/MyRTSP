#include "AudioPlay.h"
#include "assert.h"
#include "iostream"

//#include <SDL.h>
//#include <SDL_thread.h>

PacketQueue audioq;
int quit = 0;

AVFrame wanted_frame;


#define SDL_AUDIO_BUFFER_SIZE 1024 

AudioPlay::AudioPlay()
{
}


AudioPlay::~AudioPlay()
{
}


void AudioPlay::InitAudio()
{

}

// 包队列初始化
void packet_queue_init(PacketQueue* q)
{
	//memset(q, 0, sizeof(PacketQueue));
	q->last_pkt = nullptr;
	q->first_pkt = nullptr;
	q->mutext = SDL_CreateMutex();
	q->cond = SDL_CreateCond();
}

// 解码后的回调函数
void audio_callback(void* userdata, Uint8* stream, int len)
{
	AVCodecContext* aCodecCtx = (AVCodecContext*)userdata;
	int len1, audio_size;

	static uint8_t audio_buff[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
	static unsigned int audio_buf_size = 0;
	static unsigned int audio_buf_index = 0;

	SDL_memset(stream, 0, len);

	while (len > 0)
	{
		if (audio_buf_index >= audio_buf_size)
		{
			audio_size = audio_decode_frame(aCodecCtx, audio_buff, sizeof(audio_buff));
			if (audio_size < 0)
			{
				audio_buf_size = 1024;
				memset(audio_buff, 0, audio_buf_size);
			}
			else
				audio_buf_size = audio_size;

			audio_buf_index = 0;
		}
		len1 = audio_buf_size - audio_buf_index;
		if (len1 > len)
			len1 = len;

		//SDL_MixAudio(stream, audio_buff + audio_buf_index, len, SDL_MIX_MAXVOLUME);

		memcpy(stream, (uint8_t*)(audio_buff + audio_buf_index), audio_buf_size);
		len -= len1;
		stream += len1;
		audio_buf_index += len1;
	}
}

// 解码音频数据
int audio_decode_frame(AVCodecContext* aCodecCtx, uint8_t* audio_buf, int buf_size)
{
	static AVPacket pkt;
	static uint8_t* audio_pkt_data = nullptr;
	static int audio_pkt_size = 0;
	static AVFrame frame;

	int len1;
	int data_size = 0;

	SwrContext* swr_ctx = nullptr;

	while (true)
	{
		while (audio_pkt_size > 0)
		{
			int got_frame = 0;
			len1 = avcodec_decode_audio4(aCodecCtx, &frame, &got_frame, &pkt);
			if (len1 < 0) // 出错，跳过
			{
				audio_pkt_size = 0;
				break;
			}

			audio_pkt_data += len1;
			audio_pkt_size -= len1;
			data_size = 0;
			if (got_frame)
			{
				data_size = av_samples_get_buffer_size(nullptr, 
					aCodecCtx->channels,
					frame.nb_samples, 
					aCodecCtx->sample_fmt,
					1);

				assert(data_size <= buf_size);
				memcpy(audio_buf, frame.data[0], data_size);
			}

			if (frame.channels > 0 && frame.channel_layout == 0)
				frame.channel_layout = av_get_default_channel_layout(frame.channels);
			else if (frame.channels == 0 && frame.channel_layout > 0)
				frame.channels = av_get_channel_layout_nb_channels(frame.channel_layout);

			if (swr_ctx)
			{
				swr_free(&swr_ctx);
				swr_ctx = nullptr;
			}

			swr_ctx = swr_alloc_set_opts(nullptr, wanted_frame.channel_layout, (AVSampleFormat)wanted_frame.format, wanted_frame.sample_rate,
				frame.channel_layout, (AVSampleFormat)frame.format, frame.sample_rate, 0, nullptr);

			if (!swr_ctx || swr_init(swr_ctx) < 0)
			{
				cout << "swr_init failed:" << endl;
				break;
			}

			int dst_nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, frame.sample_rate) + frame.nb_samples,
				wanted_frame.sample_rate, wanted_frame.format, AVRounding(1));
			int len2 = swr_convert(swr_ctx, &audio_buf, dst_nb_samples,
				(const uint8_t**)frame.data, frame.nb_samples);
			if (len2 < 0)
			{
				cout << "swr_convert failed\n";
				break;
			}

			int rdatasize = wanted_frame.channels * len2 * av_get_bytes_per_sample((AVSampleFormat)wanted_frame.format);

			return rdatasize;

			if (data_size <= 0)
				continue; // No data yet,get more frames

			return data_size; // we have data,return it and come back for more later
		}

		if (pkt.data) {
			//av_free_packet(&pkt);
		}
		if (quit)
			return -1;

		if (packet_queue_get(&audioq, &pkt, true) < 0)
			return -1;

		audio_pkt_data = pkt.data;
		audio_pkt_size = pkt.size;
	}
}
// 从队列中取出packet
static int packet_queue_get(PacketQueue* q, AVPacket* pkt, bool block)
{
	AVPacketList* pktl;
	int ret;

	SDL_LockMutex(q->mutext);

	while (true)
	{
		if (quit)
		{
			ret = -1;
			break;
		}

		pktl = q->first_pkt;
		if (pktl)
		{
			q->first_pkt = pktl->next;
			if (!q->first_pkt)
				q->last_pkt = nullptr;

			q->nb_packets--;
			q->size -= pktl->pkt.size;

			*pkt = pktl->pkt;
			av_free(pktl);
			ret = 1;
			break;
		}
		else if (!block)
		{
			ret = 0;
			break;
		}
		else
		{
			SDL_CondWait(q->cond, q->mutext);
		}
	}

	SDL_UnlockMutex(q->mutext);

	return ret;
}
// 放入packet到队列中，不带头指针的队列
int packet_queue_put(PacketQueue*q, AVPacket *pkt)
{
	AVPacketList *pktl;
	if (av_dup_packet(pkt) < 0)
		return -1;

	pktl = (AVPacketList*)av_malloc(sizeof(AVPacketList));
	if (!pktl)
		return -1;

	pktl->pkt = *pkt;
	pktl->next = nullptr;

	SDL_LockMutex(q->mutext);

	if (!q->last_pkt) // 队列为空，新插入元素为第一个元素
		q->first_pkt = pktl;
	else // 插入队尾
		q->last_pkt->next = pktl;

	q->last_pkt = pktl;

	q->nb_packets++;
	q->size += pkt->size;

	SDL_CondSignal(q->cond);
	SDL_UnlockMutex(q->mutext);

	return 0;
}
