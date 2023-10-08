
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
	AVFormatContext *ofmtCtx = NULL;//定义输入流与输出流结构体
	//调用AVInputFormat，输入数据封装格式的结构体；包含AVIOContext（输入数据的缓存），其中比较重要的成员有AVClass结构体以及int buffer_size
	//AVIOContext，如果此AVIOContext是由avio_open2（）（此函数之后会介绍）创建的，则设置av_class并将选项传递给协议。如果此AVIOContext是手动分配的，则调用者可以设置av_class。
	//调用AVCodec结构体（编码器），结构体名即对应编码器
	//int bit_rate：平均比特率
    //AVRational time_base：根据该参数，可以把PTS转化为实际的时间（单位为秒s）
    //int width, height：如果是视频的话，代表宽和高
    //int refs：运动估计参考帧的个数（H.264的话会有多帧，MPEG2这类的一般就没有了）
    //enum AVSampleFormat sample_fmt：采样格式
    //调用AVFrame结构体
    //调用AVPacket结构体
	AVPacket pkt;//存储压缩编码数据相关信息的结构体
	//int   size：data的大小；int64_t pts：显示时间戳；int64_t dts：解码时间戳；int   stream_index：标识该AVPacket所属的视频/音频流。
	//两个时间戳参数十分重要，
	AVFrame *pFrame, *pFrameYUV;//存储一个音视频流的原始数据
	//int format：解码后原始数据类型（YUV420，YUV422，RGB24...）
    //int key_frame：是否是关键帧
    //enum AVPictureType pict_type：帧类型（I,B,P...）
    //AVRational sample_aspect_ratio：宽高比（16:9，4:3...）
    //int64_t pts：显示时间戳
	SwsContext *pImgConvertCtx;//调用AVClass结构体
	//int srcW，源宽;int srcH，源高;
	//enum AVPixelFormat dstFormat，目标pix格式；enum AVPixelFormat srcFormat；源pix格式
	AVDictionary *params = NULL;//av_dict_get参数之一
	AVCodec *pCodec;//const char *name：编解码器的名字，比较短
    //const char *long_name：编解码器的名字，全称，比较长
    //enum AVMediaType type：媒体类型，是视频，音频，还是字幕
    //enum AVCodecID id：ID，不重复
    //const AVRational *supported_framerates：支持的帧率（仅视频）
    //const enum AVPixelFormat *pix_fmts：支持的像素格式（仅视频）
    //
    //输入视频流解码器
	AVCodecContext *pCodecCtx;//enum AVMediaType codec_type：编解码器的类型（视频，音频...）；struct AVCodec  *codec：采用的解码器AVCodec（一一对应调用AVCC->AVC）
	//int bit_rate：平均比特率；int bit_rate_tolerance：容忍的最低比特率
	//AVRational time_base：时间基
    //int width, height：如果是视频的话，代表宽和高
    //int refs：运动估计参考帧的个数（）
    //enum AVSampleFormat sample_fmt：采样格式
    //输入视频流解码信息及参数
	unsigned char *outBuffer;
	AVCodecContext *pH264CodecCtx;//编码器信息及参数
	AVCodec *pH264Codec;//编码器结构体
	AVDictionary *options = NULL;//字典结构体；int count，计数（AVDictionaryEntry结构体个数）
	//AVDictionaryEntry，包含key和对应的文本内容

	int ret = 0;//用于判断条件的变量
	unsigned int i = 0;
	int videoIndex = -1;//视频流计数
	int frameIndex = 0;//帧计数

	//const char *inFilename = "/dev/video1";//输入URL
	const char *outFilename = "rtmp://119.45.138.172:1935/zbcs"; //输出URL
	const char *ofmtName = NULL;//输出流名称

	avdevice_register_all();//注册所有设备
	avformat_network_init();//格式初始化

	AVInputFormat *ifmt = av_find_input_format("video4linux2");//寻找输入的函数，linux下用video4linux2参数
	if (!ifmt)
	{
		printf("can't find input device\n");
		goto end;
	}

	// 1. 打开输入
	// 1.1 打开输入文件，获取封装格式相关信息
	av_dict_set_int(&options, "rtbufsize", 18432000  , 0);//把int转化为char并return av_dict_set(pm, key, valuestr, flags)
	//第一个参数为指向字典结构体指针的指针（这里即字典结构体指针的地址），第二个参数为关键字。第三个参数为关键字的值（int/字符串），第四个参数为标志参数。
	if ((ret = avformat_open_input(&ifmtCtx, "/dev/video1", ifmt, &options)) < 0)//打开输入流，生成AVStream结构体streams[i]，nb_streams计数
	{
		printf("can't open input file: %s\n", "/dev/video1");
		goto end;
	}

	// 1.2 解码一段数据，获取流相关信息
	if ((ret = avformat_find_stream_info(ifmtCtx, 0)) < 0)//该函数：查找解码器：find_decoder()；打开解码器：avcodec_open2()；读取完整的一帧压缩编码的数据：read_frame_internal()
	{
		printf("failed to retrieve input stream information\n");
		goto end;
	}

	// 1.3 获取输入ctx
	for (i=0; i<ifmtCtx->nb_streams; ++i)
	{
		if (ifmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)//媒体类型为视频
		{
			videoIndex = i;
			break;
		}
	}

	printf("%s:%d, videoIndex = %d\n", __FUNCTION__, __LINE__, videoIndex);

	av_dump_format(ifmtCtx, 0, "/dev/video1", 0);

	// 1.4 查找输入解码器
	pCodec = avcodec_find_decoder(ifmtCtx->streams[videoIndex]->codecpar->codec_id);//对每一个输入流寻找解码器，返回一个AVCodec结构体（解码器），否则返回null
	if (!pCodec)
	{
		printf("can't find codec\n");
		goto end;
	}

	pCodecCtx = avcodec_alloc_context3(pCodec);//为解码器分配一个对应的AVCodecContxet结构体并设为默认值
	if (!pCodecCtx)
	{
		printf("can't alloc codec context\n");
		goto end;
	}

	avcodec_parameters_to_context(pCodecCtx, ifmtCtx->streams[videoIndex]->codecpar);//填充AVCodecContxet结构体中参数

	//  1.5 打开输入解码器
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		printf("can't open codec\n");
		goto end;
	}


	// 1.6 查找FLV1编码器
	pH264Codec = avcodec_find_encoder(AV_CODEC_ID_FLV1);
	if (!pH264Codec)
	{
		printf("can't find h264 codec.\n");
		goto end;
	}

	// 1.6.1 设置参数
	pH264CodecCtx = avcodec_alloc_context3(pH264Codec);//分配对应AVCodecContxet结构体
	//以下手动填充参数
	pH264CodecCtx->codec_id = AV_CODEC_ID_FLV1;//FLV封装编码格式
	pH264CodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;//媒体类型为视频
	pH264CodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;//像素格式
	pH264CodecCtx->width = pCodecCtx->width;
	pH264CodecCtx->height = pCodecCtx->height;
	pH264CodecCtx->time_base.num = 1;
	pH264CodecCtx->time_base.den = 25;	//帧率（即一秒钟多少张图片）
	pH264CodecCtx->bit_rate = 400000;	//比特率（调节这个大小可以改变编码后视频的质量）
	pH264CodecCtx->gop_size = 250;
	pH264CodecCtx->qmin = 10;
	pH264CodecCtx->qmax = 51;
	//some formats want stream headers to be separate
