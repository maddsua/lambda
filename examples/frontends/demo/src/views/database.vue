<script lang="ts">
import terminalapi from '../apis/terminalapi';
export default {
	methods: {
		handleInput(event: Event) {
			const prompt = (this.$refs.userinput as HTMLInputElement);
			if (!prompt.value) return;
			const terminal = (this.$refs.terminal as HTMLInputElement);
			
			terminalapi(terminal, prompt.value);
			prompt.value = '';
		}
	}
}
</script>

<template>
	<main>
		<div class="wrapper">
			<header>
				<h3>It works kinda like the Redis demo</h3>
				<h3>Try typing <span class="code">get test</span> or <span class="code">help</span></h3>
			</header>
			<div class="virtual-terminal" @click="($refs.userinput as HTMLInputElement).focus()" ref="terminal"></div>
			<form @submit.prevent="handleInput">
				<div class="prompt">
					<input type="text" ref="userinput">
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