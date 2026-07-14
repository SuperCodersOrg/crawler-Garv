#pragma once

#include <string>
#include "HashMap.h"

class ConfigLoader
{
public:
    ConfigLoader() = default;
    
    // Load config from a file returns true if successful
    bool load(const std::string& filepath);

    // getters with default value
    std::string getString(const std::string& key, const std::string& defaultVal = "") const;
    int getInt(const std::string& key, int defaultVal = 0) const;
    bool getBool(const std::string& key, bool defaultVal = false) const;

    // Check if key exists
    bool hasKey(const std::string& key) const;
private:
    HashMap<std::string, std::string> settings_;
    // Helper functions for parsing
    std::string trim(const std::string& str) const;
};
