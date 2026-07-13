#include "crawler/Crawler.h"

int main()
{
    Crawler crawler;
    crawler.setmaxdepth(2);
    crawler.setmaxpages(100);
    crawler.samedomain(true);
    crawler.addSeed("https://youtube.me/");
    crawler.crawl();
}