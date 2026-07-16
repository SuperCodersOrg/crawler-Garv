#include "storage/MySQLStorage.h"
#include <cstring>
#include <iostream>
#include <cstdlib>

MySQLStorage::MySQLStorage()
    : connection_(nullptr)
{
}

MySQLStorage::~MySQLStorage()
{
    disconnect();
}

bool MySQLStorage::connect(const std::string& host,const std::string& user,const std::string& password,const std::string& database,unsigned int port)
{
    connection_ = mysql_init(nullptr);

    if(connection_ == nullptr)
        return false;

    if(mysql_real_connect(
            connection_,
            host.c_str(),
            user.c_str(),
            password.c_str(),
            database.c_str(),
            port,
            nullptr,
            0) == nullptr)
    {
        mysql_close(connection_);
        connection_ = nullptr;
        return false;
    }

    return true;
}

void MySQLStorage::disconnect()
{
    if(connection_ == nullptr)
        return;

    mysql_close(connection_);
    connection_ = nullptr;
}

bool MySQLStorage::isConnected() const
{
    return connection_ != nullptr;
}

bool MySQLStorage::createTables()
{
    const char* urlTable =
        R"(
        CREATE TABLE IF NOT EXISTS urls
        (
            id BIGINT AUTO_INCREMENT PRIMARY KEY,
            url TEXT NOT NULL,
            depth INT,
            state INT,
            crawl_time TIMESTAMP
        DEFAULT CURRENT_TIMESTAMP
        ON UPDATE CURRENT_TIMESTAMP,
            UNIQUE(url(255))
        )
        )";

    if(mysql_query(connection_, urlTable) != 0)
    {
        std::cout << "URLs table error: "
            << mysql_error(connection_)
            << '\n';
        return false;
    }

    const char* pageTable =
        R"(
        CREATE TABLE IF NOT EXISTS pages
        (
            id BIGINT AUTO_INCREMENT PRIMARY KEY,
            url TEXT NOT NULL,
            status_code INT,
            rendered BOOLEAN,
            html LONGTEXT,
            UNIQUE(url(255)),
            crawl_time TIMESTAMP
        DEFAULT CURRENT_TIMESTAMP
        ON UPDATE CURRENT_TIMESTAMP
        )
        )";

    if(mysql_query(connection_, pageTable) != 0)
    {
        std::cout << "Pagess table error: "
            << mysql_error(connection_)
            << '\n';
        return false;
    }

    return true;
}

