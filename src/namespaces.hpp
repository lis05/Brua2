#pragma once

#include "objects.hpp"
#include "names.hpp"

namespace Namespaces {

    /*
    
    namespaces are used to access objects created in the current namespace.
    some namespaces can access their parrent namespace.

    when functions are called, arguments are pushed on stack of the recently created namespace.

    any object that is created has to be added to a namespace. this way it can be tracked.
    when namespace is destroyed, it destroys all of the objects it is currently tracking

    */
    int Create(bool can_access_parent);
    void Destroy(); // destroys the topmost namespace

    int Current();
    int Parent();

    void PushOnStack(int namespace_id, Object *obj);
    void PopFromStack(int namespace_id);
    Object *AccessStack(int namespace_id, int pos);
    int StackSize(int namespace_id);

    void Track(int namespace_id, Object *obj);
    void Untrack(int namespace_id, Object *obj);
    void Add(int namespace_id, Names::Name name, Object *obj);
    bool Present(int namespace_id, Names::Name name);
    Object *Find(int namespace_id, Names::Name name);
}