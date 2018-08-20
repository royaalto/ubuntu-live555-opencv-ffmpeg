#include "rtsp_client.h"
#include <thread>

// Implementation of "DummySink":

// Even though we're not going to be doing anything with the incoming data, we still need to receive it.
// Define the size of the buffer that we'll use:

DummySink* DummySink::createNew(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId) 
{
  	return new DummySink(env, subsession, streamId);
}

DummySink::DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId)
	: MediaSink(env), fSubsession(subsession) 
{
	fStreamId = strDup(streamId);
	fReceiveBuffer = new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE];
}

void DummySink::SetFFmpeg(FFH264* ffmpeg)
{
	m_ffmpeg = ffmpeg;
}

DummySink::~DummySink() 
{
	delete[] fReceiveBuffer;
	delete[] fStreamId;
}

// If you don't want to see debugging output for each received frame, then comment out the following line:
#define DEBUG_PRINT_EACH_RECEIVED_FRAME 1

void DummySink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds) 
{

//	if (fStreamId != NULL) 
//		envir() << "Stream \"" << fStreamId << "\"; ";
//	envir() << fSubsession.mediumName() << "/" << fSubsession.codecName() << ":\tReceived " << frameSize << " bytes";
//	if (numTruncatedBytes > 0)  
//		envir() << " (with " << numTruncatedBytes << " bytes truncated)";
//	char uSecsStr[6+1]; 
//	sprintf(uSecsStr, "%06u", (unsigned)presentationTime.tv_usec);
//	envir() << ".\tPresentation time: " << (int)presentationTime.tv_sec << "." << uSecsStr;
//	if (fSubsession.rtpSource() != NULL && !fSubsession.rtpSource()->hasBeenSynchronizedUsingRTCP()) {
//		envir() << "!"; 
//	}
//	envir() << "\n";	

		//std::cout << "onData: length-> " << frameSize << "  " << presentationTime.tv_sec << " " << presentationTime.tv_usec/1000  << std::endl;
//	std::cout << fSubsession.fmtp_spropparametersets() << std::endl;
	unsigned int Num = 0;
	unsigned int &SPropRecords = Num;
	SPropRecord* p_record = parseSPropParameterSets(fSubsession.fmtp_spropparametersets(), SPropRecords);
	SPropRecord &sps = p_record[0];
	//std::cout << sps.sPropBytes << "\t" << sps.sPropLength  << std::endl;
	m_ffmpeg->DecodeFrame(sps.sPropBytes, sps.sPropLength, fReceiveBuffer, frameSize, presentationTime.tv_sec, presentationTime.tv_usec/1000);
	this->continuePlaying();
}

Boolean DummySink::continuePlaying() 
{
	if (fSource == NULL) return False; 
	fSource->getNextFrame(fReceiveBuffer, DUMMY_SINK_RECEIVE_BUFFER_SIZE, afterGettingFrame, this, onSourceClosure, this);
	return True;
}

static inline RtspClient *p_this(RTSPClient* rtspClient) { return static_cast<RtspClient*>(rtspClient); }
#define P_THIS p_this(rtspClient)


RtspClient::RtspClient(UsageEnvironment& env, char const* rtspURL, char const* sUser, char const* sPasswd, FFH264** ffmpeg) 
	: RTSPClient(env, rtspURL, 255, NULL, 0, -1), m_Authenticator(sUser, sPasswd)
{
	m_ffmpeg = *ffmpeg;
}	


void RtspClient::Play() 
{
	this->SendNextCommand();
}


RtspClient::~RtspClient() {}

