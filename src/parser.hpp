#pragma once

#include <vector>
#include <string>

#include "names.hpp"
#include "util.hpp"

#include "objects.hpp"
#include "tokenizer.hpp"
namespace Parser {
    enum NodeId {
        SET, WHILE, FOR, REPEAT, IF, CONTINUE, BREAK, RETURN, FUNC, ARG, CALL,
        BOOL_CAST, CHAR_CAST, INT_CAST, REAL_CAST, STRING_CAST, DEREF, REF, 
        INV, NOT, NEG, MULT, DIV, REM, ADD, SUB, SHL, SHR, LT, GT, LE, GE, 
        EQ, NEQ, AND, XOR, OR, CONJ, DISJ, DACCESS, DSIZE, DPRESENT, DINSERT, 
        DREMOVE, DKEYS, DVALUES, DCLEAR, SACCESS, SSIZE, SADDSUF, SADDPREF, SREMOVESUF, 
        SREMOVEPREF, NAME, BOOL_LITERAL, CHAR_LITERAL, INT_LITERAL, REAL_LITERAL, 
        STRING_LITERAL, NULL_LITERAL, DICT_LITERAL, BLOCK
    };  

    Node *Parse(std::vector<Tokenizer::Token> &tokens, int &pos);
    Node *CreateNode(NodeId id);
    NodeId &GetId(Node *node);
    std::vector<Node*> &GetKids(Node *node);
    int &GetBeginInText(Node *node);
    int &GetEndInText(Node *node);
    Names::Name &GetName(Node *node);
    BOOL_T &GetBool(Node *node);
    CHAR_T &GetChar(Node *node);
    INT_T &GetInt(Node *node);
    REAL_T &GetReal(Node *node);
    STRING_T &GetString(Node *node);
    
    // returned value is tracked in the topmost namespace before the call
    Object *Execute(Node *node, bool &do_continue, bool &do_break, bool &do_return);
}