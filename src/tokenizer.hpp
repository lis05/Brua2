#pragma once

#include "util.hpp"
#include "names.hpp"

#include <vector>
#include <fstream>

namespace Tokenizer {
    enum TokenId {
        SET, WHILE, FOR, REPEAT, IF, CONTINUE, BREAK, RETURN, FUNC, ARG, CALL,
        BOOL, CHAR, INT, REAL, STRING, DEREF, REF, INV, NOT, NEG, MULT, DIV, REM,
        ADD, SUB, SHL, SHR, LT, GT, LE, GE, EQ, NEQ, AND, XOR, OR, CONJ, DISJ,
        DACCESS, DSIZE, DPRESENT, DINSERT, DREMOVE, DKEYS, DVALUES, DCLEAR,
        SACCESS, SSIZE, SADDSUF, SADDPREF, SREMOVESUF, SREMOVEPREF, OPEN_BRACKET, 
        CLOSED_BRACKED, BOOL_LITERAL, CHAR_LITERAL, INT_LITERAL, REAL_LITERAL, 
        STRING_LITERAL, NULL_LITERAL, DICT_LITERAL, NAME, 
    };
    struct Token {
        TokenId id;
        BOOL_T bool_literal;
        CHAR_T char_literal;
        INT_T int_literal;
        REAL_T real_literal;
        STRING_T string_literal;
        Names::Name name;
        int begin_in_text, end_in_text;
    };
    std::vector<Token> Do(std::fstream &fd);
}