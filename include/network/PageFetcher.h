#pragma once
#include <string>
#include "network/Page.h"
#include "network/HTTPFetcher.h"
#include "browser/BrowserRenderer.h"
#include "parser/HTMLParser.h"
#include "config/ConfigLoader.h"

class PageFetcher
{
    public:
        Page fetch(const std::string& url);
        void initialize();
        void initialize(const ConfigLoader& config);
    private:
        HTTPFetcher http_;
        BrowserRenderer browser_;
        HTMLParser parser_;
        bool needsRendering(const Page& page);
        int renderMinSize_ = 300;
};