//	if (pH264CodecCtx->flags & AVFMT_GLOBALHEADER)
	{
		pH264CodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}


	// 1.7 打开H.264编码器
	av_dict_set(&params, "preset", "superfast", 0);
	av_dict_set(&params, "tune", "zerolatency", 0);	//实现实时编码
	if (avcodec_open2(pH264CodecCtx, pH264Codec, &params) < 0)
	{
		printf("can't open video encoder.\n");
		goto end;
	}

	// 2. 打开输出
	// 2.1 分配输出ctx
	if (strstr(outFilename, "rtmp://"))
	{
		ofmtName = "flv";
	}
	else if (strstr(outFilename, "udp://"))//根据输出名称字符串头决定封装格式（rtmp，udp）
	{
		ofmtName = "mpegts";
	}
	else
	{
		ofmtName = NULL;
	}

	avformat_alloc_output_context2(&ofmtCtx, NULL, ofmtName, outFilename);//为输出分配AVCodecContxet结构体
	if (!ofmtCtx)
	{
		printf("can't create output context\n");
		goto end;
	}

	// 2.2 创建输出流
	for (i=0; i<ifmtCtx->nb_streams; ++i)
	{
		AVStream *outStream = avformat_new_stream(ofmtCtx, NULL);
		if (!outStream)
		{
			printf("failed to allocate output stream\n");
			goto end;
		}

		avcodec_parameters_from_context(outStream->codecpar, pH264CodecCtx);
	}

	av_dump_format(ofmtCtx, 0, outFilename, 1);

	if (!(ofmtCtx->oformat->flags & AVFMT_NOFILE))
	{
		// 2.3 创建并初始化一个AVIOContext, 用以访问URL（outFilename）指定的资源
		ret = avio_open(&ofmtCtx->pb, outFilename, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
			printf("can't open output URL: %s\n", outFilename);
			goto end;
		}
	}

	// 3. 数据处理
	// 3.1 写输出文件
	ret = avformat_write_header(ofmtCtx, NULL);
	if (ret < 0)
	{
		printf("Error accourred when opening output file\n");
		goto end;
	}


	pFrame = av_frame_alloc();
	pFrameYUV = av_frame_alloc();

	outBuffer = (unsigned char*) av_malloc(
			av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width,
					pCodecCtx->height, 1));
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, outBuffer,
			AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

	pImgConvertCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
			pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
			AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);


	while (1)
	{
		// 3.2 从输入流读取一个packet
		ret = av_read_frame(ifmtCtx, &pkt);
		if (ret < 0)
		{
			break;
		}

		if (pkt.stream_index == videoIndex)
		{
			ret = avcodec_send_packet(pCodecCtx, &pkt);
			if (ret < 0)
			{
				printf("Decode error.\n");
				goto end;
			}

			if (avcodec_receive_frame(pCodecCtx, pFrame) >= 0)
			{
				sws_scale(pImgConvertCtx,
						(const unsigned char* const*) pFrame->data,
						pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data,
						pFrameYUV->linesize);


				pFrameYUV->format = pCodecCtx->pix_fmt;
				pFrameYUV->width = pCodecCtx->width;
				pFrameYUV->height = pCodecCtx->height;

				ret = avcodec_send_frame(pH264CodecCtx, pFrameYUV);
				if (ret < 0)
				{
					printf("failed to encode.\n");
					goto end;
				}

				if (avcodec_receive_packet(pH264CodecCtx, &pkt) >= 0)
				{
					// 设置输出DTS,PTS
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

		av_packet_unref(&pkt);
	}

    av_write_trailer(ofmtCtx);

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
