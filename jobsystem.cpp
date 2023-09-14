#include "jobsystem.h"
#include "jobworkerthread.h"
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
    }

    m_workerThreadsMutex.unlock();
}
