/**
 * 李震
 * 我的码云：https://git.oschina.net/git-lizhen
 * 我的CSDN博客：http://blog.csdn.net/weixin_38215395
 * 联系：QQ1039953685
 */

#include "videoplayer.h"
#include "../RTSPMainWnd.h"

#include <wx/mstream.h>
#include <wx/rawbmp.h>
 
#define SDL_AUDIO_BUFFER_SIZE 1024 

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

#include <SDL.h>
//#include <SDL_mixer.h>

}

#include <stdio.h>
#include<iostream>

using namespace std;

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


void VideoPlayer::run()
{
	wxBitmap bitmap(1920, 1080, 24);
	//wxBitmap bitmap(3840, 2160, 24);//4k

	wxMemoryDC temp_dc;

	temp_dc.SelectObject(bitmap);
	wxClientDC dc(this->m_mainWnd_->m_Panel);

    //char *file_path = mFileName.toUtf8().data();
    //cout<<file_path<<endl;
    AVFormatContext *pFormatCtx;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    AVFrame *pFrame, *pFrameRGB;
    AVPacket *packet;
    uint8_t *out_buffer;

	AVCodecContext* pCodecCtxOrgA = nullptr;
	AVCodecContext* pCodecCtxA = nullptr;

	AVCodec* pCodecA = nullptr;
	static uint8_t audio_buff[(192000 * 3) / 2];

    static struct SwsContext *img_convert_ctx;

	AVRational time_base_q = { 1,AV_TIME_BASE };

    int videoStream=-1, audioStream=-1,
		i, numBytes;
    int ret, got_picture;

    avformat_network_init();   //初始化FFmpeg网络模块，2017.8.5---lizhen
    av_register_all();         //初始化FFMPEG  调用了这个才能正常适用编码器和解码器

    //Allocate an AVFormatContext.
    pFormatCtx = avformat_alloc_context();

    //2017.8.5---lizhen
    AVDictionary *avdic=NULL;
    char option_key[]="rtsp_transport";
    char option_value[]="tcp";
    av_dict_set(&avdic,option_key,option_value,0);

    char option_key2[]="max_delay";
    char option_value2[]="100";
    av_dict_set(&avdic,option_key2,option_value2,0);

	av_dict_set(&avdic, "buffer_size", "1124000", 0);

	//CCTV1 timestamp
	char url5[] = "rtsp://182.139.226.78/PLTV/88888893/224/3221226889/10000100000000060000000000622347_0.smil?playseek=20210506190000-20210506195000";
	
	//CCTV5
	char url1[] = "rtsp://182.139.226.78/PLTV/88888893/224/3221227219/10000100000000060000000001366244_0.smil";
	
	//熊猫影院高清
	char url2[] = "rtsp://182.139.226.78/PLTV/88888893/224/3221226766/10000100000000060000000001098398_0.smil";
    
	//4K !!!!!!!!!!
	char url3[] = "rtsp://182.139.226.78/PLTV/88888893/224/3221228017/10000100000000060000000003790175_0.smil";
	
	//CCTV 1 !!!!!!!!!!
	char url4[] = "http://192.168.9.1:4000/rtp/239.93.0.214:5140";

	char* url = url5;

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

	rtspurl = "http://192.168.128.6:4000/rtp/239.93.0.184:5140";

	//http://192.168.128.6:4000/rtp/239.93.0.184:5140
	//rtsp://182.139.226.78/PLTV/88888893/224/3221227219/10000100000000060000000001366244_0.smil?playseek=2019 08 01 10 00 00-20190801113000

    if (avformat_open_input(&pFormatCtx, rtspurl.c_str(), NULL, &avdic) != 0) {
        printf("can't open the file. \n");
        return;
    }

    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        printf("Could't find stream infomation.\n");
        return;
    }

    videoStream = -1;

    ///循环查找视频中包含的流信息，直到找到视频类型的流
    ///便将其记录下来 保存到videoStream变量中

    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
        }
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audioStream = i;
		}
	}

    ///如果videoStream为-1 说明没有找到视频流
    if (videoStream == -1) {
        printf("Didn't find a video stream.\n");
        return;
    }

	AVStream *stream = pFormatCtx->streams[videoStream];

    ///查找解码器
    pCodecCtx = pFormatCtx->streams[videoStream]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

	pCodecCtxOrgA = pFormatCtx->streams[audioStream]->codec;
	pCodecA = avcodec_find_decoder(pCodecCtxOrgA->codec_id);

	// 不直接使用从AVFormatContext得到的CodecContext，要复制一个
	pCodecCtxA = avcodec_alloc_context3(pCodecA);

	if (avcodec_copy_context(pCodecCtxA, pCodecCtxOrgA) != 0)
	{
		//cout << "Could not copy codec context!" << endl;
		//return  ;
	}

	//2017.8.9---lizhen
    //pCodecCtx->bit_rate =0;   //初始化为0
    //pCodecCtx->time_base.num=1;  //下面两行：一秒钟25帧
    //pCodecCtx->time_base.den=10;
    //pCodecCtx->frame_number=1;  //每包一个视频帧

    if (pCodec == NULL) {
        printf("Codec not found.\n");
        return;
    }

    ///打开解码器
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        printf("Could not open codec.\n");
        return;
    }
	if (avcodec_open2(pCodecCtxA, pCodecA, nullptr)<0) {
		printf("Could not open codec.\n");
		return;
	}

    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();

    //2017.8.7---lizhen
    //cout<<pCodecCtx->width<<endl;

    ///这里我们改成了 将解码后的YUV数据转换成RGB32
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
            pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
            PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);

    numBytes = avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width,pCodecCtx->height);

    out_buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    avpicture_fill((AVPicture *) pFrameRGB, out_buffer, PIX_FMT_RGB24,
            pCodecCtx->width, pCodecCtx->height);

    int y_size = pCodecCtx->width * pCodecCtx->height;

    packet = (AVPacket *) malloc(sizeof(AVPacket)); //分配一个packet
    av_new_packet(packet, y_size); //分配packet的数据

    //2017.8.1---lizhen
    av_dump_format(pFormatCtx, 0, url, 0); //输出视频信息

	char buf[128] = { 0 };

	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		sprintf(buf,"Could not initialize SDL - %s\n", SDL_GetError());
		OutputDebugStringA(buf);
	}

	int count = SDL_GetNumAudioDevices(0);
	for (int i = 0; i < count; i++)
	{
		sprintf(buf, "Audio device %d : %s", i, SDL_GetAudioDeviceName(i, 0));
		//wxDO_LOGV((0, "Audio device %d : %s", i, SDL_GetAudioDeviceName(i, 0)));
		//wxLogDebug("Audio device %d : %s", i, SDL_GetAudioDeviceName(i, 0));
		OutputDebugStringA(buf);
	}

	// Set audio settings from codec info
	SDL_AudioSpec wanted_spec, spec;
	wanted_spec.freq =  pCodecCtxA->sample_rate; //44100;//
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.channels = 1; pCodecCtxA->channels;
	wanted_spec.silence = 0;
	wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
	wanted_spec.callback = audio_callback;
	wanted_spec.userdata = pCodecCtxA; //pCodecCtxOrgA pCodecCtxA

	if (SDL_OpenAudio(&wanted_spec, &spec) < 0)
	{
		cout << "Open audio failed:" << SDL_GetError() << endl;
		//getchar();
		return;
	}

	wanted_frame.format = AV_SAMPLE_FMT_S16;
	wanted_frame.sample_rate = spec.freq;
	wanted_frame.channel_layout = av_get_default_channel_layout(spec.channels);
	wanted_frame.channels = spec.channels;

	//Mix_Music * sound = Mix_LoadMUS("sky.wav");
	//Mix_PlayMusic
	packet_queue_init(&audioq);
	SDL_PauseAudio(0);


    while (!this->TestDestroy())
    {
        if (av_read_frame(pFormatCtx, packet) < 0)
        {
            break; //这里认为视频读取完了
        }

        if (packet->stream_index == videoStream) {
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture,packet);

			if (pFrame->pts == AV_NOPTS_VALUE) {
				//pFrame->pts = 0;
			}
			//int second = pFrame->pts*av_q2d(stream->time_base);
			//OutputDebugStringA(wxString::Format("\t%d\t", second).c_str());
            if (ret < 0) {
                printf("decode error.\n");
                return;
            }
			
            if (got_picture) {
                sws_scale(img_convert_ctx,
                        (uint8_t const * const *) pFrame->data,
                        pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data,
                        pFrameRGB->linesize);

                //把这个RGB数据 用QImage加载
				//if (1) {
				//QImage tmpImg((uchar *)out_buffer,pCodecCtx->width,pCodecCtx->height,QImage::Format_RGB32);
				IMG img{ pCodecCtx->width, pCodecCtx->height, numBytes, (char *)out_buffer };

				if (0) {
					wxMemoryOutputStream outs;
					wxMemoryInputStream ins((char *)out_buffer, numBytes);
					wxImage ximg;
					class M :public wxBMPHandler {
					public:
						bool DoLoadDib(wxImage *image, int width, int height, int bpp, int ncolors,
							int comp, wxFileOffset bmpOffset, wxInputStream& stream,
							bool verbose, bool IsBmp, bool hasPalette, int colEntrySize = 4) {
							return wxBMPHandler::DoLoadDib(image, width, height, bpp, ncolors,
								comp, bmpOffset, stream,
								verbose, IsBmp, hasPalette, colEntrySize);
						}
					};

					M bmph;
					bool result = bmph.DoLoadDib(&ximg, img.w, img.h, 24, 0, 0, 0, ins, 0, 1, 0);
					wxBitmap xbmp(ximg);
					temp_dc.SelectObject(xbmp);
				}
				//}

				if (0) {
					wxCommandEvent event(wxEVT_NULL, 10086);
					event.SetEventObject(this->m_mainWnd_);
					event.SetExtraLong((long)&img);
					m_mainWnd_->ProcessWindowEvent(event);
				}

				if (1) {
					int count = 0;

					//PixelData data;
					wxNativePixelData  data (bitmap);
					//bitmap.GetRawData(PixelFormat::BitsPerPixel);
					wxNativePixelData::Iterator p(data);
					//wxNativePixelData::Iterator p1(data);

					wxObjectRefData *refdata = bitmap.GetRefData();
					wxBitmapRefData *pbmpdata = bitmap.GetBitmapData();

					// we draw a (10, 10)-(20, 20) rect manually using the given r, g, b
					//p.Offset(data, 10, 10);
					unsigned char & a = p.Red();
					unsigned char * b = &a;
					//b[img.w * 3 - 1] = 0xff;

					//for (int y = 0; y < img.h; ++y)
					{
						//#pragma omp parallel for
						//for (int x = 0; x < img.w/4; ++x)
						{
							//b[3*(y*img.h+ x)] = img.buf[3 * (y*img.w + x) + 0];
						}
					}

					if (1) {
						for (int y = 0; y < img.h; ++y)
						{
							wxNativePixelData::Iterator rowStart = p;
							//unsigned char & a = p.Red();
							unsigned char * b = &p.Red();

							//#pragma omp parallel for //num_threads(3) //omp_get_num_procs()
							for (int x = 0; x < img.w; x++) //, ++p
							{
								//p.Alpha() = img.buf[4 * (y*img.w + x) + 0];
								//p.Green() = img.buf[3 * (y*img.w + x) + 1];
								//p.Blue() = img.buf[3 * (y*img.w + x) + 2];
								*(b + 3 * x + 0) = img.buf[3 * (y*img.w + x) + 3];
								*(b + 3 * x + 1) = img.buf[3 * (y*img.w + x) + 2];
								*(b + 3 * x + 2) = img.buf[3 * (y*img.w + x) + 1];
							}
							//p = rowStart;
							p.OffsetY(data, 1);
						}
					}
					wxSize s = this->m_mainWnd_->m_Panel->GetSize();

					//char* p = (char*)bitmap.GetRawData()
					count ++ ;
					dc.StretchBlit(0, 0, s.GetWidth(), s.GetHeight(), &temp_dc, 0, 0, img.w, img.h);
				}
				//img.LoadFile();
                //QImage tmpImg((uchar *)out_buffer,pCodecCtx->width,pCodecCtx->height,QImage::Format_RGB32);
                //QImage image = tmpImg.copy(); //把图像复制一份 传递给界面显示
               // emit sig_GetOneFrame(tmpImg);  //发送信号
            }
        }
		else if (packet->stream_index == audioStream) { //audioStream
			packet_queue_put(&audioq, packet);
		}

        //2017.8.7---lizhen
        //msleep(0.05); //停一停  不然放的太快了
		//wxMilliSleep(50);
		
		//2017.8.9---lizhen
        int64_t start_time=av_gettime();

        AVRational time_base=pFormatCtx->streams[videoStream]->time_base;

		double pts_time = av_q2d(time_base_q) * av_rescale_q(packet->dts, time_base, time_base_q);

		print_time();

		int64_t now_time = av_gettime() - start_time;
		//if (pts_time > now_time) 
		{
			double ti = pts_time - now_time;
			//OutputDebugStringA(wxString::Format ("dt:%f\n",ti).c_str());
			//av_usleep(ti);
			//NSSleep(ti);
		}

		//finalize_packet(0, 0, 0);
		av_free_packet(packet); //释放资源,否则内存会一直上升
    }

	dc.Clear();
    av_free(out_buffer);
    av_free(pFrameRGB);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
}
