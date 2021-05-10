/**
 * 李震
 * 我的码云：https://git.oschina.net/git-lizhen
 * 我的CSDN博客：http://blog.csdn.net/weixin_38215395
 * 联系：QQ1039953685
 */


#include <wx/mstream.h>
#include <wx/rawbmp.h>
 
#define SDL_AUDIO_BUFFER_SIZE 6024 
#define THREADEXIT_EVENT  (SDL_USEREVENT + 1)

#include "../AudioPlay.h"

extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libavutil/pixfmt.h"
    #include "libswscale/swscale.h"
	#include "libswresample/swresample.h"
    #include "libavutil/time.h"
    #include "libavutil/mathematics.h"
#include "libavutil/imgutils.h"
//#include <SDL_mixer.h>

}
#include <SDL.h>

#include <stdio.h>
#include<iostream>

#include "videoplayer.h"
#include "../RTSPMainWnd.h"

SDL_AudioDeviceID m_AudioDevice;
SDL_AudioSpec wanted_spec, have;

int thread_exit = 0;
int window_w;
int window_h;

SDL_Window *sdl_window;
SDL_Renderer *sdl_renderer;
SDL_Texture *sdl_texture;

AVStream *audio_st;
//char *filepath = 0;// (char*)rtspurl.c_str().AsChar();

#ifdef _DEBUG
void __fastcall mLogDebug(char const* const _Format, ...) {
	int _Result;
	va_list _ArgList;
	__crt_va_start(_ArgList, _Format);

	char buff__[1024];
	SYSTEMTIME st__;
	GetLocalTime(&st__);
	char timebuf[1024] = { 0 };

	//int tid = ::GetCurrentThreadId();
	//sprintf(timebuf, "%02d.%03d %8X ",st__.wSecond, st__.wMilliseconds, tid);

	sprintf(timebuf, "%02d.%03d ", st__.wSecond, st__.wMilliseconds);

#pragma warning(push)
#pragma warning(disable: 4996) // Deprecation
	_Result = _vsprintf_l(buff__, _Format, NULL, _ArgList);
#pragma warning(pop)

	strcat(timebuf, buff__);

	OutputDebugStringA(timebuf);

	__crt_va_end(_ArgList);

}
#else
#define mLogDebug( Fmt__ ) 
#endif

using namespace std;
#include <string>
std::string audio_buff;
#define audio_len audio_buff.length()

void print_time() {
	if (0) {
		SYSTEMTIME systime;
		::GetLocalTime(&systime);

		char buf[54];
		sprintf(buf, "\t%02d:%02d:%02d.%03d\t", systime.wHour, systime.wMinute, systime.wSecond,systime.wMilliseconds);
		//printf(buf);
		OutputDebugStringA(buf);
	}
	if (0) {
		LARGE_INTEGER litmp;
		LONGLONG QPart1, QPart2;
		double dfMinus, dfFreq, dfSpec;
		QueryPerformanceFrequency(&litmp);
		dfFreq = (double)litmp.QuadPart;
		QueryPerformanceCounter(&litmp);
		QPart1 = litmp.QuadPart;
		dfSpec = QPart1 / dfFreq;

		char buf[54];
		sprintf(buf, "\t%f\t", dfSpec);
		//printf(buf);
		OutputDebugStringA(buf);
	}
}

int NSSleep(int ms)
{
	HANDLE hTimer = NULL;
	LARGE_INTEGER liDueTime;
	//print_time();
	liDueTime.QuadPart = -ms;// -ms * 10000;

	//  用负值表示一个相对的时间，代表以100纳秒为单位的相对时间，（如从现在起的5秒钟，则设置为 - 5000 0000）
	//	LONG lPeriod：设置定时器周期性的自我激发，该参数的单位为毫秒。如果为0，则表示定时器只发
	//	出一次信号，大于0时，定时器没隔一段时间自动重新激活一个计时器，直至取消计时器使用

	// Create a waitable timer.
	hTimer = CreateWaitableTimerA(NULL, TRUE, "WaitableTimer");
	if (!hTimer)
	{
		printf("CreateWaitableTimer failed (%d)\n", GetLastError());
		return 1;
	}

	// Set a timer to wait for 10 seconds.
	if (!SetWaitableTimer(
		hTimer, &liDueTime, 0, NULL, NULL, 0))
	{
		printf("SetWaitableTimer failed (%d)\n", GetLastError());
		return 2;
	}

	// Wait for the timer.
	if (WaitForSingleObject(hTimer, INFINITE) != WAIT_OBJECT_0)
		printf("WaitForSingleObject failed (%d)\n", GetLastError());
	//print_time();
	return 0;
}

