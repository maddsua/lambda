
const sourceCodeFileRaw = 'https://github.com/bminor/glibc/raw/5176a830e70140cb3390c62b7d41f75dbbf33c7c/sysdeps/gnu/errlist.h';

const invocationAction = Deno.args.at(0);
if (!invocationAction) throw new Error("no action provided");

export const getErrorStrings = async () => {
	const headerFileText = await fetch(sourceCodeFileRaw).then(data => data.text());
	const errorConstants = headerFileText.match(/N\_\(\"[^\"]+\"\)/gi)?.map(item => item.slice(4, -2));
	if (!errorConstants) throw new Error('no strings matches');
	return errorConstants;
};

switch (invocationAction) {
	case 'longest': {

		const strings = await getErrorStrings();
		console.log('Parsed', strings.length, 'error strings');

		let longest = 0;

		for (const item of strings) {
			if (item.length > longest)
				longest = item.length;
		}

		console.log('Longest size:', longest);

	} break;

	case 'list': {

		console.log('Listring error strings...');

		const strings = await getErrorStrings();

		console.log(strings);

	} break;

	default: throw new Error('no action matched');
}
