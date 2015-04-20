#include "MJPEGSource.h"
#ifndef _GROUPSOCK_HELPER_HH
#include "GroupsockHelper.hh"
#endif

MJPEGSource* MJPEGSource::createNew(UsageEnvironment& env, unsigned timePerFrame) 
{
	return new MJPEGSource(env, timePerFrame);
}

//static timezone Idunno;

MJPEGSource::MJPEGSource(UsageEnvironment& env, unsigned timePerFrame)
	: JPEGVideoSource(env),
	fTimePerFrame(timePerFrame), fNeedAFrame(False) ,
	m_pcEncodedImage(NULL), m_pcImageMemory(NULL),
	m_pFrame(NULL), m_pSourceFrame(NULL),
	encodedImageSize(0), sourceImageSize(0), m_pCodecCtx(NULL)
{
	av_register_all();

}


MJPEGSource::~MJPEGSource()
{
	CloseEncoder();
}


//bool MJPEGSource::CamOpen()
//{
//	INT nRet = IS_NO_SUCCESS;
//
//	// init camera (open next available camera)
//	m_hCam = (HIDS)0;
//	nRet = InitCamera(&m_hCam);
//	if (nRet == IS_SUCCESS)
//	{
//		// Get sensor info
//		is_GetSensorInfo(m_hCam, &m_sInfo);
//
//		GetMaxImageSize(&m_nSizeX, &m_nSizeY);
//
//		// setup image size
//		IS_SIZE_2D imageSize;
//		imageSize.s32Width = m_nSizeX;
//		imageSize.s32Height = m_nSizeY;
//
//		is_AOI(m_hCam, IS_AOI_IMAGE_SET_SIZE, (void*)&imageSize, sizeof(imageSize));
//
//		// use color depth according to monochrome or color camera
//		if (m_sInfo.nColorMode == IS_COLORMODE_MONOCHROME)
//		{
//			// monochrome camera
//			m_nBitsPerPixel = 8;
//			is_SetColorMode(m_hCam, IS_CM_MONO8);
//		}
//		else
//		{
//			// color camera
//			m_nBitsPerPixel = 24;
//			INT ret = is_SetColorMode(m_hCam, IS_CM_BGR8_PACKED);
//
//			// enable auto whitebalance	
//			UEYE_AUTO_INFO autoInfo;
//			double dblVal = 1.0;
//			if (is_GetAutoInfo(m_hCam, &autoInfo) == IS_SUCCESS)
//			{
//				// sensor whitebalance is supported
//				if (autoInfo.AutoAbility & AC_SENSOR_WB)
//				{
//					is_SetAutoParameter(m_hCam, IS_SET_ENABLE_AUTO_SENSOR_WHITEBALANCE, &dblVal, NULL);
//				}
//				// sensor whitebalance is not supported
//				else
//				{
//					if (autoInfo.AutoAbility & AC_WHITEBAL)
//					{
//						// Try to activate software whitebalance
//						is_SetAutoParameter(m_hCam, IS_SET_ENABLE_AUTO_WHITEBALANCE, &dblVal, NULL);
//					}
//				}
//			}
//		}
//
//
//
//		if (m_pcImageMemory != NULL)
//		{
//			is_FreeImageMem(m_hCam, m_pcImageMemory, m_lMemoryId);
//		}
//		m_pcImageMemory = NULL;
//
//		is_AllocImageMem(m_hCam, m_nSizeX, m_nSizeY, m_nBitsPerPixel, &m_pcImageMemory, &m_lMemoryId);
//		is_SetImageMem(m_hCam, m_pcImageMemory, m_lMemoryId); // grab into this buffer
//
//		//doGetNextFrame();
//
//		return true;
//	}
//	else
//	{
//		return false;
//	}
//
//}
//
//INT MJPEGSource::InitCamera(HIDS * hCam)
//{
//	INT nRet = is_InitCamera(hCam, NULL);
//	/************************************************************************************************/
//	/*                                                                                              */
//	/*  If the camera returns with "IS_STARTER_FW_UPLOAD_NEEDED", an upload of a new firmware       */
//	/*  is necessary. This upload can take several seconds. We recommend to check the required      */
//	/*  time with the function is_GetDuration().                                                    */
//	/*                                                                                              */
//	/*  In this case, the camera can only be opened if the flag "IS_ALLOW_STARTER_FW_UPLOAD"        */
//	/*  is "OR"-ed to m_hCam. This flag allows an automatic upload of the firmware.                 */
//	/*                                                                                              */
//	/************************************************************************************************/
//	if (nRet == IS_STARTER_FW_UPLOAD_NEEDED)
//	{
//		// Time for the firmware upload = 25 seconds by default
//		INT nUploadTime = 25000;
//		is_GetDuration(*hCam, IS_STARTER_FW_UPLOAD, &nUploadTime);
//
//		//CString Str1, Str2, Str3;
//		//Str1 = L"This camera requires a new firmware. The upload will take about";
//		//Str2 = L"seconds. Please wait ...";
//		//Str3.Format(L"%s %d %s", Str1, nUploadTime / 1000, Str2);
//		//AfxMessageBox(Str3, MB_ICONWARNING);
//
//		// Try again to open the camera. This time we allow the automatic upload of the firmware by
//		// specifying "IS_ALLOW_STARTER_FIRMWARE_UPLOAD"
//		*hCam = (HIDS)(((INT)*hCam) | IS_ALLOW_STARTER_FW_UPLOAD);
//		nRet = is_InitCamera(hCam, NULL);
//	}
//	return nRet;
//}

