#include "custom_types.hpp"

#include <unordered_map>
#include <unordered_set>
#include <iostream>

#include "objects.hpp"
#include "parser.hpp"
#include "errors.hpp"
#include "hashing.hpp"

#define MAX(A, B) (((A)>(B))?(A):(B))

namespace CustomTypes {
    struct DictKey {
        Object *obj;
        bool operator==(const DictKey &other) const {
            return Objects::Equal(obj, other.obj);
        }
    };
    struct DictKeyHasher {
        uint64_t operator()(const DictKey &key) const {
            return Objects::Hash(key.obj);
        }
    };
}

struct DICT_T {
    std::unordered_map<CustomTypes::DictKey, Object*, CustomTypes::DictKeyHasher> map;
    std::unordered_set<Object*> items;
    uint64_t current_ops;
    uint64_t target_ops;
};

struct FUNC_T {
    bool is_internal;
    Object *(*internal_ptr)();
    Node *node;
};

namespace CustomTypes {
    /*
    
    dict stores copies of keys and values passed to it.
    therefore, when destroyed, it simply destroyes all of the items it has

    */

    // kind of a garbage collection cycle
    static void DictOp(DICT_T *dict) {
        dict->current_ops++;
        if (dict->current_ops == dict->target_ops) {
            dict->current_ops = 0;

            std::unordered_set<Object*> present_items;
            for (auto [key, val]: dict->map) {
                present_items.insert(key.obj);
                present_items.insert(val);
            }

            for (auto item: dict->items) {
                if (present_items.find(item) == present_items.end()) {
                    Objects::Destroy(item);
                }
            }

            dict->items = present_items;
            dict->target_ops = MAX(10000, present_items.size() * 2);
        }
    }

    DICT_T *DictCreate() {
        DICT_T *res = new DICT_T;
        res->current_ops = 0;
        res->target_ops = 10000;
        return res;
    }
    void DictDestroy(DICT_T *dict) {
        for (auto item: dict->items) {
            Objects::Destroy(item);
        }
        delete dict;
    }
    DICT_T *DictCopy(DICT_T *dict) {
        DICT_T *res = DictCreate();
        for (auto [key, val]: dict->map) {
            DictKey new_key;
            new_key.obj = Objects::Copy(key.obj, false);
            Object *new_val = Objects::Copy(val, true);

            res->map[new_key] = new_val;
            res->items.insert(new_key.obj);
            res->items.insert(new_val);
        }
        return res;
    }

    Object *DictAccess(DICT_T *dict, Object *key) {
        DictOp(dict);
        if (dict->map.find({key}) != dict->map.end()) {
            return dict->map[{key}];
        }
        RuntimeError("Key not present in dict");
        return NULL;
    }
    int DictSize(DICT_T *dict) {
        DictOp(dict);
        return dict->map.size();
    }
    bool DictPresent(DICT_T *dict, Object *key) {
        DictOp(dict);
        return dict->map.find({key}) != dict->map.end();
    }
    void DictInsert(DICT_T *dict, Object *key, Object *val) {
        DictOp(dict);
        val = Objects::Copy(val, true);
        if (!DictPresent(dict, key)) {
            key = Objects::Copy(key, false);
            dict->items.insert(key);
        }
        dict->map[{key}] = val;

        dict->items.insert(val);
    }
    void DictRemove(DICT_T *dict, Object *key) {
        DictOp(dict);
        if (!DictPresent(dict, key)) return;
        dict->map.erase({key});
    }
    DICT_T *DictKeys(DICT_T *dict) {
        DictOp(dict);
        DICT_T *res = DictCreate();
        int i = 0;
        for (auto [key, _]: dict->map) {
            Object *index = Objects::Create(Objects::INT);
            *Objects::GetInt(index) = i++;
            Object *obj = Objects::Copy(key.obj, true);

            res->map[{index}] = obj;
            res->items.insert(index);
            res->items.insert(obj);
        }
        return res;
    }
    DICT_T *DictValues(DICT_T *dict) {
        DictOp(dict);
        DICT_T *res = DictCreate();
        int i = 0;
        for (auto [_, val]: dict->map) {
            Object *index = Objects::Create(Objects::INT);
            *Objects::GetInt(index) = i++;
            Object *obj = Objects::Copy(val, true);

            res->map[{index}] = obj;
            res->items.insert(index);
            res->items.insert(obj);
        }
        return res;
    }
    std::string DictString(DICT_T *dict) {
        std::string res = "{";
        bool f = true;
        for (auto [key, val]: dict->map) {
            if (!f) res += ", ";
            res += Objects::AsString(key.obj);
            res += ": ";
            res += Objects::AsString(val);
            f = false;
        }
        res += "}";
        return res;
    }
    bool DictEqual(DICT_T *first, DICT_T *second) {
        if (first->map.size() != second->map.size()) return false;
        for (auto [key, item]: first->map) {
            if (second->map.find(key) == second->map.end()) return false;
            if (!Objects::Equal(second->map[key], item)) return false;
        }
        for (auto [key, item]: second->map) {
            if (first->map.find(key) == first->map.end()) return false;
            if (!Objects::Equal(first->map[key], item)) return false;
        }
        return true;
    }
    void DictClear(DICT_T *first) {
        for (auto item: first->items) Objects::Destroy(item);
        first->map.clear();
        first->items.clear();
        first->current_ops = 0;
        first->target_ops = 10000;
    }

    const uint64_t dict_seed = 0x17ae3a7a33c2df17ul;

    uint64_t DictHash(DICT_T *dict) {
        uint64_t res = dict_seed;
        for (auto [key, val]: dict->map) {
            res = Hashing::Hash(res, Objects::Hash(key.obj));
            res = Hashing::Hash(res, Objects::Hash(val));
        }
        return res;
    }

    /*
    
    function is implemented as a pointer to a parse tree node.
    however, some functions may be implemented internally, as C++ functions;
    
    */

    FUNC_T *FuncCreate() {
        FUNC_T *func = new FUNC_T;
        return func;
    }
    void FuncDestroy(FUNC_T *func) {
        delete func;
    }
    FUNC_T *FuncCopy(FUNC_T *func) {
        FUNC_T *res = FuncCreate();
        *res = *func;
        return res;
    }
    void FuncFromNode(FUNC_T *func, Node *node) {
        func->is_internal = false;
        func->node = node;
    }
    void FuncFromInternal(FUNC_T *func, Object *(*ptr)()) {
        func->is_internal = true;
        func->internal_ptr = ptr;
    }

    Object *FuncCall(FUNC_T *func) {
        if (func->is_internal) return func->internal_ptr();
        bool do_continue = false, do_break = false, do_return = false;
        return Parser::Execute(func->node, do_continue, do_break, do_return);
    }
    bool FuncEqual(FUNC_T *first, FUNC_T *second) {
        if (first->is_internal != second->is_internal) return false;
        if (first->is_internal) return first->internal_ptr == second->internal_ptr;
        return first->node == second->node;
    }

    const uint64_t function_seed = 0xc3bd184a0ea29f82ul;
    
    uint64_t FuncHash(FUNC_T *func) {
        if (func->is_internal) return Hashing::Hash(function_seed, (uint64_t)func->internal_ptr);
        return Hashing::Hash(function_seed, (uint64_t)func->node);
    }
}