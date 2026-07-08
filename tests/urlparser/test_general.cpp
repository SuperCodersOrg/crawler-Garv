#include <gtest/gtest.h>
#include "parser/URLParser.h"
#include "parser/ParsedURL.h"

TEST(URLParserTest, ParseSimpleURL)
{
    URLParser parser;
    ParsedURL url =parser.parse("https://google.com");

    EXPECT_TRUE(url.valid);

    EXPECT_EQ(url.scheme, "https");

    EXPECT_EQ(url.host, "google.com");

    EXPECT_TRUE(url.port.empty());

    EXPECT_TRUE(url.path.empty());

    EXPECT_TRUE(url.query.empty());
}

TEST(URLParserTest, ParseSimpleHTTPS)
{
    URLParser parser;

    ParsedURL url = parser.parse("https://google.com");

    EXPECT_TRUE(url.valid);

    EXPECT_EQ(url.scheme, "https");
    EXPECT_EQ(url.host, "google.com");

    EXPECT_TRUE(url.port.empty());
    EXPECT_TRUE(url.path.empty());
    EXPECT_TRUE(url.query.empty());
}

TEST(URLParserTest, ParseSimpleHTTP)
{
    URLParser parser;

    ParsedURL url = parser.parse("http://example.com");

    EXPECT_TRUE(url.valid);

    EXPECT_EQ(url.scheme, "http");
    EXPECT_EQ(url.host, "example.com");
}

TEST(URLParserTest, ParseRootPath)
{
    URLParser parser;

    ParsedURL url = parser.parse("https://google.com/");

    EXPECT_TRUE(url.valid);

    EXPECT_EQ(url.path, "/");
}

TEST(URLParserTest, ParsePath)
{
    URLParser parser;

    ParsedURL url = parser.parse("https://google.com/search");

    EXPECT_TRUE(url.valid);

    EXPECT_EQ(url.path, "/search");
}

TEST(URLParserTest, ParsePort)
{
    URLParser parser;

    ParsedURL url = parser.parse("https://google.com:8080");

    EXPECT_TRUE(url.valid);

    EXPECT_EQ(url.host, "google.com");
    EXPECT_EQ(url.port, "8080");
}

TEST(URLParserTest, ParsePortAndPath)
{
    URLParser parser;

    ParsedURL url = parser.parse("https://google.com:8080/search");

    EXPECT_TRUE(url.valid);

    EXPECT_EQ(url.host, "google.com");
    EXPECT_EQ(url.port, "8080");
    EXPECT_EQ(url.path, "/search");
}

TEST(URLParserTest, InvalidPort)
{
    URLParser parser;

    ParsedURL url = parser.parse("https://google.com:abcd");

    EXPECT_FALSE(url.valid);
}

TEST(URLParserTest, EmptyPort)
{
    URLParser parser;

    ParsedURL url = parser.parse("https://google.com:");

    EXPECT_FALSE(url.valid);
}

TEST(URLParserTest, SlashAfterColon)
{
    URLParser parser;

    ParsedURL url = parser.parse("https://google.com:/");

    EXPECT_FALSE(url.valid);
}

//QUERIES

TEST(URLParserTest, ParseQuery)
{
    URLParser parser;

    ParsedURL url = parser.parse("https://google.com?q=test");

    EXPECT_TRUE(url.valid);
    EXPECT_EQ(url.host, "google.com");
    EXPECT_EQ(url.query, "q=test");
}

TEST(URLParserTest, ParsePathAndQuery)
{
    URLParser parser;

    ParsedURL url = parser.parse("https://google.com/search?q=test");

    EXPECT_TRUE(url.valid);

    EXPECT_EQ(url.path, "/search");
    EXPECT_EQ(url.query, "q=test");
}

TEST(URLParserTest, EmptyQuery)
{
    URLParser parser;

    ParsedURL url = parser.parse("https://google.com?");

    EXPECT_TRUE(url.valid);

    EXPECT_TRUE(url.query.empty());
}

TEST(URLParserTest, MultipleQueryParameters)
{
    URLParser parser;

    ParsedURL url =
        parser.parse("https://google.com/search?q=cpp&page=2");

    EXPECT_EQ(url.query, "q=cpp&page=2");
}

//FRAGMENT TESTS

TEST(URLParserTest, IgnoreFragmentAfterHost)
{
    URLParser parser;

    ParsedURL url = parser.parse("https://google.com#about");

    EXPECT_TRUE(url.valid);

    EXPECT_EQ(url.host, "google.com");
    EXPECT_TRUE(url.path.empty());
    EXPECT_TRUE(url.query.empty());
}

