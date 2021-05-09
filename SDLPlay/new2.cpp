// sdlplay.cpp : 定义控制台应用程序的入口点。
//
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libavutil/avutil.h"
#include "libavutil/common.h"

}

#include "SDL.h"
//static int audio_open(void *opaque, int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate, struct AudioParams *audio_hw_params);

//#define YUV_FILE_OUTPUT

#define THREADEXIT_EVENT  (SDL_USEREVENT + 1)
/*
#include <wx/datetime.h>
#define mLogDebug( Fmt__ )    \
do{                                   \
	wxDateTime dt_t__ = wxDateTime::UNow();\
	wxString str__ = wxString::Format( "%s.%03d\t", dt_t__.FormatTime(), dt_t__.GetMillisecond() ).c_str(); \
	wxString strfmt__ = wxString::Format Fmt__ ; \
	str__ = str__ + strfmt__ +" \n";	\
	OutputDebugStringA( str__.c_str()  );	\
	logger_.appand( str__.c_str().AsChar() );\
}while(0)
*/

#include <Windows.h>

/*
#define mLogDebug( Fmt__ ) do{ \
	char buff__[1024]  ;\
	SYSTEMTIME st__; \
	GetLocalTime(&st__); \
	sprintf(buff__, Fmt__ );\
	OutputDebugStringA(buff__); \
}  while (0)
*/

void __fastcall mLogDebug(char const* const _Format,...) {
	int _Result;
	va_list _ArgList;
	__crt_va_start(_ArgList, _Format);

	char buff__[1024];
	SYSTEMTIME st__;
	GetLocalTime(&st__);
	char timebuf[1024] = { 0 };

	//int tid = ::GetCurrentThreadId();
	//sprintf(timebuf, "%02d.%03d %8X ",st__.wSecond, st__.wMilliseconds, tid);

	sprintf(timebuf, "%02d.%03d ", st__.wSecond, st__.wMilliseconds );

#pragma warning(push)
#pragma warning(disable: 4996) // Deprecation
	_Result = _vsprintf_l(buff__, _Format, NULL, _ArgList);
#pragma warning(pop)

	strcat(timebuf, buff__);

	OutputDebugStringA(timebuf);

	__crt_va_end(_ArgList);

}
// st.wMilliseconds  \

int thread_exit = 0;
int window_w;
int window_h;
 
SDL_Window *sdl_window;
SDL_Renderer *sdl_renderer;
SDL_Texture *sdl_texture;
 
SDL_AudioDeviceID m_AudioDevice;
SDL_AudioSpec wanted_spec, have;
 
AVStream *audio_st;
//Uint32  audio_len;
//Uint8  *audio_pos;
#include <string>

std::string audio_buff;

#define audio_len audio_buff.length()
static int count = 0;
void  fill_audio(void *udata, Uint8 *stream, Uint32 len) {
	//SDL 2.0
	
	SDL_memset(stream, 0, len);

	int buflen = audio_buff.length();
	mLogDebug("need len:%5d bufflen:%5d \n", len, buflen );

	if (buflen < len)
		return;
 
	char buff[64] = { 0 };

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
 
int decode_play(void *data)
{
	char *filepath = (char*)data;
	AVFormatContext	*pFormatCtx;
	unsigned int	i;
	int videoindex, audioindex;
	AVCodec			*pVideoCodec, *pAudioCodec;
	AVFrame	*pFrame, *pFrameYUV;
	AVPacket *packet;
 
	SDL_Event event;
 
	int ret, got_picture;
 
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
	audio_out_buffer = (uint8_t *)av_malloc(out_buffer_size+100);
	
	//SDL_AudioSpec
	wanted_spec.freq = out_sample_rate;
	wanted_spec.format =  AUDIO_S16SYS;AUDIO_S8;
	wanted_spec.channels = out_channels;
	wanted_spec.silence = 0;
	wanted_spec.samples =  out_nb_samples;528; out_nb_samples; 
	wanted_spec.callback = (SDL_AudioCallback)fill_audio;
	wanted_spec.userdata = audio_codecctx;
	//wanted_spec.size = 4224;
 
	//audio_open(0, 0, 0, 0, 0);

	//SDL_AudioSpec have;
	m_AudioDevice =  SDL_OpenAudio(&wanted_spec, NULL); &have;

	//m_AudioDevice = // = SDL_OpenAudio(&wanted_spec, NULL); &have;
	//	SDL_OpenAudioDevice(NULL,0, &wanted_spec,&have,1);

	if (m_AudioDevice < 0) {
		printf("can't open audio.\n");
		return -1;
	}
	SDL_PauseAudio(0);
 
	audio_convert_ctx = swr_alloc();
	audio_convert_ctx = swr_alloc_set_opts(audio_convert_ctx, out_channel_layout, 
		out_sample_fmt, out_sample_rate,in_channel_layout, 
		audio_codecctx->sample_fmt, audio_codecctx->sample_rate, 0, NULL);
 
	swr_init(audio_convert_ctx);
 
	pFrame = av_frame_alloc();
	packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	pFrameYUV = av_frame_alloc();
 
	video_out_buffer = (uint8_t *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, video_codecctx->width, video_codecctx->height, 1));
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, video_out_buffer, AV_PIX_FMT_YUV420P, video_codecpar->width, video_codecpar->height, 1);
 
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
						fwrite(pFrame->data[0], 1, y_size,  fpout);
						fwrite(pFrame->data[1], 1, y_size / 4, fpout);
						fwrite(pFrame->data[2], 1, y_size / 4, fpout);	
