<template>
    <div class="click-to-action">
        <button>Download</button>
        <button>Learn More</button>
    </div>
</template>

<style lang="scss" scoped>
.click-to-action {
    display: flex;
    justify-content: center;
    gap: 25px;
    margin-bottom: 75px;
}

button {
    padding: 14px 22px;
    width: 142px;
    height: 51px;
    border-radius: 10px;
    cursor: pointer;
    transition: .2s ease-in-out;
    font-size: 15px;

    &:first-child {
        color: white;
        background: #4696E5;
        border: 2px solid #4696E5;

        &:hover {
            color: #4696E5;
            background: inherit;
        }
    }

    &:last-child {
        color: #4696E5;
        background: inherit;
        border: 2px solid #4696E5;

        &:hover {
            color: white;
            background: #4696E5;
        }
    }
}
</style>