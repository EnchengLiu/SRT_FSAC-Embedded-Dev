
#include <stdio.h>

#define __STDC_CONSTANT_MACROS

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavutil/imgutils.h"
#include "libavutil/dict.h"
#include "libavutil/mathematics.h"
#include "libavutil/time.h"
};

int main()
{
	AVFormatContext *ifmtCtx = NULL;
	AVFormatContext *ofmtCtx = NULL;//������������������ṹ��
	//����AVInputFormat���������ݷ�װ��ʽ�Ľṹ�壻����AVIOContext���������ݵĻ��棩�����бȽ���Ҫ�ĳ�Ա��AVClass�ṹ���Լ�int buffer_size
	//AVIOContext�������AVIOContext����avio_open2�������˺���֮�����ܣ������ģ�������av_class����ѡ��ݸ�Э�顣�����AVIOContext���ֶ�����ģ�������߿�������av_class��
	//����AVCodec�ṹ�壨�����������ṹ��������Ӧ������
	//int bit_rate��ƽ��������
    //AVRational time_base�����ݸò��������԰�PTSת��Ϊʵ�ʵ�ʱ�䣨��λΪ��s��
    //int width, height���������Ƶ�Ļ��������͸�
    //int refs���˶����Ʋο�֡�ĸ�����H.264�Ļ����ж�֡��MPEG2�����һ���û���ˣ�
    //enum AVSampleFormat sample_fmt��������ʽ
    //����AVFrame�ṹ��
    //����AVPacket�ṹ��
	AVPacket pkt;//�洢ѹ���������������Ϣ�Ľṹ��
	//int   size��data�Ĵ�С��int64_t pts����ʾʱ�����int64_t dts������ʱ�����int   stream_index����ʶ��AVPacket��������Ƶ/��Ƶ����
	//����ʱ�������ʮ����Ҫ��
	AVFrame *pFrame, *pFrameYUV;//�洢һ������Ƶ����ԭʼ����
	//int format�������ԭʼ�������ͣ�YUV420��YUV422��RGB24...��
    //int key_frame���Ƿ��ǹؼ�֡
    //enum AVPictureType pict_type��֡���ͣ�I,B,P...��
    //AVRational sample_aspect_ratio����߱ȣ�16:9��4:3...��
    //int64_t pts����ʾʱ���
	SwsContext *pImgConvertCtx;//����AVClass�ṹ��
	//int srcW��Դ��;int srcH��Դ��;
	//enum AVPixelFormat dstFormat��Ŀ��pix��ʽ��enum AVPixelFormat srcFormat��Դpix��ʽ
	AVDictionary *params = NULL;//av_dict_get����֮һ
	AVCodec *pCodec;//const char *name��������������֣��Ƚ϶�
    //const char *long_name��������������֣�ȫ�ƣ��Ƚϳ�
    //enum AVMediaType type��ý�����ͣ�����Ƶ����Ƶ��������Ļ
    //enum AVCodecID id��ID�����ظ�
    //const AVRational *supported_framerates��֧�ֵ�֡�ʣ�����Ƶ��
    //const enum AVPixelFormat *pix_fmts��֧�ֵ����ظ�ʽ������Ƶ��
    //
    //������Ƶ��������
	AVCodecContext *pCodecCtx;//enum AVMediaType codec_type��������������ͣ���Ƶ����Ƶ...����struct AVCodec  *codec�����õĽ�����AVCodec��һһ��Ӧ����AVCC->AVC��
	//int bit_rate��ƽ�������ʣ�int bit_rate_tolerance�����̵���ͱ�����
	//AVRational time_base��ʱ���
    //int width, height���������Ƶ�Ļ��������͸�
    //int refs���˶����Ʋο�֡�ĸ�������
    //enum AVSampleFormat sample_fmt��������ʽ
    //������Ƶ��������Ϣ������
	unsigned char *outBuffer;
	AVCodecContext *pH264CodecCtx;//��������Ϣ������
	AVCodec *pH264Codec;//�������ṹ��
	AVDictionary *options = NULL;//�ֵ�ṹ�壻int count��������AVDictionaryEntry�ṹ�������
	//AVDictionaryEntry������key�Ͷ�Ӧ���ı�����

	int ret = 0;//�����ж������ı���
	unsigned int i = 0;
	int videoIndex = -1;//��Ƶ������
	int frameIndex = 0;//֡����

	//const char *inFilename = "/dev/video1";//����URL
	const char *outFilename = "rtmp://119.45.138.172:1935/zbcs"; //���URL
	const char *ofmtName = NULL;//���������

	avdevice_register_all();//ע�������豸
	avformat_network_init();//��ʽ��ʼ��

	AVInputFormat *ifmt = av_find_input_format("video4linux2");//Ѱ������ĺ�����linux����video4linux2����
	if (!ifmt)
	{
		printf("can't find input device\n");
		goto end;
	}

	// 1. ������
	// 1.1 �������ļ�����ȡ��װ��ʽ�����Ϣ
	av_dict_set_int(&options, "rtbufsize", 18432000  , 0);//��intת��Ϊchar��return av_dict_set(pm, key, valuestr, flags)
	//��һ������Ϊָ���ֵ�ṹ��ָ���ָ�루���Ｔ�ֵ�ṹ��ָ��ĵ�ַ�����ڶ�������Ϊ�ؼ��֡�����������Ϊ�ؼ��ֵ�ֵ��int/�ַ����������ĸ�����Ϊ��־������
	if ((ret = avformat_open_input(&ifmtCtx, "/dev/video1", ifmt, &options)) < 0)//��������������AVStream�ṹ��streams[i]��nb_streams����
	{
		printf("can't open input file: %s\n", "/dev/video1");
		goto end;
	}

	// 1.2 ����һ�����ݣ���ȡ�������Ϣ
	if ((ret = avformat_find_stream_info(ifmtCtx, 0)) < 0)//�ú��������ҽ�������find_decoder()���򿪽�������avcodec_open2()����ȡ������һ֡ѹ����������ݣ�read_frame_internal()
	{
		printf("failed to retrieve input stream information\n");
		goto end;
	}

	// 1.3 ��ȡ����ctx
	for (i=0; i<ifmtCtx->nb_streams; ++i)
	{
		if (ifmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)//ý������Ϊ��Ƶ
		{
			videoIndex = i;
			break;
		}
	}

	printf("%s:%d, videoIndex = %d\n", __FUNCTION__, __LINE__, videoIndex);

	av_dump_format(ifmtCtx, 0, "/dev/video1", 0);

	// 1.4 �������������
	pCodec = avcodec_find_decoder(ifmtCtx->streams[videoIndex]->codecpar->codec_id);//��ÿһ��������Ѱ�ҽ�����������һ��AVCodec�ṹ�壨�������������򷵻�null
	if (!pCodec)
	{
		printf("can't find codec\n");
		goto end;
	}

	pCodecCtx = avcodec_alloc_context3(pCodec);//Ϊ����������һ����Ӧ��AVCodecContxet�ṹ�岢��ΪĬ��ֵ
	if (!pCodecCtx)
	{
		printf("can't alloc codec context\n");
		goto end;
	}

	avcodec_parameters_to_context(pCodecCtx, ifmtCtx->streams[videoIndex]->codecpar);//���AVCodecContxet�ṹ���в���

	//  1.5 �����������
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		printf("can't open codec\n");
		goto end;
	}


	// 1.6 ����FLV1������
	pH264Codec = avcodec_find_encoder(AV_CODEC_ID_H264);//����IDѰ��
	if (!pH264Codec)
	{
		printf("can't find h264 codec.\n");
		goto end;
	}

	// 1.6.1 ���ò���
	pH264CodecCtx = avcodec_alloc_context3(pH264Codec);//�����ӦAVCodecContxet�ṹ��
	pH264CodecCtx->codec_id = AV_CODEC_ID_H264;//H264��װ��ʽ
	pH264CodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;//ý������Ϊ��Ƶ
	pH264CodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;//���ظ�ʽ
	pH264CodecCtx->width = pCodecCtx->width;
	pH264CodecCtx->height = pCodecCtx->height;
	pH264CodecCtx->time_base.num = 1;
	pH264CodecCtx->time_base.den = 25;	//֡�ʣ���һ���Ӷ�����ͼƬ��
	pH264CodecCtx->bit_rate = 400000;	//�����ʣ����������С���Ըı�������Ƶ��������
	pH264CodecCtx->gop_size = 250;
	pH264CodecCtx->qmin = 10;
	pH264CodecCtx->qmax = 51;
	//some formats want stream headers to be separate
