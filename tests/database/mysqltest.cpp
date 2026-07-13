#include <mysql/mysql.h>
#include <iostream>

int main()
{
    MYSQL* connection = mysql_init(nullptr);
    if(connection == nullptr)
    {
        std::cout << "mysql_init failed\n";
        return 1;
    }
    if(mysql_real_connect(
            connection,
            "127.0.0.1",
            "root",
            "8305",
            "crawler",
            3306,
            nullptr,
            0) == nullptr)
    {
        std::cout << mysql_error(connection) << '\n';
        mysql_close(connection);
        return 1;
    }

    std::cout << "Connected to MySQL!\n";

    mysql_close(connection);
}