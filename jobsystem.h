#include "job.h"
class JobSystem {
  public:
    Job* claimAJob(unsigned long channels);
    void onJobCompleted(Job &job);
  
};
