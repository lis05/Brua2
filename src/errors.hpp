#pragma once

#include <string>

namespace Errors {
    void Highlight(int begin_in_text, int end_in_text);
    void SetFile(std::string f);
    void RuntimeError(std::string message);
    void TokenizationError(std::string message);
    void ParsingError(std::string message);
}

using namespace Errors;