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

    std::cout << "Job: " << this->GetUniqueID() << "has been executed"
              << std::endl;
}
