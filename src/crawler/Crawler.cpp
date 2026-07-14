#include "crawler/Crawler.h"

#include <iostream>

Crawler::Crawler()
    : linkfilter(
        "config/blockeddomains.txt",
        "config/blockedextensions.txt")
{
    fetcher.initialize();
    if(!storage.connect(
        "127.0.0.1",
        "root",
        "8305",
        "crawler"))
    {
        std::cout << "MySQL connection failed\n";
        return;
    }
    std::cout << "Connected to MySQL\n";
    std::cout << "Tables created\n";
    storage.clearStorage();
}

Crawler::Crawler(const ConfigLoader& config)
    : linkfilter(
        "config/blockeddomains.txt",
        "config/blockedextensions.txt")
{
    fetcher.initialize(config);
    std::string db_host = config.getString("mysql_host", "127.0.0.1");
    std::string db_user = config.getString("mysql_user", "root");
    std::string db_pass = config.getString("mysql_password", "8305");
    std::string db_name = config.getString("mysql_database", "crawler");
    int db_port = config.getInt("mysql_port", 3306);
    if(!storage.connect(db_host, db_user, db_pass, db_name, db_port))
    {
        std::cout << "MySQL connection failed\n";
        return;
    }
    std::cout << "Connected to MySQL\n";
    if(!storage.createTables())
    {
        std::cout << "Failed to create tables\n";
        return;
    }
    std::cout << "Tables created\n";
    setmaxdepth(config.getInt("max_depth", 2));
    setmaxpages(config.getInt("max_pages", 1000));
    samedomain(config.getBool("same_domain", true));
    
    std::string resume_mode = config.getString("resume_mode", "");
    if(resume_mode.empty())
    {
        resume_mode=="keep";
        std::cout << "Starting fresh crawl: keeping previous database records...\n";
    }

    if(resume_mode == "resume")
    {
        loadCrawlState();
    }
    else if(resume_mode == "keep")
    {
        std::cout << "Starting fresh crawl: keeping previous database records...\n";
    }
    else // Default to clear
    {
        std::cout << "Starting fresh crawl: clearing database crawl state...\n";
        storage.clearStorage();
    }
}

void Crawler::addSeed(const std::string& url)
{
    //parse URL
    ParsedURL parsed = parser.parse(url);

    if(!parsed.valid)
    {
        std::cout << "[Seed] Invalid URL\n";
        return;
    }

    if(seedHost.empty())seedHost = parsed.host;

    //normalize URL
    NormalizedURL normalized = normalizer.normalize(parsed);

    if(!linkfilter.shouldvisit(normalized))
    {
        std::cout << "[Seed] Blocked by LinkFilter\n";
        return;
    }

    std::string finalurl = normalizer.tostring(normalized);
        if(seen.tryAdd(finalurl,0))
        {
            frontier.enqueue({finalurl,0});
            std::cout<< "[Seed]"<<finalurl<< '\n';
            storage.insertURL(finalurl,0,URLState::Queued);
        }
}

void Crawler::crawl()
{
    std::cout<< "\n========== CRAWLER START ==========\n\n";
    while(!frontier.empty())
    {
        if(maxpages != -1 && crawledpages >= maxpages)break;
        URLDepth current = frontier.dequeue();
        crawlPage(current);
        crawledpages++;
    }
    std::cout<< "\n========== CRAWLER SUMMARY ==========\n";
    std::cout<< "Pages Crawled : "<< crawledpages<< '\n';
    std::cout<< "HTTP Pages    : "<< httpPages<< '\n';
    std::cout<< "Rendered      : "<< renderedPages<< '\n';
    std::cout<< "Failed        : "<< failedPages<< "\n\n";
    std::cout<< "Links Found   : "<< extractedLinks<< '\n';
    std::cout<< "Queued        : "<< queuedLinks<< '\n';
    std::cout<< "Filtered      : "<< filteredLinks<< '\n';
    std::cout<< "Duplicates    : "<< duplicateLinks<< "\n\n";
    std::cout<< "Seen URLs     : "<< seen.size()<< '\n';
    std::cout<< "Queue Left    : "<< frontier.size()<< '\n';
    std::cout<< "====================================\n";
}

void Crawler::crawlPage(const URLDepth& page)
{
    std::cout<< "\n------------------------------------\n";
    std::cout<< "[" << crawledpages + 1 << "]\n";
    std::cout<< "URL      : "<< page.URL<< '\n';
    std::cout<< "Depth    : "<< page.depth<< '\n';

    storage.updateState(page.URL,URLState::Crawling);

    Page fetched = fetcher.fetch(page.URL);
    fetched.url = page.URL;

    if(fetched.rendered)renderedPages++;
    else httpPages++;

    std::cout<< "Status   : "<< fetched.statusCode<< '\n';
    std::cout<< "Source   : "<< (fetched.rendered ? "CDP" : "HTTP")<< '\n';
    std::cout<< "HTML     : "<< fetched.html.size()<< " bytes\n";
    if(fetched.html.empty())
    {
        failedPages++;
        seen.updateState(page.URL,URLState::Failed);
        storage.updateState(page.URL,URLState::Failed);
        std::cout<< "Result   : Failed\n";
        std::cout<< "------------------------------------\n";
        return;
    }
    seen.updateState(page.URL,URLState::Completed);
    storage.updateState(page.URL,URLState::Completed);

    ParsedURL base = parser.parse(page.URL);
    
    if(!storage.savePage(fetched))std::cout << "savePage failed\n";
    processLinks(fetched.html,base,page.depth + 1);
    std::cout<< "Frontier : "<< frontier.size()<< '\n';
    std::cout<< "Seen     : "<< seen.size()<< '\n';
    std::cout<< "------------------------------------\n";
}

