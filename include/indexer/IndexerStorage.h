#pragma once

#include <string>
#include "storage/MySQLStorage.h"
#include "DynamicArray.h"

// Struct for page data read from DB
struct RawPage
{
    int id;
    std::string url;
    std::string html;
};

class IndexerStorage : public MySQLStorage
{
public:
    IndexerStorage() = default;

    // Checks current number of unindexed completed crawl pages
    int countUnindexed();

    // Fetches all unindexed pages from MySQL
    DynamicArray<RawPage> getAllUnindexed();

    // Create the Indexer tables for Option B (words, word_occurrences, indexed_pages)
    bool createIndexerTables();

    // Word mapping helper: Checks if word exists, inserts it and returns its word_id
    int getOrInsertWord(const std::string& word);

    // Records an occurrence of word_id on page_id with its frequency
    bool insertWordOccurrence(int wordId, int pageId, int frequency);

    // Marks a page as completed in indexed_pages table with title and content
    bool markPageAsIndexed(int pageId, const std::string& title, const std::string& content);
};
