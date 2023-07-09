<script setup lang="ts">
import WelcomeItem from '../components/feature.vue'
</script>

<template>

	<main>
		<WelcomeItem>
			<template #heading>http server</template>

			Okay, it's kinda obvious that lambda is a web server framework. It's not really designed to serve files though, but rather to provide an AWS Lambda-like backend.
		</WelcomeItem>

		<WelcomeItem>
			<template #heading>http compression</template>

			Both gzip and brotli compression algorythms are supported by default
		</WelcomeItem>

		<WelcomeItem>
			<template #heading>Redis-like integrated database</template>

			A key-value in-memory database. Sounds cool so I added it. If you're familiar with JS, it works just like localStorage API
		</WelcomeItem>

		<WelcomeItem>
			<template #heading>Virtual filesystem</template>

			Allows you to bundle all the static files into single tar/tar.gz file and serve app from it. Or, you may use it as a storage for user data. Create how many instances the hardware can handle bc it's all in memory, and so it you still use hard drives VFS will make everything run much fater
		</WelcomeItem>

		<WelcomeItem>
			<template #heading>Websockets (coming in 1.0)</template>

			Not implemented yet but soon...
		</WelcomeItem>
	</main>
</template>
