<script setup lang="ts">
import HeaderButton from './HeaderButton.vue';
</script>

<template>
	<header class="header-container">

		<div class="intro">
			<img class="logo" src="/logo.png" alt="Logo">

			<div class="content">
				<h1>
					A definitely not a NodeJS-based web server
				</h1>
				<p>Written in C++ from scratch, empowered by OS-native APIs, Websockets, KV storage and virtual file system</p>
			</div>
		</div>

		<div class="click-to-action">
			<HeaderButton href="#">
				Download
			</HeaderButton>
	
			<HeaderButton href="#" :outline="true">
				Learn more
			</HeaderButton>
		</div>

	</header>
</template>

<style lang="scss">

.header-container {
	display: flex;
	width: 40rem;
	max-width: 100%;
	flex-direction: column;
	align-items: center;
	gap: 3.125rem;

	.intro {
		display: flex;
		justify-content: center;
		align-items: center;
		gap: 3rem;
		align-self: stretch;

		.logo {
			width: 8em;
			height: 8em;
			object-fit: contain;
			object-position: center;
		}

		.content {
			display: flex;
			flex-direction: column;
			align-items: flex-start;
			gap: 1em;

			h1 {
				font-size: 1.875rem; 
				font-weight: 500;
				color: var(--app-accent);
			}

			p {
				font-size: 1.125rem; 
				font-weight: 400;
			}
		}
	}

	@media (orientation: portrait) {

		width: 30em;
		max-width: 100%;

		.intro {
			flex-direction: column;
		}
	}

	.click-to-action {
		display: flex;
		justify-content: center;
		align-items: center;
		gap: 1.25rem;
	}
}

</style>
