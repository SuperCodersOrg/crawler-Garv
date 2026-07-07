# Browser Renderer Design -
This component implements the browser rendering layer used by the crawler to collect the final HTML of modern web pages after JavaScript execution. Starting from a page URL the browser renderer starts Chrome connects to the Chrome DevTools Protocol (CDP) creates a page navigates to the required URL waits for the page load event and returns the fully rendered HTML.

It is designed to work as a reusable low level module inside the crawler pipeline so the crawler can render pages before parsing them for links and storing them in SQLite.

## Design Decisions -

* Chrome is used instead of a normal HTTP client because many modern sites build their content dynamically through JavaScript and API calls. Rendering inside Chrome allows the crawler to capture the final DOM instead of only the initial server response.
* The renderer is divided into small components so process management network communication and CDP command handling remain separate.
* A temporary Chrome profile is used so each crawl session starts with a clean browser state and does not depend on the user profile.
* The rendered HTML is returned as plain text so the crawler can pass it directly to the HTMLParser and Storage components without extra transformation.
* The renderer is kept separate from crawling logic because crawling is responsible for URL discovery ordering duplicate checking and storage while the renderer is only responsible for producing the final page HTML.

## Components -

### 1. ChromeProcess -
* Need - ChromeProcess is responsible for starting and stopping the Chrome browser process used for rendering pages.

* Design - It launches Chrome with remote debugging enabled on port 9222 and creates a temporary user data directory so the renderer can connect to Chrome through CDP. A separate process manager is used instead of starting Chrome from the CDP layer because browser lifecycle control is an independent responsibility.

* Public APIs / Structure -
```cpp
class ChromeProcess
{
public:
    ChromeProcess();
    ~ChromeProcess();

    bool start(bool headless = false);
    void stop();

    bool isRunning() const;

private:
    PROCESS_INFORMATION processInfo_{};
    bool running_ = false;

    std::string chromePath_;
    std::string profilePath_;

    std::string buildCommand(bool headless) const;
};
```

* Internal Representation - //diagram

* Time Complexity -
    * start - O(1)
    * stop - O(1)
    * isRunning - O(1)
    * buildCommand - O(n) where n is the length of the generated command string

* ChromeProcess is used because the crawler needs direct control over browser startup and shutdown. A separate browser launcher also makes it easier to switch between headless and visible rendering in future versions.

### 2. HTTPClient -
* Need - HTTPClient is responsible for sending a simple HTTP request to Chrome’s local debugging endpoint.

* Design - After Chrome starts the renderer queries /json/version from the local debugging server to retrieve the websocketDebuggerUrl. A small HTTP client is sufficient here because this step only needs a single local GET request and does not need full browsing features.

* Public APIs / Structure -
```cpp
class HTTPClient
{
public:
    std::string get(const std::wstring& host,
                    int port,
                    const std::wstring& path);
};
```

* Internal Representation - //diagram

* Time Complexity -
    * get - O(n) where n is the size of the response body

* HTTPClient is limited to the CDP bootstrap step because the actual page rendering work is handled by WebSocketConnection and CDPConnection.

### 3. WebSocketConnection -
* Need - WebSocketConnection manages the transport channel used to communicate with Chrome DevTools Protocol.

* Design - CDP communicates over a WebSocket connection so the renderer parses the websocket URL connects to Chrome sends JSON messages and reads responses from the debugger socket. This class owns the low level socket handling so higher level CDP code can work with messages instead of raw network operations.

* Public APIs / Structure -
```cpp
class WebSocketConnection
{
public:
    WebSocketConnection();
    ~WebSocketConnection();
    bool connect(const std::string& url);
    bool send(const std::string& message);
    std::string receive();
    void disconnect();
    bool isConnected() const;
private:
    struct Endpoint
    {
        std::string host;
        std::string port;
        std::string target;
    };
    Endpoint parseURL(const std::string& url);
private:
    boost::asio::io_context io_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> websocket_;
    boost::beast::flat_buffer buffer_;
    bool connected_;
};
```

* Internal Representation - //diagram

* Time Complexity -
    * connect - Depends on network and socket handshake
    * send - O(n) where n is the message size
    * receive - O(n) where n is the received message size
    * disconnect - O(1)
    * isConnected - O(1)

* WebSocketConnection is isolated from the rest of the renderer because the CDP protocol layer should not depend on transport details. This also keeps reconnection and socket handling logic reusable.

### 4. CDPConnection -
* Need - CDPConnection provides the actual Chrome DevTools Protocol commands used by the renderer.

* Design - It wraps WebSocketConnection and sends JSON commands such as Target.createTarget Target.attachToTarget Page.enable Runtime.enable Page.navigate and Runtime.evaluate. The class also filters protocol events and waits for the matching response id so higher level code can call simple methods instead of handling raw JSON messages.

