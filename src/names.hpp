#pragma once

#include <cstdint>
#include <string>

namespace Names {
    struct Name {
        std::string *str; 
        uint64_t id;
    };  


    Name GetName(std::string str);
    void Destroy();

    /*
    
    all string values are stored internally in a hash map
    when a new name is found, it gets assigned an integer identifier

    */
}