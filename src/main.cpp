#include "crawler/Crawler.h"
#include "config/ConfigLoader.h"
#include <iostream>
#ifdef _WIN32
#include <crtdbg.h>
#endif

int main()
{
#ifdef _WIN32
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif 
    ConfigLoader config;
    if (!config.load("config/crawler.conf"))
    {
        std::cerr << "Warning: Could not load config/crawler.conf using default settings.\n";
    }
    Crawler crawler(config);
    std::string seed = config.getString("seed_url", "https://codequotient.com/");
    crawler.addSeed(seed);
    crawler.crawl();
}