VideoPlayer::~VideoPlayer()
{

}

void * VideoPlayer::Entry(void)
{
    ///调用 QThread 的start函数 将会自动执行下面的run函数 run函数是一个新的线程
    //this->start();
	this->run();

	return NULL;
}

void  fill_audio(void *udata, Uint8 *stream, Uint32 len) {
	//SDL 2.0

	//SDL_memset(stream, 0, len);

	int buflen = audio_buff.length();
	mLogDebug("need len:%5d bufflen:%5d \n", len, buflen);

	if (buflen < len)	return;

	len = (len>buflen ? buflen : len);    //  Mix  as  much  data  as  possible  

	memset(stream, 0, len);
	SDL_MixAudio(stream, (uint8_t*)&audio_buff[0], len, SDL_MIX_MAXVOLUME);

	audio_buff.erase(0, len);

	//audio_buff.clear();
	//buflen = audio_buff.length();
	//audio_pos += len;
	//audio_len -= len;

	//SYSTEMTIME st;
	//GetLocalTime(&st);
	//sprintf(buff, "len:%5d bufflen:%5d un:%5d,     ms: %03d \n", len, buflen, audio_buff.length(), st.wMilliseconds);
	//OutputDebugStringA(buff);

	//mLogDebug("len:%5d bufflen:%5d un:%5d \n", len, buflen, audio_buff.length());
	//mLogDebug( "len:%5d bufflen:%5d un:%5d \n", len, buflen, audio_buff.length() );
}

int decode_play(void* data) {
	AVFormatContext	*pFormatCtx;
	unsigned int	i;
	int videoindex, audioindex;
	AVCodec			*pVideoCodec, *pAudioCodec;
	AVFrame	*pFrame, *pFrameYUV;
	AVPacket *packet;

	SDL_Event event;

	int ret, got_picture;
	char * filepath = (char*)data;
	//av_register_all();
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();

	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0) {
		printf("Couldn't open input stream.\n");
		return -1;
	}
	if (avformat_find_stream_info(pFormatCtx, NULL)<0) {
		printf("Couldn't find stream information.\n");
		return -1;
	}
	videoindex = -1;
	audioindex = -1;
	videoindex = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	audioindex = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);

	//	video
	AVCodecParameters *video_codecpar = pFormatCtx->streams[videoindex]->codecpar;
	pVideoCodec = avcodec_find_decoder(video_codecpar->codec_id);
	if (pVideoCodec == NULL) {
		printf("Codec not found.\n");
		return -1;
	}

	//	audio
	audio_st = pFormatCtx->streams[audioindex];
	AVCodecParameters *audio_codecpar = pFormatCtx->streams[audioindex]->codecpar;
	pAudioCodec = avcodec_find_decoder(audio_codecpar->codec_id);

	if (pAudioCodec == NULL) {
		printf("audio Codec not found.\n");
		return -1;
	}

	AVCodecContext *video_codecctx = avcodec_alloc_context3(NULL);
	avcodec_parameters_to_context(video_codecctx, video_codecpar);

	AVCodecContext *audio_codecctx = avcodec_alloc_context3(NULL);
	avcodec_parameters_to_context(audio_codecctx, audio_codecpar);

	struct SwsContext *video_convert_ctx = NULL;
	uint8_t	*video_out_buffer = NULL;
	uint8_t	*audio_out_buffer = NULL;

	struct SwrContext *audio_convert_ctx = NULL;

	//nb_samples: AAC-1024 MP3-1152
	int out_nb_samples = audio_codecctx->frame_size;
	uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
	enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
	int out_sample_rate = audio_codecctx->sample_rate;

	uint64_t in_channel_layout = av_get_default_channel_layout(audio_codecctx->channels);
	int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);

	int out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, out_sample_fmt, 1);
	audio_out_buffer = (uint8_t *)av_malloc(out_buffer_size + 100);

	//SDL_AudioSpec
	wanted_spec.freq = out_sample_rate;
	wanted_spec.format = AUDIO_S16SYS; AUDIO_S8;
	wanted_spec.channels = out_channels;
	wanted_spec.silence = 0;
	wanted_spec.samples = out_nb_samples; 528; out_nb_samples;
	wanted_spec.callback = (SDL_AudioCallback)fill_audio;
	wanted_spec.userdata = audio_codecctx;
	//wanted_spec.size = 4224;

	//audio_open(0, 0, 0, 0, 0);

	//SDL_AudioSpec have;
	m_AudioDevice = SDL_OpenAudio(&wanted_spec, NULL); &have;

	//m_AudioDevice = // = SDL_OpenAudio(&wanted_spec, NULL); &have;
	//	SDL_OpenAudioDevice(NULL,0, &wanted_spec,&have,1);

	if (m_AudioDevice < 0) {
		printf("can't open audio.\n");
		return -1;
	}
	SDL_PauseAudio(0);

	audio_convert_ctx = swr_alloc();
	audio_convert_ctx = swr_alloc_set_opts(audio_convert_ctx, out_channel_layout,
		out_sample_fmt, out_sample_rate, in_channel_layout,
		audio_codecctx->sample_fmt, audio_codecctx->sample_rate, 0, NULL);

	swr_init(audio_convert_ctx);

	pFrame = av_frame_alloc();
	packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	pFrameYUV = av_frame_alloc();

	video_out_buffer = (uint8_t *)av_malloc(
		av_image_get_buffer_size(AV_PIX_FMT_YUV420P, video_codecctx->width, video_codecctx->height, 1)
	);

	av_image_fill_arrays(pFrameYUV->data,
		pFrameYUV->linesize, video_out_buffer,
		AV_PIX_FMT_YUV420P, video_codecpar->width, video_codecpar->height, 1);

	video_codecctx->thread_count = 6;
	video_codecctx->thread_type = FF_THREAD_FRAME;
	video_codecctx->delay = 0;

	if (avcodec_open2(video_codecctx, pVideoCodec, NULL)<0) {
		printf("Could not open video codec.\n");
		return -1;
	}

	if (avcodec_open2(audio_codecctx, pAudioCodec, NULL)<0) {
		printf("Could not open audio codec.\n");
		return -1;
	}

	sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, video_codecctx->width, video_codecctx->height);

	av_dump_format(pFormatCtx, 0, filepath, 0);

