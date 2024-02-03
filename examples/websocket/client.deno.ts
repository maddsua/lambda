
const exampleSocket = new WebSocket("ws://localhost:8180/");

exampleSocket.onopen = () => {
	console.log("Connected");
	exampleSocket.send("Yo hi there!");
};

exampleSocket.onmessage = (msg) => {
	console.log(msg.data);
};

exampleSocket.onerror = (err) => {
	console.error((err as ErrorEvent).message);
};

exampleSocket.onclose = () => {
	console.log('Disconnected');
};
