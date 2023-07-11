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
    <HeaderButton />
</template>

<style lang="scss">
.hidden {
    opacity: 0;
}

.click-to-action {
    opacity: 1;
    animation: ani 2s forwards;
}

.header-container {
    opacity: 1;
    animation: ani 2s forwards;
}

@keyframes ani {
    0% {
        opacity: 0;
    }

    100% {
        opacity: 1;
    }
}

.header-container {
    width: 945px;
    max-width: 100%;
    margin: 85px auto 30px;
    display: flex;
    align-items: center;
}

.header-logo {
    margin-right: 20px;

    img {
        width: 220px;
        height: 230px;
    }
}

.header-title {
    margin-right: 30px;

    h1 {
        font-size: 45px;
        color: #4696E5;
        font-weight: 500;
        margin-bottom: 14px;
    }

    p {
        color: #141414;
        font-size: 22px;
        font-weight: 300;
    }
}

@media (max-width: 525px) {
    .header-logo {
        margin-right: 5px;
    }

    .header-title {
        margin-right: 5px;

        h1 {
            font-size: 28px;
            margin-bottom: 10px;
        }

        p {
            font-size: 15px;
        }
    }
}
</style>