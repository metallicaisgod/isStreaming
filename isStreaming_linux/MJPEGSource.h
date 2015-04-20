#pragma once

#ifndef _JPEG_VIDEO_SOURCE_HH
#include "JPEGVideoSource.hh"
#endif

//#include "uEye.h"
#include "isStreaming.h"
extern "C" {
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
}

#define JPEG_HEADER_SIZE 623

class MJPEGSource :
	public JPEGVideoSource
{

public:
	static MJPEGSource* createNew(UsageEnvironment& env,
		unsigned timePerFrame);
	// "timePerFrame" is in microseconds
	int InitEncoder(IS_STREAM_VIDEO *);
	void CloseEncoder();
	bool DeliverFrame(IS_STREAM_PAYLOAD_DATA*);
	int findActualSizeOfFrame();

protected:
	MJPEGSource(UsageEnvironment& env, unsigned timePerFrame);
	// called only by createNew()
	virtual ~MJPEGSource();

private:
	bool deliverFrameToClient();
	int EncodeToJPEG();

	// redefined virtual functions:
	virtual void doGetNextFrame();
	virtual u_int8_t type();
	virtual u_int8_t qFactor();
	virtual u_int8_t width();
	virtual u_int8_t height();

	// camera variables
	//HIDS	m_hCam;					// handle to camera
	//INT		m_nColorMode;			// Y8/RGB16/RGB24/REG32
	//INT		m_nBitsPerPixel;		// number of bits needed store one pixel
	//INT		m_nSizeX;				// width of video 
	//INT		m_nSizeY;				// height of video
	//SENSORINFO m_sInfo;

	// memory and sequence buffers
    //INT		m_lMemoryId;	// camera memory - buffer ID
	char*	m_pcImageMemory;		// camera memory - pointer to buffer
	char*   m_pcEncodedImage;
	int	encodedImageSize;
	int sourceImageSize;

	//void GetMaxImageSize(INT *pnSizeX, INT *pnSizeY);

	// camera functions
	//bool CamOpen();
	//INT InitCamera(HIDS *);

	//encoder
	AVCodecContext *m_pCodecCtx;
	AVCodec *m_pCodec;
	AVFrame * m_pFrame;
	AVFrame * m_pSourceFrame;
	SwsContext * sws_cntxt;

	IS_STREAM_VIDEO * m_pStreamVideo;

private:
	unsigned fTimePerFrame;
	struct timeval fLastCaptureTime;
	u_int8_t fLastQFactor, fLastWidth, fLastHeight;
	Boolean fNeedAFrame;
	unsigned char fJPEGHeader[JPEG_HEADER_SIZE];

};