#endif
 
						SDL_UpdateTexture(sdl_texture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0]);
 
						int x, y;
						SDL_GetWindowSize(sdl_window, &x, &y);
						sdlRect.x = 0;
						sdlRect.y = 0;
						sdlRect.w = x ;
						sdlRect.h = y ;
	
						SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, &sdlRect);
 
						SDL_RenderPresent(sdl_renderer);
 
	//					Uint32 tick4 = SDL_GetTicks();
	//					printf("present over time [%d] ms\n", tick4 - tick3);
						
					}
			}
#endif
		}
		else if (packet->stream_index == audioindex)
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
							pFrame->nb_samples, (const uint8_t **)pFrame->data, pFrame->nb_samples  ); // 转换音频 pFrame->nb_samples

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
 
#undef main
int main(int argc, char *argv[])
{
	int i = 3;

	int quit = 0;
	char file[1024] = "";
	if (argc == 1) {
		switch (i) {
		case 0:
			sprintf_s(file, 1024, "%s", "http://192.168.7.1:4000/rtp/239.93.0.184:5140");
			break;
		case 1:
			//CCTV-1
			sprintf_s(file, 1024, "%s", "rtsp://182.139.226.78/PLTV/88888893/224/3221226889/10000100000000060000000000622347_0.smil");
			break;
		case 2:
			break;
		case 3:
			//CCTV-3
			sprintf_s(file, 1024, "%s", "rtsp://182.139.226.78/PLTV/88888893/224/3221226816/10000100000000060000000001366243_0.smil");
			break;
		case 4:
			sprintf_s(file, 1024, "%s", "http://192.168.7.237:4000/rtp/239.93.0.184:5140");
			break;
		case 5:
			sprintf_s(file, 1024, "%s", "http://192.168.7.237:4000/rtp/239.93.1.12:2224");
			break;
		case 6:
			sprintf_s(file, 1024, "%s", "F:\\Films\\大决战II.-.淮海战役\\[大决战II.-.淮海战役].The.Great.Decisive.War.II.-.The.Huaihai.Military.Campaign.1991.DVDrip.XviD-ViTAMiNC.CD1.avi");
			break;

		}

	}
//		sprintf_s(file, 1024, "%s", "D:\\123.mp4");
	else
		sprintf_s(file, 1024, "%s", argv[1]);
 
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0)
	{
		return 1;
	}
	
	window_w = 1024;
	window_h = 576;
	sdl_window = SDL_CreateWindow("SDL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_w, window_h, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
 
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
 
	SDL_Thread *play_thread = SDL_CreateThread(decode_play, NULL, file);
	SDL_Event event;
	
	while (quit != 1)
	{
		while (SDL_WaitEvent(&event))
		{
		
			if (event.type == SDL_WINDOWEVENT)
			{
			}
			else if (event.type == SDL_MOUSEMOTION)
			{
//				printf("Receive MOUSEMOTION event, x=[%d]-y=[%d]-xrel=[%d]-yrel=[%d]\n", event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN)
			{
			}
			else if (event.type == SDL_KEYDOWN)
			{
				if ((SDL_GetModState() & KMOD_SHIFT) && event.key.keysym.scancode == SDL_SCANCODE_F12) {
					quit = 1;
				}
 
//				printf("Receive KEYDOWN event keycode=[%d]-mod=[%x]--key=[%s]\n", event.key.keysym.scancode, event.key.keysym.mod, SDL_GetScancodeName(event.key.keysym.scancode));
 
			}
			else if (event.type == SDL_KEYUP)
			{
	//			printf("Receive KEYUP event keycode=[%d]-mod=[%x]--key=[%s]\n", event.key.keysym.scancode, event.key.keysym.mod, SDL_GetScancodeName(event.key.keysym.scancode));
 
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
//				printf("event.type=[%d]\n", event.type);
 
			}
		}
 
	}
	SDL_Quit();
 
    return 0;
}

static SDL_AudioDeviceID audio_dev;
#define SDL_AUDIO_MAX_CALLBACKS_PER_SEC 30
#define SDL_AUDIO_MIN_BUFFER_SIZE 512

static int audio_open(void *opaque, int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate, struct AudioParams *audio_hw_params)
{
	SDL_AudioSpec wanted_spec, spec;
	const char *env;
	static const int next_nb_channels[] = { 0, 0, 1, 6, 2, 6, 4, 6 };
	static const int next_sample_rates[] = { 0, 44100, 48000, 96000, 192000 };
	int next_sample_rate_idx = FF_ARRAY_ELEMS(next_sample_rates) - 1;

	env = SDL_getenv("SDL_AUDIO_CHANNELS");
	if (env) {
		wanted_nb_channels = atoi(env);
		wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels);
	}
	if (!wanted_channel_layout || wanted_nb_channels != av_get_channel_layout_nb_channels(wanted_channel_layout)) {
		wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels);
		wanted_channel_layout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
	}
	wanted_nb_channels = av_get_channel_layout_nb_channels(wanted_channel_layout);
	wanted_spec.channels = wanted_nb_channels;
	wanted_spec.freq = wanted_sample_rate;
	if (wanted_spec.freq <= 0 || wanted_spec.channels <= 0) {
		av_log(NULL, AV_LOG_ERROR, "Invalid sample rate or channel count!\n");
		return -1;
	}
	while (next_sample_rate_idx && next_sample_rates[next_sample_rate_idx] >= wanted_spec.freq)
		next_sample_rate_idx--;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.silence = 0;
	wanted_spec.samples = FFMAX(SDL_AUDIO_MIN_BUFFER_SIZE, 2 << av_log2(wanted_spec.freq / SDL_AUDIO_MAX_CALLBACKS_PER_SEC));
	//wanted_spec.callback = sdl_audio_callback;
	wanted_spec.userdata = opaque;
	while (!(audio_dev = SDL_OpenAudioDevice(NULL, 0, &wanted_spec, &spec,
		SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE))) {
		av_log(NULL, AV_LOG_WARNING, "SDL_OpenAudio (%d channels, %d Hz): %s\n",
			wanted_spec.channels, wanted_spec.freq, SDL_GetError());
		wanted_spec.channels = next_nb_channels[FFMIN(7, wanted_spec.channels)];
		if (!wanted_spec.channels) {
			wanted_spec.freq = next_sample_rates[next_sample_rate_idx--];
			wanted_spec.channels = wanted_nb_channels;
			if (!wanted_spec.freq) {
				av_log(NULL, AV_LOG_ERROR,
					"No more combinations to try, audio open failed\n");
				return -1;
			}
		}
		wanted_channel_layout = av_get_default_channel_layout(wanted_spec.channels);
	}
	if (spec.format != AUDIO_S16SYS) {
		av_log(NULL, AV_LOG_ERROR,
			"SDL advised audio format %d is not supported!\n", spec.format);
		return -1;
	}
	if (spec.channels != wanted_spec.channels) {
		wanted_channel_layout = av_get_default_channel_layout(spec.channels);
		if (!wanted_channel_layout) {
			av_log(NULL, AV_LOG_ERROR,
				"SDL advised channel count %d is not supported!\n", spec.channels);
			return -1;
		}
	}

	/*
	audio_hw_params->fmt = AV_SAMPLE_FMT_S16;
	audio_hw_params->freq = spec.freq;
	audio_hw_params->channel_layout = wanted_channel_layout;
	audio_hw_params->channels = spec.channels;
	audio_hw_params->frame_size = av_samples_get_buffer_size(NULL, audio_hw_params->channels, 1, audio_hw_params->fmt, 1);
	audio_hw_params->bytes_per_sec = av_samples_get_buffer_size(NULL, audio_hw_params->channels, audio_hw_params->freq, audio_hw_params->fmt, 1);
	if (audio_hw_params->bytes_per_sec <= 0 || audio_hw_params->frame_size <= 0) {
	av_log(NULL, AV_LOG_ERROR, "av_samples_get_buffer_size failed\n");
	return -1;
	}

	*/
	return spec.size;
}

