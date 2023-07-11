<script setup lang="ts">
const props = defineProps({
    featurelist: { type: Object }
})
</script>

<template>
    <div class="features-list">
        <img :src="featurelist.img" alt="img">
        <p>{{ featurelist.title }}</p>
        <span>{{ featurelist.subtitle }}</span>
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