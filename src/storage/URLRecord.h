#pragma once
#include <string>
#include "crawler/URLState.h"

struct URLRecord
{
    std::string url;

    int depth = 0;

    URLState state = URLState::Queued;
};