#pragma once
#include <string>
#include "network/Page.h"
class HTTPFetcher
{
    public:
        Page fetch(const std::string& url);
        void configure(int timeoutSec, int connectTimeoutSec);
    private:
        int timeoutSec_ = 0;
        int connectTimeoutSec_ = 0;
};