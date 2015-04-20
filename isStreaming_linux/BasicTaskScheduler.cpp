#include "BasicTaskScheduler.h"

CBasicTaskScheduler* CBasicTaskScheduler::createNew(unsigned maxSchedulerGranularity/* = 10000 microseconds*/)
{
	return new CBasicTaskScheduler(maxSchedulerGranularity);
}

CBasicTaskScheduler::CBasicTaskScheduler(unsigned maxSchedulerGranularity) :
BasicTaskScheduler(maxSchedulerGranularity)
{
}


CBasicTaskScheduler::~CBasicTaskScheduler()
{
}
