const apiPath = '/api/db?entry=';

export default async (terminalwindow: HTMLElement, input: string) => {

	const termWrite = (text: string) => {
		const entry = document.createElement('div');
		entry.textContent = text;
		terminalwindow.appendChild(entry);
	};

	termWrite(`CMD: >> ${input}`);

	const invcmd = () => termWrite('Invalid command');
	const ohshit = () => termWrite('Internal error, is lambda doing fine?');

	switch (input) {
		case "clear":
			Array.from(terminalwindow.childNodes).forEach(node => node.remove());
		break;

		case "help":
			termWrite('Getting data: GET [key]');
			termWrite('Setting data: SET [key] [value]');
			termWrite('Deleting data: DELETE [key]');
		break;
	
		default: 
			const wssp = input.indexOf(' ');
			if (wssp == -1) {
				invcmd();
				return;
			}
			const content = input.slice(wssp + 1);

			if (input.toLowerCase().startsWith('get')) {

				try {
					const response = await (await fetch(apiPath + content)).json();

					if (response.success) {
						termWrite(response.data);
					} else {
						termWrite(`[ERROR] ${response.error}`);
					}
					
				} catch (error) {
					ohshit();
				}				

			} else if (input.toLowerCase().startsWith('set')) {
				const wssp2 = content.indexOf(' ');
				if (wssp2 == -1) {
					invcmd();
					return;
				}
				const entry = content.slice(0, wssp2 + 1);
				const data = content.slice(wssp2 + 1);

				try {
					const response = await (await fetch(apiPath + entry, { method: "POST", body: data })).json();

					if (response.success) {
						termWrite('saved');
					} else {
						termWrite(`[ERROR] ${response.error}`);
					}
					
				} catch (error) {
					ohshit();
				}

			} else if (input.toLowerCase().startsWith('delete')) {

				try {
					const response = await (await fetch(apiPath + content, { method: "DELETE" })).json();

					if (response.success) {
						termWrite('deleted');
					} else {
						termWrite(`[ERROR] ${response.error}`);
					}
					
				} catch (error) {
					ohshit();
				}	
			}

		break;
	}

	terminalwindow.scrollTop = terminalwindow.scrollHeight - terminalwindow.getBoundingClientRect().height;
};