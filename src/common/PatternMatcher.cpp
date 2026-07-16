#include "common/PatternMatcher.h"
#include <cctype>

DynamicArray<int> PatternMatcher::buildLPS(std::string_view pattern, bool case_sensitive)const{
    DynamicArray<int> lps;
    if(pattern.empty())return lps;
    lps.append(0);
    size_t len =0;
    size_t i=1;
    while(i<pattern.length())
    {
        bool match = false;
        if(case_sensitive)
        {
            match = (pattern[i] == pattern[len]);
        }
        else
        {
            match = (std::tolower(static_cast<unsigned char>(pattern[i])) ==
                     std::tolower(static_cast<unsigned char>(pattern[len])));
        }

        if(match)
        {
            len++;
            lps.append(len);
            i++;
        }
        else
        {
            if(len!=0)
            {
                len = lps[len-1];
            }
            else
            {
                lps.append(0);
                i++;
            }
        }
    }
    return lps;
}

int PatternMatcher::find(std::string_view text,std::string_view pattern)const{
    if(pattern.empty())return 0;
    if(text.empty())return -1;
    DynamicArray<int> lps = buildLPS(pattern, false);
    size_t i=0;
    size_t j=0;
    while(i<text.length())
    {
        if(std::tolower(static_cast<unsigned char>(text[i])) ==
           std::tolower(static_cast<unsigned char>(pattern[j])))
        {
            i++;
            j++;
            if(j==pattern.length())return i-j;
        }
        else
        {
            if(j!=0)j=lps[j-1];
            else i++;
        }
    }
    return -1;
}

bool PatternMatcher::contains(std::string_view text,std::string_view pattern)const{
    return find(text,pattern)!=-1;
}

DynamicArray<int> PatternMatcher::findall(std::string_view text,std::string_view pattern)const{
    DynamicArray<int> matches;
    if(pattern.empty())return matches;
    if(text.empty())return matches;
    DynamicArray<int> lps = buildLPS(pattern, false);
    size_t i=0;
    size_t j=0;
    while(i<text.length())
    {
        if(std::tolower(static_cast<unsigned char>(text[i])) ==
           std::tolower(static_cast<unsigned char>(pattern[j])))
        {
            i++;
            j++;
            if(j==pattern.length())
            {
                matches.append(i-j);
                j=lps[j-1];
            }
        }
        else
        {
            if(j!=0)j=lps[j-1];
            else i++;
        }
    }
    return matches;
}

// Case-sensitive methods:
int PatternMatcher::findcs(std::string_view text,std::string_view pattern)const{
    if(pattern.empty())return 0;
    if(text.empty())return -1;
    DynamicArray<int> lps = buildLPS(pattern, true);
    size_t i=0;
    size_t j=0;
    while(i<text.length())
    {
        if(text[i]==pattern[j])
        {
            i++;
            j++;
            if(j==pattern.length())return i-j;
        }
        else
        {
            if(j!=0)j=lps[j-1];
            else i++;
        }
    }
    return -1;
}

bool PatternMatcher::containscs(std::string_view text,std::string_view pattern)const{
    return findcs(text,pattern)!=-1;
}

DynamicArray<int> PatternMatcher::findallcs(std::string_view text,std::string_view pattern)const{
    DynamicArray<int> matches;
    if(pattern.empty())return matches;
    if(text.empty())return matches;
    DynamicArray<int> lps = buildLPS(pattern, true);
    size_t i=0;
    size_t j=0;
    while(i<text.length())
    {
        if(text[i]==pattern[j])
        {
            i++;
            j++;
            if(j==pattern.length())
            {
                matches.append(i-j);
                j=lps[j-1];
            }
        }
        else
        {
            if(j!=0)j=lps[j-1];
            else i++;
        }
    }
    return matches;
}