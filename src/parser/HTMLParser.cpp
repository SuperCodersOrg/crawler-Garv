#include "parser/HTMLParser.h"
#include <cctype>
std::string HTMLParser::extractattributes(std::string_view html,size_t pos,std::string_view attribute)const
{
    size_t i = pos+attribute.length();
    //skip spaces
    while(i<html.length() && std::isspace(static_cast<unsigned char>(html[i])))
    {
        i++;
    }
    if(i>=html.length() || html[i] !='=')return "";
    i++;
    //skip spaces after =
    while(i<html.length() && std::isspace(static_cast<unsigned char>(html[i])))
    {
        i++;
    }
    if(i>=html.length())return "";
    char quote = html[i];
    if(quote != '"' && quote != '\'')return "";
    i++;
    //set start
    size_t start=i;
    //move i ahead till "
    while(i<html.length() && html[i]!=quote)
    {
        i++;
    }

    if(i>=html.length())return "";
    return trim(html.substr(start,i-start));
}
//works for href 
DynamicArray<std::string> HTMLParser::extractlinks(std::string_view html)const{
    std::string_view attribute = "href";
    DynamicArray<int> positions = matcher.findall(html,attribute);
    DynamicArray<std::string> links;
    for(int i=0;i<positions.size();i++)
    {
        if(positions[i] > 0)
        {
            char prev = html[positions[i] - 1];
            if(std::isalnum(static_cast<unsigned char>(prev)) ||prev == '-' ||prev == '_')continue;
        }
        std::string url = extractattributes(html,positions[i],attribute);
        if(!url.empty())links.append(url);
    }
    return links;
}


std::string HTMLParser::trim(std::string_view str) const
{
    size_t start = 0;
    size_t end = str.length();

    // Trim leading whitespace
    while (start < end &&std::isspace(static_cast<unsigned char>(str[start])))
    {
        start++;
    }

    // Trim trailing whitespace
    while (end > start &&std::isspace(static_cast<unsigned char>(str[end - 1])))
    {
        end--;
    }
    return std::string(str.substr(start, end - start));
}