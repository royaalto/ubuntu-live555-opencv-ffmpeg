#include "lcm_interface.h"
#include "rtsp_client.h"
#include "ffmpeg_h264.h"
//#include <opencv2/opencv.hpp>
#include <unistd.h>
// #include "lcm_interface.h"
#include "jpeg_utils.h"

int main() {
	RtspThread clientThread("rtsp://192.168.0.22/rtsp_tunnel?profile=0", "service", "1234hailee");
	std::cout<<"1"<<std::endl;
	clientThread.Run();
	std::cout<<"2"<<std::endl;
	//cv::Mat m_frameMat;
	//mjpeg


	while(1) {
		int width=0;
		int height = 0;
		clientThread.ffmpegH264->GetDecodedFrameInfo(width, height);
		
		//std::cout<<"width and heigt"<<width<<"and"<<height<<std::endl;
		if(width > 0 && height > 0 && width < 3000 && height <3000)
		{
			//std::cout<<"started"<<std::endl;
			// if(m_frameMat.empty())
			// {
			// 	std::cout<<"m_frammat is empty"<<std::endl;
			// 	m_frameMat.create(cv::Size(width, height), CV_8UC3);
			// }
			int length;
			unsigned char* jpeg_img = new unsigned char[width*height*3];
			clientThread.ffmpegH264->GetDecodedFrameData(jpeg_img, length);//VideoWidth* VideoHeight * 3
			//std::cout<<jpeg_img;
			// for(int i = 0; i < length; i += 3)
			// {
			// 	uchar temp = jpeg_img[i];
			// 	jpeg_img[i] = jpeg_img[i+2];
			// 	jpeg_img[i+2] = temp;
			// }


			unsigned int *aOutputSizePtr;
			unsigned char **aOutputPtrPtr;


			//img::jpeg::encode(jpeg_img, 1920, 1080, true,aOutputPtrPtr, aOutputSizePtr);

			delete[] jpeg_img;

			//mjpeg
			// AVCodecID mjpeg_id = AV_CODEC_ID_MJPEG;
			// AVCodec* mjpeg_codec = avcodec_find_encoder(mjpeg_id);
			// if(!mjpeg_codec)
			// {
			// 	std::cout<<"can not find mjpeg codec"<<std::endl;
			// 	break;
			// }

			// AVCodecContext* mjpeg_codeContex = avcodec_alloc_context3(mjpeg_codec);
			// //mjpeg_codeContex->codec_id = mjpeg_id;
			// //mjpeg_codeContex->codec_type = AVMEDIA_TYPE_VIDEO;
			// //mjpeg_codeContex->pix_fmt = AV_PIX_FMT_YUVJ422P;
			// //mjpeg_codeContex->pix_fmt = AV_PIX_FMT_RGB24;
			// mjpeg_codeContex->bit_rate = 400000;
			// mjpeg_codeContex->width = 320;
			// mjpeg_codeContex->height = 240;
			// mjpeg_codeContex->time_base.num = 1;
			// mjpeg_codeContex->time_base.den = 25;

			// int retval;
			// // Open the codec.
			// if ((retval = avcodec_open2(mjpeg_codeContex, mjpeg_codec, NULL)) < 0)
			// {
			// 	printf("could not open codec\n");
			// 	return -2;
			// }



			// cv::imshow("Camera", m_frameMat);
			// cv::waitKey(1);
		}
	}
}
