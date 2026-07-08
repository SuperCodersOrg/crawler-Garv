#include <gtest/gtest.h>
#include "parser/URLNormalizer.h"

TEST(URLNormalizerTest, ValidURLs)
{
    URLNormalizer normalizer;
    EXPECT_TRUE(normalizer.isValidURL("https://google.com"));
    EXPECT_TRUE(normalizer.isValidURL("http://example.com"));
    EXPECT_TRUE(normalizer.isValidURL("https://example.com/page"));
    EXPECT_TRUE(normalizer.isValidURL("https://google.com"));
    EXPECT_TRUE(normalizer.isValidURL("https://google.com/"));
    EXPECT_TRUE(normalizer.isValidURL("https://google.com:8080"));
    EXPECT_TRUE(normalizer.isValidURL("https://google.com/path?a=1#frag"));
    EXPECT_TRUE(normalizer.isValidURL("https://example.com:8080"));
}
TEST(URLNormalizerTest, InvalidURLs)
{
    URLNormalizer normalizer;
    EXPECT_FALSE(normalizer.isValidURL(""));
    EXPECT_FALSE(normalizer.isValidURL("example.com"));
    EXPECT_FALSE(normalizer.isValidURL("ftp://example.com"));
    EXPECT_FALSE(normalizer.isValidURL("mailto:test@test.com"));
    EXPECT_FALSE(normalizer.isValidURL("javascript:void(0)"));
    EXPECT_FALSE(normalizer.isValidURL("file:///abc"));
}
TEST(URLNormalizerTest, MissingHost)
{
    URLNormalizer normalizer;
    EXPECT_FALSE(normalizer.isValidURL("http://"));
    EXPECT_FALSE(normalizer.isValidURL("https://"));
}
TEST(URLNormalizerTest, MalformedURL)
{
    URLNormalizer normalizer;
    EXPECT_FALSE(normalizer.isValidURL("https:/abc"));
    EXPECT_FALSE(normalizer.isValidURL("https//abc"));
    EXPECT_FALSE(normalizer.isValidURL("://abc"));
    EXPECT_FALSE(normalizer.isValidURL("https//google.com"));
    EXPECT_FALSE(normalizer.isValidURL("http//google.com"));
    EXPECT_FALSE(normalizer.isValidURL("://google.com"));
    EXPECT_FALSE(normalizer.isValidURL("google"));
    EXPECT_FALSE(normalizer.isValidURL(" "));
    EXPECT_FALSE(normalizer.isValidURL("\n"));
}

TEST(URLNormalizerTest, IPAddresses)
{
    URLNormalizer normalizer;
    EXPECT_TRUE(normalizer.isValidURL("http://127.0.0.1"));
    EXPECT_TRUE(normalizer.isValidURL("https://192.168.1.1"));
}

TEST(URLNormalizerTest, Localhost)
{
    URLNormalizer normalizer;
    EXPECT_TRUE(normalizer.isValidURL("http://localhost"));
    EXPECT_TRUE(normalizer.isValidURL("http://localhost:8080"));
}

TEST(URLNormalizerTest, QueryStrings)
{
    URLNormalizer normalizer;
    EXPECT_TRUE(normalizer.isValidURL("https://example.com/search?q=chatgpt&page=1"));
}

TEST(URLNormalizerTest, UnicodeURL)
{
    URLNormalizer normalizer;
    EXPECT_TRUE(normalizer.isValidURL("https://example.com/こんにちは"));
}

TEST(URLNormalizerTest, PercentEncoding)
{
    URLNormalizer normalizer;
    EXPECT_TRUE(normalizer.isValidURL("https://example.com/a%20b"));
}

TEST(URLNormalizerTest, InvalidPort)
{
    URLNormalizer normalizer;
    EXPECT_FALSE(normalizer.isValidURL("https://example.com:999999"));
    EXPECT_FALSE(normalizer.isValidURL("https://example.com:abc"));
}

TEST(URLNormalizerTest, MissingScheme)
{
    URLNormalizer normalizer;

    EXPECT_FALSE(normalizer.isValidURL("//example.com"));

    EXPECT_FALSE(normalizer.isValidURL("/about"));

    EXPECT_FALSE(normalizer.isValidURL("../page"));
}

TEST(URLNormalizerTest, Authentication)
{
    URLNormalizer normalizer;

    EXPECT_TRUE(normalizer.isValidURL("https://user:password@example.com"));
}

TEST(URLNormalizerTest, LongURL)
{
    URLNormalizer normalizer;
    std::string url = "https://example.com/";
    url.append(5000, 'a');
    EXPECT_TRUE(normalizer.isValidURL(url));
}

TEST(URLNormalizerTest, EmptyHost)
{
    URLNormalizer normalizer;
    EXPECT_FALSE(normalizer.isValidURL("https://:80"));
}