#ifndef ISSTREAMING_H
#define ISSTREAMING_H

#ifdef ISSTREAMING_EXPORTS
#define ISSTREAMING_API __attribute__((visibility("default")))
#else
#define  ISSTREAMING_API __attribute__((visibility("default")))
#endif

/*! \mainpage isStreaming
 *
 * \section intro_sec Introduction
 *
 * isStreaming is a library for a standard RTP/RTCP multimedia stream. This library can be used to build streaming applications.
 * The library can be used to stream MJPEG and H.264 video. A H.264 video stream is implemented through the x264.exe.
 * Audio is at the moment not supported.
 *
 * \section install_sec How to use the library
 *
 * \subsection step1 Step 1: Initialize the stream
 * \code
 *
 * // This will create a rtsp server which will listen to port 1554
 * // Standard port is 8554
 * unsigned int port = 1554;
 * is_Stream(IS_STREAM_INIT, (void*)&port, sizeof(port));
 *
 * \endcode
 * \subsection step2 Step 2: Create a stream
 *	\code
 * // This creates a new stream
 * // The stream can be accessed by rtsp://ip-to-host:port/myStream
 * IS_STREAM_INFO streamInfo;
 * streamInfo.cbSizeOfStruct = sizeof(IS_STREAM_INFO);
 * strcpy(streamInfo.strDescription, "Description");
 * strcpy(streamInfo.strInfo, "Information");
 * strcpy(streamInfo.strName, "myStream");
 * 
 * is_Stream(IS_STREAM_CREATE, (void*)&streamInfo, sizeof(IS_STREAM_INFO));
 *
 * //Now we save the uuid to identify the stream at a later time
 * IS_GUID uuid;
 * memcpy(uuid, streamInfo.uuidStream, sizeof(IS_GUID));
 *
 * \endcode
 * \subsection step3 Step 3: Create and add a sub stream
 * \code
 *
 * // It is a video stream, specify the video parameter
 * IS_STREAM_VIDEO videoParam;
 * videoParam.cbSizeOfStruct = sizeof(IS_STREAM_VIDEO);
 * videoParam.codecDest = IS_STREAM_CODEC_H264;
 * videoParam.nFramerate = 10;
 * videoParam.nHeightDest = 512;
 * videoParam.nHeightSource = 1024;
 * videoParam.nWidthDest = 640;
 * videoParam.nWidthSource = 1280;
 * videoParam.pixelfmtSource = IS_STREAM_FMT_ABGR;
 *
 * // create the sub stream info struct
 * IS_STREAM_SUBSTREAM_INFO subStreamInfo;
 * subStreamInfo.cbSizeOfStruct = sizeof(IS_STREAM_SUBSTREAM_INFO);
 * subStreamInfo.nType = IS_STREAM_TYPE_VIDEO;
 * 
 * // We set the stream uuid
 * memcpy(subStreamInfo.uuidParentStream, uuid, sizeof(IS_GUID));
 * subStreamInfo.cbSizeOfSubstream = sizeof(IS_STREAM_VIDEO);
 * subStreamInfo.pSubstream = &videoParam;
 *
 * is_Stream(IS_STREAM_CREATE_SUBSTREAM, (void*)&subStreamInfo, sizeof(IS_STREAM_SUBSTREAM_INFO));
 *
 * //Now we save the substream uuid to identify the stream at a later time
 * IS_GUID subid;
 * memcpy(subid, subStreamInfo.uuidSubstreamStream, sizeof(IS_GUID));
 *
 * \endcode
 * \subsection step4 Step 4: Deliver data to the sub stream
 * \code
 *
 * // Create a struct for the payload data
 * IS_STREAM_PAYLOAD_DATA data;
 * data.cbSizeOfStruct = sizeof(IS_STREAM_PAYLOAD_DATA);
 * data.pData = myData;
 * data.cbSizeOfData = sizeOfmyData;
 *
 * // We set the stream uuid and sub stream uuid
 * memcpy(data.uuidStream, uuid, sizeof(IS_GUID));
 * memcpy(data.uuidSubStream, subid, sizeof(IS_GUID));
 * data.nType = IS_STREAM_TYPE_VIDEO;
 *
 * is_Stream(IS_STREAM_DELIVER_DATA, (void*)&data, sizeof(IS_STREAM_PAYLOAD_DATA));
 *
 * \endcode
 */

