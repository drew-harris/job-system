#pragma once
#include "./job.h"

class JobSystem;

class JobWorkerThread {
    friend class JobSystem;

  private:
    JobWorkerThread(const char *uniqueName, unsigned long workerJobChannels,
                    JobSystem *jobSystem);
    ~JobWorkerThread();

    void Startup(); // Kickoff the thread which will call work()
    void Work(); // Called in private thread, blocks forever until StopWorking()
                 // is called

    void Shutdown(); // Signal that work should stop at next opportunity

    bool IsStopping() const;
    void setWorkerJobChannels(unsigned long workerJobChannels);
    static void WokerThreadMain(void *workThreadObject);
};
