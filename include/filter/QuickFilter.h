#pragma once

#include <string_view>

class QuickFilter{
    public:
        static bool shouldparse(std::string_view url);
        static bool isrelative(std::string_view url);
    private:
    static bool iswhitespace(char c);
    static bool startswith(std::string_view txt,std::string_view prefix);
};