bool MySQLStorage::insertURL(const std::string& url,int depth,URLState state)
{
    const char* query =
        "INSERT INTO urls(url, depth, state) VALUES(?, ?, ?) "
        "ON DUPLICATE KEY UPDATE depth = VALUES(depth), state = VALUES(state)";
    MYSQL_STMT* stmt = mysql_stmt_init(connection_);
    if(stmt == nullptr)return false;
    if(mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
    {
        mysql_stmt_close(stmt);
        return false;
    }
    MYSQL_BIND bind[3]{};
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (void*)url.c_str();
    unsigned long length =static_cast<unsigned long>(url.length());
    bind[0].length = &length;
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = &depth;
    int s = static_cast<int>(state);
    bind[2].buffer_type = MYSQL_TYPE_LONG;
    bind[2].buffer = &s;
    if(mysql_stmt_bind_param(stmt, bind) != 0)
    {
        mysql_stmt_close(stmt);
        return false;
    }
    bool success =mysql_stmt_execute(stmt) == 0;
    mysql_stmt_close(stmt);
    return success;
}

bool MySQLStorage::updateState(const std::string& url,URLState state)
{
    const char* query =
        "UPDATE urls "
        "SET state = ? "
        "WHERE url = ?";
    MYSQL_STMT* stmt = mysql_stmt_init(connection_);
    if(stmt == nullptr)return false;
    if(mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
    {
        mysql_stmt_close(stmt);
        return false;
    }
    MYSQL_BIND bind[2]{};
    int s = static_cast<int>(state);
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = &s;
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (void*)url.c_str();
    unsigned long length =static_cast<unsigned long>(url.length());
    bind[1].length = &length;
    if(mysql_stmt_bind_param(stmt, bind) != 0)
    {
        mysql_stmt_close(stmt);
        return false;
    }
    bool success =mysql_stmt_execute(stmt) == 0;
    mysql_stmt_close(stmt);
    return success;
}

bool MySQLStorage::savePage(const Page& page)
{
    const char* query =
        "INSERT INTO pages("
        "url,"
        "status_code,"
        "rendered,"
        "html"
        ") "
        "VALUES(?,?,?,?) "
        "ON DUPLICATE KEY UPDATE "
        "status_code=VALUES(status_code),"
        "rendered=VALUES(rendered),"
        "html=VALUES(html)";
    MYSQL_STMT* stmt = mysql_stmt_init(connection_);
    if(stmt == nullptr)return false;
    if(mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
    {
        std::cout
            << "Prepare failed: "
            << mysql_stmt_error(stmt)
            << '\n';
        mysql_stmt_close(stmt);
        return false;
    }
    MYSQL_BIND bind[4]{};
    unsigned long urlLength =static_cast<unsigned long>(page.url.length());
    unsigned long htmlLength =static_cast<unsigned long>(page.html.length());
    int status = page.statusCode;
    int rendered = page.rendered ? 1 : 0;
    // URL
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (void*)page.url.c_str();
    bind[0].length = &urlLength;
    // Status Code
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = &status;
    // Rendered
    bind[2].buffer_type = MYSQL_TYPE_LONG;
    bind[2].buffer = &rendered;
    // HTML
    bind[3].buffer_type = MYSQL_TYPE_STRING;
    bind[3].buffer = (void*)page.html.c_str();
    bind[3].length = &htmlLength;
    if(mysql_stmt_bind_param(stmt, bind) != 0)
    {
        std::cout<< "Bind failed: "<< mysql_stmt_error(stmt)<< '\n';

        mysql_stmt_close(stmt);
        return false;
    }
    bool success = (mysql_stmt_execute(stmt) == 0);
    if(!success)
    {
        std::cout<< "Execute failed: "<< mysql_stmt_error(stmt)<< '\n';
    }
    mysql_stmt_close(stmt);
    return success;
}

bool MySQLStorage::urlExists(const std::string& url)
{
    const char* query =
        "SELECT 1 "
        "FROM urls "
        "WHERE url = ? "
        "LIMIT 1";
    MYSQL_STMT* stmt = mysql_stmt_init(connection_);
    if(stmt == nullptr)return false;
    if(mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
    {
        std::cout<< "Prepare failed: "<< mysql_stmt_error(stmt)<< '\n';

        mysql_stmt_close(stmt);
        return false;
    }
    MYSQL_BIND param[1]{};
    unsigned long length =static_cast<unsigned long>(url.length());
    param[0].buffer_type = MYSQL_TYPE_STRING;
    param[0].buffer = (void*)url.c_str();
    param[0].length = &length;
    if(mysql_stmt_bind_param(stmt, param) != 0)
    {
        std::cout<< "Bind: "<< mysql_stmt_error(stmt)<< '\n';

        mysql_stmt_close(stmt);
        return false;
    }
    int exists;
    MYSQL_BIND result[1]{};
    result[0].buffer_type = MYSQL_TYPE_LONG;
    result[0].buffer = &exists;
    if(mysql_stmt_bind_result(stmt, result) != 0)
    {
        mysql_stmt_close(stmt);
        return false;
    }
    if(mysql_stmt_execute(stmt) != 0)
    {
        mysql_stmt_close(stmt);
        return false;
    }
    bool found = false;
    if(mysql_stmt_fetch(stmt) == 0)found = true;
    mysql_stmt_close(stmt);
    return found;
}

bool MySQLStorage::loadURLs(DynamicArray<std::string>& urls, DynamicArray<int>& depths, DynamicArray<URLState>& states)
{
    if(connection_ == nullptr)return false;

    const char* query = "SELECT url, depth, state FROM urls";
    if(mysql_query(connection_, query) != 0)
    {
        std::cout << "loadURLs query failed: "<< mysql_error(connection_)<< '\n';
        return false;
    }

    MYSQL_RES* result = mysql_store_result(connection_);
    if(result == nullptr)return false;

    MYSQL_ROW row;
    while((row = mysql_fetch_row(result)))
    {
        if(row[0] && row[1] && row[2])
        {
            urls.append(row[0]);
            depths.append(std::atoi(row[1]));
            states.append(static_cast<URLState>(std::atoi(row[2])));
        }
    }

    mysql_free_result(result);
    return true;
}

bool MySQLStorage::clearStorage()
{
    if(connection_ == nullptr)
        return false;

    if(mysql_query(connection_, "TRUNCATE TABLE urls") != 0)
    {
        std::cout << "Failed to truncate urls table: "<< mysql_error(connection_) << '\n';
        return false;
    }

    if(mysql_query(connection_, "TRUNCATE TABLE pages") != 0)
    {
        std::cout << "Failed to truncate pages table: "<< mysql_error(connection_) << '\n';
        return false;
    }
    return true;
}

