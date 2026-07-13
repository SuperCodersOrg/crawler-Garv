#include "crawler/Crawler.h"
#include "config/ConfigLoader.h"
#include <iostream>

int main()
{
    ConfigLoader config;
    if (!config.load("config/crawler.conf"))
    {
        std::cerr << "Warning: Could not load config/crawler.conf using default settings.\n";
    }

    Crawler crawler(config);
    std::string seed = config.getString("seed_url", "https://youtube.me/");
    crawler.addSeed(seed);
    crawler.crawl();
}