void MJPEGSource::doGetNextFrame() {


	fNeedAFrame = True;

	fDurationInMicroseconds = fTimePerFrame;

}

bool MJPEGSource::DeliverFrame(IS_STREAM_PAYLOAD_DATA* payloadData)
{
	if (fNeedAFrame)
	{
		if (!m_pcImageMemory)
			return false;
		if (sourceImageSize < payloadData->cbSizeOfData)
			return false;

        gettimeofday(&fLastCaptureTime, NULL);
		fPresentationTime = fLastCaptureTime;
		payloadData->timeStamp.tv_sec = fPresentationTime.tv_sec;
		payloadData->timeStamp.tv_usec = fPresentationTime.tv_usec;
		
		memcpy(m_pcImageMemory, payloadData->pData, payloadData->cbSizeOfData);
		return deliverFrameToClient();
	}
	return false;
}

bool MJPEGSource::deliverFrameToClient() 
{
	fNeedAFrame = False;

	int size = EncodeToJPEG();
	if (size < JPEG_HEADER_SIZE)
		return false;

	memcpy(fJPEGHeader, m_pcEncodedImage, JPEG_HEADER_SIZE);

	for (int i = 0; i < JPEG_HEADER_SIZE - 8; ++i) {
		if (fJPEGHeader[i] == 0xFF && fJPEGHeader[i + 1] == 0xC0)
		{
			fLastHeight = (fJPEGHeader[i + 5] << 5) | (fJPEGHeader[i + 6] >> 3);
			fLastWidth = (fJPEGHeader[i + 7] << 5) | (fJPEGHeader[i + 8] >> 3);
			break;
		}
	}

	// Then, the JPEG payload:
    fFrameSize = size - (JPEG_HEADER_SIZE - 160);
	if (fFrameSize <= fMaxSize)
        memcpy(fTo, m_pcEncodedImage + (JPEG_HEADER_SIZE - 160), fFrameSize);

	nextTask() = envir().taskScheduler().scheduleDelayedTask(0,
		(TaskFunc*)FramedSource::afterGetting, this);
	return true;
}

