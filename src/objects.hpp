#pragma once

#include "util.hpp"
#include "custom_types.hpp"

namespace Objects {
    enum Type {
        BOOL = 1, CHAR = 2, INT = 4, REAL = 8, STRING = 16, POINTER = 32, DICT = 64, FUNCTION = 128
    };

    Type GetType(Object *obj);
    BOOL_T *GetBool(Object *obj);
    CHAR_T *GetChar(Object *obj);
    INT_T *GetInt(Object *obj);
    REAL_T *GetReal(Object *obj);
    STRING_T *GetString(Object *obj);
    PTR_T *GetPtr(Object *obj);
    DICT_T *GetDict(Object *obj);
    FUNC_T *GetFunc(Object *obj);
    bool IsReferenceable(Object *obj);
    void MakeReferenceable(Object *obj);

    Object *Create(Type type);
    void Destroy(Object *obj);
    Object *Copy(Object *obj, bool make_referenceable); // deep copy. the copy is not added to any namespace 
    void ReplaceWithCopy(Object *first, Object *second, bool make_referenceable);

    Object *FunctionCall(Object *first);
    Object *CastToBool(Object *first);
    Object *CastToChar(Object *first);
    Object *CastToInt(Object *first);
    Object *CastToReal(Object *first);
    Object *CastToString(Object *first);
    Object *Deref(Object *first);
    Object *Ref(Object *first);
    Object *CalcInv(Object *first);
    Object *CalcNot(Object *first);
    Object *CalcNeg(Object *first);
    Object *CalcMult(Object *first, Object *second);
    Object *CalcDiv(Object *first, Object *second);
    Object *CalcRem(Object *first, Object *second);
    Object *CalcAdd(Object *first, Object *second);
    Object *CalcSub(Object *first, Object *second);
    Object *CalcShl(Object *first, Object *second);
    Object *CalcShr(Object *first, Object *second);
    Object *CalcLt(Object *first, Object *second);
    Object *CalcGt(Object *first, Object *second);
    Object *CalcLe(Object *first, Object *second);
    Object *CalcGe(Object *first, Object *second);
    Object *CalcEq(Object *first, Object *second);
    Object *CalcNeq(Object *first, Object *second);
    Object *CalcAnd(Object *first, Object *second);
    Object *CalcXor(Object *first, Object *second);
    Object *CalcOr(Object *first, Object *second);
    Object *CalcConj(Object *first, Object *second);
    Object *CalcDisj(Object *first, Object *second);
    Object *DictAccess(Object *first, Object *second);
    Object *DictSize(Object *first);
    Object *DictPresent(Object *first, Object *second);
    void DictInsert(Object *first, Object *second, Object *third);
    void DictRemove(Object *first, Object *second);
    Object *DictKeys(Object *first);
    Object *DictValues(Object *first);
    void DictClear(Object *first);
    Object *StringAccess(Object *first, Object *second);
    Object *StringSize(Object *first);
    void StringAddSuf(Object *first, Object *second);
    void StringAddPref(Object *first, Object *second);
    void StringRemoveSuf(Object *first, Object *second);
    void StringRemovePref(Object *first, Object *second);

    bool Equal(Object *first, Object *second);
    uint64_t Hash(Object *obj);
    std::string AsString(Object *first);
}