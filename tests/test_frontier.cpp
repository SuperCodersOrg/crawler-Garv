#include <gtest/gtest.h>
#include <stdexcept>
#include "crawler/Frontier.h"

//no conditons for maxdepth or invalid url those are gonna be handled by crawler 

TEST(FrontierTest, EmptyInitially)
{
    Frontier frontier;
    EXPECT_TRUE(frontier.empty());
    EXPECT_EQ(frontier.size(), 0);
}

TEST(FrontierTest, EnqueueSingle)
{
    Frontier frontier;
    frontier.enqueue({"https://example.com", 0});
    EXPECT_FALSE(frontier.empty());
    EXPECT_EQ(frontier.size(), 1);
    EXPECT_EQ(frontier.front().URL, "https://example.com");
    EXPECT_EQ(frontier.front().depth, 0);
}

TEST(FrontierTest, FIFO)
{
    Frontier frontier;
    frontier.enqueue({"A", 0});
    frontier.enqueue({"B", 1});
    frontier.enqueue({"C", 2});
    EXPECT_EQ(frontier.dequeue().URL, "A");
    EXPECT_EQ(frontier.dequeue().URL, "B");
    EXPECT_EQ(frontier.dequeue().URL, "C");
    EXPECT_TRUE(frontier.empty());
}

TEST(FrontierTest, FrontDoesNotRemove)
{
    Frontier frontier;
    frontier.enqueue({"A", 5});
    EXPECT_EQ(frontier.front().URL, "A");
    EXPECT_EQ(frontier.front().depth, 5);
    EXPECT_EQ(frontier.size(), 1);
    EXPECT_EQ(frontier.front().URL, "A");
    EXPECT_EQ(frontier.size(), 1);
}

TEST(FrontierTest, Clear)
{
    Frontier frontier;
    frontier.enqueue({"A", 0});
    frontier.enqueue({"B", 1});
    frontier.enqueue({"C", 2});
    frontier.clear();
    EXPECT_TRUE(frontier.empty());
    EXPECT_EQ(frontier.size(), 0);
}

TEST(FrontierTest, DequeueEmpty)
{
    Frontier frontier;
    EXPECT_THROW(frontier.dequeue(), std::out_of_range);
}

TEST(FrontierTest, FrontEmpty)
{
    Frontier frontier;
    EXPECT_THROW(frontier.front(), std::out_of_range);
}

TEST(FrontierTest, AlternateEnqueueDequeue)
{
    Frontier frontier;
    frontier.enqueue({"A", 0});
    EXPECT_EQ(frontier.dequeue().URL, "A");
    frontier.enqueue({"B", 1});
    frontier.enqueue({"C", 2});
    EXPECT_EQ(frontier.dequeue().URL, "B");
    frontier.enqueue({"D", 3});
    EXPECT_EQ(frontier.dequeue().URL, "C");
    EXPECT_EQ(frontier.dequeue().URL, "D");
    EXPECT_TRUE(frontier.empty());
}

TEST(FrontierTest, PreserveDepth)
{
    Frontier frontier;
    frontier.enqueue({"page1", 3});
    URLDepth page = frontier.dequeue();
    EXPECT_EQ(page.URL, "page1");
    EXPECT_EQ(page.depth, 3);
}

TEST(FrontierTest, LargeNumberOfElements)
{
    Frontier frontier;
    for(int i = 0; i < 10000; i++)
    {
        frontier.enqueue({std::to_string(i), i});
    }
    EXPECT_EQ(frontier.size(), 10000);
    for(int i = 0; i < 10000; i++)
    {
        URLDepth page = frontier.dequeue();
        EXPECT_EQ(page.URL, std::to_string(i));
        EXPECT_EQ(page.depth, i);
    }
    EXPECT_TRUE(frontier.empty());
}

TEST(FrontierTest, SingleElement)
{
    Frontier frontier;
    frontier.enqueue({"A",0});
    EXPECT_EQ(frontier.front().URL,"A");
    EXPECT_EQ(frontier.dequeue().URL,"A");
    EXPECT_TRUE(frontier.empty());
    EXPECT_EQ(frontier.size(),0);
}

TEST(FrontierTest, ClearEmptyFrontier)
{
    Frontier frontier;
    EXPECT_NO_THROW(frontier.clear());
    EXPECT_TRUE(frontier.empty());
    EXPECT_EQ(frontier.size(),0);
}

TEST(FrontierTest, ClearTwice)
{
    Frontier frontier;
    frontier.enqueue({"A",0});
    frontier.clear();
    EXPECT_NO_THROW(frontier.clear());
    EXPECT_TRUE(frontier.empty());
}

TEST(FrontierTest, ReuseAfterClear)
{
    Frontier frontier;
    frontier.enqueue({"A",0});
    frontier.enqueue({"B",1});
    frontier.clear();
    frontier.enqueue({"C",2});
    EXPECT_EQ(frontier.dequeue().URL,"C");
    EXPECT_TRUE(frontier.empty());
}

TEST(FrontierTest, FrontUpdatesCorrectly)
{
    Frontier frontier;
    frontier.enqueue({"A",0});
    frontier.enqueue({"B",1});
    frontier.dequeue();
    EXPECT_EQ(frontier.front().URL,"B");
}

TEST(FrontierTest, SizeUpdatesCorrectly)
{
    Frontier frontier;
    EXPECT_EQ(frontier.size(),0);
    frontier.enqueue({"A",0});
    EXPECT_EQ(frontier.size(),1);
    frontier.enqueue({"B",1});
    EXPECT_EQ(frontier.size(),2);
    frontier.dequeue();
    EXPECT_EQ(frontier.size(),1);
    frontier.dequeue();
    EXPECT_EQ(frontier.size(),0);
}

TEST(FrontierTest, LargeDepth)
{
    Frontier frontier;
    frontier.enqueue({"A",1000000});
    EXPECT_EQ(frontier.dequeue().depth,1000000);
}

TEST(FrontierTest, EmptyURL)
{
    Frontier frontier;
    frontier.enqueue({"",0});
    EXPECT_EQ(frontier.dequeue().URL,"");
}

TEST(FrontierTest, DuplicateURLsAllowed)
{
    Frontier frontier;
    frontier.enqueue({"A",0});
    frontier.enqueue({"A",1});
    EXPECT_EQ(frontier.dequeue().depth,0);
    EXPECT_EQ(frontier.dequeue().depth,1);
}