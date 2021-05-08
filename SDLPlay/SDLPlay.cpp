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
	char * filename = "F:\\Music\\let it go.mp3";
	//scanf("%s", filename);
	if (decode_audio(filename) == 0)
		printf("Decode audio successfully.\n");

	return 0;
}
