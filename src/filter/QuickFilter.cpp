#include "filter/QuickFilter.h"


bool QuickFilter::iswhitespace(char c)
{
    return c==' '||c=='\t'||c=='\n'||c=='\r';
}

bool QuickFilter::startswith(std::string_view txt,std::string_view prefix)
{
    if(txt.size() < prefix.size())return false;

    for(size_t i = 0; i < prefix.size(); i++)
    {
        char a = txt[i];
        char b = prefix[i];
        if(a >= 'A' && a <= 'Z')a = a - 'A' + 'a';
        if(b >= 'A' && b <= 'Z')b = b - 'A' + 'a';
        if(a != b)return false;
    }

    return true;
}

bool QuickFilter::shouldparse(std::string_view url)
{
    //remove leading whitespace
    while(!url.empty()&&iswhitespace(url.front()))
        url.remove_prefix(1);

    //remove trailing whitespace
    while(!url.empty()&&iswhitespace(url.back()))
        url.remove_suffix(1);
    
    if(url.empty())return false;

    if(url.front()=='#')return false;

    if(startswith(url,"javascript:"))return false;

    if(startswith(url,"mailto:"))return false;
    
    if(startswith(url,"tel:"))return false;
    
    if(startswith(url,"data:"))return false;

    if(startswith(url,"blob:"))return false;

    if(startswith(url,"file:"))return false;

    return true;
}

bool QuickFilter::isrelative(std::string_view url)
{
    if(url.empty())return false;

    //absolute urls
    if(startswith(url,"http://"))return false;

    if(startswith(url,"https://"))return false;

    if(startswith(url,"//"))return false;

    return true;
}