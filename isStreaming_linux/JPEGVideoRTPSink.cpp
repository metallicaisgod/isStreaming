#include "JPEGVideoRTPSink.h"


CJPEGVideoRTPSink
::CJPEGVideoRTPSink(UsageEnvironment& env, Groupsock* RTPgs)
: JPEGVideoRTPSink(env, RTPgs) {
}

CJPEGVideoRTPSink::~CJPEGVideoRTPSink() {
}

CJPEGVideoRTPSink*
CJPEGVideoRTPSink::createNew(UsageEnvironment& env, Groupsock* RTPgs) {
	return new CJPEGVideoRTPSink(env, RTPgs);
}

void CJPEGVideoRTPSink::incMaxSizeTo(int newMaxSize)
{
	OutPacketBuffer::increaseMaxSizeTo(newMaxSize);
	setPacketSizes(1000, 1448);
}

