#include <gtest/gtest.h>
#include "config/ConfigLoader.h"
#include <fstream>

class ConfigLoaderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        std::ofstream tempConf("temp_test.conf");
        tempConf << "# This is a test comment\n"
                 << "  mysql_host = 127.0.0.1  \n"
                 << "mysql_user=root\n"
                 << "\n" // Empty line
                 << "mysql_port = 3306\n"
                 << "headless = true\n"
                 << "same_domain = OFF\n"
                 << "max_depth = -1\n"
                 << "max_pages = 500\n"
                 << "seed_url = https://test.com\n"
                 << "invalid_int = notanynumber\n";
        tempConf.close();
    }

    void TearDown() override
    {
        std::remove("temp_test.conf");
    }
};

TEST_F(ConfigLoaderTest, LoadAndRetrieveValues)
{
    ConfigLoader loader;
    ASSERT_TRUE(loader.load("temp_test.conf"));

    // String loading & case-insensitivity
    EXPECT_EQ(loader.getString("mysql_host"), "127.0.0.1");
    EXPECT_EQ(loader.getString("MYSQL_HOST"), "127.0.0.1"); // Case-insensitivity check
    EXPECT_EQ(loader.getString("mysql_user"), "root");
    EXPECT_EQ(loader.getString("seed_url"), "https://test.com");

    // Integer loading & invalid formatting fallback
    EXPECT_EQ(loader.getInt("mysql_port"), 3306);
    EXPECT_EQ(loader.getInt("max_depth"), -1);
    EXPECT_EQ(loader.getInt("max_pages"), 500);
    EXPECT_EQ(loader.getInt("invalid_int", 99), 99); // Fallback to default on parse error
    EXPECT_EQ(loader.getInt("nonexistent_key", 10), 10); // Fallback on missing key

    // Boolean loading
    EXPECT_TRUE(loader.getBool("headless"));
    EXPECT_FALSE(loader.getBool("same_domain")); // "OFF" must parse to false
    EXPECT_TRUE(loader.getBool("nonexistent_bool", true)); // Missing key fallback
}

TEST_F(ConfigLoaderTest, MissingFileReturnsFalse)
{
    ConfigLoader loader;
    EXPECT_FALSE(loader.load("nonexistent_file.conf"));
}
