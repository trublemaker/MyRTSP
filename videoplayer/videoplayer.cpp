﻿/**
 * 李震
 * 我的码云：https://git.oschina.net/git-lizhen
 * 我的CSDN博客：http://blog.csdn.net/weixin_38215395
 * 联系：QQ1039953685
 */

#include "videoplayer.h"
#include "../RTSPMainWnd.h"

#include <wx/mstream.h>
#include <wx/rawbmp.h>

extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libavutil/pixfmt.h"
    #include "libswscale/swscale.h"
    //2017.8.9---lizhen
    #include "libavutil/time.h"
    #include "libavutil/mathematics.h"
}

#include <stdio.h>
#include<iostream>
using namespace std;

void print_time() {
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

int NSSleep(int ms)
{
	HANDLE hTimer = NULL;
	LARGE_INTEGER liDueTime;
	print_time();
	liDueTime.QuadPart = -ms * 10000;

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
	print_time();
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

wxBitmap bitmap(1920, 1080, 24);
wxMemoryDC temp_dc;

void VideoPlayer::run()
{
	temp_dc.SelectObject(bitmap);
	wxClientDC dc(this->m_mainWnd_->m_Panel);
	wxSize s = this->m_mainWnd_->m_Panel->GetSize();

    //char *file_path = mFileName.toUtf8().data();
    //cout<<file_path<<endl;
    AVFormatContext *pFormatCtx;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    AVFrame *pFrame, *pFrameRGB;
    AVPacket *packet;
    uint8_t *out_buffer;

    static struct SwsContext *img_convert_ctx;

    int videoStream, i, numBytes;
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

	av_dict_set(&avdic, "buffer_size", "2024000", 0);
    //char url[]="rtsp://192.168.10.213:8554/h264ESVideoTest";

    char url[]="rtsp://182.139.226.78/PLTV/88888893/224/3221227219/10000100000000060000000001366244_0.smil?playseek=20190805101000-20190805113000";

    //rtsp://182.139.226.78/PLTV/88888893/224/3221227219/10000100000000060000000001366244_0.smil?playseek=20190801100000-20190801113000
    //"rtsp://admin:admin@192.168.1.18:554/h264/ch1/main/av_stream";

    if (avformat_open_input(&pFormatCtx, url, NULL, &avdic) != 0) {
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
    ///这里我们现在只处理视频流  音频流先不管他
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
        }
    }

    ///如果videoStream为-1 说明没有找到视频流
    if (videoStream == -1) {
        printf("Didn't find a video stream.\n");
        return;
    }

    ///查找解码器
    pCodecCtx = pFormatCtx->streams[videoStream]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
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

    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();

    //2017.8.7---lizhen
    //cout<<pCodecCtx->width<<endl;

    ///这里我们改成了 将解码后的YUV数据转换成RGB32
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
            pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
            PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);

    numBytes = avpicture_get_size(PIX_FMT_RGB32, pCodecCtx->width,pCodecCtx->height);

    out_buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    avpicture_fill((AVPicture *) pFrameRGB, out_buffer, PIX_FMT_RGB32,
            pCodecCtx->width, pCodecCtx->height);

    int y_size = pCodecCtx->width * pCodecCtx->height;

    packet = (AVPacket *) malloc(sizeof(AVPacket)); //分配一个packet
    av_new_packet(packet, y_size); //分配packet的数据

    //2017.8.1---lizhen
    av_dump_format(pFormatCtx, 0, url, 0); //输出视频信息

    while (!this->TestDestroy())
    {
        if (av_read_frame(pFormatCtx, packet) < 0)
        {
            break; //这里认为视频读取完了
        }

        if (packet->stream_index == videoStream) {
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture,packet);

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
				//}

				if (0) {
					wxCommandEvent event(wxEVT_NULL, 10086);
					event.SetEventObject(this->m_mainWnd_);
					event.SetExtraLong((long)&img);
					m_mainWnd_->ProcessWindowEvent(event);
				}

				if (1) {
					int count = 0;

					wxNativePixelData  data(bitmap);
					wxNativePixelData::Iterator p(data);
					wxNativePixelData::Iterator p1(data);

					int x = data.GetWidth();
					x = data.GetHeight();
					// we draw a (10, 10)-(20, 20) rect manually using the given r, g, b
					//p.Offset(data, 0, 0);

					for (int y = 0; y < img.h ; ++y)
					{
						wxNativePixelData::Iterator rowStart = p;

						//p++; 
						//#pragma omp parallel for
						for (int x = 0; x < img.w; ++x, ++p)
						{
							//p.Alpha() = img.buf[4 * (y*img.w + x) + 0];
							p.Red() = img.buf[4 * (y*img.w + x)+2];
							p.Green() = img.buf[4 * (y*img.w + x) + 1];
							p.Blue() = img.buf[4 * (y*img.w + x) + 0];
							count++;
						}
						p = rowStart;
						p.OffsetY(data, 1);
					}

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

        //2017.8.7---lizhen
        //msleep(0.05); //停一停  不然放的太快了
		//wxMilliSleep(50);


		AVRational time_base_q = { 1,AV_TIME_BASE };
		
		//2017.8.9---lizhen
        int64_t start_time=av_gettime();
        AVRational time_base=pFormatCtx->streams[videoStream]->time_base;

		double pts_time = av_q2d(time_base_q) * av_rescale_q(packet->dts, time_base, time_base_q);

		print_time();
		
		int64_t now_time = av_gettime() - start_time;
		if (pts_time > now_time) 
		{
			long ti = pts_time - now_time;
			OutputDebugStringA(wxString::Format ("\tdelta:%d\n",ti).c_str());
			//av_usleep(ti);
		}

		av_free_packet(packet); //释放资源,否则内存会一直上升
    }

    av_free(out_buffer);
    av_free(pFrameRGB);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
}