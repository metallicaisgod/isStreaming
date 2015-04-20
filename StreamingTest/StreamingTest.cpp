// StreamingTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "uEye.h"
#include "isStreaming.h"

HIDS	m_hCam;					// handle to camera

INT InitCamera(HIDS * hCam)
{
	INT nRet = is_InitCamera(hCam, NULL);
	/************************************************************************************************/
	/*                                                                                              */
	/*  If the camera returns with "IS_STARTER_FW_UPLOAD_NEEDED", an upload of a new firmware       */
	/*  is necessary. This upload can take several seconds. We recommend to check the required      */
	/*  time with the function is_GetDuration().                                                    */
	/*                                                                                              */
	/*  In this case, the camera can only be opened if the flag "IS_ALLOW_STARTER_FW_UPLOAD"        */
	/*  is "OR"-ed to m_hCam. This flag allows an automatic upload of the firmware.                 */
	/*                                                                                              */
	/************************************************************************************************/
	if (nRet == IS_STARTER_FW_UPLOAD_NEEDED)
	{
		// Time for the firmware upload = 25 seconds by default
		INT nUploadTime = 25000;
		is_GetDuration(*hCam, IS_STARTER_FW_UPLOAD, &nUploadTime);

		//CString Str1, Str2, Str3;
		//Str1 = L"This camera requires a new firmware. The upload will take about";
		//Str2 = L"seconds. Please wait ...";
		//Str3.Format(L"%s %d %s", Str1, nUploadTime / 1000, Str2);
		//AfxMessageBox(Str3, MB_ICONWARNING);

		// Try again to open the camera. This time we allow the automatic upload of the firmware by
		// specifying "IS_ALLOW_STARTER_FIRMWARE_UPLOAD"
		*hCam = (HIDS)(((INT)*hCam) | IS_ALLOW_STARTER_FW_UPLOAD);
		nRet = is_InitCamera(hCam, NULL);
	}
	return nRet;
}

