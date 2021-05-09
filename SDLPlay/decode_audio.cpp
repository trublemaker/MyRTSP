#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
}

static void decode(AVCodecContext *cdc_ctx, AVFrame *frame, AVPacket *pkt, FILE *fp_out)
{
	int ret = 0;
	int data_size = 0;
	int i, ch;

	if ((ret = avcodec_send_packet(cdc_ctx, pkt)) < 0)
	{
		fprintf(stderr, "avcodec_send_packet failed.\n");
		exit(1);
	}

	data_size = av_get_bytes_per_sample(cdc_ctx->sample_fmt);

	while ((ret = avcodec_receive_frame(cdc_ctx, frame)) >= 0)
	{
		printf("Write 1 frame.\n");

		for (i = 0; i < frame->nb_samples; i++)
		{
			for (ch = 0; ch < cdc_ctx->channels; ch++)
			{
				fwrite(frame->data[ch] + data_size * i, 1, data_size, fp_out);
			}
		}
	}

	if ((ret != AVERROR(EAGAIN)) && (ret != AVERROR_EOF))
	{
		fprintf(stderr, "avcodec_receive_packet failed.\n");
		exit(1);
	}
}

void decode_audio(const char *input_file, const char *output_file)
{
	int ret = 0;
	AVCodec *codec = NULL;
	AVCodecContext *cdc_ctx = NULL;
	AVPacket *pkt = NULL;
	AVFrame *frame = NULL;
	FILE *fp_out;
	AVFormatContext *fmt_ctx = NULL;

	if ((ret = avformat_open_input(&fmt_ctx, input_file, NULL, NULL)) < 0)
	{
		fprintf(stderr, "avformat_open_input failed.\n");
		goto ret1;
	}

	if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0)
	{
		fprintf(stderr, "avformat_find_stream_info failed.\n");
		goto ret2;
	}

	if ((ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0)) < 0)
	{
		fprintf(stderr, "av_find_best_stream failed.\n");
		goto ret2;
	}

	if ((codec = avcodec_find_decoder(fmt_ctx->streams[ret]->codecpar->codec_id)) == NULL)
	{
		fprintf(stderr, "avcodec_find_decoder failed.\n");
		goto ret2;
	}

	if ((cdc_ctx = avcodec_alloc_context3(codec)) == NULL)
	{
		fprintf(stderr, "avcodec_alloc_context3 failed.\n");
		goto ret2;
	}

	if ((ret = avcodec_open2(cdc_ctx, codec, NULL)) < 0)
	{
		fprintf(stderr, "avcodec_open2 failed.\n");
		goto ret3;
	}

	if ((pkt = av_packet_alloc()) == NULL)
	{
		fprintf(stderr, "av_packet_alloc failed.\n");
		goto ret4;
	}

	if ((frame = av_frame_alloc()) == NULL)
	{
		fprintf(stderr, "av_frame_alloc failed.\n");
		goto ret5;
	}

	if ((fp_out = fopen(output_file, "wb")) == NULL)
	{
		fprintf(stderr, "fopen %s failed.\n", output_file);
		goto ret6;
	}

	while ((ret = av_read_frame(fmt_ctx, pkt)) == 0)
	{
		if (pkt->size > 0)
			decode(cdc_ctx, frame, pkt, fp_out);
	}

	decode(cdc_ctx, frame, NULL, fp_out);


	fclose(fp_out);
	av_frame_free(&frame);
	av_packet_free(&pkt);
	avcodec_close(cdc_ctx);
	avcodec_free_context(&cdc_ctx);
	avformat_close_input(&fmt_ctx);
	return;
ret6:
	av_frame_free(&frame);
ret5:
	av_packet_free(&pkt);
ret4:
	avcodec_close(cdc_ctx);
ret3:
	avcodec_free_context(&cdc_ctx);
ret2:
	avformat_close_input(&fmt_ctx);
ret1:
	exit(1);
}

int main(int argc, const char *argv[])
{
	if (argc < 3)
	{
		//fprintf(stderr, "Uage:<input file> <output file>\n");
		//exit(0);
	}
	char* filename = "C:\\Qt\\RTSP\\zsyf.mp3";
	char *outfilename = "C:\\Qt\\RTSP\\zsyf1.mp3";

	decode_audio(filename, outfilename);

	return 0;
}