TEST(URLParserTest, IgnoreFragmentAfterPath)
{
    URLParser parser;

    ParsedURL url = parser.parse("https://google.com/search#abc");

    EXPECT_TRUE(url.valid);

    EXPECT_EQ(url.path, "/search");
}

TEST(URLParserTest, IgnoreFragmentAfterQuery)
{
    URLParser parser;

    ParsedURL url =
        parser.parse("https://google.com/search?q=cpp#intro");

    EXPECT_TRUE(url.valid);

    EXPECT_EQ(url.query, "q=cpp");
}

//IPv6 handling
TEST(URLParserTest, IPv6Host)
{
    URLParser parser;

    ParsedURL url =
        parser.parse("https://[2001:db8::1]");

    EXPECT_TRUE(url.valid);

    EXPECT_EQ(url.host,"[2001:db8::1]");
}
TEST(URLParserTest, IPv6Port)
{
    URLParser parser;

    ParsedURL url =
        parser.parse("https://[2001:db8::1]:8080");

    EXPECT_TRUE(url.valid);

    EXPECT_EQ(url.host,"[2001:db8::1]");

    EXPECT_EQ(url.port,"8080");
}
TEST(URLParserTest, IPv6Path)
{
    URLParser parser;

    ParsedURL url =
        parser.parse("https://[2001:db8::1]/docs");

    EXPECT_TRUE(url.valid);

    EXPECT_EQ(url.path,"/docs");
}
TEST(URLParserTest, IPv6Query)
{
    URLParser parser;

    ParsedURL url =
        parser.parse("https://[2001:db8::1]?a=1");

    EXPECT_TRUE(url.valid);

    EXPECT_EQ(url.query,"a=1");
    EXPECT_FALSE(parser.parse("https://[2001:db8::1").valid);
    EXPECT_FALSE(parser.parse("https://[2001:db8::1]]").valid);
}
//CREDS
TEST(URLParserTest, Credentials)
{
    URLParser parser;

    ParsedURL url =
        parser.parse("https://user@google.com");

    EXPECT_TRUE(url.valid);

    EXPECT_EQ(url.credentials,"user");

    EXPECT_EQ(url.host,"google.com");
}
TEST(URLParserTest, CredentialsPort)
{
    URLParser parser;

    ParsedURL url =
        parser.parse("https://user@google.com:8080");

    EXPECT_TRUE(url.valid);

    EXPECT_EQ(url.credentials,"user");

    EXPECT_EQ(url.host,"google.com");

    EXPECT_EQ(url.port,"8080");
}
TEST(URLParserTest, CredentialsPath)
{
    URLParser parser;

    ParsedURL url =
        parser.parse("https://user@google.com/docs");

    EXPECT_TRUE(url.valid);

    EXPECT_EQ(url.path,"/docs");
}

TEST(URLParserTest, ClosingBracketWithoutOpening)
{
    URLParser parser;

    EXPECT_FALSE(parser.parse("https://google.com]").valid);
}
TEST(URLParserTest, DoubleOpeningBracket)
{
    URLParser parser;

    EXPECT_FALSE(parser.parse("https://[[::1]").valid);
}


// TEST(URLParserTest, ParsePath)
// {
//     URLParser parser;

//     ParsedURL url =
//         parser.parse("https://google.com/search");

//     EXPECT_EQ(url.path, "/search");
// }

// TEST(URLParserTest, ParseQuery)
// {
//     URLParser parser;

//     ParsedURL url =
//         parser.parse("https://google.com/search?q=chatgpt");

//     EXPECT_EQ(url.query, "q=chatgpt");
// }

// TEST(URLParserTest, ParsePort)
// {
//     URLParser parser;

//     ParsedURL url =
//         parser.parse("https://google.com:8080");

//     EXPECT_EQ(url.port, "8080");
// }

// TEST(URLParserTest, ParseCredentials)
// {
//     URLParser parser;

//     ParsedURL url =
//         parser.parse("https://user:pass@google.com");

//     EXPECT_EQ(url.credentials, "user:pass");

//     EXPECT_EQ(url.host, "google.com");
// }

// TEST(URLParserTest, ParseIPv6)
// {
//     URLParser parser;

//     ParsedURL url =
//         parser.parse("https://[2001:db8::1]:8080");

//     EXPECT_EQ(url.host, "[2001:db8::1]");

//     EXPECT_EQ(url.port, "8080");
// }

// TEST(URLParserTest, InvalidScheme)
// {
//     URLParser parser;

//     ParsedURL url =
//         parser.parse("ftp://google.com");

//     EXPECT_FALSE(url.valid);
// }

// TEST(URLParserTest, MissingHost)
// {
//     URLParser parser;

//     ParsedURL url =
//         parser.parse("https://");

//     EXPECT_FALSE(url.valid);
// }