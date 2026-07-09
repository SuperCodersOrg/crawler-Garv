#include "common/PatternMatcher.h"

DynamicArray<int> PatternMatcher::buildLPS(std::string_view pattern)const{
    DynamicArray<int> lps;
    if(pattern.empty())return lps;
    lps.append(0);
    size_t len =0;
    size_t i=1;
    while(i<pattern.length())
    {
        if(pattern[i]==pattern[len])
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
    DynamicArray<int> lps = buildLPS(pattern);
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

bool PatternMatcher::contains(std::string_view text,std::string_view pattern)const{
    return find(text,pattern)!=-1;
}

DynamicArray<int> PatternMatcher::findall(std::string_view text,std::string_view pattern)const{
    DynamicArray<int> matches;
    if(pattern.empty())return matches;
    if(text.empty())return matches;
    DynamicArray<int> lps = buildLPS(pattern);
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