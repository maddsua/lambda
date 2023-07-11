<template>

    <footer>

		<div class="copyright">
			<div>2023 maddsua</div>
			<a href="https://github.com/maddsua/lambda/tree/main">Github</a>
		</div>

    </footer>

</template>

<style lang="scss" scoped>

footer {
	display: flex;
	padding: 1.5em;
	flex-direction: column;
	align-items: center;
	gap: 2rem;
	background: rgba(240, 240, 240, 0.40);
	backdrop-filter: blur(2px);

	.copyright {
		display: flex;
		justify-content: center;
		align-items: center;
		gap: 1.25rem;
		font-weight: 300;

		a {
			color: var(--app-accent);
			transition: .1s ease-in-out;

			&:hover {
				text-decoration: underline;
			}
		}
	}
}

</style>