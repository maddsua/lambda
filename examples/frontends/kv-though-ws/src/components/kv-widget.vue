<script setup lang="ts">

import { ref } from 'vue';

const terminalwindowRef = ref(null as HTMLElement);

const termWrite = (text: string) => {
	const entry = document.createElement('div');
	entry.textContent = text;
	terminalwindowRef.value.appendChild(entry);
	terminalwindowRef.value.scrollTop = terminalwindowRef.value.scrollHeight - terminalwindowRef.value.getBoundingClientRect().height;
};

const webSocket = new WebSocket('ws://localhost:8080/ws');

webSocket.addEventListener('message', (event) => {
	
	const response = event.data as string;

	if (response.includes('\n'))
		(response as string).split('\n').forEach(line => termWrite(line));
	else termWrite('KV >> ' + response);

});

const searchbarRef = ref(null as HTMLInputElement);

const handleInput = () => {

	/*if (webSocket.CLOSED) {
		termWrite('DEMO >> KV disconnected')
		return;
	}*/

	const input = searchbarRef.value.value;
	termWrite('USER >> ' + input)
	webSocket.send(input)
	searchbarRef.value.value = '';
}

</script>

<template>
	<div class="kv-live-widget">
		<div class="virtual-terminal terminal" ref="terminalwindowRef"></div>
		<form @submit.prevent="handleInput">
			<div class="prompt">
				<input type="text" ref="searchbarRef" class="terminal">
			</div>
		</form>
	</div>
</template>

<style scoped>

.kv-live-widget {

}

.terminal {
	font-family: monospace;
	background-color: hsl(0, 0%, 10%);
	color: white;
	width: 30rem;
	max-width: 90vw;
}

.virtual-terminal {
	height: 15em;
	padding-left: 0.25em;
	overflow: hidden auto;
}

.prompt {
	position: relative;
}

.prompt::before {
	content: "CMD: >> ";
	position: absolute;
	left: 0.25em;
	top: calc(50% - (1em * 0.75));
	z-index: 1;
	color: white;
}

.prompt input {
	height: 2em;
	overflow: hidden;
	font-size: 1.125em;
	border: none !important;
	outline: none !important;
	padding-left: 4em;
}

</style>
