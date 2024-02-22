#include "objects.hpp"
#include "errors.hpp"
#include "parser.hpp"
#include "hashing.hpp"

#include <cmath>
#include <iostream>

struct Object {
    Objects::Type type;
    union {
        BOOL_T *_bool;
        CHAR_T *_char;
        INT_T *_int;
        REAL_T *_real;
        STRING_T *_string;
        PTR_T *_ptr;
        DICT_T *_dict;
        FUNC_T *_func;
    };
    bool is_referenceable;
};

namespace Objects {
    static void CheckNULL(Object *obj) {
        if (obj == NULL) RuntimeError("NULL object");
    }

    static void CheckType(Object *obj, int types) {
        if ((obj->type & types) == 0) RuntimeError("Invalid type");
    }


    Type GetType(Object *obj) {
        return obj->type;
    }
    BOOL_T *GetBool(Object *obj) {
        return obj->_bool;
    }
    CHAR_T *GetChar(Object *obj) {
        return obj->_char;
    }
    INT_T *GetInt(Object *obj) {
        return obj->_int;
    }
    REAL_T *GetReal(Object *obj) {
        return obj->_real;
    }
    STRING_T *GetString(Object *obj) {
        return obj->_string;
    }
    PTR_T *GetPtr(Object *obj) {
        return obj->_ptr;
    }
    DICT_T *GetDict(Object *obj) {
        return obj->_dict;
    }
    FUNC_T *GetFunc(Object *obj) {
        return obj->_func;
    }
    bool IsReferenceable(Object *obj) {
        return obj->is_referenceable;
    }


    Object *Create(Type type) {
        Object *res = new Object;
        res->type = type;
        switch (type) {
            case BOOL: res->_bool = new BOOL_T; break;
            case CHAR: res->_char = new CHAR_T; break;
            case INT: res->_int = new INT_T; break;
            case REAL: res->_real = new REAL_T; break;
            case STRING: res->_string = new STRING_T; break;
            case POINTER: res->_ptr = new PTR_T; break;
            case DICT: res->_dict = CustomTypes::DictCreate(); break;
            case FUNCTION: res->_func = CustomTypes::FuncCreate(); break;
        }
        res->is_referenceable = false;
        return res;
    }
    void Destroy(Object *obj) {
        CheckNULL(obj);
        switch (obj->type) {
            case BOOL: delete obj->_bool; break;
            case CHAR: delete obj->_char; break;
            case INT: delete obj->_int; break;
            case REAL: delete obj->_real; break;
            case STRING: delete obj->_string; break;
            case POINTER: delete obj->_ptr; break;
            case DICT: CustomTypes::DictDestroy(obj->_dict); break;
            case FUNCTION: CustomTypes::FuncDestroy(obj->_func); break;
        }
        delete obj;
    }
    Object *Copy(Object *obj, bool make_referenceable) {
        if (obj == NULL) return NULL;

        Object *res = new Object;
        res->type = obj->type;
        res->is_referenceable = make_referenceable;

        switch (res->type) {
            case BOOL: res->_bool = new BOOL_T{*obj->_bool}; break;
            case CHAR: res->_char = new CHAR_T{*obj->_char}; break;
            case INT: res->_int = new INT_T{*obj->_int}; break;
            case REAL: res->_real = new REAL_T{*obj->_real}; break;
            case STRING: res->_string = new STRING_T{*obj->_string}; break;
            case POINTER: res->_ptr = new PTR_T{*obj->_ptr}; break;
            case DICT: res->_dict = CustomTypes::DictCopy(obj->_dict); break;
            case FUNCTION: res->_func = CustomTypes::FuncCopy(obj->_func); break;
        }

        return res;
    }
    void ReplaceWithCopy(Object *first, Object *second, bool make_referenceable) {
        CheckNULL(first);
        CheckNULL(second);

        switch (first->type) {
            case BOOL: delete first->_bool; break;
            case CHAR: delete first->_char; break;
            case INT: delete first->_int; break;
            case REAL: delete first->_real; break;
            case STRING: delete first->_string; break;
            case POINTER: delete first->_ptr; break;
            case DICT: CustomTypes::DictDestroy(first->_dict); break;
            case FUNCTION: CustomTypes::FuncDestroy(first->_func); break;
        }

        Object *copy = Copy(second, make_referenceable);
        *first = *copy;
        first->is_referenceable = true;
        delete copy;
    }
    void MakeReferenceable(Object *first) {
        CheckNULL(first);
        first->is_referenceable = true;
    }

