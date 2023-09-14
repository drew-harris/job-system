#include "jobsystem.h"
#include "jobworkerthread.h"
#include <deque>
#include <iostream>
#include <vector>

JobSystem *JobSystem::s_jobSystem = nullptr;

using JobCallback = void (*)(Job *completedJob);

JobSystem::JobSystem() { m_jobHistory.reserve(256 * 1024); }

JobSystem::~JobSystem() {
    m_workerThreadsMutex.lock();
    int numWorkerThreads = (int)m_workerThreads.size();

    // Tell each worker thread to stop picking up new jobs
    for (int i = 0; i < numWorkerThreads; ++i) {
        m_workerThreads.at(i)->Shutdown();
    }

    while (!m_workerThreads.empty()) {
        delete m_workerThreads.back();
        m_workerThreads.pop_back();
    }

    m_workerThreadsMutex.lock();
}

JobSystem *JobSystem::CreateOrGet() {
    if (s_jobSystem == nullptr) {
        s_jobSystem = new JobSystem();
    }
    return s_jobSystem;
}

void JobSystem::Destroy() {
    if (s_jobSystem != nullptr) {
        delete s_jobSystem;
        s_jobSystem = nullptr;
    }
}

void JobSystem::CreateWorkerThread(const char *uniqueName,
                                   unsigned long workerJobChannels) {

    JobWorkerThread *newWorker =
        new JobWorkerThread(uniqueName, workerJobChannels, this);
    m_workerThreadsMutex.lock();
    m_workerThreads.push_back(newWorker);
    m_workerThreadsMutex.unlock();
    m_workerThreads.back()->Startup();
}

void JobSystem::DestroyWorkerThread(const char *uniqueName) {
    m_workerThreadsMutex.lock();

    JobWorkerThread *doomedWorker = nullptr;

    std::vector<JobWorkerThread *>::iterator it = m_workerThreads.begin();

    for (; it != m_workerThreads.end(); ++it) {
        if (strcmp((*it)->m_uniqueName, uniqueName) == 0) {
            doomedWorker = *it;
            m_workerThreads.erase(it);
            break;
        }
    }

    if (doomedWorker) {
        doomedWorker->Shutdown();
        delete doomedWorker;
    }

    m_workerThreadsMutex.unlock();
}

void JobSystem::QueueJob(Job *job) {
    m_jobsQueuedMutex.lock();
    m_jobHistoryMutex.lock();
    m_jobHistory.emplace_back(
        JobHistoryEntry(job->m_jobType, JobStatus::QUEUED));
    m_jobHistoryMutex.unlock();

    m_jobsQueued.push_back(job);
    m_jobsQueuedMutex.unlock();
}

// BUG: Possibly broken
JobStatus JobSystem::GetJobStatus(int jobId) const {
    m_jobHistoryMutex.lock();
    JobStatus status = JobStatus::NEVER_SEEN;
    m_jobHistoryMutex.unlock();

    if (jobId < m_jobHistory.size()) {
        status = m_jobHistory[jobId].m_jobStatus;
    }

    return status;
}

bool JobSystem::IsJobComplete(int jobId) const {
    return JobStatus(jobId) == JobStatus::COMPLETED;
}

void JobSystem::FinishCompletedJobs() {
    std::deque<Job *> jobsCompleted;
    m_jobsCompletedMutex.lock();
    jobsCompleted.swap(m_jobsCompleted);
    m_jobsCompletedMutex.unlock();

    // TODO: fix???
    m_jobHistoryMutex.lock();
    for (Job *job : jobsCompleted) {
        job->JobCompleteCallback();
        m_jobHistory[job->m_jobID].m_jobStatus = JobStatus::RETIRED;
    }
    m_jobHistoryMutex.unlock();
}

void JobSystem::FinishJob(int jobId) {
    while (!IsJobComplete(jobId)) {
        JobStatus status = GetJobStatus(jobId);
        if (status == JobStatus::NEVER_SEEN || status == JobStatus::RETIRED) {
            std::cout << "ERROR waiting for job " << jobId << "no such job"
                      << std::endl;
        }

        m_jobsCompletedMutex.lock();
        Job *thisCompletedJob = nullptr;

        for (auto jcIter = m_jobsCompleted.begin();
             jcIter != m_jobsCompleted.end(); ++jcIter) {
            Job *someCompletedJob = *jcIter;
            if (someCompletedJob->m_jobID == jobId) {
                thisCompletedJob = someCompletedJob;
                m_jobsCompleted.erase(jcIter);
                break;
            }
        }

        m_jobsCompletedMutex.unlock();

        if (thisCompletedJob == nullptr) {
            std::cout << "ERROR: job: " << jobId
                      << " was status complete but not found in completed list"
                      << std::endl;
        }

        m_jobHistoryMutex.lock();

        m_jobHistory[thisCompletedJob->m_jobID].m_jobStatus =
            JobStatus::RETIRED;
        m_jobHistoryMutex.unlock();

        delete thisCompletedJob;
    }
}

void JobSystem::OnJobCompleted(Job *jobJustExected) {
    totalJobs++;
    m_jobsCompletedMutex.lock();
    m_jobsRunningMutex.lock();

    std::deque<Job *>::iterator runningJobItr = m_jobsRunning.begin();
    for (; runningJobItr != m_jobsRunning.end(); ++runningJobItr) {
        if (jobJustExected == *runningJobItr) {
            m_jobHistoryMutex.lock();
            m_jobsRunning.erase(runningJobItr);
            m_jobsCompleted.push_back(jobJustExected);
            m_jobHistory[jobJustExected->m_jobID].m_jobStatus =
                JobStatus::COMPLETED;

            m_jobHistoryMutex.unlock();
            break;
        }
    }
    m_jobsCompletedMutex.unlock();
    m_jobsRunningMutex.unlock();
}

Job *JobSystem::claimAJob(unsigned long channels) {
    m_jobsQueuedMutex.lock();
    m_jobsRunningMutex.lock();

    Job* claimedJob = nullptr;

    std::deque<Job*>::iterator queuedJobIter  = m_jobsQueued.begin();
    for (; queuedJobIter != m_jobsQueued.end(); ++queuedJobIter) {
        Job* queuedJob = *queuedJobIter;

        if ((queuedJob->m_jobChannels & channels) != 0 ) {
            claimedJob = queuedJob;

            m_jobHistoryMutex.lock();
            m_jobHistory[claimedJob->m_jobID].m_jobStatus = JobStatus::RUNNING;
            m_jobHistoryMutex.unlock();
            m_jobsQueued.erase(queuedJobIter);
            break;
        }
    }

    m_jobsRunningMutex.unlock();
    m_jobsQueuedMutex.unlock();

    return claimedJob;
}
