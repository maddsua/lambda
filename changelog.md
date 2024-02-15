Please note: this changelog is more like a history of the project,
not literally the changelog for each and every version.
As it's hosted on github, please refer to releases page for that.

---

## v2 (somewhere in Feb 2024, I’m writing it rn and don’t know when it will be done)

A complete and total rewrite of everything once again, and I mean everything.
There’s no changelog for it, it’s a totally new codebase.
Why did I do it? Well, the old one was a mess and fixing it
would introduce even more issues.

Now it’s like almost perfect, you can use it in a project without
constantly expecting it to crash or something.

Some mad performance gains were achieved on the way here, so here's that too.

Refer to readme to learn about the features, and to examples, to, well, see them.


## v1.0 (Jul 13, 2023)

Features and fixes:
- Added Support of basic web APIs
- Added Brotli and gzip http compression
- Solved a ton of random crashes
- Ditched super unsafe parts of code
- Fixed multiple memory leaks
- Added Websocket server basic functionality
- Added fetch client (just like in javascript)

And probably some more things I can't remember.


## v0.5 (Mar 8, 2023)

Got it moved to a public GH repo, and as a side note, rewrote the entire thing
to a more sensible form. Now it’s not a huge stinky mess of code garbage.
It wasn’t really stable but at least the API wouldn’t make your eyes bleed.


## v0.2 (Sep 26, 2022)

500 commits later it got a bit better, not quality wise though. A JSON parser/serializer
was added (which would break if you had any repeating keys in a JSON string, not cool)...
and that’s basically it. All the other changes were project-specific and had
nothing to do with the server part itself.


## v0.1 (Feb 01, 2022)

I started working on an IoT project that needed to have a web server that would collect
data from multiple devices connected via serial port and send a report to the cloud.
Sounds simple until you realize that npm packages that were supposed to help are barely working.
And me being me was like: fck it I’ll write it from ground up in C++.

Was it a good server? Hell no it wasn't - it was slow, unstable, the code was horrific and it
wasn’t even a full implementation. But it was working and it was enough at the time.
