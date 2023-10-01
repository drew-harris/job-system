#pragma once
#include <deque>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

class Job {
    friend class JobSystem;
    friend class JobWorkerThread;

  public:
    Job(unsigned long jobChannels = 0xFFFFFFFF, int jobType = -1)
        : m_jobChannels(jobChannels), m_jobType(jobType) {
        static int s_nextJobID = 0;
        m_jobID = s_nextJobID++;
    }

    virtual ~Job(){};

    virtual void Execute() = 0; // Must inherit execute function
    virtual void JobCompleteCallback() = 0;

    int GetUniqueID() const { return m_jobID; }

  private:
    int m_jobID = -1;
    int m_jobType = -1;

    unsigned long m_jobChannels = 0xFFFFFFFF;
};
