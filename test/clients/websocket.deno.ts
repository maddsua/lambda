
const exampleSocket = new WebSocket("ws://localhost:8180/");

exampleSocket.onopen = () => {
	exampleSocket.send("Yo hi there!");
};

exampleSocket.onmessage = (msg) => {
	console.log(msg.data);
}

exampleSocket.onerror = (err) => {
	console.log((err as ErrorEvent).message);
}