int MJPEGSource::InitEncoder(IS_STREAM_VIDEO * streamVideo)
{
	CloseEncoder();

	if (streamVideo->codecDest != IS_STREAM_CODEC_MJPEG)
		return -1;

	m_pStreamVideo = (IS_STREAM_VIDEO*)malloc(sizeof(IS_STREAM_VIDEO));
	memcpy(m_pStreamVideo, streamVideo, sizeof(IS_STREAM_VIDEO));

    m_pCodec = avcodec_find_encoder(CODEC_ID_MJPEG);
	m_pCodecCtx = avcodec_alloc_context3(m_pCodec);
    avcodec_get_context_defaults3(m_pCodecCtx, m_pCodec);

	//m_pCodecCtx->bit_rate = 1000000;
	//m_pCodecCtx->bit_rate_tolerance = 4000000;
	/* resolution must be a multiple of two */
	m_pCodecCtx->width = streamVideo->nWidthDest;
	m_pCodecCtx->height = streamVideo->nHeightDest;
	fLastQFactor = 75;
    m_pCodecCtx->qmin = 3;
	if (streamVideo->quality == IS_STREAM_QUALITY_GOOD)
        m_pCodecCtx->qmin = 6;//fLastQFactor = 50;
	else if (streamVideo->quality == IS_STREAM_QUALITY_BETTER)
        m_pCodecCtx->qmin = 4;// fLastQFactor = 75;
	else if (streamVideo->quality == IS_STREAM_QUALITY_BEST)
        m_pCodecCtx->qmin = 2;// fLastQFactor = 100;
	//m_pCodecCtx->qmin = 32 - fLastQFactor * 31 / 100;
	m_pCodecCtx->qmax = m_pCodecCtx->qmin;
	m_pCodecCtx->max_qdiff = 0;

	m_pCodecCtx->time_base.den = (int)(streamVideo->nFramerate * 100);
	m_pCodecCtx->time_base.num = 100;

    m_pCodecCtx->pix_fmt = PIX_FMT_YUVJ420P;

	if (avcodec_open2(m_pCodecCtx, m_pCodec, NULL))
		return -2;

    m_pFrame = avcodec_alloc_frame();
	if (!m_pFrame) {
		return -3;
	}

	int pictsize = avpicture_fill((AVPicture *)m_pFrame, NULL/*picture_buf*/,
		m_pCodecCtx->pix_fmt, m_pCodecCtx->width, m_pCodecCtx->height);
	m_pFrame->pict_type = AV_PICTURE_TYPE_I;

	//AVPixelFormat scale_fmt;
	//if (m_nBitsPerPixel == 8)
	//	scale_fmt = PIX_FMT_GRAY8;
	//else if (m_nBitsPerPixel == 24)
	//	scale_fmt = PIX_FMT_BGR24;

	sws_cntxt = sws_getCachedContext(NULL,
		streamVideo->nWidthSource, streamVideo->nHeightSource,
        (PixelFormat)streamVideo->pixelfmtSource,
		m_pCodecCtx->width, m_pCodecCtx->height,//m_MC.frame_size.cx, m_MC.frame_size.cy,//
        m_pCodecCtx->pix_fmt, SWS_BICUBIC,
		NULL, NULL, NULL);

	m_pFrame->data[0] = (uint8_t *)av_malloc(m_pFrame->linesize[0] * m_pCodecCtx->height);
	m_pFrame->data[1] = (uint8_t *)av_malloc(m_pFrame->linesize[1] * m_pCodecCtx->height);
	m_pFrame->data[2] = (uint8_t *)av_malloc(m_pFrame->linesize[2] * m_pCodecCtx->height);

    m_pSourceFrame = avcodec_alloc_frame();
	avpicture_fill((AVPicture *)m_pSourceFrame, NULL/*picture_buf*/,
        (PixelFormat)streamVideo->pixelfmtSource, streamVideo->nWidthSource, streamVideo->nHeightSource);
    m_pSourceFrame->height = streamVideo->nHeightSource;

	if (!m_pcImageMemory)
	{
        sourceImageSize = avpicture_get_size((PixelFormat)streamVideo->pixelfmtSource, streamVideo->nWidthSource, streamVideo->nHeightSource);
		m_pcImageMemory = (char*)av_malloc(sourceImageSize);
	}

	if (!m_pcEncodedImage)
	{
		encodedImageSize = avpicture_get_size(m_pCodecCtx->pix_fmt, m_pCodecCtx->width, m_pCodecCtx->height);
		m_pcEncodedImage = (char*)av_malloc(encodedImageSize);
	}
	return 0;

}

