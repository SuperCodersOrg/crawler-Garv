#pragma once

#include <string_view>

struct ParsedURL{
    std::string_view scheme;
    std::string_view credentials;
    std::string_view host;
    std::string_view port;
    std::string_view path;
    std::string_view query;
    bool valid = false;
};