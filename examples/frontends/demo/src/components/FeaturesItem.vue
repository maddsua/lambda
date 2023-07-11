<script setup lang="ts">

interface Props {
	item: {
		img: string;
		title: string;
		desc: string;
	};
}
const { item } = defineProps<Props>();

</script>

<template>
    <div class="feature-item">
        <img class="icon" :src="item.img" alt="img">
		<div class="title">{{ item.title }}</div>
        <p>{{ item.desc }}</p>
    </div>
</template>

<style lang="scss" scoped>

.feature-item {
	display: flex;
	min-width: 10rem;
	max-width: 12rem;
	flex-direction: column;
	justify-content: center;
	align-items: center;
	gap: 1em;

	.icon {
		width: 4em;
		height: 4em;
	}

	.title {
		color: var(--app-accent);
		text-align: center;
		font-size: 1.25rem;
		font-weight: 500;
	}

	p {
		font-size: 0.95em;
		font-weight: 300;
	}
}

</style>