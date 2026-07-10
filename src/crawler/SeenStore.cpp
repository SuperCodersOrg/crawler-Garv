#include "crawler/SeenStore.h"

bool SeenStore::tryAdd(const std::string& url, int depth)
{
    if (urls.exists(url))
        return false;
    URLInfo info;
    info.depth = depth;
    info.state = URLState::Queued;
    info.crawlTime = 0;
    urls.insert(url, info);
    return true;
}

bool SeenStore::contains(const std::string& url) const
{
    return urls.exists(url);
}

bool SeenStore::updateState(const std::string& url, URLState state)
{
    URLInfo info;
    if (!urls.get(url, info))
        return false;
    info.state = state;
    if (state == URLState::Completed)
    {
        info.crawlTime = std::time(nullptr);
    }
    urls.insert(url, info);
    return true;
}

bool SeenStore::get(const std::string& url, URLInfo& info) const
{
    return urls.get(url, info);
}

int SeenStore::size() const
{
    return urls.size();
}