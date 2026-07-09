#pragma once
#include <string_view>
#include "DynamicArray.h"

class PatternMatcher{
    private:
        DynamicArray<int> buildLPS(std::string_view pattern)const;
    public:
        int find(std::string_view text,std::string_view pattern)const;
        DynamicArray<int> findall(std::string_view text,std::string_view pattern)const;
        bool contains(std::string_view text,std::string_view pattern)const;
};