int MJPEGSource::findActualSizeOfFrame()
{
	return encodedImageSize;
}

void MJPEGSource::CloseEncoder()
{
	if (m_pCodecCtx)
    {
        avcodec_close(m_pCodecCtx);
        av_free(m_pCodecCtx);
    }
	m_pCodecCtx = NULL;
	if (m_pFrame)
	{
		for (int i = 0; i < 8; i++)
		{
			if (m_pFrame->data[i])
				av_free(m_pFrame->data[i]);
		}
		av_free(m_pFrame);
	}
	m_pFrame = NULL;
	if (m_pSourceFrame)
	{
		av_free(m_pSourceFrame);
	}
	m_pSourceFrame = NULL;
	if (m_pStreamVideo)
		free(m_pStreamVideo);
	m_pStreamVideo = NULL;
	if (m_pcImageMemory)
		av_free(m_pcImageMemory);
	m_pcImageMemory = NULL;

	if(m_pcEncodedImage)
		av_free(m_pcEncodedImage);
	m_pcEncodedImage = NULL; 
}

int MJPEGSource::EncodeToJPEG()
{
	m_pSourceFrame->data[0] = (uint8_t*)m_pcImageMemory;
    int ret = sws_scale(sws_cntxt, m_pSourceFrame->data, m_pSourceFrame->linesize, 0, m_pSourceFrame->height, m_pFrame->data, m_pFrame->linesize);
    //if (ret != m_pCodecCtx->height)
    //	return 0;

	AVPacket  pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;
	int got_pkt_ptr;

	//int ret = avcodec_encode_video(m_pCodecCtx, (uint8_t*)m_pcEncodedImage, encodedImageSize, frame, &got_pkt_ptr);
	ret = avcodec_encode_video2(m_pCodecCtx, &pkt, m_pFrame, &got_pkt_ptr);
	if (!got_pkt_ptr)
		return 0;
	int size = 0;
	if (encodedImageSize >= pkt.size)
	{
		memcpy(m_pcEncodedImage, pkt.data, pkt.size);
		size = pkt.size;
	}
	av_free_packet(&pkt);

	return size;
}

u_int8_t MJPEGSource::type() {
	return 1;
}
u_int8_t MJPEGSource::qFactor() {
	return fLastQFactor;
}
u_int8_t MJPEGSource::width() {
	return fLastWidth;//fLastWidth;
}
u_int8_t MJPEGSource::height() {
	return fLastHeight;//fLastHeight;
}

//void MJPEGSource::GetMaxImageSize(INT *pnSizeX, INT *pnSizeY)
//{
//	// Check if the camera supports an arbitrary AOI
//	// Only the ueye xs does not support an arbitrary AOI
//	INT nAOISupported = 0;
//	BOOL bAOISupported = TRUE;
//	if (is_ImageFormat(m_hCam,
//		IMGFRMT_CMD_GET_ARBITRARY_AOI_SUPPORTED,
//		(void*)&nAOISupported,
//		sizeof(nAOISupported)) == IS_SUCCESS)
//	{
//		bAOISupported = (nAOISupported != 0);
//	}
//
//	if (bAOISupported)
//	{
//		// All other sensors
//		// Get maximum image size
//		SENSORINFO sInfo;
//		is_GetSensorInfo(m_hCam, &sInfo);
//		*pnSizeX = sInfo.nMaxWidth;
//		*pnSizeY = sInfo.nMaxHeight;
//	}
//	else
//	{
//		// Only ueye xs
//		// Get image size of the current format
//		IS_SIZE_2D imageSize;
//		is_AOI(m_hCam, IS_AOI_IMAGE_GET_SIZE, (void*)&imageSize, sizeof(imageSize));
//
//		*pnSizeX = imageSize.s32Width;
//		*pnSizeY = imageSize.s32Height;
//	}
//}
