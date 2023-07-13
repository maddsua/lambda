import path from 'path';
import fs from 'fs';
import process from 'process';

if (!process.cwd().endsWith('lambda')) {
	console.error('Run this script from project root, not wherever you currently are');
	process.exit(0);
}

const findSourceFiles = (directory) => {
	const contents = [];
	const listDir = (dirpath) => fs.readdirSync(dirpath).forEach(item => {
		const location = path.join(dirpath, item);
		if (fs.statSync(location).isDirectory()) return listDir(location);
		else if (item.endsWith('.c')) contents.push(location);
	});
	listDir(directory);
	return contents.map(item => item.replace(/[\\\/]+/g, '/'));
}

const inputs = [
	{
		target: 'deps/libbrotli-static.a',
		srcdirs: [ 'deps/brotli/c/common/', 'deps/brotli/c/dec/', 'deps/brotli/c/enc/' ],
		objdir: 'deps/temp/brotli/'
	},
	{
		target: 'deps/libz-static.a',
		srcdirs: [ 'deps/zlib/' ],
		filter: /(example)|(test)|(contrib)/i,
		objdir: 'deps/temp/zlib/'
	}
];

const recepies = inputs.map(lib => {
	let files = lib.srcdirs.map(dir => findSourceFiles(dir)).flat();
	if (lib?.filter) files = files.filter(item => !lib.filter.test(item));

	const objname = 'OBJECTS_' + lib.target.replace(/^.+lib/, '').replace(/\.a$/, '').replaceAll('-', '_').toUpperCase();

	const objects = files.map(fpath => fpath.replace(/\.c$/, '.o')).join(' ');

	let block = `${objname} = ${objects}\n${lib.target}: $(${objname})`;
	block += `\n\tar rvs ${lib.target} $(${objname})\n`;

	return `\n${block}\n` + files.map(fpath => {
		const opath = fpath.replace(/\.c$/, '.o');
		return `${opath}: ${fpath}\n\t$(CC) -c ${fpath} -o ${opath} $(FLAGS)`;
	}).join('\n\n') + '\n';
});

const contentBefore = [
	'#	This is an automatically generated file',
	'#	Don\'t edit it directly - it may be overwritten',
	'#	Modify template.mk to add something on top, or generate_deps_makefile.mjs to change automatic imports',
	'\nCC = gcc',
	'FLAGS = -std=c11',
	'\n.PHONY: all all-before all-after action-custom',
	`\nlibs: ${inputs.map(item => item.target).join(' ')}`,
];

fs.writeFileSync('deps.mk', [contentBefore, recepies].flat().join('\n'))

console.log('Now, run "make -f deps.mk" to compile static dependency libs');
