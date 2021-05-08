// SDLPlay.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

//���ߣ��ֶܲ�����
//���ӣ�https ://www.zhihu.com/question/51040587/answer/123823358
//��Դ��֪��
//����Ȩ���������С���ҵת������ϵ���߻����Ȩ������ҵת����ע��������

//
//FFMPEG+SDL��Ƶ�������
//������
//�й���ý��ѧ/���ֵ��Ӽ���
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

//ȫ�ֱ���---------------------
 Uint8  *audio_chunk;
 Uint32  audio_len=0;
 Uint8  *audio_pos;

int iCount = 0;

//-----------------
/*  The audio function callback takes the following parameters:
stream: A pointer to the audio buffer to be filled
len: The length (in bytes) of the audio buffer (���ǹ̶���4096��)
�ص�����
ע�⣺mp3Ϊʲô���Ų�˳����
len=4096;audio_len=4608;�������512��Ϊ����512�������ٵ���һ�λص�����������
m4a,aac�Ͳ����ڴ�����(����4096)��
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
	SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
	audio_pos += len;
	audio_len -= len;
	printf("  audio_len:%8d len:%8d, %d\n",   audio_len,len, iCount++);
	//::Sleep(10);
}
//-----------------


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

	//֧������������
	avformat_network_init();
	//��ʼ��
	pFormatCtx = avformat_alloc_context();
	//�в���avdic
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
		//ԭΪcodec_type==CODEC_TYPE_AUDIO
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
	fseek(pFile, 44, SEEK_SET); //Ԥ���ļ�ͷ��λ��
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
	//�ѽṹ���Ϊָ��
	AVPacket *packet = (AVPacket *)malloc(sizeof(AVPacket));
	av_init_packet(packet);

	//��Ƶ����Ƶ�������ͳһ��
	//�¼�
	AVFrame	*pFrame;
	pFrame = av_frame_alloc();

	//---------SDL--------------------------------------
	//��ʼ��
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		printf("Could not initialize SDL - %s\n", SDL_GetError());
		exit(1);
	}

	//�ṹ�壬����PCM���ݵ������Ϣ
	SDL_AudioSpec wanted_spec;
	wanted_spec.freq =  pCodecCtx->sample_rate;
	//wanted_spec.freq = 48000;
	wanted_spec.format = AUDIO_S16SYS; AUDIO_F32SYS; AUDIO_U16SYS; AUDIO_S32SYS; AUDIO_S16SYS;
	wanted_spec.channels =  pCodecCtx->channels;
	wanted_spec.silence = 0;
	wanted_spec.samples = 1024; //����AAC��M4a���������Ĵ�С 
								//wanted_spec.samples = 1152; //����MP3��WMAʱ����
	wanted_spec.callback = fill_audio;
	wanted_spec.userdata = pCodecCtx;

	if (SDL_OpenAudio(&wanted_spec, NULL)<0)//���裨2������Ƶ�豸
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
	//�°治����Ҫ
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
			//ԭΪavcodec_decode_audio2
			//ret = avcodec_decode_audio4( pCodecCtx, decompressed_audio_buf,
			//&decompressed_audio_buf_size, packet.data, packet.size );
			//��Ϊ
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
				//ֱ��д��
				//ע�⣺������data��0����������linesize��0��
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
			//������Ƶ���ݻ���,PCM����
			audio_chunk = (Uint8*)pFrame->data[0];
			//������Ƶ���ݳ���
			audio_len = pFrame->linesize[0];
			//audio_len = 4096;
			//����mp3��ʱ���Ϊaudio_len = 4096
			//���Ƚ���������������������MP3һ֡����4608
			//ʹ��һ�λص�������4096�ֽڻ��壩���Ų��꣬���Ի�Ҫʹ��һ�λص����������²��Ż���������
			//���ó�ʼ����λ��
			audio_pos = audio_chunk;
			//�ط���Ƶ����
			SDL_PauseAudio(0);
			//printf("don't close, audio playing...\n");
			while (audio_len>0)//�ȴ�ֱ����Ƶ���ݲ������!
				SDL_Delay(0);
			//---------------------------------------
#endif
		}
		// Free the packet that was allocated by av_read_frame
		//�Ѹ�
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
	SDL_CloseAudio();//�ر���Ƶ�豸
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

	//char * filename = "F:\\Music\\������Ҳ��ȯ - �����.mp3";// argv[1];
	//char * filename = "F:\\Music\\����MV����ء��¾�����ɭ ��Զ�ĺ�ħ��˧��.flv";
	char * filename = "F:\\Music\\let it go.mp3";
	//scanf("%s", filename);
	if (decode_audio(filename) == 0)
		printf("Decode audio successfully.\n");

	return 0;
}
