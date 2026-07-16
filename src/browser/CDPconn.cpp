#include "browser/CDPconn.h"
#include <chrono>
using json = nlohmann::json;

bool CDPConnection::connect(const std::string& url)
{
    return websocket_.connect(url);
}

void CDPConnection::disconnect()
{
    websocket_.disconnect();
}

json CDPConnection::sendCommand(const std::string& method,const json& params)
{
    try
    {
        json request;
        request["id"] = nextId_++;
        request["method"] = method;
        if (!sessionId_.empty())request["sessionId"] = sessionId_;
        if (!params.empty())request["params"] = params;
        websocket_.send(request.dump());
        while (true)
        {
            json reply = json::parse(websocket_.receive());
            // Ignore events.
            if (!reply.contains("id"))continue;
            if (reply["id"] == request["id"])return reply;
        }
    }
    catch (...)
    {
        return json::object();
    }
}

bool CDPConnection::createPage()
{
    try
    {
        auto reply =sendCommand("Target.createTarget",{{"url", "about:blank"}});
        if (!reply.contains("result"))return false;
        std::string targetId =reply["result"]["targetId"];
        reply =sendCommand("Target.attachToTarget",{{"targetId", targetId},{"flatten", true}});
        if (!reply.contains("result"))return false;
        sessionId_ =reply["result"]["sessionId"];
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool CDPConnection::enablePage()
{
    try
    {
        auto reply =sendCommand("Page.enable");
        return reply.contains("result");
    }
    catch (...)
    {
        return false;
    }
}

bool CDPConnection::enableRuntime()
{
    try
    {
        auto reply =sendCommand("Runtime.enable");
        return reply.contains("result");
    }
    catch (...)
    {
        return false;
    }
}

bool CDPConnection::navigate(const std::string& url)
{
    try
    {
        auto reply =sendCommand("Page.navigate",{{"url", url}});
        return reply.contains("result");
    }
    catch (...)
    {
        return false;
    }
}

std::string CDPConnection::getHTML()
{
    try
    {
        auto reply =sendCommand("Runtime.evaluate",{{"expression","document.documentElement.outerHTML"},{"returnByValue",true}});
        if (reply.contains("result") && 
            reply["result"].contains("result") && 
            reply["result"]["result"].contains("value"))
        {
            return reply["result"]["result"]["value"];
        }
    }
    catch (...)
    {
    }
    return "";
}

bool CDPConnection::waitForLoad(int timeoutMs)
{
    try
    {
        auto start = std::chrono::steady_clock::now();
        while (true)
        {
            auto now = std::chrono::steady_clock::now();
            int elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
            int remaining = timeoutMs - elapsed;
            if (remaining <= 0) return false;
            
            std::string raw = websocket_.receive(remaining);
            if (raw.empty()) return false;
            
            json message;
            try
            {
                message = json::parse(raw);
            }
            catch (...)
            {
                continue;
            }
            
            if (!message.contains("method"))continue;
            if (message["method"] == "Page.loadEventFired" ||
                message["method"] == "Page.domContentEventFired")return true;
        }
    }
    catch (...)
    {
        return false;
    }
}