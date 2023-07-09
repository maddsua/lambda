<script lang="ts">
export default {
	data() {
		return {
			loadedFromVFS: false
		}
	},
	created() {
		fetch('/').then(data => {
			const storageHeader = data.headers.get('X-Storage');
			this.loadedFromVFS = storageHeader == 'lambda VFS';
		});
	}
}

</script>

<template>
	<main>
		<header>
			<h3 v-if="loadedFromVFS">This demo is loaded from lambda VFS</h3>
			<h3 v-else="loadedFromVFS">Uhh, it does not look like VFS is runnig</h3>
		</header>

		<div class="vfs">
			<img class="icon" src="../assets/targz.svg" alt="">
			<p>VFS is a way to mount tar/tar.gz archive as a filesystem, therefore reduce app loading time.</p>
			<p>Keep in mind that an entire mounted archive gets loaded into RAM, so it's not the best idea to use VFS for serving movies</p>
		</div>
	</main>
</template>

<style lang="scss">
header {
	text-align: center;

	h3 {
		font-size: 1.5em;
	}
}

.vfs {
	display: flex;
	flex-flow: column nowrap;
	margin: 2em;

	p {
		margin: 0.5em 0;
	}

	.icon {
		margin: auto;
		width: 10em;
		height: 10em;
		margin-bottom: 1em;
	}
}
</style>