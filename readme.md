# lambda

**A zero dependency C++ application server library**

So this is my fun-project. It started as a really basic REST API server that had to talk to the cloud on one side and some embedded hardware on the other.

Over time it became a self skill measurement contest for me. The goals are pretty simple - make it as good as possible.

## The design

The initial design was trying to follow AWS Lambda (yeah fr), hence the name. The problem with it was that it's not really extendable. As the result, V3 had it's design done from scratch (for the fourth time in total, not even kidding).

Anyway, have a look at the standard measure of web framework badassness - the "hello world" sample app:

```c++
#include "../lambda.hpp"

void handler_fn(Lambda::Request& req, Lambda::ResponseWriter& wrt) {
	wrt.write("yo, mr white");
};

int main() {
	Lambda::Server(handler_fn, { .port = 8100 }).serve();
	return 0;
}
```

Doesn't look too bad for a C++ app, eh?

<img src="docs/what-have-i-done.webp" style="max-width: 720px" />

**Here, a Vue project is being served by lambda for no reason at all**

## Features

- Mostly memory safe design
- HTTP/1.1 implementation with keep-alive and deferred responses
- ServerSideEvents
- WebSocket
- Fileserver extension
- Custom JSON parser/serializer implementation
- Custom logger and formatter
- [in progress] Request router

## Performace (v2, outdated)

<img src="docs/bench-v1-vs-v2.webp" style="max-width: 720px" />

**A latency comparison between v1 and v2** (50 rps, 10s)

I was able to push it all the way up to 25k rps but it's not stable at all at that rate.

Removing the task join list would help smooth that graph, but it's not what you wan't for memory safety.
Although, if you are not planning to destroy a lambda instance (if you only create it once in program lifetime),
you may want to disable the task join list.

Now let's get a bit crazy and crank it up to 5000 rps. At this point v1 just died, and v2 didn't even feel it:

<img src="docs/bench-v2-5krps.webp" style="max-width: 720px" />

## Error handling/exceptions

By default all exceptions should be caught by the `serve` handler. However, if you linkt the c++ standard library statically the exception handling may not work, which will break the app. So don't do that.


## Building a server app with lambda

I'm using GCC 12 here, you should be fine with any other version that supports C++ 20
but don't expect it to work with MSVC without doing a shaman dance first.

### Linking

You can link lambda both as a dll (shared object) or a static (.a) library.
Both ways should be fine. In case you need a dll import object (.dll.a) it's generated too when building the `libshared` target.

When on Windows, don't forget to link with winsock2 (add `-lws2_32`) or it will bite you in the ass.

### Compiling your project

Link `lambda.dll` and you're good to go. Oh, don't forget to include some headers, obviously.

Go see [some examples or smthng](/examples)

### A quick note before you start blasting

Use GCC 12 or never. The project uses a few of the c++23 features, and it might not compile with older versions.

Other compilers are not \*officially\* supported, but there aren't any super-GCC-specific tricks employed in this project, so theoretically with some tweaking it should compile anywhere.
