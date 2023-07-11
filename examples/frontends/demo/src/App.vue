<script setup lang="ts">

import Header from './components/Header.vue';
import Features from './components/Features.vue';
import Console from './components/Console.vue';
import Footer from './components/Footer.vue';

</script>

<template>
	<main>
		<Header />
		<Features/>
		<Console/>
	</main>
	<Footer/>
</template>

<style scoped>

main {
	display: flex;
	flex-direction: column;
	align-items: center;
	gap: 5rem;
	width: 80rem;
	max-width: 100%;
	padding: 0 2em;
	margin: auto;
	margin-top: 4em;
}

</style>