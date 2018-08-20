#include "ffmpeg_h264.h"
#include <iostream>
FFH264::FFH264()
{
	m_videondex = -1;
	m_isRecord = -1;
	m_swsContext = nullptr;
	m_codecContext = nullptr;
	
	/**
	 * @brief Construct a new av register all object
	 * av_register_all() register muxer demuxer
	 */
	
	av_register_all();
	m_frame = av_frame_alloc();
	m_frameBGR = av_frame_alloc();
}

FFH264::~FFH264()
{
	// av_free(buffer);
	// av_frame_free(&pframe);
	// av_frame_free(&pframeYUV);
	// avcodec_free_context(&pcodecContext);
	// avformat_close_input(&pformatContext);
	av_frame_free(&m_frame);
}

bool FFH264::InitH264DecodeEnv()
{
	do {
		m_codec = avcodec_find_decoder(AV_CODEC_ID_H264);
		if(!m_codec) 
		{
			break;
		}
		/**
		 * @brief 
		 * 
		 */
		m_codecContext = avcodec_alloc_context3(m_codec);
		/**
		 * @brief initiate codeccontext to specific m_codec h264
		 * 
		 */

		if(avcodec_open2(m_codecContext, m_codec, nullptr) < 0)
		{
			break;
		}
		return true;
	} while(0);
	return false;
}

void FFH264::SetPlayState(bool pause)
{
	if(pause) 
	{
		m_playMutex.lock();
	}
	else 
	{
		m_playMutex.unlock();
	}
}

void FFH264::DecodeFrame(unsigned char* sPropBytes, int sPropLength, uint8_t* frameBuffer, int frameLength, long second, long microSecond)
{
	if(frameLength <= 0) return;

	unsigned char nalu_header[4] = {0x00, 0x00, 0x00, 0x01};
	int totalSize = 4 + sPropLength + 4 + frameLength;
	unsigned char* tmp = new unsigned char[totalSize];

	memcpy(tmp, nalu_header, 4);
	memcpy(tmp + 4, sPropBytes, sPropLength);
	memcpy(tmp + 4 +  sPropLength, nalu_header, 4);
	memcpy(tmp + 4 +  sPropLength + 4,  frameBuffer, frameLength);


	int frameFinished = 0;
	AVPacket framePacket;
	av_init_packet(&framePacket);

	framePacket.size = totalSize;
	framePacket.data = tmp;


	/**
	 * @brief avcodec_decode_video2
	 * input AVCodecContext*, AVFrame*,int frameFinished,
	 */
	int ret = avcodec_decode_video2(m_codecContext, m_frame, &frameFinished, &framePacket);
	if(ret < 0)
	{
		std::cout << "Decodec Error!" << std::endl;
	}


	if(frameFinished)
	{
		m_playMutex.lock();
		VideoWidth = m_frame->width;
		VideoHeight = m_frame->height;

		if(m_swsContext == nullptr)
		{
			/**
			 * if m_swsContext is null getcachedcontext equals to sws_getcontext()
			 * change format
			 */
			m_swsContext = sws_getCachedContext(m_swsContext, VideoWidth, VideoHeight, AVPixelFormat::AV_PIX_FMT_YUV420P, VideoWidth, VideoHeight,
				AVPixelFormat::AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
			//sws_getCachedContext(pswscontext, pcodecContext->width, pcodecContext->height, pcodecContext->pix_fmt,
		//pcodecContext->width, pcodecContext->height, AV_PIX_FMT_YUV420P, SWS_BILINEAR, nullptr, nullptr, nullptr);
			
			int size = avpicture_get_size(AV_PIX_FMT_RGB24, m_codecContext->width, m_codecContext->height);
			
			out_buffer = (uint8_t* )av_malloc(size);
			/**
			 * m_frameBGR and outbuffer should be allocated already
			 * put m_framebgr data to out_buffer as avpix rgb24 format
			 * 
			 */
			avpicture_fill((AVPicture*)m_frameBGR, out_buffer, AV_PIX_FMT_RGB24, m_codecContext->width, m_codecContext->height);
		}
		
		//transfer yuv to rgb
		sws_scale(m_swsContext, (const uint8_t* const* )m_frame->data, m_frame->linesize, 0, VideoHeight, m_frameBGR->data, m_frameBGR->linesize);
		
		m_playMutex.unlock();
	}

	av_free_packet(&framePacket);
	delete[] tmp;
	tmp = nullptr; //avoid pointer error
}

void FFH264::GetDecodedFrameData(unsigned char* data, int& length)
{
	m_playMutex.lock();
	length = VideoWidth* VideoHeight * 3 ;
	/**
	 * destination and source
	 * 
	 */
	memcpy(data, out_buffer, length);
	m_playMutex.unlock();
}

void FFH264::GetDecodedFrameInfo(int& width, int& heigth)
{
	width = VideoWidth;
	heigth = VideoHeight;
}
