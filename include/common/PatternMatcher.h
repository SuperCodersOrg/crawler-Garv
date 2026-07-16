#pragma once
#include <string_view>
#include "DynamicArray.h"

class PatternMatcher{
    private:
        DynamicArray<int> buildLPS(std::string_view pattern, bool case_sensitive = false)const;
    public:
        // Case-insensitive search
        int find(std::string_view text,std::string_view pattern)const;
        DynamicArray<int> findall(std::string_view text,std::string_view pattern)const;
        bool contains(std::string_view text,std::string_view pattern)const;

        // Case-sensitive search
        int findcs(std::string_view text,std::string_view pattern)const;
        DynamicArray<int> findallcs(std::string_view text,std::string_view pattern)const;
        bool containscs(std::string_view text,std::string_view pattern)const;
};
