<template>
    <div class="coopyright">
        <span>2023 maddsua</span>
        <a href="https://github.com/maddsua/lambda/tree/main">Github</a>
    </div>
</template>

<style lang="scss" scoped>
.coopyright {
    display: flex;
    justify-content: center;
    align-items: center;
    height: 80px;
    background: rgba(240, 240, 240, 0.40);

    span,
    a {
        font-weight: 300;
    }

    span {
        margin-right: 10px;
    }

    a {
        color: #4696E5;
        transition: .1s ease-in-out;

        &:hover {
            color: #111;
        }
    }
}
</style>