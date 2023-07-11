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
    <div class="features-list">
        <img :src="item.img" alt="img">
        <p>{{ item.title }}</p>
        <span>{{ item.desc }}</span>
    </div>
</template>

<style lang="scss" scoped>
.features-list {
    width: 180px;
    display: flex;
    flex-flow: column;
    align-items: center;
    gap: 10px;

    img {
        width: 60px;
        height: 60px;
    }

    p {
        color: #4696E5;
        text-align: center;
        font-size: 20px;
    }

    span {
        font-size: 15px;
        font-weight: 300;
    }
}
</style>