typedef unsigned char IS_GUID[16];

// Streaming errors
#define IS_STREAM_NO_SUCCESS -1 // Unknown error
#define IS_STREAM_SUCCESS 0
#define IS_STREAM_NOT_INITIALIZED 1
#define IS_STREAM_ALREADY_INITIALIZED 2
#define IS_STREAM_PORT_IN_USE 3
#define IS_STREAM_NAME_IN_USE 4
#define IS_STREAM_INVALID_STREAM_UUID 5
#define IS_STREAM_INVALID_SUBSTREAM_UUID 6
#define IS_STREAM_ENCODER_WRONG_SIZE 7
#define IS_STREAM_ENCODER_ERROR 8
#define IS_STREAM_INVALID_PARAMETER 9
#define IS_STREAM_X264_ERROR 10

/*!
 * \brief Enumeration of various pixel formats.
 */
typedef enum E_STREAM_PIXELFMT 
{ 
	/*! Unkown pixel format, library will not accept this pixel format */
    IS_STREAM_FMT_NONE = -1,
    IS_STREAM_FMT_ABGR = 30,
    IS_STREAM_FMT_ARGB = 27,
    IS_STREAM_FMT_RGBA = 28,
    IS_STREAM_FMT_BGRA = 29, 
    IS_STREAM_FMT_RGB24 = 2,
    IS_STREAM_FMT_BGR24 = 3,
    IS_STREAM_FMT_YUVJ422P = 13,
    IS_STREAM_FMT_YUVJ420P = 15,
    IS_STREAM_FMT_YUV420P = 0,
    IS_STREAM_FMT_YUV422P = 4           
} IS_STREAM_PIXELFMT;

/*! Not implemented yet
 * \brief Enumeration of various codec types.
 */
typedef enum E_STREAM_QUALITY
{
    IS_STREAM_QUALITY_GOOD = 0,
    IS_STREAM_QUALITY_BETTER = 1,
    IS_STREAM_QUALITY_BEST = 2
} IS_STREAM_QUALITY;

/*!
 * \brief Enumeration of various codec types.
 */
typedef enum E_STREAM_CODEC 
{ 
	/*! Use this if incoming data has no codec*/
    IS_STREAM_CODEC_NONE = 0,
	/*! Library will stream in H264 */
    IS_STREAM_CODEC_H264 = 28,
	/*! Library will stream in motion jpeg */
    IS_STREAM_CODEC_MJPEG = 8,
    
} IS_STREAM_CODEC;

/*!
 * \brief Enumeration of various stream types.
 */
typedef enum E_STREAM_TYPES 
{
    IS_STREAM_TYPE_VIDEO = 0,
    IS_STREAM_TYPE_AUDIO = 1

} IS_STREAM_TYPES;

/*!
 * \brief Enumeration of commands supported by the stream access function, \see is_Stream.
 */