#ifdef YUV_FILE_OUTPUT
	char yuvfile[1024] = "";
	sprintf_s(yuvfile, 1024, "Hunantv_%d_%d.yuv", video_codecctx->width, video_codecctx->height);
	int y_size;
	FILE *fpout;
	fopen_s(&fpout, yuvfile, "wb+");
#endif
	SDL_Rect sdlRect;

	while (!thread_exit) {
		if (av_read_frame(pFormatCtx, packet) < 0)
		{
			event.type = THREADEXIT_EVENT;
			SDL_PushEvent(&event);
			break;
		}
		if (packet->stream_index == videoindex) {
#if 1

			//		Uint32 tick0 = SDL_GetTicks();
			ret = avcodec_send_packet(video_codecctx, packet);
			if (ret < 0) {
				printf("Decode Error.\n");
				//break;
			}
			//SDL_Delay(0);//延迟播放

			got_picture = avcodec_receive_frame(video_codecctx, pFrame);
			/*
			switch (pFrame->pict_type)
			{
			case AV_PICTURE_TYPE_I:
			printf("I\n");
			break;
			case AV_PICTURE_TYPE_P:
			printf("P\n");
			break;
			case AV_PICTURE_TYPE_B:
			printf("B\n");
			break;
			default:
			printf("No picture type\n");
			break;
			}
			*/

			//		Uint32 tick1 = SDL_GetTicks();
			//		printf("decode time [%d] ms\n", tick1 - tick0);

			if (!got_picture)
			{
				if (pFrameYUV)
				{
					//					Uint32 tick2 = SDL_GetTicks();

					video_convert_ctx = sws_getContext(video_codecctx->width, video_codecctx->height, video_codecctx->pix_fmt, video_codecctx->width, video_codecctx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
					sws_scale(video_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, video_codecctx->height, pFrameYUV->data, pFrameYUV->linesize);
					sws_freeContext(video_convert_ctx);
					//					Uint32 tick3 = SDL_GetTicks();
					//					printf("sws_scale time [%d] ms\n", tick3 - tick2);
#ifdef YUV_FILE_OUTPUT					
					y_size = video_codecctx->width * video_codecctx->height;
					fwrite(pFrame->data[0], 1, y_size, fpout);
					fwrite(pFrame->data[1], 1, y_size / 4, fpout);
					fwrite(pFrame->data[2], 1, y_size / 4, fpout);
#endif

					SDL_UpdateTexture(sdl_texture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0]);

					int x, y;
					SDL_GetWindowSize(sdl_window, &x, &y);
					sdlRect.x = 0;
					sdlRect.y = 0;
					sdlRect.w = x;
					sdlRect.h = y;

					SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, &sdlRect);

					SDL_RenderPresent(sdl_renderer);

					//					Uint32 tick4 = SDL_GetTicks();
					//					printf("present over time [%d] ms\n", tick4 - tick3);

				}
			}
#endif
		}
		else if ( packet->stream_index == audioindex ) //mao
		{
			ret = avcodec_send_packet(audio_codecctx, packet);
			if (ret < 0) {
				printf("Decode Error.\n");
				//break;
			}

			SDL_Delay(0);//延迟播放

			//while (0==(
			got_picture = avcodec_receive_frame(audio_codecctx, pFrame);
			//))
			{
				pFrame->pts;
				pFrame->best_effort_timestamp;
				pFrame->pts;
				pFrame->pkt_dts;

				//SDL_Log("%llu %llu %llu ", pFrame->pts, pFrame->pkt_dts, pFrame->best_effort_timestamp);

				AV_NOPTS_VALUE;
				if (ret >= 0) {
					/*
					AVRational tb = (AVRational) { 1, frame->sample_rate };
					if (pFrame->pts != AV_NOPTS_VALUE)
					pFrame->pts = av_rescale_q(pFrame->pts, d->avctx->pkt_timebase, tb);
					else if (d->next_pts != AV_NOPTS_VALUE)
					pFrame->pts = av_rescale_q(d->next_pts, d->next_pts_tb, tb);
					if (pFrame->pts != AV_NOPTS_VALUE) {
					d->next_pts = pFrame->pts + pFrame->nb_samples;
					d->next_pts_tb = tb;
					}
					*/
				}
				//printf("avcodec_decode_video2 size=[%d]-got_picture=[%d]\n", ret, got_picture);
				if (got_picture == 0)
				{
					try
					{
						uint8_t temp[8192] = { 0 };
						uint8_t* p = temp;
						int val = swr_convert(audio_convert_ctx, (uint8_t **)&p,//&audio_out_buffer,
							pFrame->nb_samples, (const uint8_t **)pFrame->data, pFrame->nb_samples); // 转换音频 pFrame->nb_samples

						audio_codecctx->frame_size;
						pFrame->nb_samples;

						if (val >= 0) {
							//audio_pos = (uint8_t *)audio_out_buffer;
							audio_buff.append((char*)temp, out_buffer_size); out_buffer_size;
							//audio_len = out_buffer_size;
							mLogDebug("%llu  %d   %f \n", pFrame->pts,
								audio_len, pFrame->pts * av_q2d(audio_st->time_base));
						}
						//SDL_PauseAudio(0);
						//Sleep(1);

						while (audio_len > 0)
						{
							SDL_Delay(0);//延迟播放
						}
						//SDL_Delay(10);//延迟播放
					}
					catch (const std::exception& e)
					{
						const char* msg = e.what();
						OutputDebugStringA(e.what());
					}

				}//if
			}//while
		}
	}