    Object *FunctionCall(Object *first) {
        CheckNULL(first);
        CheckType(first, FUNCTION);
        return CustomTypes::FuncCall(first->_func);
    }
    Object *CastToBool(Object *first) {
        CheckNULL(first);
        Object *res = Create(BOOL);
        res->is_referenceable = false;
        switch (first->type) {
            case BOOL: *res->_bool = *first->_bool; break;
            case CHAR: *res->_bool = *first->_char != 0; break;
            case INT: *res->_bool = *first->_int != 0; break;
            case REAL: *res->_bool = *first->_real != 0; break;
            case STRING: *res->_bool = !first->_string->empty(); break;
            case POINTER: *res->_bool = *first->_ptr != NULL; break;
            case DICT: *res->_bool = CustomTypes::DictSize(first->_dict) != 0; break;
            case FUNCTION: *res->_bool = true; break;
        }
        return res;
    }
    Object *CastToChar(Object *first) {
        CheckNULL(first);
        CheckType(first, BOOL | CHAR | INT);
        Object *res = Create(CHAR);
        res->is_referenceable = false;
        switch (first->type) {
            case BOOL: *res->_char = *first->_bool; break;
            case CHAR: *res->_char = *first->_char; break;
            case INT: *res->_char = *first->_int; break;
        }
        return res;
    }
    Object *CastToInt(Object *first) {
        CheckNULL(first);
        CheckType(first, BOOL | CHAR | INT | REAL | STRING);
        Object *res = Create(INT);
        res->is_referenceable = false;
        switch (first->type) {
            case BOOL: *res->_int = *first->_bool; break;
            case CHAR: *res->_int = *first->_char; break;
            case INT: *res->_int = *first->_int; break;
            case REAL: *res->_int = *first->_real; break;
            case STRING: {
                try {
                    *res->_int = std::stoll(*first->_string); break;
                } catch (...) {
                    RuntimeError("Conversion to int has failed");
                }
            }
        }
        return res;
    }
    Object *CastToReal(Object *first) {
        CheckNULL(first);
        CheckType(first, BOOL | CHAR | INT | REAL | STRING);
        Object *res = Create(REAL);
        res->is_referenceable = false;
        switch (first->type) {
            case BOOL: *res->_real = *first->_bool; break;
            case CHAR: *res->_real = *first->_char; break;
            case INT: *res->_real = *first->_int; break;
            case REAL: *res->_real = *first->_real; break;
            case STRING: {
                try {
                    *res->_real = std::stod(*first->_string); break;
                } catch (...) {
                    RuntimeError("Conversion to real has failed");
                }
            }
        }
        return res;
    }
    Object *CastToString(Object *first) {
        CheckNULL(first);
        Object *res = Create(STRING);
        res->is_referenceable = false;
        switch (first->type) {
            case BOOL: *res->_string = (*first->_bool?"true":"false"); break;
            case CHAR: *res->_string = std::string{*first->_char}; break;
            case INT: *res->_string = std::to_string(*first->_int); break;
            case REAL: *res->_string = std::to_string(*first->_real); break;
            case STRING: *res->_string = *first->_string; break;
            case POINTER: *res->_string = std::to_string((uint64_t)*first->_ptr); break;
            case DICT: *res->_string = CustomTypes::DictString(first->_dict); break;
            case FUNCTION: *res->_string = "function"; break;
        }
        return res;
    }
    Object *Deref(Object *first) {
        CheckNULL(first);
        CheckType(first, POINTER);

        if (*first->_ptr == NULL) RuntimeError("Deref of NULL pointer");
        return (Object*)*first->_ptr;
    }
    Object *Ref(Object *first) {
        CheckNULL(first);

        Object *res = Create(POINTER);
        *res->_ptr = (PTR_T)first;
        return res;
    }
    Object *CalcInv(Object *first) {
        CheckNULL(first);
        CheckType(first, INT);
        Object *res = Create(INT);
        *res->_int = ~*first->_int;
        return res;
    }
    Object *CalcNot(Object *first) {
        CheckNULL(first);
        CheckType(first, BOOL);
        Object *res = Create(BOOL);
        *res->_bool = !*first->_bool;
        return res;
    }
    Object *CalcNeg(Object *first) {
        CheckNULL(first);
        CheckType(first, INT | REAL);
        Object *res;
        if (first->type == INT) {
            res = Create(INT);
            *res->_int = -*first->_int;
        }
        else {
            res = Create(REAL);
            *res->_real = -*first->_real;
        }
        return res;
    }
    Object *CalcMult(Object *first, Object *second) {
        CheckNULL(first);
        CheckNULL(second);
        CheckType(first, INT | REAL);
        CheckType(second, INT | REAL);
        Object *res;
        if (first->type == INT && second->type == INT) {
            res = Create(INT);
            *res->_int = *first->_int * *second->_int;
        }
        else if (first->type == INT && second->type == REAL) {
            res = Create(REAL);
            *res->_real = *first->_int * *second->_real;
        }
        else if (first->type == REAL && second->type == INT) {
            res = Create(REAL);
            *res->_real = *first->_real * *second->_int;
        }
        else {
            res = Create(REAL);
            *res->_real = *first->_real * *second->_real;
        }
        return res;
    }
    Object *CalcDiv(Object *first, Object *second) {
        CheckNULL(first);
        CheckNULL(second);
        CheckType(first, INT | REAL);
        CheckType(second, INT | REAL);
        Object *res;
        if (first->type == INT && second->type == INT) {
            res = Create(INT);
            *res->_int = *first->_int / *second->_int;
        }
        else if (first->type == INT && second->type == REAL) {
            res = Create(REAL);
            *res->_real = *first->_int / *second->_real;
        }
        else if (first->type == REAL && second->type == INT) {
            res = Create(REAL);
            *res->_real = *first->_real / *second->_int;
        }
        else {
            res = Create(REAL);
            *res->_real = *first->_real / *second->_real;
        }
        return res;
    }
    Object *CalcRem(Object *first, Object *second) {
        CheckNULL(first);
        CheckNULL(second);
        CheckType(first, INT | REAL);
        CheckType(second, INT | REAL);
        Object *res;
        if (first->type == INT && second->type == INT) {
            res = Create(INT);
            *res->_int = *first->_int % *second->_int;
        }
        else if (first->type == INT && second->type == REAL) {
            res = Create(REAL);
            *res->_real = std::remainder(*first->_int, *second->_real);
        }
        else if (first->type == REAL && second->type == INT) {
            res = Create(REAL);
            *res->_real = std::remainder(*first->_real, *second->_int);
        }
        else {
            res = Create(REAL);
            *res->_real = std::remainder(*first->_real, *second->_real);
        }
        return res;
    }
    Object *CalcAdd(Object *first, Object *second) {
        CheckNULL(first);
        CheckNULL(second);
        CheckType(first, INT | REAL);
        CheckType(second, INT | REAL);
        Object *res;
        if (first->type == INT && second->type == INT) {
            res = Create(INT);
            *res->_int = *first->_int + *second->_int;
        }
        else if (first->type == INT && second->type == REAL) {
            res = Create(REAL);
            *res->_real = *first->_int + *second->_real;
        }
        else if (first->type == REAL && second->type == INT) {
            res = Create(REAL);
            *res->_real = *first->_real + *second->_int;
        }
        else {
            res = Create(REAL);
            *res->_real = *first->_real + *second->_real;
        }
        return res;
    }
    Object *CalcSub(Object *first, Object *second) {
        CheckNULL(first);
        CheckNULL(second);
        CheckType(first, INT | REAL);
        CheckType(second, INT | REAL);
        Object *res;
        if (first->type == INT && second->type == INT) {
            res = Create(INT);
            *res->_int = *first->_int - *second->_int;
        }
        else if (first->type == INT && second->type == REAL) {
            res = Create(REAL);
            *res->_real = *first->_int - *second->_real;
        }
        else if (first->type == REAL && second->type == INT) {
            res = Create(REAL);
            *res->_real = *first->_real - *second->_int;
        }
        else {
            res = Create(REAL);
            *res->_real = *first->_real - *second->_real;
        }
        return res;
    }
    Object *CalcShl(Object *first, Object *second) {
        CheckNULL(first);
        CheckNULL(second);
        CheckType(first, INT);
        CheckType(second, INT);
        Object *res = Create(INT);
        *res->_int = *first->_int << *second->_int;
        return res;
    }
    Object *CalcShr(Object *first, Object *second) {
        CheckNULL(first);
        CheckNULL(second);
        CheckType(first, INT);
        CheckType(second, INT);
        Object *res = Create(INT);
        *res->_int = *first->_int >> *second->_int;
        return res;
    }
    Object *CalcLt(Object *first, Object *second) {
        CheckNULL(first);
        CheckNULL(second);
        CheckType(first, INT | REAL);
        CheckType(second, INT | REAL);
        Object *res = Create(BOOL);
        if (first->type == INT && second->type == INT) {
            *res->_bool = *first->_int < *second->_int;
        }
        else if (first->type == INT && second->type == REAL) {
            *res->_bool = *first->_int < *second->_real;
        }
        else if (first->type == REAL && second->type == INT) {
            *res->_bool = *first->_real < *second->_int;
        }
        else {
            *res->_bool = *first->_real < *second->_real;
        }
        return res;
    }
    Object *CalcGt(Object *first, Object *second) {
        return CalcLt(second, first);
    }
    Object *CalcLe(Object *first, Object *second) {
        CheckNULL(first);
        CheckNULL(second);
        CheckType(first, INT | REAL);
        CheckType(second, INT | REAL);
        Object *res = Create(BOOL);
        if (first->type == INT && second->type == INT) {
            *res->_bool = *first->_int <= *second->_int;
        }
        else if (first->type == INT && second->type == REAL) {
            *res->_bool = *first->_int <= *second->_real;
        }
        else if (first->type == REAL && second->type == INT) {
            *res->_bool = *first->_real <= *second->_int;
        }
        else {
            *res->_bool = *first->_real <= *second->_real;
        }
        return res;
    }
    Object *CalcGe(Object *first, Object *second) {
        return CalcLe(second, first);
    }
    Object *CalcEq(Object *first, Object *second) {
        CheckNULL(first);
        CheckNULL(second);
        Object *res = Create(BOOL);
        if (first->type != second->type) {
            *res->_bool = false;
            return res;
        }
        switch (first->type) {
            case BOOL: *res->_bool = *first->_bool == *second->_bool; break;
            case CHAR: *res->_bool = *first->_char == *second->_char; break;
            case INT: *res->_bool = *first->_int == *second->_int; break;
            case REAL: *res->_bool = *first->_real == *second->_real; break;
            case STRING: *res->_bool = *first->_string == *second->_string; break;
            case POINTER: *res->_bool = *first->_ptr == *second->_ptr; break;
            case DICT: *res->_bool = CustomTypes::DictEqual(first->_dict, second->_dict); break;
            case FUNCTION: *res->_bool = CustomTypes::FuncEqual(first->_func, second->_func); break;
        }
        return res;
    }
    Object *CalcNeq(Object *first, Object *second) {
        Object *res = CalcEq(first, second);
        *res->_bool = !*res->_bool;
        return res;
    }
    Object *CalcAnd(Object *first, Object *second) {
        CheckNULL(first);
        CheckNULL(second);
        CheckType(first, INT);
        CheckType(second, INT);
        Object *res = Create(INT);
        *res->_int = *first->_int & *second->_int;
        return res;
    }
    Object *CalcXor(Object *first, Object *second) {
        CheckNULL(first);
        CheckNULL(second);
        CheckType(first, INT);
        CheckType(second, INT);
        Object *res = Create(INT);
        *res->_int = *first->_int ^ *second->_int;
        return res;
    }
    Object *CalcOr(Object *first, Object *second) {
        CheckNULL(first);
        CheckNULL(second);
        CheckType(first, INT);
        CheckType(second, INT);
        Object *res = Create(INT);
        *res->_int = *first->_int | *second->_int;
        return res;
    }
    Object *CalcConj(Object *first, Object *second) {
        CheckNULL(first);
        CheckNULL(second);
        CheckType(first, BOOL);
        CheckType(second, BOOL);
        Object *res = Create(BOOL);
        *res->_bool = *first->_bool && *second->_bool;
        return res;
    }
    Object *CalcDisj(Object *first, Object *second) {
        CheckNULL(first);
        CheckNULL(second);
        CheckType(first, BOOL);
        CheckType(second, BOOL);
        Object *res = Create(BOOL);
        *res->_bool = *first->_bool || *second->_bool;
        return res;
    }
    Object *DictAccess(Object *first, Object *second) {
        CheckNULL(first);
        CheckNULL(second);
        CheckType(first, DICT);
        if (!first->is_referenceable) RuntimeError("Expected a referenceable argument");
        return CustomTypes::DictAccess(first->_dict, second);
    }
    Object *DictSize(Object *first) {
        CheckNULL(first);
        CheckType(first, DICT);
        if (!first->is_referenceable) RuntimeError("Expected a referenceable argument");
        Object *res = Create(INT);
        *res->_int = CustomTypes::DictSize(first->_dict);
        return res;
    }
    Object *DictPresent(Object *first, Object *second) {
        CheckNULL(first);
        CheckNULL(second);
        CheckType(first, DICT);
        if (!first->is_referenceable) RuntimeError("Expected a referenceable argument");
        Object *res = Create(BOOL);
        *res->_bool = CustomTypes::DictPresent(first->_dict, second);
        return res;
    }
    void DictInsert(Object *first, Object *second, Object *third) {
        CheckNULL(first);
        CheckNULL(second);
        CheckNULL(third);
        CheckType(first, DICT);
        if (!first->is_referenceable) RuntimeError("Expected a referenceable argument");
        CustomTypes::DictInsert(first->_dict, second, third);
    }
    void DictRemove(Object *first, Object *second) {
        CheckNULL(first);
        CheckNULL(second);
        CheckType(first, DICT);
        if (!first->is_referenceable) RuntimeError("Expected a referenceable argument");
        CustomTypes::DictRemove(first->_dict, second);
    }
    Object *DictKeys(Object *first) {
        CheckNULL(first);
        CheckType(first, DICT);
        if (!first->is_referenceable) RuntimeError("Expected a referenceable argument");
        Object *res = new Object;
        res->type = DICT;
        res->is_referenceable = true;
        res->_dict = CustomTypes::DictKeys(first->_dict);
        return res;
    }
    Object *DictValues(Object *first) {
        CheckNULL(first);
        CheckType(first, DICT);
        if (!first->is_referenceable) RuntimeError("Expected a referenceable argument");
        Object *res = new Object;
        res->type = DICT;
        res->is_referenceable = true;
        res->_dict = CustomTypes::DictValues(first->_dict);
        return res;
    }
    void DictClear(Object *first) {
        CheckNULL(first);
        CheckType(first, DICT);
        if (!first->is_referenceable) RuntimeError("Expected a referenceable argument");
        CustomTypes::DictClear(first->_dict);
    }
    Object *StringAccess(Object *first, Object *second) {
        CheckNULL(first);
        CheckNULL(second);
        CheckType(first, STRING);
        CheckType(second, INT);
        if (!first->is_referenceable) RuntimeError("Expected a referenceable argument");
        STRING_T *str = first->_string;
        INT_T index = *second->_int;

        if (!(0 <= index && index < str->size())) RuntimeError("String access out of bounds");
        Object *res = Create(CHAR);
        *res->_char = str->operator[](index);
        return res;
    }
    Object *StringSize(Object *first) {
        CheckNULL(first);
        CheckType(first, STRING);
        if (!first->is_referenceable) RuntimeError("Expected a referenceable argument");

        Object *res = Create(INT);
        *res->_int = first->_string->size();
        return res;
    }
    void StringAddSuf(Object *first, Object *second) {
        CheckNULL(first);
        CheckNULL(second);
        CheckType(first, STRING);
        CheckType(second, STRING);
        if (!first->is_referenceable) RuntimeError("Expected a referenceable argument");

        *first->_string += *second->_string;
    }
    void StringAddPref(Object *first, Object *second) {
        CheckNULL(first);
        CheckNULL(second);
        CheckType(first, STRING);
        CheckType(second, STRING);
        if (!first->is_referenceable) RuntimeError("Expected a referenceable argument");

        *first->_string = *second->_string + *first->_string;
    }
    void StringRemoveSuf(Object *first, Object *second) {
        CheckNULL(first);
        CheckNULL(second);
        CheckType(first, STRING);
        CheckType(second, INT);
        if (!first->is_referenceable) RuntimeError("Expected a referenceable argument");

        STRING_T *str = first->_string;
        INT_T n = *second->_int;

        if (n < 0 || n > str->size()) RuntimeError("Invalid number of characters to remove");

        str->erase(str->end() - n, str->end());
    }
    void StringRemovePref(Object *first, Object *second) {
        CheckNULL(first);
        CheckNULL(second);
        CheckType(first, STRING);
        CheckType(second, INT);
        if (!first->is_referenceable) RuntimeError("Expected a referenceable argument");

        STRING_T *str = first->_string;
        INT_T n = *second->_int;

        if (n < 0 || n > str->size()) RuntimeError("Invalid number of characters to remove");

        str->erase(str->begin(), str->begin() + n);
    }

