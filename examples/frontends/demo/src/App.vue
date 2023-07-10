<script setup lang="ts">
import Header from './components/Header.vue';
import Features from './components/Features.vue';
import Console from './components/Console.vue';
import Footer from './components/Footer.vue';
</script>

<template>
  <header>
    <Header />
  </header>
  <main>
    <Features/>
    <Console/>
  </main>
  <footer>
    <Footer/>
  </footer>
</template>