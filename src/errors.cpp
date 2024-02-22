#include <string>
#include <iostream>
#include <fstream>

#include "errors.hpp"
#ifndef _COLORS_
#define _COLORS_

/* FOREGROUND */
#define RST  "\x1B[0m"
#define KRED  "\x1B[31m"
#endif  /* _COLORS_ */


namespace Errors {
    static int start, end;
    std::string file;

    void SetFile(std::string f) {
        file = f;
    }
    void Highlight(int begin_in_text, int end_in_text) {
        start = begin_in_text;
        end = end_in_text;
    }

    const int N = 50;
    void PrintTextNearby() {
        std::fstream fd(file, std::fstream::in);
        if (start - N > 0) fd.seekg(start - N);
        std::cerr << "=Error===============\n";
        while (true) {
            int pos = fd.tellg(); 
            char c = fd.get();
            if (fd.fail()) break;

            if (pos == start) std::cerr << KRED;

            std::cerr << c;

            if (pos == end) std::cerr << RST;
            if (end + N < pos) break;
        }
        std::cerr << "\n=Error===============\n";
        fd.close();
    }

    void RuntimeError(std::string message) {
        PrintTextNearby();
        std::cerr << "RuntimeError (" << start << "..." << end << "):\n";
        std::cerr << message << std::endl;
        exit(1);
    }
    void TokenizationError(std::string message) {
        PrintTextNearby();
        std::cerr << "TokenizationError (" << start << "..." << end << "):\n";
        std::cerr << message << std::endl;
        exit(1);
    }
    void ParsingError(std::string message) {
        PrintTextNearby();
        std::cerr << "ParsingError (" << start << "..." << end << "):\n";
        std::cerr << message << std::endl;
        exit(1);
    }
}