void RtspClient::ContinueAfterDescribe(int resultCode, char* resultString) 
{
	if (resultCode != 0) 
	{
		std::cout << "Failed to DESCRIBE: " << resultString << std::endl;
	}
	else
	{
		std::cout << "Got SDP: " << resultString << std::endl;
		m_session = MediaSession::createNew(envir(), resultString);
		m_subSessionIter = new MediaSubsessionIterator(*m_session);
		this->SendNextCommand();  
	}
	delete[] resultString;

}
void RtspClient::ContinueAfterSetup(int resultCode, char* resultString)
{
	if (resultCode != 0) 
	{
		std::cout << "Failed to SETUP: " << resultString << std::endl;
	}
	else
	{	
		//Live555CodecType codec = GetSessionCodecType(m_subSession->mediumName(), m_subSession->codecName());
		m_subSession->sink = DummySink::createNew(envir(), *m_subSession, this->url());
		((DummySink*)(m_subSession->sink))->SetFFmpeg(m_ffmpeg);
		if (m_subSession->sink == NULL) 
		{
			std::cout << "Failed to create a data sink for " << m_subSession->mediumName() << "/" << m_subSession->codecName() << " subsession: " << envir().getResultMsg() << "\n";
		}
		else
		{
			std::cout << "Created a data sink for the \"" << m_subSession->mediumName() << "/" << m_subSession->codecName() << "\" subsession";
			m_subSession->sink->startPlaying(*(m_subSession->readSource()), NULL, NULL);
		}
	}
	delete[] resultString;
	this->SendNextCommand();  
}	

void RtspClient::ContinueAfterPlay(int resultCode, char* resultString)
{
	if (resultCode != 0) 
	{
		std::cout << "Failed to PLAY: \n" << resultString << std::endl;
	}
	else
	{
		std::cout << "PLAY OK\n" << std::endl;
	}
	delete[] resultString;
}

void RtspClient::continueAfterSetup(RTSPClient* rtspClient, int resultCode, char* resultString) {
	P_THIS->ContinueAfterSetup(resultCode, resultString);
}

void RtspClient::continueAfterPlay(RTSPClient* rtspClient, int resultCode, char* resultString) {
	P_THIS->ContinueAfterPlay(resultCode, resultString);
}

void RtspClient::continueAfterDescribe(RTSPClient* rtspClient, int resultCode, char* resultString) {
	P_THIS->ContinueAfterDescribe(resultCode, resultString);
}

void RtspClient::SendNextCommand() 
{
	if (m_subSessionIter == NULL)
	{
		// no SDP, send DESCRIBE
		this->sendDescribeCommand(continueAfterDescribe, &m_Authenticator); 
	}
	else
	{
		m_subSession = m_subSessionIter->next();
		if (m_subSession != NULL) 
		{
			// still subsession to SETUP
			if (!m_subSession->initiate()) 
			{
				std::cout << "Failed to initiate " << m_subSession->mediumName() << "/" << m_subSession->codecName() << " subsession: " << envir().getResultMsg() << std::endl;
				this->SendNextCommand();
			} 
			else 
			{					
				std::cout << "Initiated " << m_subSession->mediumName() << "/" << m_subSession->codecName() << " subsession" << std::endl;
			}

			/* Change the multicast here */
			this->sendSetupCommand(*m_subSession, continueAfterSetup, False, m_isTcp, False, &m_Authenticator);
		}
		else
		{
			// no more subsession to SETUP, send PLAY
			this->sendPlayCommand(*m_session, continueAfterPlay, (double)0, (double)-1, (float)0, &m_Authenticator);
		}
	}
}

void RtspThread::Run() {
	std::thread threadPlay(std::mem_fn(&RtspThread::OpenCameraPlay), this);
	threadPlay.detach();
}

void RtspThread::OpenCameraPlay() 
{
	ffmpegH264 = new FFH264();
	if(!ffmpegH264->InitH264DecodeEnv()) 
	{
		std::cout << "Error:----> FFmpeg AV_CODEC_ID_H264 Init Error" << std::endl;
		return;
	}

	m_scheduler = BasicTaskScheduler::createNew();
	m_env = BasicUsageEnvironment::createNew(*m_scheduler);
	
	RtspClient* m_rtspClient = new RtspClient(*m_env, m_url.c_str(), m_user.c_str(), m_passwd.c_str(), &ffmpegH264);
	m_rtspClient->Play();
	char eventLoopWatchVariable = 0;
	m_env->taskScheduler().doEventLoop(&eventLoopWatchVariable);
}


