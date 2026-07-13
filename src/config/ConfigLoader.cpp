#include "config/ConfigLoader.h"
#include <fstream>
#include <sstream>

bool ConfigLoader::load(const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        // Trim leading and trailing whitespace from the line
        line = trim(line);

        // Skip comments and empty lines
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        // Find theDelimiter '='
        size_t delimiterPos = line.find('=');
        if (delimiterPos == std::string::npos)
        {
            continue; // Invalid line format, skip
        }

        std::string key = trim(line.substr(0, delimiterPos));
        std::string value = trim(line.substr(delimiterPos + 1));

        if (!key.empty())
        {
            // Make keys case-insensitive by storing them in lowercase manually
            for (char &c : key)
            {
                if (c >= 'A' && c <= 'Z')
                {
                    c = c - 'A' + 'a';
                }
            }
            settings_.insert(key, value);
        }
    }

    return true;
}

std::string ConfigLoader::getString(const std::string& key, const std::string& defaultVal) const
{
    std::string lowerKey = key;
    for (char &c : lowerKey)
    {
        if (c >= 'A' && c <= 'Z')
        {
            c = c - 'A' + 'a';
        }
    }

    std::string val;
    if (settings_.get(lowerKey, val))
    {
        return val;
    }
    return defaultVal;
}

int ConfigLoader::getInt(const std::string& key, int defaultVal) const
{
    std::string valStr = getString(key, "");
    if (valStr.empty())
    {
        return defaultVal;
    }
    try
    {
        return std::stoi(valStr);
    }
    catch (...)
    {
        return defaultVal;
    }
}

bool ConfigLoader::getBool(const std::string& key, bool defaultVal) const
{
    std::string valStr = getString(key, "");
    if (valStr.empty())
    {
        return defaultVal;
    }
    
    // Normalize string to lowercase
    for (char &c : valStr)
    {
        if (c >= 'A' && c <= 'Z')
        {
            c = c - 'A' + 'a';
        }
    }

    if (valStr == "true" || valStr == "1" || valStr == "yes" || valStr == "on")
    {
        return true;
    }
    if (valStr == "false" || valStr == "0" || valStr == "no" || valStr == "off")
    {
        return false;
    }
    return defaultVal;
}

bool ConfigLoader::hasKey(const std::string& key) const
{
    std::string lowerKey = key;
    for (char &c : lowerKey)
    {
        if (c >= 'A' && c <= 'Z')
        {
            c = c - 'A' + 'a';
        }
    }
    return settings_.exists(lowerKey);
}

std::string ConfigLoader::trim(const std::string& str) const
{
    if (str.empty())
    {
        return "";
    }
    
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
    {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}
