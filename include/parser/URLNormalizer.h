#pragma once
#include <string>

class URLNormalizer{
    private:
        std::string resolveRelative(const std::string& url,const std::string& baseurl)const;
        std::string lowercastHost(const std::string& url)const;
        std::string removeFragment(const std::string& url)const;
        std::string normalizeTraliningSlash(const std::string& url)const;
    public:
        std::string normalize(const std::string& url,const std::string& baseurl)const;
        bool isValidURL(const std::string& url)const;
};