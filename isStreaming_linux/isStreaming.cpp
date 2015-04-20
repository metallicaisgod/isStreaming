#include <time.h>
#include "isStreaming.h"
#include "liveMedia.hh"
#include "GroupsockHelper.hh"
#include "BasicUsageEnvironment.hh"
#include "MJPEGSource.h" 
#include "BasicTaskScheduler.h"
#include "JPEGVideoRTPSink.h"
#include "pthread.h"

UsageEnvironment* env;

struct sessionState_t {
	FramedSource* source;
	RTPSink* sink;
	Groupsock* rtpGroupsock;
	RTSPServer* rtspServer;
	ServerMediaSession* sms;
} sessionState;

void GenerateGuid(unsigned char *guidStr)
{
	unsigned char *pGuidStr = guidStr;
	int i;

	srand(static_cast<unsigned int> (time(NULL)));  /*Randomize based on time.*/

	/*16 characters.*/
	for (i = 0; i < sizeof(IS_GUID); i++, pGuidStr++)
		((*pGuidStr = (rand() % 16)) < 10) ? *pGuidStr += 48 : *pGuidStr += 55;
}

IS_STREAM_INFO * streamInfo = NULL;
IS_STREAM_SUBSTREAM_INFO * subStreamInfo = NULL;

Boolean started_server = False;
pthread_t thread;
pthread_mutex_t mxq;

/* Returns 1 (true) if the mutex is unlocked, which is the
* thread's signal to terminate.
*/
int needQuit(pthread_mutex_t *mtx)
{
	switch (pthread_mutex_trylock(mtx)) {
	case 0: /* if we got the lock, unlock and return 1 (true) */
		pthread_mutex_unlock(mtx);
		return 1;
	case EBUSY: /* return 0 (false) if the mutex was locked */
		return 0;
	}
	return 1;
}

static void *thread_func(void *vptr_args)
{
	//pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	pthread_mutex_t *mx = (pthread_mutex_t *)vptr_args;
	while (!needQuit(mx))
	{
		((CBasicTaskScheduler*)(&env->taskScheduler()))->SingleStep(0);
	}
	//env->taskScheduler().doEventLoop();
	return NULL;
}

void afterPlaying(void* /*clientData*/) {
	*env << "...done streaming\n";

	// End by closing the media:
	sessionState.sink->stopPlaying();
	Medium::close(sessionState.rtspServer);
	Medium::close(sessionState.sink);
	delete sessionState.rtpGroupsock;
	Medium::close(sessionState.source);

	// We're done:
	//exit(0);
}


