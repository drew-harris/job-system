#pragma once
#include "./job.h"

class JobSystem;

class JobWorkerThread {
    friend class JobSystem;

  private:
    JobWorkerThread(const char *uniqueName, unsigned long workerJobChannels,
                    JobSystem *jobSystem);
    ~JobWorkerThread();
};