    bool Equal(Object *first, Object *second) {
        CheckNULL(first);
        CheckNULL(second);
        if (first->type != second->type) {
            return false;
        }
        switch (first->type) {
            case BOOL: return *first->_bool == *second->_bool;
            case CHAR: return *first->_char == *second->_char;
            case INT: return *first->_int == *second->_int;
            case REAL: return *first->_real == *second->_real;
            case STRING: return *first->_string == *second->_string;
            case POINTER: return *first->_ptr == *second->_ptr;
            case DICT: return CustomTypes::DictEqual(first->_dict, second->_dict);
            case FUNCTION: return CustomTypes::FuncEqual(first->_func, second->_func);
        }
        
        return false;
    }

    const uint64_t bool_seed = 0x997810ba245f42e8ul;
    const uint64_t char_seed = 0x2b5d5602af50d4cbul;
    const uint64_t int_seed = 0x8174c17cc45eebb6ul;
    const uint64_t real_seed = 0xc16d72a377d599f9ul;
    const uint64_t string_seed = 0xed3e324c65d07a7bul;
    const uint64_t pointer_seed = 0x682bfe2b82aa8171ul;

    uint64_t Hash(Object *obj) {
        CheckNULL(obj);
        switch (obj->type) {
            case BOOL: return Hashing::Hash(bool_seed, (uint64_t)*obj->_bool);
            case CHAR: return Hashing::Hash(char_seed, (uint64_t)*obj->_char);
            case INT: return Hashing::Hash(int_seed, (uint64_t)*obj->_int);
            case REAL: return Hashing::Hash(real_seed, (uint64_t)*obj->_real);
            case STRING: {
                uint64_t res = string_seed;
                for (auto c:*obj->_string) res = Hashing::Hash(res, (uint64_t)c);
                return res;
            }
            case POINTER: return Hashing::Hash(pointer_seed, (uint64_t)*obj->_ptr);
            case DICT: return CustomTypes::DictHash(obj->_dict);
            case FUNCTION: return CustomTypes::FuncHash(obj->_func);
        }
        return -1;
    }

    INT_T *IntPart(Object *first) {
        return first->_int; // unsafe but fast, haha
    }

    std::string AsString(Object *first) {
        CheckNULL(first);
        std::string res;
        switch (first->type) {
            case BOOL: res = (*first->_bool?"true":"false"); break;
            case CHAR: res = std::string{*first->_char}; break;
            case INT: res = std::to_string(*first->_int); break;
            case REAL: res = std::to_string(*first->_real); break;
            case STRING: res = *first->_string; break;
            case POINTER: res = std::to_string((uint64_t)*first->_ptr); break;
            case DICT: res = CustomTypes::DictString(first->_dict); break;
            case FUNCTION: res = "function"; break;
        }
        return res;
    }
}