#include "compilejob.h"
#include <array>
#include <iostream>
#include <ostream>

void CompileJob::Execute() {
    std::array<char, 128> buffer;
    std::string command = "make automated";

    // Redirect cerr to cout
    command.append(" 2>&1");

    FILE *pipe = popen(command.c_str(), "r");

    if(!pipe) {
        std::cout << "popen Failed: Error: failed to open pipe" << std::endl;
        return;
    }
}
