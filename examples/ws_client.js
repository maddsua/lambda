const ws = new WebSocket('ws://localhost:8000/ws');

ws.addEventListener('message', (event) => console.log('-> ', event.data));

ws.addEventListener('open', () => {
	
	ws.send('hello there');
	ws.send(new Uint8Array(2));

	ws.send('A very important notice');

	ws.send('Gimme 50 Ewro');

	ws.close();
});