typedef enum E_STREAM_COMMAND 
{
	/*!
	 * \brief Initialize the stream and creates a Rtsp server.
	 * \note This command must be called before any other command.
	 * \note The standard port the Rtsp server will listen to is 8554.
	 */
    IS_STREAM_INIT,
	/*!
	 * \brief Tells the stream library which function to call if any data is received.
	 * \note The callback function have to defined as: void callback(IS_STREAM_CONFIG_DATA *data).
	 * \note typedef void (*IS_CALLBACK)(IS_STREAM_CONFIG_DATA *data);
	 */
    IS_STREAM_SET_CALLBACK_FUNCTION,
	/*!
	 * \brief Deinitialize the stream.
	 * \note This command should be called to close the rtsp server.
	 */
    IS_STREAM_EXIT,
	/*!
	 * \brief Initialize the command server.
	 * This command should be called to listen to a new connection.
	 * \note If a client sends a command the callback function will be called /see IS_STREAM_SET_CALLBACK_FUNCTION.
	 * \note The standard port the Rtsp server will listen to is 9554.
	 */
    IS_STREAM_START_LISTEN,
	/*!
	 * \brief Deinitialize the command server.
	 * \note This command stops the command server and new connections are not anymore accepted.
	 */
    IS_STREAM_STOP_LISTEN,
	/*!
	 * \brief Creates a new stream.
	 * \note The stream is accessed through the name of the stream
	 * \note The param is a pointer to \see IS_STREAM_INFO.
	 */
    IS_STREAM_CREATE,
	/*!
	 * \brief Creates a sub stream an adds the new sub stream to the previously created stream.
	 * \note The previously created stream is identified by the \see IS_GUID.
	 * \note The param is a pointer to \see IS_STREAM_SUBSTREAM_INFO.
	 */
    IS_STREAM_CREATE_SUBSTREAM,
	/*!
	 * \brief Sends any data to the client.
	 * \note The param is a pointer to \see IS_STREAM_CONFIG_DATA.
	 */
    IS_STREAM_SEND_DATA,
	/*!
	 * \brief Tells the stream that new data is avaiable
	 * \note The param is a pointer to \see IS_STREAM_PAYLOAD_DATA
	 */
    IS_STREAM_DELIVER_DATA,

    IS_STREAM_SEND_CLIENT_COMMAND,

    /*! 
     * \brief Receive information about the stream
     * \note The param is a pointer to \see IS_STREAM_INFO
     * Stream have to be identified by UUID
     */
    IS_STREAM_GET_INFO
	
} IS_STREAM_COMMAND;

typedef enum E_STREAM_RECEIVE_CMD_CLIENT
{
    IS_STREAM_CLIENT_CONNECT,
    IS_STREAM_CLIENT_DISCONNECT,
    IS_STREAM_CLIENT_TIMEOUT

}IS_STREAM_RECEIVE_CMD_CLIENT;

typedef enum E_STREAM_SEND_CMD_CLIENT
{
    IS_STREAM_CLIENT_KICK,

} IS_STREAM_SEND_CMD_CLIENT;

typedef struct 
{
    E_STREAM_SEND_CMD_CLIENT typeCommand;
    IS_GUID uuidClient;

} IS_STREAM_SEND_CLIENT;

/*!
 * \brief Saves a timestamp
 */
typedef struct 
{
    long tv_sec;
    long tv_usec;

} IS_STREAM_TIME_VALUE;

/*!
 * \brief Struct of IS_STREAM_CREATE \see IS_STREAM_COMMAND.
 */
typedef struct
{
	/*! Size of the struct */
	int cbSizeOfStruct;
	/*! 
	 * \brief Identifier for the created stream 
	 * The IS_GUID will be filled automaticly
	 */
	IS_GUID uuidStream;
	/*! Name of the stream */
	char strName[20];
	/*! Information about the stream */
	char strInfo[20];
	/*! Description about the stream */
	char strDescription[20];
	/*! Stream URL will be filled automaticly */
    char strURL[255];

} IS_STREAM_INFO;

/*!
 * \brief Struct of IS_STREAM_CREATE_SUBSTREAM \see IS_STREAM_COMMAND.
 * \note Struct is used to create and add a new substream to a previously created stream.
 */
typedef struct
{
	/*! Size of the struct */
	int cbSizeOfStruct;
	/*! 
	 * \brief
	 * Identifier to the stream
	 */
	IS_GUID uuidParentStream;
	/*! 
	 * \brief Identifier for the created substream.
	 * The IS_GUID will be filled automaticly.
	 */
	IS_GUID uuidSubstreamStream;
	/*!
	 * \brief What type of substream is it \see IS_STREAM_TYPES.
	 */
	IS_STREAM_TYPES nType;
	/*!
	 * \brief Pointer to specify the data \see IS_STREAM_VIDEO.
	 * \note At the moment only Video data.
	 */
	void *pSubstream;
	/*! 
	 * \brief Size of pSubstream
	 */
	int cbSizeOfSubstream;
	
} IS_STREAM_SUBSTREAM_INFO;

