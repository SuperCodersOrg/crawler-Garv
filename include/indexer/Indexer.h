#pragma once

#include "config/ConfigLoader.h"
#include "indexer/IndexerStorage.h"
#include "indexer/StopwordFilter.h"
#include "indexer/TextExtractor.h"
#include "indexer/Tokenizer.h"

class Indexer
{
public:
    // Initializes the indexer with ConfigLoader settings
    Indexer(const ConfigLoader& config);

    //enters the continuous cycle index loop
    void run();
private:
    // Indexes a single RawPage, extracting unique words and storing frequencies
    void indexPage(const RawPage& page);

    IndexerStorage storage_;
    StopwordFilter stopwordFilter_;
    TextExtractor extractor_;
    Tokenizer tokenizer_;

    std::string stopwordFile_;
    int batchThreshold_;
    int pollIntervalSeconds_;
    bool adaptivePolling_;
};
