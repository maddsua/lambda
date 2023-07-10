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
    img {
        display: flex;
        margin: 0 auto;
        width: 60px;
        height: 60px;
        margin-bottom: 15px;
    }

    p {
        color: #4696E5;
        text-align: center;
        font-size: 20px;
        font-weight: medium;
        margin-bottom: 15px;
    }

    span {
        font-size: 15px;
        font-weight: 300;
    }
}
</style>