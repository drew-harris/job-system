#include "compilejob.h"
#include <array>
#include <iostream>
#include <ostream>

void CompileJob::Execute() {
    std::array<char, 128> buffer;
    std::string command = "make compile";

    // Redirect cerr to cout
    command.append(" 2>&1");

    // open pipe and run command
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) {
        std::cout << "popen Failed: Error: failed to open pipe" << std::endl;
        return;
    }

    // read till end of process:
    while (fgets(buffer.data(), 128, pipe) != nullptr) {
        this->output.append(buffer.data());
    }

    // close pipe and get return code
    this->returnCode = pclose(pipe);

    std::cout << "Job " << this->GetUniqueID() << " has been executed"
              << std::endl;
}

void CompileJob::JobCompleteCallback() {
    std::cout << "Compile job " << this->GetUniqueID() << " Return code"
              << this->returnCode << std::endl;
    std::cout << "Compile Job " << this->GetUniqueID() << " Output: \n"
              << this->output << std::endl;
}
