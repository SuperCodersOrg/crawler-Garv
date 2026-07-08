#include "parser/URLNormalizer.h"
#include <curl/curl.h>
#include <string>

std::string URLNormalizer::resolveRelative(const std::string& url,const std::string& baseurl)const
{
}
std::string URLNormalizer::lowercastHost(const std::string& url)const{}
std::string URLNormalizer::removeFragment(const std::string& url)const{}
std::string URLNormalizer::normalizeTraliningSlash(const std::string& url)const{}
std::string URLNormalizer::normalize(const std::string& url,const std::string& baseurl)const
{
    CURLU* handle = curl_url();
    if(handle==nullptr)return url;
    if(curl_url_set(handle,CURLUPART_URL,url.c_str(),0)!=CURLUE_OK)
    {
        curl_url_cleanup(handle);
        return url;
    }
    curl_url_set(handle,CURLUPART_FRAGMENT,nullptr,0);
    char * normalized = nullptr;
    if(curl_url_get(handle,CURLUPART_URL,&normalized,0)!=CURLUE_OK)
    {
        curl_url_cleanup(handle);
        return url;
    }
    std::string result = normalized;
    curl_free(normalized);
    curl_url_cleanup(handle);
    return result;
}
bool URLNormalizer::isValidURL(const std::string& url) const
{

}


//CURLCODE
    // // Reject empty URLs
    // if(url.empty())
    //     return false;
    // // Require an URL containing "://"
    // if(url.find("://") == std::string::npos)
    //     return false;
    // // Create a libcurl URL parser
    // CURLU* handle = curl_url();
    // // Abort if parser creation fails
    // if(handle == nullptr)
    //     return false;
    // // Parse the complete URL
    // CURLUcode code =curl_url_set(handle,CURLUPART_URL,url.c_str(),0);
    // // Parsing failed
    // if(code != CURLUE_OK)
    // {
    //     curl_url_cleanup(handle);
    //     return false;
    // }
    // // Will receive parsed scheme and host
    // char* scheme = nullptr;
    // char* host = nullptr;
    // bool valid = false;
    // // Extract scheme and host from the parsed URL
    // if(curl_url_get(handle,CURLUPART_SCHEME,&scheme,0) == CURLUE_OK &&
    //    curl_url_get(handle,CURLUPART_HOST,&host,0) == CURLUE_OK)
    // {
    //     // Convert scheme to std::string
    //     std::string protocol = scheme;
    //     // Accept only HTTP/HTTPS URLs with a valid host
    //     valid =(protocol == "http" ||protocol == "https") &&host != nullptr &&strlen(host) > 0;
    // }
    // // Free memory allocated by libcurl
    // curl_free(scheme);
    // curl_free(host);
    // // Destroy the parser object
    // curl_url_cleanup(handle);
    // return valid;