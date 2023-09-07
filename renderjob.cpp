#include "renderjob.h"
#include <vector>

void RenderJob::Execute() {
    int total = 0;
    std::vector<int>::iterator it = data.begin();

    for (; it != data.end(); ++it) {
        total += *it;
    }

    it = data.begin();

    for (; it != data.end(); ++it) {
        total += *it;
    }

    data[0] = total; // [0] is the sum of the vector
    std::cout << "Job: " << this->GetUniqueID() << "has been executed"
              << std::endl;
}

void RenderJob::JobCompleteCallback() {
    std::cout << "Job: " << this->GetUniqueID() << "has been completed"
        << "Calculated Sum: " << this->data[0] << std::endl;
}
