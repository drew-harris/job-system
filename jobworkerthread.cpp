#include "jobworkerthread.h"
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
