<script lang="ts">
export default {
	methods: {
		proxy() {
			const proxyTo = (this.$refs.userinput as HTMLInputElement).value;
			fetch(`/api/proxy?target=${proxyTo}`).then(data => data.json()).then(data => {
				
				this.clearTable();

				this.populateTable('Proxy to', [data['proxy to']]);
				this.populateTable('Success', [data['success'] ? 'true' : 'false']),
				this.populateTable('Errors', [data['errors']]);

				if (!data['success']) return;

				this.populateTable('Response status', [data['response']]);
				this.populateTable('Headers', data['headers'].map((item: any) => `${item['name']} : ${item['value']}`));
				this.populateTable('body size', [data['body']]);

			}).catch(error => {
				this.populateTable('Request failed on:', [error]);
			});
		},
		clearTable() {
			const frame = this.$refs.output as HTMLElement;
			Array.from(frame.childNodes).forEach(node => node.remove());
		},
		populateTable(header: string, rows: string[]) {
			const frame = this.$refs.output as HTMLElement;
			const item = document.createElement('div');
				item.classList.add('block');
			frame.appendChild(item);

			const headerElem = document.createElement('h4');
				headerElem.textContent = header;
			item.appendChild(headerElem);

			const table = document.createElement('ul');
			item.appendChild(table);

			rows.forEach(row => {
				const litem = document.createElement('li');
				litem.textContent = row;
				table.appendChild(litem);
			});

		}
	}
}

</script>

<template>
	<main>
		<header>
			<h3>This demo fetches url and returns the result</h3>
		</header>

		<form @submit.prevent="proxy">
			<div>
				<input type="text" name="url" ref="userinput" placeholder="http://www.google.com">
				<button type="submit">Fetch</button>
			</div>
		</form>

		<div class="fetchresults" ref="output"></div>	
	</main>
</template>

<style lang="scss">

main {
	overflow-y: auto;
	max-height: 100vh;
	scrollbar-width: thin;
}

header {
	text-align: center;

	h3 {
		font-size: 1.5em;
	}
}

form {
	display: flex;
	flex-flow: column nowrap;
	
	& > div {
		display: flex;
		flex-flow: row nowrap;
		margin: 1em auto;
		width: 20em;
		max-width: 100%;

		input {
			flex-shrink: 1;
			flex-grow: 1;
			height: 2.5em;
			margin-right: 1em;
		}
	
		button {
			padding: 0.5em 1em;
		}
	}
}

.fetchresults {

	.block {
		margin: 1em auto;

		h4 {
			font-size: 1.25em;
		}

		ul {
			list-style: none;
			padding: 0;

			li {
				padding: 0.5em 1em;
				word-wrap: break-word;

				&:nth-child(2n) {
					background-color: #dddddd;
				}
			}
		}
	}
}
</style>