/*!
 * \brief Struct to specify video settings
 * \note Struct is used by IS_STREAM_SUBSTREAM_INFO
 */
typedef struct
{
	/*! Size of the struct */
	int cbSizeOfStruct;
	/*!
	 * \brief Width of the video frame
	 */
	int nWidthSource;
	/*!
	 * \brief Height of the video frame
	 */
	int nHeightSource;
	/*!
	 * \brief Width of the streaming frame
	 * \note The streaming frame will automaticly be scaled
	 */
	int nWidthDest;
	/*!
	 * \brief Height of the streaming frame
	 * \note The streaming frame will automaticly be scaled
	 */
	int nHeightDest;
	/*!
	 * \brief How many frames will arrive in one second
	 */
	int nFramerate;
	/*!
	 * \brief Pixel format of the incoming frames
	 */
	IS_STREAM_PIXELFMT pixelfmtSource;
	/*!
	 * \brief To choose the pixel format the encoder should use
	 * \note H264 encoder ignores this parameter
	 */
	IS_STREAM_PIXELFMT pixelfmtDest;
	/*! Not implemented yet
	 * \brief Codec of the incoming frames.
	 * \note Specify it only if the incoming frames are already encoded
	 */
	IS_STREAM_CODEC codecSource;
	/*!
	 * \brief Codec to encode the frames
	 */
	IS_STREAM_CODEC codecDest;
	/*! Not implemented yet
	 * \brief Specify the encoding quality
	 */
    IS_STREAM_QUALITY quality;
	
} IS_STREAM_VIDEO;

/*!
 * \brief Struct to deliver a substream data
 * \note Struct is used by IS_STREAM_DELIVER_DATA
 */
typedef struct
{
	/*! Size of the struct */
	int cbSizeOfStruct;
    int nType;
	/*! 
	 * \brief Identifier to the stream
	 */
    IS_GUID uuidStream;
	/*! 
	 * \brief Identifier to the substream
	 */
    IS_GUID uuidSubStream;
	/*! 
	 * \brief Timestamp of the frame
	 * \note If no timestamp is specified the timestamp will be automaticly set when new data is delivered to the library
	 * \see IS_STREAM_TIME_VALUE
	 */
	IS_STREAM_TIME_VALUE timeStamp;
	/*! 
	 * \brief Size of pData
	 */
    int cbSizeOfData;
	/*!
	 * \brief Pointer to the data to deliver
	 */
	unsigned char* pData;

} IS_STREAM_PAYLOAD_DATA;

/*!
 * \brief Struct to send or receive data from a client
 * \note Struct is used by IS_STREAM_SET_CALLBACK_FUNCTION
 */
typedef struct 
{
	/*! Size of the struct */
	int cbSizeOfStruct;
	/*! 
	 * \brief Identifier to the client 
	 * \note If the data is received the IS_GUID is filled with the client id
	 * \note If the data will be send the IS_GUID must be filled with the client id
	 */
	IS_GUID uuidClient;
	/*!
	 * \brief Type of data that will be send
	 */
	int nType;					
	/*! 
	 * \brief Size of pData
	 */
	int cbSizeOfData;
	/*!
	 * \brief Pointer to the data to send to a client or to the data received from a client
	 */
    char* pData;

}IS_STREAM_CONFIG_DATA;


/*!
 * \brief Generic interface to setup and create a stream.
 * \param   nCommand        code, indicates requested access and accessed value, \see IS_STREAM_COMMAND enumeration.
 * \param   pParam          input or output storage for the accessed param.
 * \param   cbSizeOfParam   size of *pParam.
 * \return  error code
 *
 * \note To initialize a stream IS_STREAM_INIT must be called first.
 */
ISSTREAMING_API int is_Stream(int nCommand, void* pParam, int cbSizeOfParam);



#endif //ISSTREAMING_H
