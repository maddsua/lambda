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
	padding: 2em;
}

</style>