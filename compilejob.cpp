#include "compilejob.h"
#include <array>
#include <iostream>
#include <ostream>

void CompileJob::Execute() {
    std::array<char, 128> buffer;
    std::string command = "make automated";

    // Redirect
    command.append(" 2>&1");
}
