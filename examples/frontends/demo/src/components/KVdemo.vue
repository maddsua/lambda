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
    <section class="kv-demo-component">

		<header>			
			<h2>
				This demo uses websockets to manipulate KV storage
			</h2>
			<p>
				This is an interactive console. Type help to learn commands!
			</p>
		</header>

        <div class="console" @click="searchbarRef.focus()">

			<div class="virtual-terminal terminal" ref="terminalwindowRef">
				<div>Type help to get available commands</div>
			</div>

			<form class="prompt" @submit.prevent="handleInput">
				<div class="fakeuser">KV >></div>
				<input type="text" ref="searchbarRef" class="terminal">
			</form>

        </div>

    </section>
</template>

<style lang="scss">

.kv-demo-component {
	display: flex;
	max-width: 90rem;
	flex-direction: column;
	gap: 3.125rem;

	header {
		display: flex;
		flex-direction: column;
		gap: 0.625rem;

		h2 {
			color: var(--app-accent);
			font-size: 1.875rem;
			font-weight: 500;
		}

		p {
			font-size: 1.125rem;
			font-weight: 400;
		}
	}

	.console {
		display: flex;
		padding: 1.25em;
		flex-direction: column;
		justify-content: space-between;
		align-items: flex-start;
		border-radius: 1.5em;
		background: #0F172A;
		color: white;
		gap: 2em;

		.virtual-terminal {
			display: flex;
			flex-direction: column;
			gap: 0.5rem;
			width: 100%;
			height: 20em;
			overflow-y: scroll;

			div {
				font-size: 1rem;
				font-family: monospace;
			}
		}

		.prompt {
			display: flex;
			width: 100%;
			gap: 0.5em;

			.fakeuser {
				flex-shrink: 0;
				font-family: monospace;
			}

			input {
				width: 100%;
				font-size: 1rem;
				color: white;
				background-color: transparent;
				font-weight: 400;
				font-family: monospace;
			}
		}
	}
}

</style>