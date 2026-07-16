#pragma once
#include <string>
#include "browser/ChromeProcess.h"
#include "browser/HTTPClient.h"
#include "browser/CDPconn.h"

class BrowserRenderer
{
public:
    BrowserRenderer();
    ~BrowserRenderer();
    bool start(bool headless = true);
    void stop();
    std::string render(const std::string& url);
    void configure(int connectRetries, int connectDelayMs, int loadTimeoutMs = 10000);
private:
    ChromeProcess chrome_;
    HTTPClient http_;
    CDPConnection cdp_;
    bool initialized_ = false;
    int connectRetries_ = 15;
    int connectDelayMs_ = 200;
    int loadTimeoutMs_ = 10000;
    bool initializeCDP();
};