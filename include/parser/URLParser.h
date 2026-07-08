#pragma once
#include <string_view>
#include "parser/ParsedURL.h"

class URLParser{
    public:
        ParsedURL parse(std::string_view url)const;
};