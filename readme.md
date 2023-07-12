# A definitely not a NodeJS-based web server framework

## Why?

I wanna have the power of C++ on my server apps, but I really don't like the APIs that the vast majority of C/C++ server libs provide. I like Deno and it's APIs, but you cant easily make it work with windows-specific APIs/some socially awkward devices/cryptic af dlls from 1995 and some other crazy stuff I work with.

Does it sound like a deal? It does to me.


## Why lambda?

It was very similar to AWS lambda before 1.0, that's why the name. I didn't spend a second thinking of it. A true engineer move, let the marketing department fix it later.

**Here, a Vue project is being served by lambda for no reason at all**

<img src="docs/what-have-i-done.png">

<br>

## JSON

You might not like my JSON implementation (I had a reason to put it here), so you might want to try [JSON for Modern C++ ](https://github.com/nlohmann/json) as a replacement. My implementation will still be used under the hood where performance is desirable.

## Building a server app with lambda

### Requirements

1. GCC 10+ and autotools or just GCC 10+ and make (any version really)
2. Get `deps/` submodules downloaded too

On windows you can get the compiler from [msys2](https://packages.msys2.org/package/)

Oh and yes, Visual Studio can go touch grass itself.

### Compiling library

If the mix of nodejs and python for generating secondary source files scares you, it really shouldn't, the resulting files are included in this repo, you don’t have to think about it at all. I just can't decide which one to use.

`make` command will produce `liblambda.dll.a` and `lambda.dll`. Never it was simpler than that.

Don't forget to add `-lws2_32` on Windows or it will bite you in the ass.

### Compiling your project

Link `lambda.dll` and you're good to go. Oh, don't forget to include some headers, obviously.

Go see [some examples or smthng](/examples)


## A few quick notes before you start blasting

### OS support

- Any modern linux
- Windows 7+ (might actually work on 7, but is not tested and I don't care about it anyway)

### Compiler version

Use GCC 10 or never. The project uses a few of the c++20 features, and is not gonna compile under GCC's older version.

Other compilers are not tested, the reason being that I don't care. Contribute to the project if you do.
