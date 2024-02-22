#include "tokenizer.hpp"

#include <string>
#include <cctype>
#include <algorithm>

#include "errors.hpp"

namespace Tokenizer {
    const std::vector<std::string> keywords = {
        "set", "while", "for", "repeat", "if", "continue", "break", "return", "func",
        "arg", "call", "bool", "char", "int", "real", "string", "deref", "ref", "inv",
        "not", "neg", "mult", "div", "rem", "add", "sub", "shl", "shr", "lt", "gt", "le",
        "ge", "eq", "neq", "and", "xor", "or", "conj", "disj", "[d]", "[dn]", "[d?]",
        "[d+]", "[d-]", "[dk]", "[dv]", "[dc]", "[s]", "[sn]", "[s+]", "[+s]", "[s-]", 
        "[-s]", "(", ")"
    };

    bool IsKeyword(std::string s) {
        return std::find(keywords.begin(), keywords.end(), s) != keywords.end();
    }

    int KeywordId(std::string s) {
        return std::find(keywords.begin(), keywords.end(), s) - keywords.begin();
    }

    bool IsBool(std::string &s, BOOL_T &res) {
        if (s == "true") {
            res = true;
            return true;
        }
        if (s == "false") {
            res = false;
            return true;
        }
        return false;
    }
    bool IsInt(std::string &s, INT_T &res) {
        size_t pos;
        try {
            res = std::stoll(s, &pos);
            if (pos != s.size()) return false;
            return true;
        } 
        catch (...) {
            return false;
        }
        return false;
    }
    bool IsReal(std::string &s, REAL_T &res) {
        size_t pos;
        try {
            res = std::stod(s, &pos);
            if (pos != s.size()) return false;
            return true;
        } 
        catch (...) {
            return false;
        }
        return false;
    }
    bool IsNULL(std::string &s) {
        return s == "NULL";
    }
    bool IsDict(std::string &s) {
        return s == "{}";
    }

    std::vector<Token> Do(std::fstream &fd) {
        std::vector<Token> res;
        std::string stack;
        int pos;
        while (true) {
            pos = fd.tellg();
            char c = fd.get();
            if (fd.fail() || c == EOF) break;
            if (isspace(c)) {
                if (stack.empty()) continue;

                for (auto kw: keywords) {
                    if (stack != kw) continue;    
                    res.push_back(Token{(TokenId)KeywordId(kw), 0, 0, 0, 0, "", 
                                        {}, pos - (int)kw.size() + 1, pos});
                    stack = "";
                    break;
                }
                if (!stack.empty())
                res.push_back(Token{NAME, 0, 0, 0, 0, "", 
                            Names::GetName(stack), pos - (int)stack.size(), pos - 1});
                stack = "";
                continue;
            }
            
            if (c == '\"') {
                Token token;
                token.id = STRING_LITERAL;
                token.begin_in_text = pos;

                while (true) {
                    pos = fd.tellg();
                    c = fd.get();
                    if (fd.fail() || c == EOF) {
                        Highlight(token.begin_in_text, pos);
                        TokenizationError("Expected a double quote");
                    }

                    if (c == '\\') {
                        pos = fd.tellg();
                        c = fd.get();
                        if (fd.fail() || c == EOF) {
                            Highlight(token.begin_in_text, pos);
                            TokenizationError("Expected a character");
                        }
                        if (c == 'n') token.string_literal += '\n';
                        else if (c == 't') token.string_literal += '\t';
                        else if (c == 'r') token.string_literal += '\r';
                        else if (c == 'b') token.string_literal += '\b';
                        else token.string_literal += c;
                    }
                    else if (c == '"') {
                        break;
                    }
                    else {
                        token.string_literal += c;
                    }
                }

                token.end_in_text = pos;
                res.push_back(token);
                continue;
            }
            if (c == '\'') {
                Token token;
                token.id = CHAR_LITERAL;
                token.begin_in_text = pos;

                pos = fd.tellg();
                c = fd.get();
                if (fd.fail() || c == EOF) {
                    Highlight(token.begin_in_text, pos);
                    TokenizationError("Expected a character");
                }

                if (c == '\\') {
                    pos = fd.tellg();
                    c = fd.get();
                    if (fd.fail() || c == EOF) {
                        Highlight(token.begin_in_text, pos);
                        TokenizationError("Expected a character");
                    }
                    if (c == 'n') token.char_literal = '\n';
                    else if (c == 't') token.char_literal = '\t';
                    else if (c == 'r') token.char_literal = '\r';
                    else if (c == 'b') token.char_literal = '\b';
                    else token.char_literal = c;
                }
                else {
                    token.char_literal = c;
                }

                pos = fd.tellg();
                c = fd.get();
                if (fd.fail() || c != '\'') {
                    Highlight(token.begin_in_text, pos);
                    TokenizationError("Expected a single quote");
                }
                res.push_back(token);
                continue;
            }
            
            std::string kw;
            if (c == '(') {
                if (!stack.empty())
                    res.push_back(Token{NAME, 0, 0, 0, 0, "",
                                Names::GetName(stack), 
                                pos - (int)stack.size(), 
                                pos - 1});
                stack = "";

                res.push_back(Token{OPEN_BRACKET, 0, 0, 0, 0, "", 
                                    {}, pos, pos});
                continue;
            }
            if (c == ')') {
                if (!stack.empty()) 
                    for (auto kw: keywords) {
                        if (stack != kw) continue;    
                        res.push_back(Token{(TokenId)KeywordId(kw), 0, 0, 0, 0, "", 
                                            {}, pos - (int)kw.size() + 1, pos});
                        stack = "";
                        break;
                    }
                if (!stack.empty())
                    res.push_back(Token{NAME, 0, 0, 0, 0, "",
                                Names::GetName(stack), 
                                pos - (int)stack.size(), 
                                pos - 1});
                stack = "";

                res.push_back(Token{CLOSED_BRACKED, 0, 0, 0, 0, "", 
                                    {}, pos, pos});
                continue;
            }
            stack += c;
        }
        if (!stack.empty()) {
            res.push_back(Token{NAME, 0, 0, 0, 0, "", Names::GetName(stack), 
                                pos - (int)stack.size() + 1, pos});
        }

        // stage2

        for (auto &token: res) {
            if (token.id != NAME) continue;

            BOOL_T bool_res;
            if (IsBool(*token.name.str, bool_res)) {
                token.id = BOOL_LITERAL;
                token.bool_literal = bool_res;
                continue;
            }
            INT_T int_res;
            if (IsInt(*token.name.str, int_res)) {
                token.id = INT_LITERAL;
                token.int_literal = int_res;
                continue;
            }
            REAL_T real_res;
            if (IsReal(*token.name.str, real_res)) {
                token.id = REAL_LITERAL;
                token.real_literal = real_res;
                continue;
            }
            if (IsNULL(*token.name.str)) {
                token.id = NULL_LITERAL;
                continue;
            }
            if (IsDict(*token.name.str)) {
                token.id = DICT_LITERAL;
                continue;
            }
        }
        return res;
    }
}