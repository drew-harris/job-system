#include "job.h"
class JobSystem {
  public:
    // Not implemented
    Job *claimAJob(unsigned long channels);
    void onJobCompleted(Job &job);
};
