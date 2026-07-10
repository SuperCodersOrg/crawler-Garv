#include <gtest/gtest.h>

#include "browser/ChromeProcess.h"
#include "browser/HTTPClient.h"
#include "browser/CDPconn.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

TEST(CDPConnectionTest, MultipleWebsiteNavigation)
{
    ChromeProcess chrome;
    ASSERT_TRUE(chrome.start());
    HTTPClient http;
    std::string response =http.get(L"127.0.0.1",9222,L"/json/version");
    json versionInfo = json::parse(response);
    std::string websocketURL =versionInfo["webSocketDebuggerUrl"];
    CDPConnection cdp;
    ASSERT_TRUE(cdp.connect(websocketURL));
    ASSERT_TRUE(cdp.createPage());
    ASSERT_TRUE(cdp.enablePage());
    ASSERT_TRUE(cdp.enableRuntime());
    struct Website
    {
        std::string url;
        std::string expectedText;
    };
    Website websites[] =
    {
        {
            "https://example.com",
            "Example Domain"
        },
        {
            "https://codequotient.com",
            "CodeQuotient"
        },
        {
            "https://ssipmt.edu.in",
            "SSIPMT"
        },
        {
            "https://github.com",
            "GitHub"
        },
        {
            "https://www.wikipedia.org",
            "Wikipedia"
        }
    };
    for (const auto& site : websites)
    {
        ASSERT_TRUE(cdp.navigate(site.url));
        ASSERT_TRUE(cdp.waitForLoad());
        std::string html = cdp.getHTML();
        EXPECT_FALSE(html.empty());
        EXPECT_NE(html.find(site.expectedText),std::string::npos)<< "Expected text not found for "<< site.url;
    }
    cdp.disconnect();
    chrome.stop();
}