void GetMaxImageSize(INT *pnSizeX, INT *pnSizeY)
{
	// Check if the camera supports an arbitrary AOI
	// Only the ueye xs does not support an arbitrary AOI
	INT nAOISupported = 0;
	BOOL bAOISupported = TRUE;
	if (is_ImageFormat(m_hCam,
		IMGFRMT_CMD_GET_ARBITRARY_AOI_SUPPORTED,
		(void*)&nAOISupported,
		sizeof(nAOISupported)) == IS_SUCCESS)
	{
		bAOISupported = (nAOISupported != 0);
	}

	if (bAOISupported)
	{
		// All other sensors
		// Get maximum image size
		SENSORINFO sInfo;
		is_GetSensorInfo(m_hCam, &sInfo);
		*pnSizeX = sInfo.nMaxWidth;
		*pnSizeY = sInfo.nMaxHeight;
	}
	else
	{
		// Only ueye xs
		// Get image size of the current format
		IS_SIZE_2D imageSize;
		is_AOI(m_hCam, IS_AOI_IMAGE_GET_SIZE, (void*)&imageSize, sizeof(imageSize));

		*pnSizeX = imageSize.s32Width;
		*pnSizeY = imageSize.s32Height;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	// camera variables
	
	//INT		m_nColorMode;			// Y8/RGB16/RGB24/REG32
	INT		m_nBitsPerPixel;		// number of bits needed store one pixel
	INT		m_nSizeX;				// width of video 
	INT		m_nSizeY;				// height of video
	SENSORINFO m_sInfo;

	// memory and sequence buffers
	INT		m_lMemoryId = 0;	// camera memory - buffer ID
	char*	m_pcImageMemory = NULL;		// camera memory - pointer to buffer
	//char*   m_pcEncodedImage;

	INT nRet = IS_NO_SUCCESS;

	// init camera (open next available camera)
	m_hCam = (HIDS)0;
	int fps = 10;
	nRet = InitCamera(&m_hCam);
	if (nRet == IS_SUCCESS)
	{
		// Get sensor info
		is_GetSensorInfo(m_hCam, &m_sInfo);

		GetMaxImageSize(&m_nSizeX, &m_nSizeY);
		

		//is_GetFramesPerSecond(m_hCam, &fps);

		// setup image size
		IS_SIZE_2D imageSize;
		imageSize.s32Width = m_nSizeX;
		imageSize.s32Height = m_nSizeY;

		is_AOI(m_hCam, IS_AOI_IMAGE_SET_SIZE, (void*)&imageSize, sizeof(imageSize));

		// use color depth according to monochrome or color camera
		if (m_sInfo.nColorMode == IS_COLORMODE_MONOCHROME)
		{
			// monochrome camera
			m_nBitsPerPixel = 8;
			is_SetColorMode(m_hCam, IS_CM_MONO8);
		}
		else
		{
			// color camera
			m_nBitsPerPixel = 24;
			INT ret = is_SetColorMode(m_hCam, IS_CM_BGR8_PACKED);

			// enable auto whitebalance	
			UEYE_AUTO_INFO autoInfo;
			double dblVal = 1.0;
			if (is_GetAutoInfo(m_hCam, &autoInfo) == IS_SUCCESS)
			{
				// sensor whitebalance is supported
				if (autoInfo.AutoAbility & AC_SENSOR_WB)
				{
					is_SetAutoParameter(m_hCam, IS_SET_ENABLE_AUTO_SENSOR_WHITEBALANCE, &dblVal, NULL);
				}
				// sensor whitebalance is not supported
				else
				{
					if (autoInfo.AutoAbility & AC_WHITEBAL)
					{
						// Try to activate software whitebalance
						is_SetAutoParameter(m_hCam, IS_SET_ENABLE_AUTO_WHITEBALANCE, &dblVal, NULL);
					}
				}
			}
		}



		if (m_pcImageMemory != NULL)
		{
			is_FreeImageMem(m_hCam, m_pcImageMemory, m_lMemoryId);
		}
		m_pcImageMemory = NULL;

		is_AllocImageMem(m_hCam, m_nSizeX, m_nSizeY, m_nBitsPerPixel, &m_pcImageMemory, &m_lMemoryId);
		is_SetImageMem(m_hCam, m_pcImageMemory, m_lMemoryId); // grab into this buffer
		
	

		//subsection step1 Step 1: Initialize the stream
		
			 // This will create a rtsp server which will listen to port 1554
			// Standard port is 8554
		unsigned int port = 8554;
		is_Stream(IS_STREAM_INIT, (void*)&port, sizeof(port));

		//subsection step2 Step 2: Create a stream
		// This creates a new stream
		// The stream can be accessed by rtsp://ip-to-host:port/myStream
		IS_STREAM_INFO streamInfo;
		streamInfo.cbSizeOfStruct = sizeof(IS_STREAM_INFO);
		strcpy(streamInfo.strDescription, "Description");
		strcpy(streamInfo.strInfo, "Information");
		strcpy(streamInfo.strName, "ueye_cockpit_stream");
	
		is_Stream(IS_STREAM_CREATE, (void*)&streamInfo, sizeof(IS_STREAM_INFO));
	
		//Now we save the uuid to identify the stream at a later time
		IS_GUID uuid;
		memcpy(uuid, streamInfo.uuidStream, sizeof(IS_GUID));

		//subsection step3 Step 3: Create and add a sub stream
		// It is a video stream, specify the video parameter
		IS_STREAM_VIDEO videoParam;
		videoParam.cbSizeOfStruct = sizeof(IS_STREAM_VIDEO);
		videoParam.codecDest = IS_STREAM_CODEC_MJPEG;
		videoParam.nFramerate = fps;
		videoParam.nHeightDest = m_nSizeY;
		videoParam.nHeightSource = m_nSizeY;
		videoParam.nWidthDest = m_nSizeX;
		videoParam.nWidthSource = m_nSizeX;
		videoParam.pixelfmtSource = IS_STREAM_FMT_NONE;
		if (m_nBitsPerPixel == 24)
			videoParam.pixelfmtSource = IS_STREAM_FMT_BGR24;
		// create the sub stream info struct
		IS_STREAM_SUBSTREAM_INFO subStreamInfo;
		subStreamInfo.cbSizeOfStruct = sizeof(IS_STREAM_SUBSTREAM_INFO);
		subStreamInfo.nType = IS_STREAM_TYPE_VIDEO;
		// We set the stream uuid
		memcpy(subStreamInfo.uuidParentStream, uuid, sizeof(IS_GUID));
		subStreamInfo.cbSizeOfSubstream = sizeof(IS_STREAM_VIDEO);
		subStreamInfo.pSubstream = &videoParam;
	
		is_Stream(IS_STREAM_CREATE_SUBSTREAM, (void*)&subStreamInfo, sizeof(IS_STREAM_SUBSTREAM_INFO));
		//Now we save the substream uuid to identify the stream at a later time
		IS_GUID subid;
		memcpy(subid, subStreamInfo.uuidSubstreamStream, sizeof(IS_GUID));
		//subsection step4 Step 4: Deliver data to the sub stream
		// Create a struct for the payload data

		for (int i = 0; i < 70; i++)
		{
			//Sleep(int(1000 / fps));
			INT ret = is_FreezeVideo(m_hCam, IS_WAIT);
			INT pitch;
			is_GetImageMemPitch(m_hCam, &pitch);
			IS_STREAM_PAYLOAD_DATA data;
			data.cbSizeOfStruct = sizeof(IS_STREAM_PAYLOAD_DATA);
			int sizeOfmyData = pitch * m_nSizeY;
			data.pData = (unsigned char *)m_pcImageMemory;
			data.cbSizeOfData = sizeOfmyData;
			// We set the stream uuid and sub stream uuid
			memcpy(data.uuidStream, uuid, sizeof(IS_GUID));
			memcpy(data.uuidSubStream, subid, sizeof(IS_GUID));
			data.nType = IS_STREAM_TYPE_VIDEO;

			is_Stream(IS_STREAM_DELIVER_DATA, (void*)&data, sizeof(IS_STREAM_PAYLOAD_DATA));
		}
		Sleep(int(1000 / fps));
		is_Stream(IS_STREAM_EXIT, NULL, 0);

		// exit camera 
		if (m_hCam)
		{
			// stop capture
			is_StopLiveVideo(m_hCam, IS_FORCE_VIDEO_STOP);

			// memory and events are automatically released
			is_ExitCamera(m_hCam);
		}
	}
	return 0;
}

