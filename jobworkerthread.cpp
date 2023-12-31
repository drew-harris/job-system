#include "jobworkerthread.h"
#include "jobsystem.h"
#include <chrono>
#include <iostream>
#include <ostream>
#include <thread>

JobWorkerThread::JobWorkerThread(const char *uniqueName,
                                 unsigned long workerJobChannels,
                                 JobSystem *jobSystem)
    : m_uniqueName(uniqueName), m_workerJobChannels(workerJobChannels),
      m_jobSystem(jobSystem) {}

JobWorkerThread::~JobWorkerThread() {
    // If we haven't already signal thread that we should exit as soon as it can
    // (after its current job if any)
    this->Shutdown();

    // Block on the threads main until it has a chance to finish its current job
    // and exit
    m_thread->join();

    delete m_thread;
    m_thread = nullptr;
}

void JobWorkerThread::Startup() {
    m_thread = new std::thread(WokerThreadMain, this);
}

void JobWorkerThread::Work() {
    while (!IsStopping()) {
        m_workerStatusMutex.lock();
        unsigned long workerJobChannels = m_workerJobChannels;
        m_workerStatusMutex.unlock();

        Job *job = m_jobSystem->claimAJob(m_workerJobChannels);
        if (job) {
            job->Execute();
            m_jobSystem->OnJobCompleted(job);
        }

        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}

void JobWorkerThread::Shutdown() {
    m_workerStatusMutex.lock();
    m_isStopping = true;
    m_workerStatusMutex.unlock();
}

bool JobWorkerThread::IsStopping() const {
    bool isStopping = false;
    m_workerStatusMutex.lock();
    isStopping = m_isStopping;
    m_workerStatusMutex.unlock();

    return isStopping;
}

void JobWorkerThread::setWorkerJobChannels(unsigned long workerJobChannels) {
    m_workerStatusMutex.lock();
    m_workerJobChannels = workerJobChannels;
    m_workerStatusMutex.unlock();
}

 void JobWorkerThread::WokerThreadMain(JobWorkerThread *workerThreadObject){
    JobWorkerThread* thisWorker = workerThreadObject;
    thisWorker->Work();
};