* Public APIs / Structure -
```cpp
class CDPConnection
{
public:

    bool connect(const std::string& websocketURL);

    void disconnect();

    bool createPage();

    bool enablePage();

    bool enableRuntime();

    bool navigate(const std::string& url);

    std::string getHTML();

private:

    nlohmann::json sendCommand(
        const std::string& method,
        const nlohmann::json& params = {});

private:

    int nextId_ = 1;

    std::string sessionId_;

    WebSocketConnection websocket_;
};
```

* Internal Representation - //diagram

* Time Complexity -
    * connect - Depends on the WebSocket handshake
    * disconnect - O(1)
    * createPage - Depends on the CDP round trips
    * enablePage - Depends on the CDP round trip
    * enableRuntime - Depends on the CDP round trip
    * navigate - Depends on the CDP round trip and page load
    * getHTML - Depends on the size of the evaluated page DOM

* CDPConnection is the main protocol layer because it turns raw debugger messages into renderer actions. The crawler does not need to know how CDP works internally and only receives the final HTML string.

### 5. Browser Rendering Workflow -
* Need - The browser renderer workflow coordinates Chrome startup connection setup page creation navigation and DOM extraction.

* Design - The workflow is executed in a fixed order so each step prepares the next one. Chrome must be started first then the debugger version endpoint is queried then the WebSocket connection is established then a page is created and enabled then the target URL is loaded and finally the fully rendered HTML is read from the DOM.

* Workflow -
```text
Start Chrome

↓

HTTP GET /json/version

↓

Get websocketDebuggerUrl

↓

Connect WebSocket

↓

Create Page

↓

Enable Runtime

↓

Enable Page

↓

Navigate

↓

Wait for Page.loadEventFired

↓

Get HTML

↓

Return Response
```

* Internal Representation - //diagram

* Time Complexity - The overall running time depends mainly on browser startup network latency page rendering time and the size of the returned DOM. The browser itself performs the heavy work so the renderer mostly coordinates the steps.

* This workflow is necessary because modern sites often require JavaScript execution before the final page content becomes visible to the crawler.

## Usage in the Crawl Project -

* The crawler uses BrowserRenderer after a URL has been normalized checked against SeenStore and dequeued from the Frontier.
* Once the renderer returns the final HTML the crawler stores that HTML in SQLite through Storage.
* The crawler then passes the rendered HTML to HTMLParser so new links can be extracted and normalized.
* Those extracted links are added back into SeenStore and Frontier if they have not been discovered before.
* This means the browser renderer sits between Frontier processing and HTML parsing inside the overall crawl loop.

## Crawler Integration Flow -

1. The crawler takes a seed URL and normalizes it using URLNormalizer.
2. The normalized URL is checked in SeenStore to prevent duplicate discovery.
3. If the URL is new it is added to SeenStore and pushed into the Frontier as a URLDepth entry.
4. The crawler dequeues the next URL from the Frontier and passes it to BrowserRenderer.
5. BrowserRenderer starts Chrome if needed connects to the local debugging endpoint and creates a CDP session.
6. The page is navigated to the requested URL and the renderer waits for the load event.
7. The final HTML is read from the DOM and returned to the crawler.
8. Storage saves the rendered HTML into SQLite for later indexing.
9. HTMLParser extracts outgoing links from the rendered HTML.
10. Each extracted link is normalized checked against SeenStore and if new added back to the Frontier.
11. The process continues until the Frontier becomes empty or the maximum crawl depth is reached.

## Failure Handling -

* Chrome Start Failure - If Chrome cannot start the renderer returns failure and the crawler can skip the current URL.
* Debug Port Unavailable - If the local CDP endpoint is not reachable the connection step fails and crawling continues with the next URL.
* WebSocket Failure - If the debugger socket closes unexpectedly the renderer disconnects and the current page is treated as failed.
* Navigation Failure - If the page does not load or returns an invalid navigation result the crawler skips that page and continues.
* Empty HTML - If the final DOM is empty the page is not stored and the crawler moves on to the next URL.
* Unsupported URL - If the input URL is invalid or unsupported the renderer is not invoked and the URL is skipped earlier by URLNormalizer.

## Future Compatibility -

* The browser renderer has been designed so the next project can reuse the rendered DOM directly from Storage without downloading pages again.
* Because the rendering step is isolated from parsing and indexing the same BrowserRenderer output can be used by the indexer to process titles visible text and metadata later.
* The renderer can also be extended in future versions to support screenshots richer waiting strategies or multiple tabs without changing the crawler control flow.

## Complexity Summary -
| Component | Primary Operation | Complexity |
| --------- | ----------------- | ---------- |
| ChromeProcess | Start / Stop Chrome | O(1) |
| HTTPClient | Fetch debugger version endpoint | O(n) |
| WebSocketConnection | Send / Receive CDP messages | O(n) per message |
| CDPConnection | CDP commands and HTML extraction | Depends on protocol round trips and page size |
| Browser Renderer | Render a page and return HTML | Depends on browser startup network and rendering time |

## Overall Role -

The browser renderer is the layer that makes the crawler capable of handling modern JavaScript heavy websites. It sits between the crawl frontier and the parser and turns each discovered URL into rendered HTML that can be stored and reused by the rest of the project.