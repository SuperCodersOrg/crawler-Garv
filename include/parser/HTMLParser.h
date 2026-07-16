#pragma once
#include <string_view>
#include <string>
#include "DynamicArray.h"
#include "common/PatternMatcher.h"

class HTMLParser{
    private:
    PatternMatcher matcher;
    std::string extractattributes(std::string_view html,size_t pos,std::string_view attribute)const;
    std::string trim(std::string_view str) const;
    std::string extractMetaRedirect(std::string_view html) const;
    public:
    DynamicArray<std::string> extractlinks(std::string_view html)const;
};