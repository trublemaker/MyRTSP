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
void packet_queue_init(PacketQueue *q) {
	memset(q, 0, sizeof(PacketQueue));
	q->mutex = SDL_CreateMutex();
	q->cond = SDL_CreateCond();
}

int packet_queue_put(PacketQueue *q, AVPacket *pkt) {

	AVPacketList *pkt1;
	if (av_dup_packet(pkt) < 0) {
		return -1;
	}
	pkt1 = (AVPacketList*)av_malloc(sizeof(AVPacketList));
	if (!pkt1)
		return -1;
	pkt1->pkt = *pkt;
	pkt1->next = NULL;

	SDL_LockMutex(q->mutex);

	if (!q->last_pkt)
		q->first_pkt = pkt1;
	else
		q->last_pkt->next = pkt1;
	q->last_pkt = pkt1;
	q->nb_packets++;
	q->size += pkt1->pkt.size;
	SDL_CondSignal(q->cond);

	SDL_UnlockMutex(q->mutex);
	return 0;
}

static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block)
{
	AVPacketList *pkt1;
	int ret;

	SDL_LockMutex(q->mutex);

	for (;;) {

		if (quit) {
			ret = -1;
			break;
		}

		pkt1 = q->first_pkt;
		if (pkt1) {
			q->first_pkt = pkt1->next;
			if (!q->first_pkt)
				q->last_pkt = NULL;
			q->nb_packets--;
			q->size -= pkt1->pkt.size;
			*pkt = pkt1->pkt;
			av_free(pkt1);
			ret = 1;
			break;
		}
		else if (!block) {
			ret = 0;
			break;
		}
		else {
			SDL_CondWait(q->cond, q->mutex);
		}
	}
	SDL_UnlockMutex(q->mutex);
	return ret;
}

int audio_decode_frame(AVCodecContext *aCodecCtx, uint8_t *audio_buf, int buf_size) {

	static AVPacket pkt;
	static uint8_t *audio_pkt_data = NULL;
	static int audio_pkt_size = 0;
	static AVFrame frame;

	int len1, data_size = 0;

	SwrContext* swr_ctx = nullptr;

	for (;;) {
		while (audio_pkt_size > 0) {
			int got_frame = 0;
			len1 = avcodec_decode_audio4(aCodecCtx, &frame, &got_frame, &pkt);
			if (len1 < 0) {
				/* if error, skip frame */
				audio_pkt_size = 0;
				break;
			}
			audio_pkt_data += len1;
			audio_pkt_size -= len1;
			data_size = 0;
			if (got_frame) {
				data_size = av_samples_get_buffer_size(NULL,
					aCodecCtx->channels,
					frame.nb_samples,
					aCodecCtx->sample_fmt,
					1);
				assert(data_size <= buf_size);
				memcpy(audio_buf, frame.data[0], data_size);
			}

			//转换后就放不出声音来了？？？？
			if (0) {
				if (frame.channels > 0 && frame.channel_layout == 0)
					frame.channel_layout = av_get_default_channel_layout(frame.channels);
				else if (frame.channels == 0 && frame.channel_layout > 0)
					frame.channels = av_get_channel_layout_nb_channels(frame.channel_layout);

				if (swr_ctx)
				{
					swr_free(&swr_ctx);
					swr_ctx = nullptr;
				}

				swr_ctx = swr_alloc_set_opts(nullptr, wanted_frame.channel_layout,
					(AVSampleFormat)wanted_frame.format,
					wanted_frame.sample_rate,
					frame.channel_layout, 
					(AVSampleFormat)frame.format, 
					frame.sample_rate, 0, nullptr);

				if (!swr_ctx || swr_init(swr_ctx) < 0)
				{
					cout << "swr_init failed:" << endl;
					break;
				}

				unsigned char *tempbuf;// = new unsigned char[8192];
				int dst_nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, frame.sample_rate) + frame.nb_samples,
					wanted_frame.sample_rate, wanted_frame.format, AVRounding(1));

				int len2 = swr_convert(swr_ctx, &audio_buf, dst_nb_samples, //audio_buf tempbuf
					(const uint8_t**)frame.data, frame.nb_samples);

				if (len2 < 0)
				{
					cout << "swr_convert failed\n";
					break;
				}
				int len= wanted_frame.channels * len2 * av_get_bytes_per_sample((AVSampleFormat)wanted_frame.format);
				return len;
			}

			if (data_size <= 0) {
				/* No data yet, get more frames */
				continue;
			}
			/* We have data, return it and come back for more later */
			return data_size;
		}
		
		//if (pkt.data)	av_free_packet(&pkt);

		if (quit) {
			return -1;
		}

		if (packet_queue_get(&audioq, &pkt, 1) < 0) {
			return -1;
		}
		audio_pkt_data = pkt.data;
		audio_pkt_size = pkt.size;
	}
}

void audio_callback(void *userdata, Uint8 *stream, int len) {

	AVCodecContext *aCodecCtx = (AVCodecContext *)userdata;
	int len1, audio_size;

	static uint8_t audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
	static unsigned int audio_buf_size = 0;
	static unsigned int audio_buf_index = 0;

	while (len > 0) {
		if (audio_buf_index >= audio_buf_size) {
			/* We have already sent all our data; get more */
			audio_size = audio_decode_frame(aCodecCtx, audio_buf, sizeof(audio_buf));
			if (audio_size < 0) {
				/* If error, output silence */
				audio_buf_size = 1024; // arbitrary?
				memset(audio_buf, 0, audio_buf_size);
			}
			else {
				audio_buf_size = audio_size;
			}
			audio_buf_index = 0;
		}
		len1 = audio_buf_size - audio_buf_index;
		if (len1 > len)
			len1 = len;

		SDL_MixAudio(stream, (uint8_t *)audio_buf + audio_buf_index, len1,68);
		memcpy(stream, (uint8_t *)audio_buf + audio_buf_index, len1);
		
		len -= len1;
		stream += len1;
		audio_buf_index += len1;
	}
}