void Crawler::processLinks(std::string_view html,const ParsedURL& base,int nextDepth)
{
    auto links = htmlparser.extractlinks(html);
    extractedLinks += links.size();
    int queued = 0;
    int filtered = 0;
    int duplicate = 0;
    int invalid = 0;
    int maxDepthReached = 0;

    for(int i = 0; i < links.size(); i++)
    {
        EnqueueResult result =enqueue(links[i],base,nextDepth);
        switch(result)
        {
            case EnqueueResult::Queued:
                queued++;
                queuedLinks++;
                break;

            case EnqueueResult::Filtered:
                filtered++;
                filteredLinks++;
                break;

            case EnqueueResult::Duplicate:
                duplicate++;
                duplicateLinks++;
                break;

            case EnqueueResult::Invalid:
                invalid++;
                break;

            case EnqueueResult::MaxDepth:
                maxDepthReached++;
                break;
        }
    }

    std::cout
        << "\nLinks Found           : " << links.size()
        << "\nQueued into Frontier  : " << queued
        << "\nFiltered these links  : " << filtered
        << "\nDuplicate links       : " << duplicate
        << "\nInvalid links         : " << invalid
        << "\nMax Depth Reached     : " << maxDepthReached
        << "\n";
}

EnqueueResult Crawler::enqueue(const std::string& url,const ParsedURL& base,int depth)
{
    if(maxdepth != -1 && depth > maxdepth)return EnqueueResult::MaxDepth;
    if(!QuickFilter::shouldparse(url))return EnqueueResult::Filtered;
    ParsedURL parsed = parser.parse(url);
    NormalizedURL normalized;
    if(parsed.valid)
    {
        normalized =normalizer.normalize(parsed);
    }
    else if(QuickFilter::isrelative(url))
    {
        normalized =normalizer.resolverelative(url,base);
    }
    else
    {
        return EnqueueResult::Invalid;
    }

    // Stay on same domain
    if(sameDomainOnly)
    {
        if(normalized.host != seedHost)return EnqueueResult::Filtered;
    }

    if(!linkfilter.shouldvisit(normalized))return EnqueueResult::Filtered;

    std::string finalurl = normalizer.tostring(normalized);
    if(!seen.tryAdd(finalurl, depth))return EnqueueResult::Duplicate;
    storage.insertURL(finalurl,depth,URLState::Queued);
    frontier.enqueue({finalurl,depth});
    return EnqueueResult::Queued;
}

void Crawler::samedomain(bool enable)
{
    sameDomainOnly = enable;
}

void Crawler::setmaxdepth(int depth)
{
    if(depth >= 0 || depth == -1)maxdepth = depth;
}

void Crawler::setmaxpages(int pages)
{
    if(pages > 0 || pages == -1)maxpages = pages;
}

void Crawler::loadCrawlState()
{
    DynamicArray<std::string> urls;
    DynamicArray<int> depths;
    DynamicArray<URLState> states;
    if(!storage.loadURLs(urls, depths, states))
    {
        std::cout<< "Failed to load crawl state from database\n";
        return;
    }
    int loadedCount = urls.size();
    if(loadedCount == 0)
    {
        std::cout<< "No previous crawl state found in database.\n";
        return;
    }
    std::cout<< "Loading "<<loadedCount<<" URLs from previous crawl\n";

    for(int i = 0; i < loadedCount; ++i)
    {
        std::string url = urls[i];
        int depth = depths[i];
        URLState state = states[i];

        // Populate seen store
        if(seen.tryAdd(url, depth))
        {
            if(state != URLState::Queued)
            {
                seen.updateState(url, state);
            }
        }

        // If the URL was Queued or was interrupted while Crawling, reset it to Queued
        // and enqueue it into the Frontier queue.
        if(state == URLState::Queued || state == URLState::Crawling)
        {
            if(state == URLState::Crawling)
            {
                seen.updateState(url, URLState::Queued);
                storage.updateState(url, URLState::Queued);
            }
            frontier.enqueue({url, depth});
            queuedLinks++;
        }
        else if(state == URLState::Completed)
        {
            crawledpages++;
        }
        else if(state == URLState::Failed)
        {
            failedPages++;
        }
    }

    std::cout<<"Crawl state restored. Frontier size: "<<frontier.size()<<", Already Crawled: "<< crawledpages<<", Failed: "<< failedPages<<"\n";
}