//	if (pH264CodecCtx->flags & AVFMT_GLOBALHEADER)
	{
		pH264CodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}


	// 1.7 ��H.264������
	av_dict_set(&params, "fflags", "nobuffer", 0);
	av_dict_set(&params, "preset", "superfast", 0);
	av_dict_set(&params, "tune", "zerolatency", 0);	//ʵ��ʵʱ����
	if (avcodec_open2(pH264CodecCtx, pH264Codec, &params) < 0)
	{
		printf("can't open video encoder.\n");
		goto end;
	}

	// 2. �����
	// 2.1 �������ctx
	if (strstr(outFilename, "rtmp://"))
	{
		ofmtName = "flv";
	}
	else if (strstr(outFilename, "udp://"))//������������ַ���ͷ������װ��ʽ��rtmp��udp��
	{
		ofmtName = "mpegts";
	}
	else
	{
		ofmtName = NULL;
	}

	avformat_alloc_output_context2(&ofmtCtx, NULL, ofmtName, outFilename);//Ϊ�������AVCodecContxet�ṹ��
	if (!ofmtCtx)
	{
		printf("can't create output context\n");
		goto end;
	}

	// 2.2 ���������
	for (i=0; i<ifmtCtx->nb_streams; ++i)
	{
		AVStream *outStream = avformat_new_stream(ofmtCtx, NULL);
		if (!outStream)
		{
			printf("failed to allocate output stream\n");
			goto end;
		}

		avcodec_parameters_from_context(outStream->codecpar, pH264CodecCtx);//����������������Ӧcontext�ṹ��
	}

	av_dump_format(ofmtCtx, 0, outFilename, 1);

	if (!(ofmtCtx->oformat->flags & AVFMT_NOFILE))
	{
		// 2.3 ��������ʼ��һ��AVIOContext, ���Է���URL��outFilename��ָ������Դ
		ret = avio_open(&ofmtCtx->pb, outFilename, AVIO_FLAG_WRITE);//avio��ӦЭ��㣬��url��Ϊ��������Э��������������źŴ���
		if (ret < 0)
		{
			printf("can't open output URL: %s\n", outFilename);
			goto end;
		}
	}

	// 3. ���ݴ���
	// 3.1 д����ļ�
	ret = avformat_write_header(ofmtCtx, NULL);//�������
	if (ret < 0)
	{
		printf("Error accourred when opening output file\n");
		goto end;
	}


	pFrame = av_frame_alloc();
	pFrameYUV = av_frame_alloc();//���ڶ�ԭʼ��Ƶ����֡��ʽ���д���Ľṹ��

	outBuffer = (unsigned char*) av_malloc(
			av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width,
					pCodecCtx->height, 1));
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, outBuffer,
			AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

	pImgConvertCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
			pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
			AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);//�������úͳ߶ȱ任


	while (1)
	{
		// 3.2 ����������ȡһ��packet
		ret = av_read_frame(ifmtCtx, &pkt);//����֡ԭʼ����
		if (ret < 0)
		{
			break;
		}

		if (pkt.stream_index == videoIndex)
		{
			ret = avcodec_send_packet(pCodecCtx, &pkt);//���Ͱ�
			if (ret < 0)
			{
				printf("Decode error.\n");
				goto end;
			}

			if (avcodec_receive_frame(pCodecCtx, pFrame) >= 0)//���ճɹ�
			{
				sws_scale(pImgConvertCtx,
						(const unsigned char* const*) pFrame->data,
						pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data,
						pFrameYUV->linesize);


				pFrameYUV->format = pCodecCtx->pix_fmt;
				pFrameYUV->width = pCodecCtx->width;
				pFrameYUV->height = pCodecCtx->height;

				ret = avcodec_send_frame(pH264CodecCtx, pFrameYUV);//����֡����
				if (ret < 0)
				{
					printf("failed to encode.\n");
					goto end;
				}

				if (avcodec_receive_packet(pH264CodecCtx, &pkt) >= 0)//���ձ������ݰ�
				{
					// �������DTS,PTS
			        pkt.pts = pkt.dts = frameIndex * (ofmtCtx->streams[0]->time_base.den) /ofmtCtx->streams[0]->time_base.num / 25;
			        frameIndex++;

					ret = av_interleaved_write_frame(ofmtCtx, &pkt);
					if (ret < 0)
					{
						printf("send packet failed: %d\n", ret);
					}
					else
					{
						printf("send %5d packet successfully!\n", frameIndex);
					}
				}
			}
		}

		av_packet_unref(&pkt);//����������ͷſռ�
	}

    av_write_trailer(ofmtCtx);//��1��ѭ������interleave_packet()�Լ�write_packet()������δ�����AVPacket��������� ��2������AVOutputFormat��write_trailer()������ļ�β

end:
    avformat_close_input(&ifmtCtx);

    /* close output */
    if (ofmtCtx && !(ofmtCtx->oformat->flags & AVFMT_NOFILE)) {
        avio_closep(&ofmtCtx->pb);
    }
    avformat_free_context(ofmtCtx);

    if (ret < 0 && ret != AVERROR_EOF) {
        printf("Error occurred\n");
        return -1;
    }

    return 0;
}

