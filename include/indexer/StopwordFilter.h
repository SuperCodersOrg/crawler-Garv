#pragma once

#include <string>
#include "HashMap.h"

class StopwordFilter
{
public:
    StopwordFilter() = default;

    // Loads stop words from a text file into the custom HashMap
    bool loadStopwords(const std::string& filepath);

    // Checks if the given token is a stop word
    bool isStopword(const std::string& token) const;

private:
    HashMap<std::string, bool> stopwords_;
};
