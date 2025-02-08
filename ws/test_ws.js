const ws = new WebSocket('ws://localhost:8000/ws');

ws.addEventListener('message', (event) => console.log('-> ', event.data));

ws.addEventListener('open', () => {
	ws.send('hello there');
	ws.send(new Uint8Array(350));
	ws.send(new Uint8Array(768));
	ws.send(new Uint8Array(78));
	ws.send(new Uint8Array(5581));
	ws.send(new Uint8Array(100768));
	ws.send(new Uint8Array(78));
	ws.close();
});
