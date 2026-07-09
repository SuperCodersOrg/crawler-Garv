#include "parser/URLNormalizer.h"
#include <string>
#include "parser/NormalizedURL.h"
#include "parser/ParsedURL.h"
#include "DynamicArray.h"

NormalizedURL URLNormalizer::normalize(const ParsedURL& parsed)const
{
    NormalizedURL url;
    url.scheme = parsed.scheme;
    url.credentials = parsed.credentials;
    url.host = parsed.host;
    url.port = parsed.port;
    url.path = parsed.path;
    url.query = parsed.query;
    url.hasport = parsed.hasport;

    finalnormalize(url);
    return url;
}

void URLNormalizer::lowercaseScheme(NormalizedURL& url)const{
    for(char& c:url.scheme)c=std::tolower(static_cast<unsigned char>(c));
}

void URLNormalizer::lowercaseHost(NormalizedURL& url)const{
    for(char& c:url.host)c=std::tolower(static_cast<unsigned char>(c));
}

void URLNormalizer::removedefaultport(NormalizedURL& url)const{
    if(!url.hasport)return;
    if(url.scheme == "http" && url.port == "80")
    {
        url.port.clear();
        url.hasport = false;
    }
    else if(url.scheme=="https" && url.port == "443")
    {
        url.port.clear();
        url.hasport=false;
    }
} 

void URLNormalizer::normalizepath(NormalizedURL& url)const
{
    if(url.path.empty())
    {
        url.path="/";
        return;
    }
    DynamicArray<std::string> segments;
    size_t s=0;
    if(url.path[0]=='/')s=1;
    for(size_t e = s;e<=url.path.length();e++)
    {
        if(e == url.path.length() || url.path[e]=='/')
        {
            std::string segment = url.path.substr(s,e-s);
            if(segment.empty())
            {
                s=e+1;
                continue;
            }
            else if(segment == ".")
            {
                s=e+1;
                continue;
            }
            else if(segment == "..")
            {
                if(!segments.isEmpty())segments.popback();
            }
            else{
                segments.append(segment);
            }
            s=e+1;
        }
    }
    url.path.clear();
    if(segments.isEmpty())
    {
        url.path="/";
        return;
    }
    for(int i=0;i<segments.size();i++)
    {
        url.path+="/"; 
        url.path+=segments[i];
    }
}

void URLNormalizer::removetrailingdot(NormalizedURL& url)const
{
    if(!url.host.empty()&&url.host.back()=='.')url.host.pop_back();
}

std::string URLNormalizer::dirof(const std::string& path)const
{
    if(path.empty()||path=="/")
    {
        return "/";
    }
    size_t slash = path.find_last_of('/');
    if(slash == std::string::npos)
        return "/";

    return path.substr(0,slash+1);
}

NormalizedURL URLNormalizer::resolverelative(const std::string& relative,const ParsedURL& base)const
{
    NormalizedURL url;
    url.scheme = base.scheme;
    url.credentials = base.credentials;
    url.host = base.host;
    url.port = base.port;
    url.hasport = base.hasport;
    //fragmentonly
    if(!relative.empty()&&relative[0]=='#')
    {
        url.path = base.path;
        url.query = base.query;
        finalnormalize(url);
        return url;
    }
    //queryonly
    if(!relative.empty() && relative[0] == '?')
    {
        url.path = base.path;
        url.query = relative.substr(1);
        finalnormalize(url);
        return url;
    }
    //protocolonly(cdn)
    if(relative.size()>=2&&relative[0]=='/'&& relative[1]=='/' )
    {
        std::string temp = relative.substr(2);
        size_t slash = temp.find('/');
        if(slash == std::string::npos)
        {
            url.host =temp;
            url.path="/";
        }
        else
        {
            url.host = temp.substr(0,slash);
            url.path = temp.substr(slash);
        }
        url.credentials.clear();
        url.port.clear();
        url.hasport=false;
        finalnormalize(url);
        return url;
    }
    //rootrelative
    if(!relative.empty()&&relative[0]=='/')
    {
        url.path = relative;
    }
    else
    {
        //relativefile
        url.path = dirof(std::string(base.path));
        url.path += relative;
    }
    lowercaseScheme(url);
    lowercaseHost(url);
    removedefaultport(url);
    removetrailingdot(url);
    normalizepath(url);
    return url;
}

std::string URLNormalizer::tostring(const NormalizedURL& url)const
{
    std::string result;
    result+=url.scheme;
    result+="://";

    if(!url.credentials.empty())
    {
        result+=url.credentials;
        result+="@";
    }
    result+=url.host;
    if(url.hasport)
    {
        result+=":";
        result+=url.port;
    }
    result += url.path;
    if(!url.query.empty())
    {
        result+="?";
        result+=url.query;
    }
    return result;
}

void URLNormalizer::finalnormalize(NormalizedURL& url)const
{
    lowercaseScheme(url);
    lowercaseHost(url);
    removedefaultport(url); 
    removetrailingdot(url);
    normalizepath(url);
}