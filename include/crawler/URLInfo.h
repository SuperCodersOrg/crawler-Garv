#pragma once
#include "crawler/URLState.h"
#include <ctime>

struct URLInfo
{
    URLState state = URLState::Queued;
    int depth = 0;
    std::time_t crawlTime = 0;
};