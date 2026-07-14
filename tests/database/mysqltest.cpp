#include "storage/MySQLStorage.h"
#include "DynamicArray.h"
#include <iostream>

int main()
{
    MySQLStorage storage;
    if(!storage.connect("127.0.0.1", "root", "8305", "crawler", 3306))
    {
        std::cout << "Connection failed\n";
        return 1;
    }
    std::cout << "Connected using MySQLStorage!\n";

    // Insert trial URLs to verify
    storage.insertURL("https://example.com/test_resume", 2, URLState::Queued);
    storage.insertURL("https://example.com/test_resume_done", 1, URLState::Completed);

    DynamicArray<std::string> urls;
    DynamicArray<int> depths;
    DynamicArray<URLState> states;

    if(storage.loadURLs(urls, depths, states))
    {
        std::cout << "Successfully loaded " << urls.size() << " URLs from DB:\n";
        for(int i = 0; i < urls.size(); ++i)
        {
            std::cout << " - " << urls[i] << " (depth: " << depths[i] 
                      << ", state: " << static_cast<int>(states[i]) << ")\n";
        }
    }
    else
    {
        std::cout << "Failed to load URLs!\n";
    }

    storage.disconnect();
    return 0;
}