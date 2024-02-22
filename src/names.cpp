#include "names.hpp"

#include <string>
#include <unordered_map>

namespace Names {
    static std::unordered_map<std::string, Name> map;

    Name GetName(std::string str) {
        if (map.find(str) != map.end()) return map[str];
        int sz = map.size();
        map[str].id = sz;
        map[str].str = new std::string{str};
        return map[str];
    }
    void Destroy() {
        for (auto [key, val]: map) {
            delete val.str;
        }
    }
}