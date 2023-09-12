#include "jobsystem.h"
#include "jobworkerthread.h"

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