int is_Stream(int nCommand, void* pParam, int cbSizeOfParam)
{
	switch (nCommand)
	{
	case IS_STREAM_INIT:
		{
			TaskScheduler* scheduler = CBasicTaskScheduler::createNew();
			env = BasicUsageEnvironment::createNew(*scheduler);
			unsigned int port = *((unsigned int*)(pParam));
			if (port == 0)
				port = 8554;
			sessionState.rtspServer
				= RTSPServer::createNew(*env, port);
			if (sessionState.rtspServer == NULL) {
				return IS_STREAM_PORT_IN_USE;
			}
			return IS_STREAM_SUCCESS;
		}
	case IS_STREAM_CREATE:
		{
			if (sessionState.rtspServer == NULL)
				return IS_STREAM_NOT_INITIALIZED;
			IS_STREAM_INFO *tempStreamInfo = (IS_STREAM_INFO *)pParam;
			if (tempStreamInfo == NULL || cbSizeOfParam != sizeof(IS_STREAM_INFO))
				return IS_STREAM_NO_SUCCESS;
			if (streamInfo != NULL && !strcmp(streamInfo->strName, tempStreamInfo->strName))
				return IS_STREAM_NAME_IN_USE;
			if (streamInfo == NULL)
				streamInfo = (IS_STREAM_INFO*)malloc(sizeof(IS_STREAM_INFO));
			memcpy(streamInfo, tempStreamInfo, sizeof(IS_STREAM_INFO));

			sessionState.sms
				= ServerMediaSession::createNew(*env, streamInfo->strName, streamInfo->strInfo,
				streamInfo->strDescription, True/*SSM*/);
            strcpy(streamInfo->strURL, sessionState.rtspServer->rtspURL(sessionState.sms));
            strcpy(tempStreamInfo->strURL, streamInfo->strURL);
			
			GenerateGuid(streamInfo->uuidStream);
			memcpy(tempStreamInfo->uuidStream, streamInfo->uuidStream, sizeof(IS_GUID));

			return IS_STREAM_SUCCESS;
		}
	case IS_STREAM_CREATE_SUBSTREAM:
		{
			if (sessionState.rtspServer == NULL || streamInfo == NULL)
				return IS_STREAM_NOT_INITIALIZED;
			IS_STREAM_SUBSTREAM_INFO * tempSubStreamInfo = (IS_STREAM_SUBSTREAM_INFO *)pParam;
			if (!tempSubStreamInfo || cbSizeOfParam != sizeof(IS_STREAM_SUBSTREAM_INFO) || tempSubStreamInfo->nType != IS_STREAM_TYPE_VIDEO)
				return IS_STREAM_NO_SUCCESS;
			for (int i = 0; i < sizeof(IS_GUID); i++)
			{
				if (streamInfo->uuidStream[i] != tempSubStreamInfo->uuidParentStream[i])
					return IS_STREAM_INVALID_STREAM_UUID;
			}
			if (!tempSubStreamInfo->pSubstream)
				return IS_STREAM_NO_SUCCESS;
			if (!subStreamInfo)
				subStreamInfo = (IS_STREAM_SUBSTREAM_INFO*)malloc(sizeof(IS_STREAM_SUBSTREAM_INFO));
			memcpy(subStreamInfo, tempSubStreamInfo, sizeof(IS_STREAM_SUBSTREAM_INFO));

			IS_STREAM_VIDEO * steamVideo = (IS_STREAM_VIDEO *)subStreamInfo->pSubstream;
			unsigned timePerFrame = 1000000 / 10;
			if (steamVideo->nFramerate != 0)
				timePerFrame = 1000000 / steamVideo->nFramerate; // microseconds

			sessionState.source
				= MJPEGSource::createNew(*env, timePerFrame);
			if (sessionState.source == NULL) {
				return IS_STREAM_NO_SUCCESS;
			}

			int ret = ((MJPEGSource*)sessionState.source)->InitEncoder(steamVideo);
			if (ret < 0)
			{
				return IS_STREAM_ENCODER_ERROR;
			}

			// Create 'groupsocks' for RTP:
			struct in_addr destinationAddress;
			destinationAddress.s_addr = chooseRandomIPv4SSMAddress(*env);

			const unsigned short rtpPortNum = 2222;
			const unsigned char ttl = 255;

			const Port rtpPort(rtpPortNum);

			sessionState.rtpGroupsock
				= new Groupsock(*env, destinationAddress, rtpPort, ttl);
			sessionState.rtpGroupsock->multicastSendOnly(); // we're a SSM source

			// Create an appropriate RTP sink from the RTP 'groupsock':
			sessionState.sink
				= CJPEGVideoRTPSink::createNew(*env, sessionState.rtpGroupsock);

			sessionState.sms->addSubsession(PassiveServerMediaSubsession
				::createNew(*sessionState.sink));
			sessionState.rtspServer->addServerMediaSession(sessionState.sms);

			int maxSize = ((MJPEGSource*)sessionState.source)->findActualSizeOfFrame();
			((CJPEGVideoRTPSink*)sessionState.sink)->incMaxSizeTo(maxSize);

			GenerateGuid(subStreamInfo->uuidSubstreamStream);
			memcpy(tempSubStreamInfo->uuidSubstreamStream, subStreamInfo->uuidSubstreamStream, sizeof(IS_GUID));

			return IS_STREAM_SUCCESS;
		}
	case IS_STREAM_GET_INFO:
		{
			if (sessionState.rtspServer == NULL || streamInfo == NULL)
				return IS_STREAM_NOT_INITIALIZED;
			IS_STREAM_INFO *tempStreamInfo = (IS_STREAM_INFO *)pParam;
			if (tempStreamInfo == NULL || cbSizeOfParam != sizeof(IS_STREAM_INFO))
				return IS_STREAM_NO_SUCCESS;
			for (int i = 0; i < sizeof(IS_GUID); i++)
			{
				if (streamInfo->uuidStream[i] != tempStreamInfo->uuidStream[i])
					return IS_STREAM_INVALID_STREAM_UUID;
			}
			memcpy(tempStreamInfo, streamInfo, sizeof(IS_STREAM_INFO));
			return IS_STREAM_SUCCESS;

		}
	case IS_STREAM_DELIVER_DATA:
		{
			if (sessionState.rtspServer == NULL || streamInfo == NULL || subStreamInfo == NULL)
				return IS_STREAM_NOT_INITIALIZED;
			IS_STREAM_PAYLOAD_DATA * tempPayloadData = (IS_STREAM_PAYLOAD_DATA *)pParam;
			if (!tempPayloadData || cbSizeOfParam != sizeof(IS_STREAM_PAYLOAD_DATA))
				return IS_STREAM_NO_SUCCESS;
			for (int i = 0; i < sizeof(IS_GUID); i++)
			{
				if (streamInfo->uuidStream[i] != tempPayloadData->uuidStream[i])
					return IS_STREAM_INVALID_STREAM_UUID;
				if (subStreamInfo->uuidSubstreamStream[i] != tempPayloadData->uuidSubStream[i])
					return IS_STREAM_INVALID_SUBSTREAM_UUID;
			}
			if (tempPayloadData->nType != IS_STREAM_TYPE_VIDEO)
				return IS_STREAM_INVALID_PARAMETER;
			if(!sessionState.source)
				return IS_STREAM_NO_SUCCESS;

			if (!started_server)
			{
				sessionState.sink->startPlaying(*sessionState.source, afterPlaying, NULL);

				//env->taskScheduler().
				//env->taskScheduler().doEventLoop();
				pthread_mutex_init(&mxq, NULL);
				pthread_mutex_lock(&mxq);
				if (pthread_create(&thread, NULL, thread_func, &mxq) != 0)
				{
					return IS_STREAM_NO_SUCCESS;
				}
				started_server = True;
			}
						
			if (!((MJPEGSource*)sessionState.source)->DeliverFrame(tempPayloadData))
				return IS_STREAM_NO_SUCCESS;
			
			return IS_STREAM_SUCCESS;			
		}
	case IS_STREAM_EXIT:
		{
			if (sessionState.rtspServer == NULL || streamInfo == NULL || subStreamInfo == NULL)
				return IS_STREAM_NOT_INITIALIZED;
			if (started_server)
			{
				sessionState.source->handleClosure();
				pthread_mutex_unlock(&mxq);
				pthread_join(thread, NULL);
				//pthread_cancel(thread);
				free(streamInfo);
				free(subStreamInfo);
				started_server = False;
				return IS_STREAM_SUCCESS;
			}
			return IS_STREAM_NO_SUCCESS;			
		}
	default:
		return IS_STREAM_NO_SUCCESS;
	}
}
