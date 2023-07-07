import { globSync } from "glob";
import fs from 'fs';
import { exit } from "process";

const transformSources = [

	{
		target: 'libbrotli-static.a',
		patterns: [
			'./brotli/c/common/*.c',
			'./brotli/c/dec/*.c',
			'./brotli/c/enc/*.c',
		],
		out: './temp/brotli/'
	},
	{
		target: 'libz-static.a',
		patterns: [
			'./zlib/*.c'
		],
		out: './temp/zlib/'
	}

].map(source => {

	if (!fs.existsSync(source.out)) {
		fs.mkdirSync(source.out, { recursive: true });
	}

	const filesFound = [];
	source.patterns.map(pattern => globSync(pattern.replace(/[\\\/]+/g, '/'), { nodir: true }).forEach(item => filesFound.push(item.replace(/\\+/g, '/'))));

	if (!filesFound.length) {
		console.error("No files imported for " + source.target);
		exit(1);
	}

	const objects = filesFound.map(item => `${source.out}/${item.replace(/.+[\/]/, '').replace(/\.c$/, '.o')}`.replace(/[\\\/]+/g, '/'));

	const pairs = filesFound.map(item => ({
		source: item,
		object: `${source.out}/${item.replace(/.+[\/]/, '').replace(/\.c$/, '.o')}`.replace(/[\\\/]+/g, '/')
	}));

	return {
		target: source.target,
		objects: objects,
		subtargets: pairs,
		outdir: source.out.replace(/[\\/]$/, '')
	};
});

const tasks = transformSources.map(item => {

	const objects = item.subtargets.map(subtarget =>
		`${subtarget.object}: ${subtarget.source}\n\t$(CC) -c ${subtarget.source} -o ${subtarget.object} $(FLAGS)`
	);

	const taskline = `${item.target}: ${item.objects.join(' ')}\n\tar rvs ${item.target} ${item.outdir}/*.o`;

	return taskline + '\n\n' + objects.join('\n\n');
}).join('\n\n\n');

const templateContent = fs.readFileSync('./template.mk').toString();

const warningMsg = [
	'#	This is an automatically generated file',
	'#	Don\'t edit it directly - it may be overwritten by gen.js',
	'#	Modify template.mk to add something on top, or gen.js to change automatic imports',
	'#	Sorry, honey, no python in this project =('
];

const output = templateContent.replace('${warningmsg}', warningMsg.join('\n')).replace('${sources}', tasks);

fs.writeFileSync('./makefile', output);
