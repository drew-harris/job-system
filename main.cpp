#include <algorithm>
#include <chrono>
#include <cstdio>
#include <functional>
#include <iostream>
#include <ostream>
#include <string>
#include <thread>
#include <vector>

#include "job.h"

#include "compilejob.h"
#include "jobsystem.h"
#include "renderjob.h"

int main() {

    std::cout << "Creating Job System" << std::endl;

    JobSystem *js = JobSystem::CreateOrGet();

    std::cout << "Creating Worker Threads" << std::endl;

    js->CreateWorkerThread("Thread1", 0xFFFFFFFF);
    js->CreateWorkerThread("Thread2", 0xFFFFFFFF);
    js->CreateWorkerThread("Thread3", 0xFFFFFFFF);
    js->CreateWorkerThread("Thread4", 0xFFFFFFFF);
    js->CreateWorkerThread("Thread5", 0xFFFFFFFF);
    js->CreateWorkerThread("Thread6", 0xFFFFFFFF);
    js->CreateWorkerThread("Thread7", 0xFFFFFFFF);
    js->CreateWorkerThread("Thread8", 0xFFFFFFFF);
    js->CreateWorkerThread("Thread9", 0xFFFFFFFF);

    std::cout << " Create Jobs " << std::endl;

    std::vector<Job *> jobs;

    CompileJob *cjb = new CompileJob(0xFFFFFFFF, 1);
    jobs.push_back(cjb);

    for (int j = 0; j < 10; j++) {
        RenderJob *rjb = new RenderJob(0xFFFFFFFF, 1);
        for (int i = 0; i < 1000; i++) {
            rjb->data.push_back(i + j);
        }

        jobs.push_back(rjb);
    }

    std::vector<Job *>::iterator it = jobs.begin();
    for (; it != jobs.end(); it++) {
        js->QueueJob(*it);
    }

    int running = 1;

    while (running) {
        std::string command;
        std::cout << "Enter stop destroy, finish or status" << std::endl;
        std::cin >> command;

        if (command == "stop") {
            std::cout << "Stopping Job System" << std::endl;
            running = 0;
        } else if (command == "destroy") {
            std::cout << "Destroying Job System" << std::endl;
            js->FinishCompletedJobs();
            js->Destroy();
            running = 0;
        } else if (command == "finish") {
            std::cout << "Finishing Jobs" << js->totalJobs << std::endl;
            js->FinishCompletedJobs();
        } else if (command == "status") {
            std::cout << "Job Status" << std::endl;
            for (int i = 0; i < jobs.size(); i++) {
                std::cout << "Job " << i << " Status: " << (int)js->GetJobStatus(i)
                          << std::endl;
            }
        } else {
            std::cout << "Unknown Command" << std::endl;
        }
    }

    return 0;
}
