<script lang="ts" setup>
function isElementInViewport(el: Element): boolean {
    const rect = el.getBoundingClientRect();
    return (
        rect.top >= 0 &&
        rect.left >= 0 &&
        rect.bottom <= (window.innerHeight || document.documentElement.clientHeight) &&
        rect.right <= (window.innerWidth || document.documentElement.clientWidth)
    );
}

function handleScroll(): void {
    const console = document.querySelectorAll('.console');
    console.forEach((element) => {
        if (isElementInViewport(element)) {
            element.classList.add('visible');
        }
    });
}

window.addEventListener('scroll', handleScroll);
</script>

<template>
    <section class="console hidden">
        <h2>
            This demo uses websockets to manipulate KV storage
        </h2>
        <span>This is an interactive console. Type help to learn commands!</span>

        <div class="demo">
            <div class="demo-title">
                <p>LAMBDA >> KV SET user_id 001</p>
                <p>LAMBDA >> KV GET user_id</p>
                <p>LAMBDA >> <span>“001”</span></p>
            </div>

            <div class="demo-footer">
                <p>LAMBDA >> KV SET test_record 123</p>
                <button>Send</button>
            </div>
        </div>
    </section>
</template>

<style lang="scss" scoped>
.hidden {
    opacity: 0;
    transition: opacity 1s;
}

.visible {
    opacity: 1;
}

.console {
    margin: 75px 0 50px 0;
}

h2 {
    color: #4696E5;
    font-size: 30px;
    font-weight: 500;
    margin-bottom: 10px;
}

span {
    font-size: 18px;
    font-weight: 300;
    color: #141414;
}

.demo {
    margin-top: 50px;
    width: 720px;
    max-width: 100%;
    height: 380px;
    background: #0F172A;
    border-radius: 25px;
    display: flex;
    flex-direction: column;
    justify-content: space-between;

    &-title,
    &-footer {
        font-size: 15px;
        font-weight: 300;
        line-height: 2;
        padding: 20px 0 0 20px;
        color: #fff;
    }

    &-title {
        span {
            font-size: 15px;
            font-weight: 300;
            color: #19DB95;
        }
    }

    &-footer {
        display: flex;
        justify-content: space-between;
        align-items: flex-end;
        margin-bottom: 20px;

        button {
            margin-right: 20px;
            padding: 6px 20px;
            border-radius: 10px;
            transition: .1s ease-in-out;
            color: white;
            background: #4696E5;
            border: 2px solid #4696E5;
            cursor: pointer;

            &:hover {
                color: #4696E5;
                background: inherit;
            }
        }
    }
}
</style>