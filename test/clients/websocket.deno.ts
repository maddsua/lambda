
const exampleSocket = new WebSocket("ws://localhost:8180/");

exampleSocket.onopen = () => {
	exampleSocket.send("Yo hi there!");
};
