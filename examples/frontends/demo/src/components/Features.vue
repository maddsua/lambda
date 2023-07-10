<script lang="ts" setup>
import FeaturesList from './FeaturesList.vue';

const features = [
    {
        id: '01',
        title: 'Custom JSON',
        subtitle: 'Lambda’s JSON parser and serializer were created with a priority of being super simple to use, without sacrificing the performance',
        img: '/svg/json.svg'
    },
    {
        id: '02',
        title: 'KV storage',
        subtitle: 'Key-value storages offer fast read and write operations, making them suitable for scenarios where quick access to data is crucial',
        img: '/svg/data.svg'
    },
    {
        id: '03',
        title: 'Virtual file system',
        subtitle: 'Deploy all the frontend code from a single archive file, use lambda’s own lvfs2 format to gain extra speed',
        img: '/svg/targs.svg'
    },
    {
        id: '04',
        title: 'Websockets',
        subtitle: 'Deploy all the frontend code from a single archive file, use lambda’s own lvfs2 format to gain extra speed',
        img: '/svg/ws.svg'
    },
    {
        id: '05',
        title: 'Host OS API',
        subtitle: "Full access to Windows or other OS native API.Do any crazy shit you want to. It’s C++ bro, there’s noting to scream at you 😈",
        img: '/svg/os.svg'
    },
]
</script>

<template>
    <section class="features">
        <div class="features-container">
            <h2>Features</h2>
            <div class="features-link">
                <FeaturesList v-for="featurelist in features" :key="featurelist.id" v-bind:featurelist="featurelist" />
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
    display: grid;
    grid-template-columns: repeat(5, 180px);
    grid-column-gap: 45px;
    justify-content: center;
}
</style>