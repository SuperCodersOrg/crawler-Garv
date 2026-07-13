#pragma once
#include "ParsedURL.h"
#include "NormalizedURL.h"  
#include <string>

class URLNormalizer{
    private:
        void lowercaseScheme(NormalizedURL& url)const;
        void lowercaseHost(NormalizedURL& url)const;
        void removedefaultport(NormalizedURL& url)const;    
        void normalizepath(NormalizedURL& url)const; // . .. // emptypath trailingslash
        void removetrailingdot(NormalizedURL& url)const;
        //helper method for rr
        std::string dirof(const std::string& path)const;
        void finalnormalize(NormalizedURL& url)const;
        void removefragment(std::string& str)const;
    public:
        NormalizedURL normalize(const ParsedURL& url)const;
        //cases -   
        //absolute
        //rootrelative
        //relativefile
        //parentdir
        //currdir
        NormalizedURL resolverelative(const std::string& url,const ParsedURL& base)const;
        std::string tostring(const NormalizedURL& url)const;

};