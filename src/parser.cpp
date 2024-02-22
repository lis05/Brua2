#include "parser.hpp"

#include "namespaces.hpp"
#include "names.hpp"
#include "errors.hpp"
#include "tokenizer.hpp"

#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <vector>

struct Node {
    Parser::NodeId id;
    std::vector<Node*> kids;
    int begin_in_text, end_in_text;
    Names::Name name;
    BOOL_T bool_literal;
    CHAR_T char_literal;
    INT_T int_literal;
    REAL_T real_literal;
    STRING_T string_literal;
};

namespace Parser {
    void CheckEnd(std::vector<Tokenizer::Token> &tokens, int pos) {
        if (pos >= tokens.size()) {
            Errors::Highlight(tokens[pos - 1].end_in_text + 1, tokens[pos - 1].end_in_text + 1);
            ParsingError("Expected a token");
        }
    }
    Node *Parse(std::vector<Tokenizer::Token> &tokens, int &pos) {
        if (tokens.empty()) {
            return NULL;
        }

        Node *res = new Node;
        int start_pos = pos;

        Tokenizer::Token kw = tokens[pos];
        if (kw.id == Tokenizer::BOOL_LITERAL) {
            res->id = BOOL_LITERAL;
            res->bool_literal = kw.bool_literal;

            res->begin_in_text = tokens[start_pos].begin_in_text;
            res->end_in_text = tokens[pos].end_in_text;
            pos++;
            return res;
        }
        if (kw.id == Tokenizer::CHAR_LITERAL) {
            res->id = CHAR_LITERAL;
            res->char_literal = kw.char_literal;

            res->begin_in_text = tokens[start_pos].begin_in_text;
            res->end_in_text = tokens[pos].end_in_text;
            pos++;
            return res;
        }
        if (kw.id == Tokenizer::INT_LITERAL) {
            res->id = INT_LITERAL;
            res->int_literal = kw.int_literal;

            res->begin_in_text = tokens[start_pos].begin_in_text;
            res->end_in_text = tokens[pos].end_in_text;
            pos++;
            return res;
        }
        if (kw.id == Tokenizer::REAL_LITERAL) {
            res->id = REAL_LITERAL;
            res->real_literal = kw.real_literal;

            res->begin_in_text = tokens[start_pos].begin_in_text;
            res->end_in_text = tokens[pos].end_in_text;
            pos++;
            return res;
        }
        if (kw.id == Tokenizer::STRING_LITERAL) {
            res->id = STRING_LITERAL;
            res->string_literal = kw.string_literal;

            res->begin_in_text = tokens[start_pos].begin_in_text;
            res->end_in_text = tokens[pos].end_in_text;
            pos++;
            return res;
        }
        if (kw.id == Tokenizer::NULL_LITERAL) {
            res->id = NULL_LITERAL;

            res->begin_in_text = tokens[start_pos].begin_in_text;
            res->end_in_text = tokens[pos].end_in_text;
            pos++;
            return res;
        }
        if (kw.id == Tokenizer::DICT_LITERAL) {
            res->id = DICT_LITERAL;

            res->begin_in_text = tokens[start_pos].begin_in_text;
            res->end_in_text = tokens[pos].end_in_text;
            pos++;
            return res;
        }
        if (kw.id == Tokenizer::NAME) {
            res->id = NAME;
            res->name = kw.name;

            res->begin_in_text = tokens[start_pos].begin_in_text;
            res->end_in_text = tokens[pos].end_in_text;
            pos++;
            return res;
        }
        if (kw.id != Tokenizer::OPEN_BRACKET) {
            Errors::Highlight(kw.begin_in_text, kw.end_in_text);
            ParsingError("Expected an open bracket");
        }

        pos++;
        CheckEnd(tokens, pos);
        
        kw = tokens[pos];
        if (kw.id == Tokenizer::CLOSED_BRACKED) {
            res->id = BLOCK;
            res->begin_in_text = tokens[start_pos].begin_in_text;
            res->end_in_text = tokens[pos].end_in_text;
            pos++;
            return res;
        }

        if (kw.id == Tokenizer::OPEN_BRACKET) {
            res->id = BLOCK;
            pos--;
        }
        else {
            if (kw.id > Tokenizer::SREMOVEPREF) {
                Errors::Highlight(kw.begin_in_text, kw.end_in_text);
                ParsingError("Expected a keyword or an open bracket");
            }
            res->id = (NodeId)kw.id;
        }
        pos++;

        while (true) {
            CheckEnd(tokens, pos);
            if (tokens[pos].id == Tokenizer::CLOSED_BRACKED) {
                res->begin_in_text = tokens[start_pos].begin_in_text;
                res->end_in_text = tokens[pos].end_in_text;
                pos++;
                return res;
            }

            res->kids.push_back(Parse(tokens, pos));                
        }

        return NULL;
    }
    Node *CreateNode(NodeId id) {
        Node *res = new Node;
        res->id = id;
        return res;
    }
    NodeId &GetId(Node *node) {
        return node->id;
    }
    std::vector<Node*> &GetKids(Node *node) {
        return node->kids;
    }
    int &GetBeginInText(Node *node) {
        return node->begin_in_text;
    }
    int &GetEndInText(Node *node) {
        return node->end_in_text;
    }
    Names::Name &GetName(Node *node) {
        return node->name;
    }
    BOOL_T &GetBool(Node *node) {
        return node->bool_literal;
    }
    CHAR_T &GetChar(Node *node) {
        return node->char_literal;
    }
    INT_T &GetInt(Node *node) {
        return node->int_literal;
    }
    REAL_T &GetReal(Node *node) {
        return node->real_literal;
    }
    STRING_T &GetString(Node *node) {
        return node->string_literal;
    }

