#include <iostream>
#include <fstream>

#include "parser.hpp"
#include "tokenizer.hpp"
#include "namespaces.hpp"
#include "predefined.hpp"
#include "errors.hpp"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Error: expected a file\n";
        return 1;
    }


    Errors::SetFile(argv[1]);
    std::fstream fd(argv[1], std::fstream::in);
    std::vector<Tokenizer::Token> tokens = Tokenizer::Do(fd);
    //for (auto token: tokens) std::cout << token.id << "(" 
    //                        << token.begin_in_text << " " << token.end_in_text << ") ";
    
    Namespaces::Create(false); // namespace 0;
    Predefined::Install();
    
    int pos = 0; Node *node;
    while (pos < tokens.size()) {
        node = Parser::Parse(tokens, pos);
        bool do_continue = false, do_break = false, do_return = false;
        Parser::Execute(node, do_continue, do_break, do_return);
    }
    
    Namespaces::Destroy();
    return 0;
}