#ifdef YUV_FILE_OUTPUT
	fclose(fpout);
#endif

	av_frame_free(&pFrameYUV);
	av_frame_free(&pFrame);
	avcodec_close(video_codecctx);
	avcodec_close(audio_codecctx);
	avformat_close_input(&pFormatCtx);

	return 0;

}

void VideoPlayer::run()
{
	//wxBitmap bitmap(1920, 1080, 24);
	//wxBitmap bitmap(3840, 2160, 24);//4k

	//wxMemoryDC temp_dc;

	//temp_dc.SelectObject(bitmap);
	//wxClientDC dc(this->m_mainWnd_->m_Panel);

	//CCTV1 timestamp
	char url5[] = "rtsp://182.139.226.78/PLTV/88888893/224/3221226889/10000100000000060000000000622347_0.smil?playseek=20210506190000-20210506195000";
	
	//CCTV5
	char url1[] = "rtsp://182.139.226.78/PLTV/88888893/224/3221227219/10000100000000060000000001366244_0.smil";
	
	//熊猫影院高清
	char url2[] = "rtsp://182.139.226.78/PLTV/88888893/224/3221226766/10000100000000060000000001098398_0.smil";
    
	//4K !!!!!!!!!!
	char url3[] = "rtsp://182.139.226.78/PLTV/88888893/224/3221228017/10000100000000060000000003790175_0.smil";
	
	//CCTV 1 !!!!!!!!!!
	char url4[] = "http://192.168.128.10:4000/rtp/239.93.0.214:5140";

	char* url = url4;

	wxString rtsp = m_mainWnd_->m_cmb_RTSP->GetValue();

	wxDateTime dt = m_mainWnd_->m_Date->GetValue();

	wxString datestr = dt.Format("%Y%m%d");
	wxString starttime = m_mainWnd_->m_StartTime->GetValue().Format("%H%M%S");
	wxString endtime = m_mainWnd_->m_EndTime->GetValue().Format("%H%M%S");

	wxString seekstr = wxString::Format("?playseek=%s%s-%s%s",datestr,starttime,datestr,endtime);
	wxString rtspurl = rtsp ;
	int pos = rtsp.Find("?play");
	if (pos != -1) {
		rtsp = rtsp.SubString(0, pos-1);
		rtspurl = rtsp + seekstr;
	}
	else {

	}

	//rtspurl = "http://192.168.128.6:4000/rtp/239.93.0.99:5140";

	//http://192.168.128.6:4000/rtp/239.93.0.184:5140
	//rtsp://182.139.226.78/PLTV/88888893/224/3221227219/10000100000000060000000001366244_0.smil?playseek=2019 08 01 10 00 00-20190801113000

	//rtspurl = "C:\\Qt\\RTSP\\ffmpeg-4.4-full_build-shared\\bin\\ffplay.exe " + rtspurl;
	//wxExecute(rtspurl);
	//if (1) return;
	//filepath = (char*)rtspurl.c_str().AsChar();

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0)
	{
		return  ;
	}

	//window_w = 1024 / 3;
	//window_h = 576 / 3;
	//sdl_window = SDL_CreateWindow("SDL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_w, window_h, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	sdl_window = SDL_CreateWindowFrom(m_mainWnd_->m_Panel->GetHWND());

	int displayindex;
	SDL_DisplayMode mode0;
	displayindex = SDL_GetWindowDisplayIndex(sdl_window);
	SDL_GetCurrentDisplayMode(displayindex, &mode0);

	//SDL_Surface *iconSurface = SDL_LoadBMP(".\\face.bmp");
	//SDL_SetWindowIcon(sdl_window, iconSurface);

	//	0 SDL_VIDEO_RENDER_D3D
	//	1 SDL_VIDEO_RENDER_D3D11
	//	2 SDL_VIDEO_RENDER_OGL
	//	3 SDL_VIDEO_RENDER_OGL_ES2
	sdl_renderer = SDL_CreateRenderer(sdl_window, 0, SDL_RENDERER_PRESENTVSYNC);

	SDL_Thread *play_thread = SDL_CreateThread(decode_play, NULL, (void*)rtspurl.c_str().AsChar());
	SDL_Event event;

	//if (1) return;

	while (quit != 1)
	{
		while (SDL_WaitEvent(&event))
		{
			if (event.type == SDL_WINDOWEVENT)
			{
				mLogDebug("SDL_WINDOWEVENT");
			}
			else if (event.type == SDL_MOUSEMOTION)
			{
				//printf("Receive MOUSEMOTION event, x=[%d]-y=[%d]-xrel=[%d]-yrel=[%d]\n", event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN)
			{
			}
			else if (event.type == SDL_KEYDOWN)
			{
				if ((SDL_GetModState() & KMOD_SHIFT) && event.key.keysym.scancode == SDL_SCANCODE_F12) {
					quit = 1;
				}
				//printf("Receive KEYDOWN event keycode=[%d]-mod=[%x]--key=[%s]\n", event.key.keysym.scancode, event.key.keysym.mod, SDL_GetScancodeName(event.key.keysym.scancode));
			}
			else if (event.type == SDL_KEYUP)
			{
				//printf("Receive KEYUP event keycode=[%d]-mod=[%x]--key=[%s]\n", event.key.keysym.scancode, event.key.keysym.mod, SDL_GetScancodeName(event.key.keysym.scancode));
			}
			else if (event.type = SDL_MOUSEWHEEL)
			{
				printf("Recv MOUSEWHEEL  direction=[%d],x=[%d],y=[%d]", event.wheel.direction, event.wheel.x, event.wheel.y);
			}
			else if (event.type == SDL_QUIT)
			{
				thread_exit = 1;
				quit = 1;
			}
			else if (event.type == THREADEXIT_EVENT)
			{

			}
			else
			{
				//printf("event.type=[%d]\n", event.type);
			}
		}

	}
	
	SDL_Quit();
}
