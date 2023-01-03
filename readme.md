# A basic HTTP/1.1 Web Server in C++

My idea was to replicate overall feeling of Netlify Functions but in C++, so I could use all the good Windows API like serial ports. Need that for the other project, nevermind.

I've tried half a dozen different libs and they are either require lib-boost, which I personally hate to use, or they do everything the hard way. Ok, there is a third case, where the library is just garbage.

This code is actually a refactoring of my old project that contained http server functionality.

Well, if you just need to create fast and cool backend without all the stuff I need, just use Deno or NodeJS.

But if you are as crazy as me, welcome bro, nice C 🤝

# How is it similar to AWS Lambda?

Take a look at the code side by side:

Netlify Functions (AWS Lambda) goes first (TypeScript) :

```
import { Handler, HandlerEvent, HandlerContext } from "@netlify/functions";

const handler: Handler = async (event: HandlerEvent, context: HandlerContext) => {
  return {
    statusCode: 200,
    body: JSON.stringify({ message: "Hello World" }),
  };
};

export { handler };
```

And here goes maddsua lambda (C/C++):

```
#include "include/lambda.hpp"

maddsuahttp::lambdaResponse requesthandeler(maddsuahttp::lambdaEvent event) {

	return {
		200,	//	status code
		{},		//	no headers in this example
		"{\"message\":\"Hello World\"}"
	};
}
```

Find 10 differences...

# How to use

So basically you init the server providing a callback function that gets called everytime the http request is coming:

```
auto server = maddsuahttp::lambda();
auto startresult = server.init("27015", &requesthandeler);
```

There are two requirements for callback function (`requesthandeler` in this snippet):

1. Return type is `maddsuahttp::lambdaResponse`
2. The function should have single `maddsuahttp::lambdaEvent event` argument.

Apart from it, you are free to do anything.

Create handler function:

```
maddsuahttp::lambdaResponse requesthandeler(maddsuahttp::lambdaEvent event) {

	if (maddsuahttp::findSearchQuery("user", &event.searchQuery) == "maddsua")
		return {
			200,
			{
				{"test", "maddsua"}
			},
			"Good night, my Dark Lord"
		};

	std::string body = "<h1>hello darkness my old friend</h1>";
		body += "Your user agent is: " + maddsuahttp::findHeader("User-Agent", &event.headers);

	return {
		200,
		{},
		body
	};
}
```

This example functions will return "hello darkness my old friend" with your user-agent below for any request that does not have `user=maddsua` search query. For requests that have, it will show different greeting.

By the way, just run [test/main.cpp](test/main.cpp). It's easy as pie.

## Data types

### lambdaResponse

```
uint16_t statusCode;
std::vector < std::string name, std::string value > headers;
std::string body;
```

### lambdaEvent

```
std::string method;
std::string httpversion;
std::string path;
std::vector < std::string name, std::string value > searchQuery;
std::vector < std::string name, std::string value > headers;
std::string body;
```

If you still don't have a clue what is this library, take a look at [Netlify Docs](https://docs.netlify.com/functions/overview/) about their functions. This lib tries to be as close to them as possible.

Check out the `test` directory, a lot of cool stuff there

# How to build

## Requirements:

1. MS Windows as your OS. Gonna add Linux support later, but now all the networking relies on windosk2 and other windows-specific stuff
2. GCC 10+ (probably older versions will do too)
3. Make. Old good one, don't confuse with CMake

### Type literally two commands:

```
cd lib
make
```
Now grab the `libmaddsualambda.a` from `lib` directory and link it to your project.

Don't forget to include `lambda.hpp` from the `include` directory.
