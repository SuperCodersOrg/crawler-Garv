#include "network/PageFetcher.h"

Page PageFetcher::fetch(const std::string& url)
{
    Page page =http_.fetch(url);
    if(needsRendering(page))
    {
        page.html=browser_.render(url);
        page.rendered = true;
        // If rendering succeeded treat it as a successful fetch.
        if(!page.html.empty())page.statusCode = 200;
    }
    return page;
}

void PageFetcher::initialize()
{
    browser_.start(false);   // headful (headless = false)
}

void PageFetcher::initialize(const ConfigLoader& config)
{
    bool headless = config.getBool("headless", false);
    renderMinSize_ = config.getInt("render_min_size", 300);

    int connectRetries = config.getInt("browser_connect_retries", 15);
    int connectDelayMs = config.getInt("browser_connect_delay_ms", 200);
    int loadTimeoutSec = config.getInt("browser_load_timeout_seconds", 10);
    browser_.configure(connectRetries, connectDelayMs, loadTimeoutSec * 1000);

    int httpTimeoutSec = config.getInt("http_timeout_seconds", 10);
    int httpConnectTimeoutSec = config.getInt("http_connect_timeout_seconds", 5);
    http_.configure(httpTimeoutSec, httpConnectTimeoutSec);
    browser_.start(headless);
}

bool PageFetcher::needsRendering(const Page& page)
{
    if(page.statusCode == 401)
        return false;

    if(page.statusCode == 404)
        return false;

    if(page.statusCode >= 500)
        return false;

    if(page.html.empty())return true;

    if(static_cast<int>(page.html.size()) < renderMinSize_)return true;

    if(page.html.find("id=\"root\"") != std::string::npos)return true;

    if(page.html.find("id=\"app\"") != std::string::npos)return true;

    if(page.html.find("id=\"__next\"") != std::string::npos)return true;

    if(page.html.find("id=\"__nuxt\"") != std::string::npos)return true;
    
    auto links = parser_.extractlinks(page.html);
    if(links.size() == 0)
        return true;
        
    return false;
}