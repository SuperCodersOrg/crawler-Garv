#pragma once

#include <string>

#include "HashMap.h"
#include "crawler/URLInfo.h"

class SeenStore
{
public:
    bool tryAdd(const std::string& url, int depth);
    bool contains(const std::string& url) const;
    bool updateState(const std::string& url, URLState state);
    bool get(const std::string& url, URLInfo& info) const;
    int size() const;
private:
    HashMap<std::string, URLInfo> urls;
};