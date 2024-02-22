#pragma once

#include "objects.hpp"
#include "parser.hpp"

namespace CustomTypes {
    /*
    
    dict stores copies of keys and values passed to it.
    therefore, when destroyed, it simply destroyes all of the items it has

    */

    DICT_T *DictCreate();
    void DictDestroy(DICT_T *dict);
    DICT_T *DictCopy(DICT_T *dict); // deep copy

    Object *DictAccess(DICT_T *dict, Object *key);
    int DictSize(DICT_T *dict);
    bool DictPresent(DICT_T *dict, Object *key);
    void DictInsert(DICT_T *dict, Object *key, Object *val);
    void DictRemove(DICT_T *dict, Object *key);
    DICT_T *DictKeys(DICT_T *dict);
    DICT_T *DictValues(DICT_T *dict);
    std::string DictString(DICT_T *dict);
    bool DictEqual(DICT_T *first, DICT_T *second);
    void DictClear(DICT_T *first);

    uint64_t DictHash(DICT_T *dict);

    /*
    
    function is implemented as a pointer to a parse tree node.
    however, some functions may be implemented internally, as C++ functions;
    
    */

    FUNC_T *FuncCreate(); // creates an empty function which should be modified later.
    void FuncDestroy(FUNC_T *func); // frees memory of the function structure.
    FUNC_T *FuncCopy(FUNC_T *func); // simply copies the structure.
    void FuncFromNode(FUNC_T *func, Node *node);
    void FuncFromInternal(FUNC_T *func, Object *(*ptr)());

    Object *FuncCall(FUNC_T *func);
    bool FuncEqual(FUNC_T *first, FUNC_T *second);
    
    uint64_t FuncHash(FUNC_T *func);
}