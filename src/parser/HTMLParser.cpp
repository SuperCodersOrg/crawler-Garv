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

    std::string redirectUrl = extractMetaRedirect(html);
    if(!redirectUrl.empty())
    {
        links.append(redirectUrl);
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

std::string HTMLParser::extractMetaRedirect(std::string_view html) const
{
    size_t pos = 0;
    while (pos < html.length())
    {
        int offset = matcher.find(html.substr(pos), "http-equiv");
        if (offset == -1) break;
        
        size_t absolutePos = pos + offset;
        std::string eqVal = extractattributes(html, absolutePos, "http-equiv");
        std::string eqValLower = eqVal;
        for (char& c : eqValLower) c = std::tolower(static_cast<unsigned char>(c));
        
        if (eqValLower == "refresh")
        {
            size_t tagStart = html.rfind('<', absolutePos);
            size_t tagEnd = html.find('>', absolutePos);
            if (tagStart != std::string_view::npos && tagEnd != std::string_view::npos && tagStart < absolutePos)
            {
                std::string_view tagContent = html.substr(tagStart, tagEnd - tagStart + 1);
                int contentOffset = matcher.find(tagContent, "content");
                if (contentOffset != -1)
                {
                    std::string contentVal = extractattributes(html, tagStart + contentOffset, "content");
                    int urlOffset = matcher.find(contentVal, "url=");
                    if (urlOffset != -1)
                    {
                        std::string targetUrl = contentVal.substr(urlOffset + 4);
                        while (!targetUrl.empty() && std::isspace(static_cast<unsigned char>(targetUrl.front())))
                            targetUrl.erase(0, 1);
                        while (!targetUrl.empty() && std::isspace(static_cast<unsigned char>(targetUrl.back())))
                            targetUrl.pop_back();
                        if (!targetUrl.empty() && (targetUrl.front() == '\'' || targetUrl.front() == '"'))
                        {
                            char q = targetUrl.front();
                            if (targetUrl.back() == q)
                            {
                                targetUrl = targetUrl.substr(1, targetUrl.size() - 2);
                            }
                        }
                        return targetUrl;
                    }
                }
            }
        }
        pos = absolutePos + 10;
    }
    return "";
}