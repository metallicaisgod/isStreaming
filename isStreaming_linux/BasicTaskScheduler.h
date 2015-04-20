#pragma once
#include "BasicUsageEnvironment.hh"
class CBasicTaskScheduler :
	public BasicTaskScheduler
{
public:
	static CBasicTaskScheduler* createNew(unsigned maxSchedulerGranularity = 10000/*microseconds*/);
	virtual void SingleStep(unsigned maxDelayTime)
	{
		BasicTaskScheduler::SingleStep(maxDelayTime);
	}
protected:
	CBasicTaskScheduler(unsigned maxSchedulerGranularity);
	virtual ~CBasicTaskScheduler();
};

