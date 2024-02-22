#include "namespaces.hpp"

#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <vector>

#include "objects.hpp"
#include "errors.hpp"

// todo: garbage collection

namespace Namespaces {
    struct Namespace {
        bool can_access_parent;

        std::unordered_map<uint64_t, Object*> map;
        std::vector<Object*> stack;
        std::unordered_set<Object*> tracked;
    };

    static std::vector<Namespace> vec;

    static void Check(int namespace_id) {
        if (!(0 <= namespace_id && namespace_id < vec.size())) RuntimeError("Invalid namespace id");
    }

    int Create(bool can_access_parent) {
        vec.push_back({can_access_parent, {}, {}, {}});
        if (can_access_parent && vec.size() >= 2) {
            for (auto arg: vec[vec.size() - 2].stack) {
                arg = Objects::Copy(arg, true);
                Track(vec.size() - 1, arg);
                PushOnStack(vec.size() - 1, arg);
            }
        }
        return vec.size() - 1;
    }
    void Destroy() {
        int cur = Current();
        for (auto obj: vec[cur].tracked) {
            Objects::Destroy(obj);
        }
        vec.pop_back();
    }

    int Current() {
        if (vec.empty()) RuntimeError("No current namespace");
        return vec.size() - 1;
    }
    int Parent() {
        if (vec.size() < 2) RuntimeError("No parent namespace");
        return vec.size() - 2;
    }

    void PushOnStack(int namespace_id, Object *obj) {
        Check(namespace_id);
        
        vec[namespace_id].stack.push_back(obj);
        vec[namespace_id].tracked.insert(obj);
    }
    void PopFromStack(int namespace_id) {
        Check(namespace_id);

        if (vec[namespace_id].stack.empty()) RuntimeError("Pop from empty stack");

        vec[namespace_id].stack.pop_back();
    }
    Object *AccessStack(int namespace_id, int pos) {
        Check(namespace_id);

        if (!(0 <= pos && pos < vec[namespace_id].stack.size()))
            RuntimeError("Stack access pos out of range");
        
        return vec[namespace_id].stack[vec[namespace_id].stack.size() - pos - 1];
    }
    int StackSize(int namespace_id) {
        Check(namespace_id);

        return vec[namespace_id].stack.size();
    }

    void Track(int namespace_id, Object *obj) {
        if (obj == NULL) return;
        Check(namespace_id);

        vec[namespace_id].tracked.insert(obj);
    }
    void Untrack(int namespace_id, Object *obj) {
        if (obj == NULL) return;
        Check(namespace_id);

        vec[namespace_id].tracked.erase(obj);
    }
    void Add(int namespace_id, Names::Name name, Object *obj) {
        if (obj == NULL) return;
        Check(namespace_id);

        vec[namespace_id].tracked.insert(obj);
        vec[namespace_id].map[name.id] = obj;
    }
    bool Present(int namespace_id, Names::Name name) {
        Check(namespace_id);

        while (namespace_id >= 0) {
            if (vec[namespace_id].map.find(name.id) != vec[namespace_id].map.end()) {
                return true;
            }

            if (vec[namespace_id].can_access_parent) {
                namespace_id--;
            }
            else break;
        }

        if (namespace_id != 0) {
            namespace_id = 0;
            if (vec[namespace_id].map.find(name.id) != vec[namespace_id].map.end()) {
                return true;
            }
        }
        return false;
    }
    Object *Find(int namespace_id, Names::Name name) {
        Check(namespace_id);

        while (namespace_id >= 0) {
            if (vec[namespace_id].map.find(name.id) != vec[namespace_id].map.end()) {
                return vec[namespace_id].map[name.id];
            }

            if (vec[namespace_id].can_access_parent) {
                namespace_id--;
            }
            else break;
        }

        if (namespace_id != 0) {
            namespace_id = 0;
            if (vec[namespace_id].map.find(name.id) != vec[namespace_id].map.end()) {
                return vec[namespace_id].map[name.id];
            }
        }

        RuntimeError("Couldn't find object by name");
        return NULL;
    }
}