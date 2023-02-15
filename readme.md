# A request-isolating multi thread HTTP/1.1 web server framework

\* At the moment lambda uses M$ Windows implementation of sockets, so in order to build it for Linux you will need to do some work replacing them. I'll probably do it myself in the future, but for not it is what it is.

<br>

### What?

The idea behind this project is to bring the simplicity of building serverless backends with AWS Lambda to your C++ projects. Define a callback function, start a server and watch things just work.

### Why?

OK, I just needed to have http server functionality and serial port communications in the same app. I tried doing that with NodeJS with results that did not satisfy me. Unfortunally, Deno didn't have Web Serial API implemented just yet and all the libs that claim to bring it are, IMHO, garbage.

But it does not end on COM ports. It's C++, do whatever you want to do 😎👍

### Why not any other C++ server library?

Have you ever seen their code structure? It was easier for me to build a web server from scratch, than to find out how their monstrosity works. Well, you can tell that I've done exactly that.

<br>

**Here, a Vue project is being served by lambda for no reason at all**

<img src="docs/what-have-i-done.png">

<br>

## Requirements and dependencies

### Building

- M$ Windows build of [GCC 4+](https://packages.msys2.org/base/mingw-w64-gcc) ( I use GCC 12.2 UCRT by the way )
- [JSON for Modern C++](https://github.com/nlohmann/json)
- `make` utility to make stuff. Get it?

### Running

- Windows 7+
- [Universal C Runtime (URCT)](https://support.microsoft.com/en-us/topic/update-for-universal-c-runtime-in-windows-c0514201-7fe6-95a3-b0a5-287930f3560c)

<br>

# How is it similar to AWS Lambda?

Take a look at these two code snippets:

<br>

Netlify Functions (AWS Lambda under the hood) goes first:

```
import { Handler, HandlerEvent, HandlerContext } from "@netlify/functions";

const handler: Handler = async (event: HandlerEvent, context: HandlerContext) => {
  return {
    statusCode: 200,
    headers: {
      "x-maddsua": "test"
    },
    body: JSON.stringify({ message: "Hello World" }),
  };
};

export { handler };
```
<br>

And here goes my lambda (C/C++):

```
#include "include/lambda.hpp"
#include <nlohmann/json.hpp>

maddsuaHTTP::lambdaResponse requesthandeler(maddsuaHTTP::lambdaEvent event) {
    return {
        200,    //    status code
        {       //    headers
          { "x-maddsua", "test" }
        },
                //    body
        json({ {"message", "Hello World"} }).dump()
    };
}
```

<br>

The main function is not included (obviously) as long as AWS'es backend stuff that actually runs their Lambda.

See [demo code](./main.cpp) for more. It will make sence, I promise.

<br>

# How to use

So basically you init the server providing a callback function that gets called everytime the http request is coming:

```
auto server = maddsuaHTTP::lambda();
auto startresult = server.init(27015, &requesthandeler);
```

There are two requirements for callback function (`requesthandeler` in this snippet):

1. Return type is `maddsuaHTTP::lambdaResponse`
2. The function should have single `maddsuaHTTP::lambdaEvent event` argument.

Apart from it, you are free to do anything.

Create handler function:

```
maddsuaHTTP::lambdaResponse requesthandeler(maddsuaHTTP::lambdaEvent event) {

    std::string body = "<h1>hello darkness my old friend</h1>";
        body += "Your user agent is: " + maddsuaHTTP::findHeader("User-Agent", &event.headers);

    if (maddsuaHTTP::findSearchQuery("user", &event.searchQuery) == "maddsua") {
        body = "Good night, my Dark Lord";
    }
    
    return {
        200,
        {
            {"test", "maddsua"}
        },
        body
    };
}
```

This example functions will return "hello darkness my old friend" with your user-agent below for any request that does not have `user=maddsua` search query. For requests that have, it will show different greeting.

## Types:

### lambdaResponse

```
uint16_t statusCode;
std::vector < std::string name, std::string value > headers;
std::string body;
```

### lambdaEvent

```
std::string httpversion;
std::string requestID;

std::string method;
std::string path;
std::vector < std::string name, std::string value > searchQuery;
std::vector < std::string name, std::string value > headers;
std::string body;
```

If you still don't have a clue what is this library, take a look at [Netlify Docs](https://docs.netlify.com/functions/overview/) about their functions. This lib tries to be as close to them as possible.

# How to build

## Requirements:

1. MS Windows as your OS. Gonna add Linux support later, but now all the networking relies on windosk2 and other windows-specific stuff
2. GCC 10+ (probably older versions will do too)
3. Make. Old good one, don't confuse with CMake
4. zlib aka libz (deflate "compressor")

### Type in `make` to compile. Link the libmdslambda to your project.


Don't forget to include `lambda.hpp` from the `include` directory, and to also link `-lmswsock -lwinmm -lws2_32`.

### Note:
If you are experiencing weird errors during compilation, try putting `#include <winsock2.h>` at the top of the main source file (let's say, main.cpp). That's just winapi being a Microsoft creature in all it's beauty.

# Notes

## WSA

The idea is that you start only one lambda server per application, and handle diffirenet requests trough the handeler function.

When not using lambda server, before calling `fetch()` make sure that windows sockets are initialized. Check that by invoking `socketsReady()` - true indicates that you are good to go. If not, call `WSAStartup()` yourself and hope for best 😈.

Seriously, it would be a better idea to use curl or something similar for the http CLIENT things, in case you don't need the SERVER.


# Dependencies

 - libbrotli <https://github.com/google/brotli>

 - zlib <https://github.com/madler/zlib>

 - Windows Sockets 2 API
