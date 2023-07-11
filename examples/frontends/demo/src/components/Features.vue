<script lang="ts" setup>

import FeaturesItem from './FeaturesItem.vue';

const features = [
    {
        title: 'Custom JSON',
        desc: 'Lambda’s JSON parser and serializer were created with a priority of being super simple to use, without sacrificing the performance',
        img: '/svg/json.svg'
    },
    {
        title: 'KV storage',
        desc: 'Key-value storages offer fast read and write operations, making them suitable for scenarios where quick access to data is crucial',
        img: '/svg/data.svg'
    },
    {
        title: 'Virtual file system',
        desc: 'Deploy all the frontend code from a single archive file, use lambda’s own lvfs2 format to gain extra speed',
        img: '/svg/targs.svg'
    },
    {
        title: 'Websockets',
        desc: 'Deploy all the frontend code from a single archive file, use lambda’s own lvfs2 format to gain extra speed',
        img: '/svg/ws.svg'
    },
    {
        title: 'Host OS API',
        desc: "Full access to Windows or other OS native API. Do whatever crazy shit you want to. It’s C++ bro, there’s noting to scream at you",
        img: '/svg/os.svg'
    },
];
</script>

<template>
    <section class="features">

        <div class="features-container">

            <h2>Features</h2>

            <div class="features-link">
                <FeaturesItem v-for="item of features" :item="item" />
            </div>

        </div>

    </section>
</template>

<style lang="scss" scoped>
h2 {
    color: #4696E5;
    font-size: 45px;
    font-weight: 500;
    text-align: center;
    margin-bottom: 50px;
}

.features-link {
    display: flex;
    justify-content: center;
    align-items: flex-start;
    align-content: flex-start;
    gap: 45px;
    align-self: stretch;
    flex-wrap: wrap;
}

.hidden {
    opacity: 0;
}

.features {
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
</style>