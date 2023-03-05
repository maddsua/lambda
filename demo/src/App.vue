<script setup lang="ts">
import { RouterLink, RouterView } from 'vue-router';
</script>

<template>
	<header>
		<img alt="Vue logo" class="logo" src="@/assets/icon.svg" width="125" height="125" />

		<div class="wrapper">
			<div class="greetings">
				<h1 class="green">lambda demo app</h1>
				<h3>This isn't a real page but a tech demo. Don't pay much attention to the design. It's just a slightly changed Vue's starter app</h3>
			</div>

			<nav>
				<RouterLink to="/">Home</RouterLink>
				<RouterLink to="/db">Database</RouterLink>
				<RouterLink to="/vfs">VFS</RouterLink>
			</nav>

			<nav class="links">
				<a href="https://github.com/maddsua/lambda/releases" target="_blank">Download</a>
				<a href="https://github.com/maddsua/lambda#readme" target="_blank">Learn more</a>
			</nav>
		</div>
	</header>

	<RouterView />
</template>

<style scoped lang="scss">

.links {
	margin: 1em 0.5em;

	a {
		border: 2px solid #4696e5;
		color: #4696e5;
		text-decoration: none;
		padding: 0.5em 1em;
		border-radius: 0.5em;
		margin: 0 0.5em;

		&:hover {
			background-color: #4696e5;
			color: white;
		}
	}
}

header {
	line-height: 1.5;
	max-height: 100vh;
}

h1 {
	color: #4696e5;
	font-weight: 500;
	font-size: 2.6rem;
	top: -10px;
}

h3 {
	font-size: 1.2rem;
}

.greetings {

	h1, h3 {
		text-align: center;

		@media (min-width: 1024px) {
			text-align: left;
		}
	}
}


.logo {
  display: block;
  margin: 0 auto 2rem;
}

nav {
	width: 100%;
	font-size: 12px;
	text-align: center;
	margin-top: 2rem;

	a {
		color: #4696e5;
		display: inline-block;
		padding: 0 1rem;
		border-left: 1px solid #cacaca;

		&:hover {
			background-color: hsl(210, 75%, 59%);
			color: white;
		}

		&.router-link-exact-active {
			color: var(--color-text);
		}
		&.router-link-exact-active:hover {
			background-color: transparent;
		}
		&:first-of-type {
			border: 0;
		}
	}
}

@media (min-width: 1024px) {
	header {
			display: flex;
			place-items: center;
			padding-right: 2em;
	}

	.logo {
		margin: 0 2rem 0 0;
	}

	header .wrapper {
		display: flex;
		place-items: flex-start;
		flex-wrap: wrap;
	}

	nav {
		text-align: left;
		margin-left: -1rem;
		font-size: 1rem;

		padding: 1rem 0;
		margin-top: 1rem;
	}
}
</style>
