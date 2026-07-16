#include "browser/BrowserRenderer.h"

#include <windows.h>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

BrowserRenderer::BrowserRenderer(){}

BrowserRenderer::~BrowserRenderer()
{
    stop();
}

bool BrowserRenderer::initializeCDP()
{
    std::string response;
    bool success = false;
    for (int i = 0; i < connectRetries_; ++i)
    {
        try
        {
            response = http_.get(L"127.0.0.1", 9222, L"/json/version");//wont work on linux wide char
            success = true;
            break;
        }
        catch (const std::exception&)
        {
            Sleep(connectDelayMs_);
        }
    }
    if (!success) return false;

    json version;
    try
    {
        version = json::parse(response);
    }
    catch (const std::exception&)
    {
        return false;
    }

    if (!version.contains("webSocketDebuggerUrl")) return false;
    std::string websocketURL = version["webSocketDebuggerUrl"];

    try
    {
        if(!cdp_.connect(websocketURL))return false;
        if(!cdp_.createPage())return false;
        if(!cdp_.enablePage())return false;
        if(!cdp_.enableRuntime())return false;
    }
    catch (const std::exception&)
    {
        return false;
    }
    return true;
}

bool BrowserRenderer::start(bool headless)
{
    if(initialized_)return true;
    if(!chrome_.start(headless))return false;
    if(!initializeCDP())return false;
    initialized_ = true;
    return true;
}

void BrowserRenderer::stop()
{
    if(!initialized_)return;
    cdp_.disconnect();
    chrome_.stop();
    initialized_ = false;
}

std::string BrowserRenderer::render(const std::string& url)
{
    if(!initialized_)
    {
        if(!start())return "";
    }

    try
    {
        if(!cdp_.navigate(url))
        {
            stop();
            return "";
        }
        cdp_.waitForLoad(loadTimeoutMs_);
        std::string html = cdp_.getHTML();
        if(html.empty())
        {
            stop();
        }
        return html;
    }
    catch (...)
    {
        stop();
        return "";
    }
}

void BrowserRenderer::configure(int connectRetries, int connectDelayMs, int loadTimeoutMs)
{
    connectRetries_ = connectRetries;
    connectDelayMs_ = connectDelayMs;
    loadTimeoutMs_ = loadTimeoutMs;
}

