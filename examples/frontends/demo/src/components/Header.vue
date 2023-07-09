<script setup lang="ts">
import HeaderButton from './HeaderButton.vue';
</script>

<template>
    <div class="header-container">
        <div class="header-logo"><img src="/logo.png" alt=""></div>
        <div class="header-title">
            <h1>
                A definitely not NodeJS-based web server
            </h1>
            <p>Written in C++ from scratch, empowered by OS-native APIs, Websockets, KV storage and virtual file system</p>
        </div>
    </div>
    <HeaderButton/>
</template>

<style lang="scss" scoped>
.header-container {
    width: 945px;
    margin: 85px auto 30px;
    display: flex;
    align-items: center;
}

.header-logo {
    margin: 0 auto;

    img {
        width: 220px;
        height: 230px;
    }
}

.header-title {

    h1 {
        width: 680px;
        font-size: 45px;
        color: #4696E5;
        font-weight: 500;
        margin-bottom: 14px;
    }

    p {
        width: 650px;
        color: #141414;
        font-size: 22px;
        font-weight: 300;
    }
}
</style>