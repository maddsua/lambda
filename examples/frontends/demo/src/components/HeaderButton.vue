<script setup lang="ts">

interface Props {
	href: string;
	outline?: boolean;
}
const { href, outline } = defineProps<Props>();

</script>

<template>
	<a :href="href" :class="{ outline }">
		<slot>
			[Link text]
		</slot>
	</a>
</template>

<style lang="scss" scoped>

a {
	display: flex;
	padding: 0.625rem 1.25rem;
	justify-content: center;
	align-items: center; 
	border-radius: 0.625rem;
	background: var(--app-accent);
	color: white;
	text-align: center;
	font-size: 1rem;
	font-weight: 500;

	&.outline {
		border: 2px solid var(--app-accent);
		background-color: transparent;
		color: var(--app-accent);
	}
}

</style>
