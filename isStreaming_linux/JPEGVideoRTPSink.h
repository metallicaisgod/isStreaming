#pragma once
#include "JPEGVideoRTPSink.hh"
class CJPEGVideoRTPSink :
	public JPEGVideoRTPSink
{
public:
	static CJPEGVideoRTPSink* createNew(UsageEnvironment& env, Groupsock* RTPgs);
	void incMaxSizeTo(int newMaxSize);
protected:
	CJPEGVideoRTPSink(UsageEnvironment& env, Groupsock* RTPgs);
	virtual ~CJPEGVideoRTPSink();
};