    void Highlight(Node *node) {
        Errors::Highlight(node->begin_in_text, node->end_in_text);
    }

    void TryDestroying(Object *obj) {
        if (obj == NULL) return;
        if (!Objects::IsReferenceable(obj)) {
            Namespaces::Untrack(Namespaces::Current(), obj);
            Objects::Destroy(obj);
        }
    }

    Object *Execute(Node *node, bool &do_continue, bool &do_break, bool &do_return) {
        Highlight(node);
        do_continue = false;
        do_break = false;
        do_return = false;
        std::vector<Node*> kids = node->kids;

        switch (node->id) {
            case SET: {
                if (kids.size() != 2) RuntimeError("Expected 2 arguments");
                if (kids[0]->id == NAME) {
                    Names::Name name = kids[0]->name;
                    
                    Object *second = Execute(kids[1], do_continue, do_break, do_return);
                    if (second == NULL) {
                        Highlight(kids[1]);
                        RuntimeError("Expected a value");
                    }

                    if (Namespaces::Present(Namespaces::Current(), name)) {
                        Objects::ReplaceWithCopy(Namespaces::Find(Namespaces::Current(), name), second, true);
                    }
                    else {
                        Object *second_copy = Objects::Copy(second, true);
                        Namespaces::Track(Namespaces::Current(), second_copy);
                        Namespaces::Add(Namespaces::Current(), name, second_copy);
                    }

                    TryDestroying(second);
                }
                else {
                    Object *first = Execute(kids[0], do_continue, do_break, do_return);
                    if (first == NULL) {
                        Highlight(kids[0]);
                        RuntimeError("Expected a value");
                    }
                    if (!Objects::IsReferenceable(first)) {
                        Highlight(kids[0]);
                        RuntimeError("Not referenceable");
                    }

                    Object *second = Execute(kids[1], do_continue, do_break, do_return);
                    if (second == NULL) {
                        Highlight(kids[1]);
                        RuntimeError("Expected a value");
                    }

                    Objects::ReplaceWithCopy(first, second, true);

                    TryDestroying(second);
                }

                do_continue = false; do_break = false; do_return = false;
                return NULL;
            }
            case WHILE: {
                if (kids.size() != 2) RuntimeError("Expected 2 arguments");

                while (true) {
                    Object *cond = Execute(kids[0], do_continue, do_break, do_return);
                    if (cond == NULL || Objects::GetType(cond) != Objects::BOOL) {
                        Highlight(kids[0]);
                        RuntimeError("Expected bool value");
                    }

                    if (!*Objects::GetBool(cond)) {
                        TryDestroying(cond);
                        do_continue = false; do_break = false; do_return = false;
                        return NULL;
                    }
                    TryDestroying(cond);

                    Namespaces::Create(true);
                    Object *body = Execute(kids[1], do_continue, do_break, do_return);
                    Object *ret = Objects::Copy(body, false);
                    Namespaces::Track(Namespaces::Parent(), ret);
                    Namespaces::Destroy();

                    if (do_break) {
                        do_continue = false; do_break = false; do_return = false;
                        TryDestroying(ret);
                        return NULL;
                    }
                    if (do_return) {
                        do_continue = false; do_break = false; do_return = true;
                        return ret;
                    }
                    TryDestroying(ret);
                }
            }
            case FOR: {
                if (kids.size() != 4) RuntimeError("Expected 4 arguments");

                Object *init = Execute(kids[0], do_continue, do_break, do_return);
                TryDestroying(init);

                while (true) {
                    Object *cond = Execute(kids[1], do_continue, do_break, do_return);
                    if (cond == NULL || Objects::GetType(cond) != Objects::BOOL) {
                        Highlight(kids[1]);
                        RuntimeError("Expected bool value");
                    }

                    if (!*Objects::GetBool(cond)) {
                        TryDestroying(cond);
                        do_continue = false; do_break = false; do_return = false;
                        return NULL;
                    }
                    TryDestroying(cond);

                    Namespaces::Create(true);
                    Object *body = Execute(kids[3], do_continue, do_break, do_return);
                    Object *ret = Objects::Copy(body, false);
                    Namespaces::Track(Namespaces::Parent(), ret);
                    Namespaces::Destroy();

                    if (do_break) {
                        do_continue = false; do_break = false; do_return = false;
                        TryDestroying(ret);
                        return NULL;
                    }
                    if (do_return) {
                        do_continue = false; do_break = false; do_return = true;
                        return ret;
                    }
                    TryDestroying(ret);

                    Object *step = Execute(kids[2], do_continue, do_break, do_return);
                    TryDestroying(step);
                }
            }
            case REPEAT: {
                if (kids.size() != 2) RuntimeError("Expected 2 arguments");

                while (true) {
                    Namespaces::Create(true);
                    Object *body = Execute(kids[0], do_continue, do_break, do_return);
                    Object *ret = Objects::Copy(body, false);
                    Namespaces::Track(Namespaces::Parent(), ret);
                    Namespaces::Destroy();

                    if (do_break) {
                        do_continue = false; do_break = false; do_return = false;
                        TryDestroying(ret);
                        return NULL;
                    }
                    if (do_return) {
                        do_continue = false; do_break = false; do_return = true;
                        return ret;
                    }
                    TryDestroying(ret);

                    Object *cond = Execute(kids[1], do_continue, do_break, do_return);
                    if (cond == NULL || Objects::GetType(cond) != Objects::BOOL) {
                        Highlight(kids[0]);
                        RuntimeError("Expected bool value");
                    }

                    if (*Objects::GetBool(cond)) {
                        TryDestroying(cond);
                        do_continue = false; do_break = false; do_return = false;
                        return NULL;
                    }
                    TryDestroying(cond);
                }
            }
            case IF: {
                if (kids.size() != 3) RuntimeError("Expected 3 arguments");

                Object *cond = Execute(kids[0], do_continue, do_break, do_return);
                if (cond == NULL || Objects::GetType(cond) != Objects::BOOL) {
                    Highlight(kids[0]);
                    RuntimeError("Expected bool value");
                }

                if (*Objects::GetBool(cond)) {
                    TryDestroying(cond);

                    Namespaces::Create(true);
                    Object *body = Execute(kids[1], do_continue, do_break, do_return);
                    Object *ret = Objects::Copy(body, false);
                    Namespaces::Track(Namespaces::Parent(), ret);
                    Namespaces::Destroy();

                    if (do_continue) {
                        do_continue = true; do_break = false; do_return = false;
                        TryDestroying(ret);
                        return NULL;
                    }
                    if (do_break) {
                        do_continue = false; do_break = true; do_return = false;
                        TryDestroying(ret);
                        return NULL;
                    }
                    if (do_return) {
                        do_continue = false; do_break = false; do_return = true;
                        return ret;
                    }
                    TryDestroying(ret);
                }
                else {
                    TryDestroying(cond);

                    Namespaces::Create(true);
                    Object *body = Execute(kids[2], do_continue, do_break, do_return);
                    Object *ret = Objects::Copy(body, false);
                    Namespaces::Track(Namespaces::Parent(), ret);
                    Namespaces::Destroy();

                    if (do_continue) {
                        do_continue = true; do_break = false; do_return = false;
                        TryDestroying(ret);
                        return NULL;
                    }
                    if (do_break) {
                        do_continue = false; do_break = true; do_return = false;
                        TryDestroying(ret);
                        return NULL;
                    }
                    if (do_return) {
                        do_continue = false; do_break = false; do_return = true;
                        return ret;
                    }
                    TryDestroying(ret);
                }

                do_continue = false; do_break = false; do_return = false;
                return NULL;
            }
            case CONTINUE: {
                do_continue = true; do_break = false; do_return = false;
                return NULL;
            }
            case BREAK: {
                do_continue = false; do_break = true; do_return = false;
                return NULL;
            }
            case RETURN: {
                if (kids.size() == 0) {
                    do_continue = false; do_break = false; do_return = true;
                    return NULL;
                }
                else if (kids.size() == 1) {
                    Object *ret = Execute(kids[0], do_continue, do_break, do_return);
                    do_continue = false; do_break = false; do_return = true;
                    return ret;
                }
                else {
                    RuntimeError("Expected at most 1 argument");
                }
            }
            case FUNC: {
                if (kids.size() != 1) RuntimeError("Expected 1 argument");

                Object *func = Objects::Create(Objects::FUNCTION);
                Namespaces::Track(Namespaces::Current(), func);

                CustomTypes::FuncFromNode(Objects::GetFunc(func), kids[0]);
                
                do_continue = false; do_break = false; do_return = false;
                return func;
            }
            case ARG: {
                if (kids.size() != 1) RuntimeError("Expected 1 argument");

                Object *index = Execute(kids[0], do_continue, do_break, do_return);
                if (index == NULL || Objects::GetType(index) != Objects::INT) {
                    Highlight(kids[0]);
                    RuntimeError("Argument index must be int");
                }
                Object *arg = Namespaces::AccessStack(Namespaces::Current(), *Objects::GetInt(index));
                
                TryDestroying(index);

                do_continue = false; do_break = false; do_return = false;
                return arg;
            }
            case CALL: {
                if (kids.size() < 1) RuntimeError("Expected at least 1 argument");

                Object *func = Execute(kids[0], do_continue, do_break, do_return);
                if (func == NULL || Objects::GetType(func) != Objects::FUNCTION) {
                    Highlight(kids[0]);
                    RuntimeError("Expected a function value");
                }
                
                std::vector<Object*> args;
                for (int i = 1; i < kids.size(); i++) {
                    Object *arg = Execute(kids[i], do_continue, do_break, do_return);
                    args.push_back(arg);
                }
                std::reverse(args.begin(), args.end());

                Namespaces::Create(false);
                for (auto arg: args) {
                    arg = Objects::Copy(arg, true);
                    Namespaces::Track(Namespaces::Current(), arg);
                    Namespaces::PushOnStack(Namespaces::Current(), arg);
                }
                Object *ret = CustomTypes::FuncCall(Objects::GetFunc(func));
                Object *res = Objects::Copy(ret, false);
                Namespaces::Track(Namespaces::Parent(), res);
                Namespaces::Destroy();

                for (auto arg: args) TryDestroying(arg);
                TryDestroying(func);

                do_continue = false; do_break = false; do_return = false;
                return res;
            }
            case BOOL_CAST: {
                if (kids.size() != 1) RuntimeError("Expected 1 argument");

                Object *arg = Execute(kids[0], do_continue, do_break, do_return);

                Object *res = Objects::CastToBool(arg);
                Namespaces::Track(Namespaces::Current(), res);

                TryDestroying(arg);
                do_continue = false; do_break = false; do_return = false;
                return res;
            }
            case CHAR_CAST: {
                if (kids.size() != 1) RuntimeError("Expected 1 argument");

                Object *arg = Execute(kids[0], do_continue, do_break, do_return);

                Object *res = Objects::CastToChar(arg);
                Namespaces::Track(Namespaces::Current(), res);

                TryDestroying(arg);
                do_continue = false; do_break = false; do_return = false;
                return res;
            }
            case INT_CAST: {
                if (kids.size() != 1) RuntimeError("Expected 1 argument");

                Object *arg = Execute(kids[0], do_continue, do_break, do_return);

                Object *res = Objects::CastToInt(arg);
                Namespaces::Track(Namespaces::Current(), res);

                TryDestroying(arg);
                do_continue = false; do_break = false; do_return = false;
                return res;
            }
            case REAL_CAST: {
                if (kids.size() != 1) RuntimeError("Expected 1 argument");

                Object *arg = Execute(kids[0], do_continue, do_break, do_return);

                Object *res = Objects::CastToReal(arg);
                Namespaces::Track(Namespaces::Current(), res);

                TryDestroying(arg);
                do_continue = false; do_break = false; do_return = false;
                return res;
            }
            case STRING_CAST: {
                if (kids.size() != 1) RuntimeError("Expected 1 argument");

                Object *arg = Execute(kids[0], do_continue, do_break, do_return);

                Object *res = Objects::CastToString(arg);
                Namespaces::Track(Namespaces::Current(), res);

                TryDestroying(arg);
                do_continue = false; do_break = false; do_return = false;
                return res;
            }
            case DEREF: {
                if (kids.size() != 1) RuntimeError("Expected 1 argument");

                Object *arg = Execute(kids[0], do_continue, do_break, do_return);
                if (arg == NULL || Objects::GetType(arg) != Objects::POINTER) {
                    Highlight(kids[0]);
                    RuntimeError("Expected a pointer value");
                }

                Object *res = Objects::Deref(arg);
                TryDestroying(arg);

                do_continue = false; do_break = false; do_return = false;
                return res;
            }
            case REF: {
                if (kids.size() != 1) RuntimeError("Expected 1 argument");

                Object *arg = Execute(kids[0], do_continue, do_break, do_return);
                if (arg == NULL) {
                    Highlight(kids[0]);
                    RuntimeError("Expected a value");
                }

                Object *res = Objects::Ref(arg);
                Namespaces::Track(Namespaces::Current(), res);

                do_continue = false; do_break = false; do_return = false;
                return res;
            }
            case INV: 
            case NOT:
            case NEG: {
                if (kids.size() != 1) RuntimeError("Expected 1 argument");

                Object *arg = Execute(kids[0], do_continue, do_break, do_return);
                if (arg == NULL) {
                    Highlight(kids[0]);
                    RuntimeError("Expected a value");
                }
                
                Object *res;
                switch (node->id) {
                    case INV: res = Objects::CalcInv(arg); break;
                    case NOT: res = Objects::CalcNot(arg); break;
                    case NEG: res = Objects::CalcNeg(arg); break;
                }
                Namespaces::Track(Namespaces::Current(), res);
                TryDestroying(arg);

                do_continue = false; do_break = false; do_return = false;
                return res;
            }
            case MULT:
            case DIV:
            case REM:
            case ADD:
            case SUB:
            case SHL:
            case SHR:
            case LT:
            case GT:
            case LE:
            case GE:
            case EQ:
            case NEQ:
            case AND:
            case XOR:
            case OR:
            case CONJ:
            case DISJ: {
                if (kids.size() != 2) RuntimeError("Expected 2 arguments");

                Object *arg1 = Execute(kids[0], do_continue, do_break, do_return);
                if (arg1 == NULL) {
                    Highlight(kids[0]);
                    RuntimeError("Expected a value");
                }

                Object *arg2 = Execute(kids[1], do_continue, do_break, do_return);
                if (arg2 == NULL) {
                    Highlight(kids[1]);
                    RuntimeError("Expected a value");
                }

                Object *res;
                switch (node->id) {
                    case MULT: res = Objects::CalcMult(arg1, arg2); break;
                    case DIV: res = Objects::CalcDiv(arg1, arg2); break;
                    case REM: res = Objects::CalcRem(arg1, arg2); break;
                    case ADD: res = Objects::CalcAdd(arg1, arg2); break;
                    case SUB: res = Objects::CalcSub(arg1, arg2); break;
                    case SHL: res = Objects::CalcShl(arg1, arg2); break;
                    case SHR: res = Objects::CalcShr(arg1, arg2); break;
                    case LT: res = Objects::CalcLt(arg1, arg2); break;
                    case GT: res = Objects::CalcGt(arg1, arg2); break;
                    case LE: res = Objects::CalcLe(arg1, arg2); break;
                    case GE: res = Objects::CalcGe(arg1, arg2); break;
                    case EQ: res = Objects::CalcEq(arg1, arg2); break;
                    case NEQ: res = Objects::CalcNeq(arg1, arg2); break;
                    case AND: res = Objects::CalcAnd(arg1, arg2); break;
                    case XOR: res = Objects::CalcXor(arg1, arg2); break;
                    case OR: res = Objects::CalcOr(arg1, arg2); break;
                    case CONJ: res = Objects::CalcConj(arg1, arg2); break;
                    case DISJ: res = Objects::CalcDisj(arg1, arg2); break;
                }
                Namespaces::Track(Namespaces::Current(), res);

                TryDestroying(arg1);
                TryDestroying(arg2);

                do_continue = false; do_break = false; do_return = false;
                return res;
            }
            case DACCESS: {
                if (kids.size() != 2) RuntimeError("Expected 2 arguments");

                Object *dict = Execute(kids[0], do_continue, do_break, do_return);
                if (dict == NULL || Objects::GetType(dict) != Objects::DICT) {
                    Highlight(kids[0]);
                    RuntimeError("Expected a dict value");
                }

                Object *arg = Execute(kids[1], do_continue, do_break, do_return);

                Object *res = Objects::DictAccess(dict, arg);

                TryDestroying(dict);
                TryDestroying(arg);

                do_continue = false; do_break = false; do_return = false;
                return res;
            }
            case DSIZE: {
                if (kids.size() != 1) RuntimeError("Expected 1 argument");

                Object *dict = Execute(kids[0], do_continue, do_break, do_return);
                if (dict == NULL || Objects::GetType(dict) != Objects::DICT) {
                    Highlight(kids[0]);
                    RuntimeError("Expected a dict value");
                }

                Object *res = Objects::DictSize(dict);
                Namespaces::Track(Namespaces::Current(), res);

                TryDestroying(dict);

                do_continue = false; do_break = false; do_return = false;
                return res;
            }
            case DPRESENT: {
                if (kids.size() != 2) RuntimeError("Expected 2 arguments");

                Object *dict = Execute(kids[0], do_continue, do_break, do_return);
                if (dict == NULL || Objects::GetType(dict) != Objects::DICT) {
                    Highlight(kids[0]);
                    RuntimeError("Expected a dict value");
                }

                Object *arg = Execute(kids[1], do_continue, do_break, do_return);

                Object *res = Objects::DictPresent(dict, arg);
                Namespaces::Track(Namespaces::Current(), res);

                TryDestroying(dict);
                TryDestroying(arg);

                do_continue = false; do_break = false; do_return = false;
                return res;
            }
            case DINSERT: {
                if (kids.size() != 3) RuntimeError("Expected 3 arguments");

                Object *dict = Execute(kids[0], do_continue, do_break, do_return);
                if (dict == NULL || Objects::GetType(dict) != Objects::DICT) {
                    Highlight(kids[0]);
                    RuntimeError("Expected a dict value");
                }

                Object *arg1 = Execute(kids[1], do_continue, do_break, do_return);
                Object *arg2 = Execute(kids[2], do_continue, do_break, do_return);

                Objects::DictInsert(dict, arg1, arg2);

                TryDestroying(dict);
                TryDestroying(arg1);
                TryDestroying(arg2);

                do_continue = false; do_break = false; do_return = false;
                return NULL;
            }
            case DREMOVE: {
                if (kids.size() != 2) RuntimeError("Expected 2 arguments");

                Object *dict = Execute(kids[0], do_continue, do_break, do_return);
                if (dict == NULL || Objects::GetType(dict) != Objects::DICT) {
                    Highlight(kids[0]);
                    RuntimeError("Expected a dict value");
                }

                Object *arg = Execute(kids[1], do_continue, do_break, do_return);

                Objects::DictRemove(dict, arg);

                TryDestroying(dict);
                TryDestroying(arg);

                do_continue = false; do_break = false; do_return = false;
                return NULL;
            }
            case DKEYS: {
                if (kids.size() != 1) RuntimeError("Expected 1 argument");

                Object *dict = Execute(kids[0], do_continue, do_break, do_return);
                if (dict == NULL || Objects::GetType(dict) != Objects::DICT) {
                    Highlight(kids[0]);
                    RuntimeError("Expected a dict value");
                }

                Object *res = Objects::DictKeys(dict);
                Namespaces::Track(Namespaces::Current(), res);

                TryDestroying(dict);

                do_continue = false; do_break = false; do_return = false;
                return res;
            }
            case DVALUES: {
                if (kids.size() != 1) RuntimeError("Expected 1 argument");

                Object *dict = Execute(kids[0], do_continue, do_break, do_return);
                if (dict == NULL || Objects::GetType(dict) != Objects::DICT) {
                    Highlight(kids[0]);
                    RuntimeError("Expected a dict value");
                }

                Object *res = Objects::DictValues(dict);
                Namespaces::Track(Namespaces::Current(), res);

                TryDestroying(dict);

                do_continue = false; do_break = false; do_return = false;
                return res;
            }
            case DCLEAR: {
                if (kids.size() != 1) RuntimeError("Expected 1 argument");

                Object *dict = Execute(kids[0], do_continue, do_break, do_return);
                if (dict == NULL || Objects::GetType(dict) != Objects::DICT) {
                    Highlight(kids[0]);
                    RuntimeError("Expected a dict value");
                }

                Objects::DictClear(dict);

                do_continue = false; do_break = false; do_return = false;
                return NULL;
            }
            case SACCESS: {
                if (kids.size() != 2) RuntimeError("Expected 2 arguments");

                Object *str = Execute(kids[0], do_continue, do_break, do_return);
                if (str == NULL || Objects::GetType(str) != Objects::STRING) {
                    Highlight(kids[0]);
                    RuntimeError("Expected a string value");
                }

                Object *arg = Execute(kids[1], do_continue, do_break, do_return);

                Object *res = Objects::StringAccess(str, arg);
                Namespaces::Track(Namespaces::Current(), res);

                TryDestroying(str);
                TryDestroying(arg);

                do_continue = false; do_break = false; do_return = false;
                return res;
            }
            case SSIZE: {
                if (kids.size() != 1) RuntimeError("Expected 1 argument");

                Object *str = Execute(kids[0], do_continue, do_break, do_return);
                if (str == NULL || Objects::GetType(str) != Objects::STRING) {
                    Highlight(kids[0]);
                    RuntimeError("Expected a string value");
                }

                Object *res = Objects::StringSize(str);
                Namespaces::Track(Namespaces::Current(), res);

                TryDestroying(str);

                do_continue = false; do_break = false; do_return = false;
                return res;
            }
            case SADDSUF: {
                if (kids.size() != 2) RuntimeError("Expected 2 arguments");

                Object *str = Execute(kids[0], do_continue, do_break, do_return);
                if (str == NULL || Objects::GetType(str) != Objects::STRING) {
                    Highlight(kids[0]);
                    RuntimeError("Expected a string value");
                }

                Object *arg = Execute(kids[1], do_continue, do_break, do_return);

                Objects::StringAddSuf(str, arg);

                TryDestroying(arg);

                do_continue = false; do_break = false; do_return = false;
                return NULL;
            }
            case SADDPREF: {
                if (kids.size() != 2) RuntimeError("Expected 2 arguments");

                Object *str = Execute(kids[0], do_continue, do_break, do_return);
                if (str == NULL || Objects::GetType(str) != Objects::STRING) {
                    Highlight(kids[0]);
                    RuntimeError("Expected a string value");
                }

                Object *arg = Execute(kids[1], do_continue, do_break, do_return);

                Objects::StringAddPref(str, arg);

                TryDestroying(arg);

                do_continue = false; do_break = false; do_return = false;
                return NULL;
            }
            case SREMOVESUF: {
                if (kids.size() != 2) RuntimeError("Expected 2 arguments");

                Object *str = Execute(kids[0], do_continue, do_break, do_return);
                if (str == NULL || Objects::GetType(str) != Objects::STRING) {
                    Highlight(kids[0]);
                    RuntimeError("Expected a string value");
                }

                Object *arg = Execute(kids[1], do_continue, do_break, do_return);

                Objects::StringRemoveSuf(str, arg);

                TryDestroying(arg);

                do_continue = false; do_break = false; do_return = false;
                return NULL;
            }
            case SREMOVEPREF: {
                if (kids.size() != 2) RuntimeError("Expected 2 arguments");

                Object *str = Execute(kids[0], do_continue, do_break, do_return);
                if (str == NULL || Objects::GetType(str) != Objects::STRING) {
                    Highlight(kids[0]);
                    RuntimeError("Expected a string value");
                }

                Object *arg = Execute(kids[1], do_continue, do_break, do_return);

                Objects::StringRemovePref(str, arg);

                TryDestroying(arg);

                do_continue = false; do_break = false; do_return = false;
                return NULL;
            }
            case BOOL_LITERAL: {
                Object *res = Objects::Create(Objects::BOOL);
                *Objects::GetBool(res) = node->bool_literal;
                Namespaces::Track(Namespaces::Current(), res);
                do_continue = false; do_break = false; do_return = false;
                return res;
            }
            case CHAR_LITERAL: {
                Object *res = Objects::Create(Objects::CHAR);
                *Objects::GetChar(res) = node->char_literal;
                Namespaces::Track(Namespaces::Current(), res);
                do_continue = false; do_break = false; do_return = false;
                return res;
            }
            case INT_LITERAL: {
                Object *res = Objects::Create(Objects::INT);
                *Objects::GetInt(res) = node->int_literal;
                Namespaces::Track(Namespaces::Current(), res);
                do_continue = false; do_break = false; do_return = false;
                return res;
            }
            case REAL_LITERAL: {
                Object *res = Objects::Create(Objects::REAL);
                *Objects::GetReal(res) = node->real_literal;
                Namespaces::Track(Namespaces::Current(), res);
                do_continue = false; do_break = false; do_return = false;
                return res;
            }
            case STRING_LITERAL: {
                Object *res = Objects::Create(Objects::STRING);
                *Objects::GetString(res) = node->string_literal;
                Namespaces::Track(Namespaces::Current(), res);
                do_continue = false; do_break = false; do_return = false;
                return res;
            }
            case NULL_LITERAL: {
                Object *res = Objects::Create(Objects::POINTER);
                *Objects::GetPtr(res) = NULL;
                Namespaces::Track(Namespaces::Current(), res);
                do_continue = false; do_break = false; do_return = false;
                return res;
            }
            case DICT_LITERAL: {
                Object *res = Objects::Create(Objects::DICT);
                Namespaces::Track(Namespaces::Current(), res);
                do_continue = false; do_break = false; do_return = false;
                return res;
            }
            case NAME: {
                Object *res = Namespaces::Find(Namespaces::Current(), node->name);
                do_continue = false; do_break = false; do_return = false;
                return res;
            }
            case BLOCK: {
                Namespaces::Create(true);
                Object *ret = NULL;
                for (auto kid: kids) {
                    Object *res = Execute(kid, do_continue, do_break, do_return);
                    if (do_continue) {
                        do_continue = true; do_break = false; do_return = false;
                        TryDestroying(res);
                        break;
                    }
                    if (do_break) {
                        do_continue = false; do_break = true; do_return = false;
                        TryDestroying(res);
                        break;
                    }
                    if (do_return) {
                        do_continue = false; do_break = false; do_return = true;
                        ret = Objects::Copy(res, false);
                        Namespaces::Track(Namespaces::Parent(), ret);
                        TryDestroying(res);
                        break;
                    }
                    TryDestroying(res);
                }
                Namespaces::Destroy();
                return ret;
            }
        }

        return NULL;
    }
}