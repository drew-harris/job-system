#pragma once
#include "job.h"
#include "jobworkerthread.h"
#include <deque>

constexpr int JOB_TYPE_ANY = -1;

class JobWorkerThread;

enum class JobStatus {
    NEVER_SEEN,
    QUEUED,
    RUNNING,
    COMPLETED,
    RETIRED,
    NUM_JOB_STATUSES
};

struct JobHistoryEntry {
    JobHistoryEntry(int jobType, JobStatus jobStatus)
        : m_jobType(jobType), m_jobStatus(jobStatus) {}
    int m_jobType = -1;
    JobStatus m_jobStatus = JobStatus::NEVER_SEEN;
};

class Job;

class JobSystem {
    friend class JobWorkerThread;

  public:
    // Not implemented
    JobSystem();
    ~JobSystem();
    static JobSystem *CreateOrGet();
    static void Destroy();
    int totalJobs = 0;

    void CreateWorkerThread(const char *uniqueName,
                            unsigned long workerJobChannels = 0xFFFFFFFF);
    void DestroyWorkerThread(const char *uniqueName);

    void QueueJob(Job *job);

    JobStatus GetJobStatus(int jobID) const;
    bool IsJobComplete(int jobID) const;

  private:
    Job *claimAJob(unsigned long channels);
    void OnJobCompleted(Job *jobJustExecuted);
    void FinishCompletedJobs();
    void FinishJob(int jobId);
    static JobSystem *s_jobSystem;
    std::vector<JobWorkerThread *> m_workerThreads;
    mutable std::mutex m_workerThreadsMutex;

    std::deque<Job *> m_jobsQueued;
    std::deque<Job *> m_jobsRunning;
    std::deque<Job *> m_jobsCompleted;

    mutable std::mutex m_jobsQueuedMutex;
    mutable std::mutex m_jobsRunningMutex;
    mutable std::mutex m_jobsCompletedMutex;

    std::vector<JobHistoryEntry> m_jobHistory;
    mutable int m_JobHistoryLowestActiveIndex = 0; // keeps track of oldest job
    mutable std::mutex m_jobHistoryMutex;
};
