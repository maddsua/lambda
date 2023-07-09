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
	const input = searchbarRef.value.value;
	termWrite('USER >> ' + input)
	webSocket.send(input)
	searchbarRef.value.value = '';
}

</script>

<template>
	<main>
		<div class="wrapper">
			<header>
				<h3>It works kinda like the Redis demo</h3>
				<h3>Try typing <span class="code">get test 123</span> or <span class="code">help</span></h3>
			</header>
			<div class="virtual-terminal" ref="terminalwindowRef"></div>
			<form @submit.prevent="handleInput">
				<div class="prompt">
					<input type="text" ref="searchbarRef">
				</div>
			</form>
		</div>
	</main>
</template>

<style scoped lang="scss">

main {
	display: flex;
	flex-flow: column nowrap;
	padding: 2em 0;

	.wrapper {
		margin: auto;

		header {
			margin: 1em 0;

			h3 {
				margin: 1em 0;

				.code {
					background-color: #333333;
					color: white;
					padding: 0.125em 0.5em;
					border-radius: 0.5em;
				}
			}
		}
	
		@mixin terminal {
			font-family: monospace;
			background-color: hsl(0, 0%, 10%);
			color: white;
			width: 30rem;
			max-width: 90vw;
		};
		.virtual-terminal {
			@include terminal;
			height: 15em;
			padding-left: 0.25em;
			overflow: hidden auto;
		}
		
		.prompt {
			position: relative;
		
			input {
				@include terminal;
				height: 2em;
				overflow: hidden;
				font-size: 1.125em;
				border: none !important;
				outline: none !important;
				padding-left: 4em;
			}
		
			&::before {
				content: "CMD: >> ";
				position: absolute;
				left: 0.25em;
				top: calc(50% - (1em * 0.75));
				z-index: 1;
				color: white;
			}
		}
	}
}

</style>
