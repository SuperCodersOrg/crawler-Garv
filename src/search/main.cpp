#include <iostream>
#include <string>
#include <cstring> //strlen
#include <cctype> //isspace,alnum
#include "config/ConfigLoader.h"
#include "storage/MySQLStorage.h"
#include "indexer/StopwordFilter.h"

int main()
{
    std::cout << "Starting Search CLI...\n";

    ConfigLoader config;
    if (!config.load("config/search.conf"))
    {
        std::cerr << "[Warning] Failed to load config/search.conf, using default settings.\n";
    }

    std::string host = config.getString("mysql_host", "127.0.0.1");
    std::string user = config.getString("mysql_user", "root");
    std::string password = config.getString("mysql_password", "8305");
    std::string dbname = config.getString("mysql_database", "crawler");
    int port = config.getInt("mysql_port", 3306);

    MySQLStorage storage;
    if (!storage.connect(host, user, password, dbname, port))
    {
        std::cerr << "[Search] Database connection failed!\n";
        return 1;
    }
    StopwordFilter stopwordFilter;
    if (!stopwordFilter.loadStopwords("config/stopwords.txt"))
    {
        std::cout << "[Warning] Stopwords list not loaded.\n";
    }
    while (true)
    {
        std::cout << "\nEnter search word (or exit to quit): ";
        std::string input;
        if (!std::getline(std::cin, input)) break;
        if (input.empty()) continue;
        // Trim
        while (!input.empty() && std::isspace(static_cast<unsigned char>(input.front()))) input.erase(0, 1);
        while (!input.empty() && std::isspace(static_cast<unsigned char>(input.back()))) input.pop_back();
        // Lowercase input
        std::string word = input;
        for (char &c : word)
        {
            c = std::tolower(static_cast<unsigned char>(c));
        }
        if (input == "exit") break;
        // Filter non-alphanumeric chars to isolate the base word
        std::string processedWord;
        for (char c : word)
        {
            if (std::isalnum(static_cast<unsigned char>(c)))
            {
                processedWord += c;
            }
        }

        if (processedWord.empty())
        {
            std::cout << "Invalid search term.\n";
            continue;
        }

        if (stopwordFilter.isStopword(processedWord))
        {
            std::cout << "Word '" << input << "' is a stopword.\n";
            continue;
        }

        // Query the database to retrieve top 5 matching URLs by frequency (Option B Schema + Title JOIN)
        const char* query = 
            "SELECT p.url, o.frequency, COALESCE(i.title, '') FROM word_occurrences o "
            "JOIN words w ON o.word_id = w.id "
            "JOIN pages p ON o.page_id = p.id "
            "LEFT JOIN indexed_pages i ON p.id = i.page_id "
            "WHERE w.word = ? "
            "ORDER BY o.frequency DESC "
            "LIMIT 5";

        MYSQL_STMT* stmt = mysql_stmt_init(storage.getConnection());
        if (!stmt)
        {
            std::cerr << "Database statement allocation failed.\n";
            continue;
        }

        if (mysql_stmt_prepare(stmt, query, std::strlen(query)) != 0)
        {
            std::cerr << "Database statement compilation failed: " << mysql_stmt_error(stmt) << "\n";
            mysql_stmt_close(stmt);
            continue;
        }

        MYSQL_BIND bindParam[1]{};
        unsigned long len = processedWord.length();
        bindParam[0].buffer_type = MYSQL_TYPE_STRING;
        bindParam[0].buffer = (void*)processedWord.c_str();
        bindParam[0].length = &len;

        if (mysql_stmt_bind_param(stmt, bindParam) != 0)
        {
            std::cerr << "Binding query parameters failed: " << mysql_stmt_error(stmt) << "\n";
            mysql_stmt_close(stmt);
            continue;
        }

        if (mysql_stmt_execute(stmt) != 0)
        {
            std::cerr << "Executing query failed: " << mysql_stmt_error(stmt) << "\n";
            mysql_stmt_close(stmt);
            continue;
        }

        // Bind results
        char resUrl[512]{};
        unsigned long resUrlLen = 0;
        int resFreq = 0;
        char resTitle[512]{};
        unsigned long resTitleLen = 0;
        my_bool urlIsNull = 0;
        my_bool freqIsNull = 0;
        my_bool titleIsNull = 0;

        MYSQL_BIND resultBind[3]{};
        resultBind[0].buffer_type = MYSQL_TYPE_STRING;
        resultBind[0].buffer = resUrl;
        resultBind[0].buffer_length = sizeof(resUrl);
        resultBind[0].length = &resUrlLen;
        resultBind[0].is_null = &urlIsNull;

        resultBind[1].buffer_type = MYSQL_TYPE_LONG;
        resultBind[1].buffer = &resFreq;
        resultBind[1].is_null = &freqIsNull;

        resultBind[2].buffer_type = MYSQL_TYPE_STRING;
        resultBind[2].buffer = resTitle;
        resultBind[2].buffer_length = sizeof(resTitle);
        resultBind[2].length = &resTitleLen;
        resultBind[2].is_null = &titleIsNull;

        if (mysql_stmt_bind_result(stmt, resultBind) != 0)
        {
            std::cerr << "Binding result columns failed: " << mysql_stmt_error(stmt) << "\n";
            mysql_stmt_close(stmt);
            continue;
        }

        std::cout << "\nResults for word'" << processedWord << "':\n";
        int count = 0;
        while (mysql_stmt_fetch(stmt) == 0)
        {
            count++;
            std::cout << " [" << count << "] Title:     " << (titleIsNull ? "No Title" : resTitle) << "\n"
                      << "     URL:       " << resUrl << "\n"
                      << "     Frequency: " << resFreq << "\n";
        }

        if (count == 0)
        {
            std::cout << " No URLs found containing this word.\n";
        }

        mysql_stmt_close(stmt);
    }
    return 0;
}
