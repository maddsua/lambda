// Start listening on port 8080 of localhost.
const server = Deno.listen({ port: 8080 });
console.log(`HTTP webserver running.  Access it at:  http://localhost:8080/`);

// Connections to the server will be yielded up as an async iterable.
for await (const conn of server) {
	// In order to not be blocking, we need to handle each connection individually
	// without awaiting the function
	serveHttp(conn);
}

async function serveHttp(conn: Deno.Conn) {

	const httpConn = Deno.serveHttp(conn);

	for await (const rqevent of httpConn) {

		const { request: req } = rqevent;

		if (req.headers.get("upgrade") != "websocket") {
			rqevent.respondWith(new Response(null, { status: 501 }));
		}
		
		const { socket, response } = Deno.upgradeWebSocket(req);
		
		socket.addEventListener("open", () => {
			console.log("a client connected!");
			socket.send("test");
		});

		socket.addEventListener('close', (ev) => console.log(ev));
		socket.addEventListener('error', (ev) => console.log(ev));
		
		socket.addEventListener("message", (event) => {

			if (event.data === "ping") {
			socket.send("pong");
			} else console.log(event.data)
		});

		rqevent.respondWith(response);
	}
}
