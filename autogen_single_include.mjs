import fs from 'fs';
import path from 'path';

const entryPoint = 'lambda.hpp';

const patterns = {
	include: /\#include\s*\"[a-z0-9\-\_\.\\\/]+\"/gi,
	path: /(?<=\")[a-z0-9\-\_\.\\\/]+(?=\")/i,
	globalInclude: /#include\s*\<[a-z0-9\-\_]+\>/g,
	includeGuard: /\#ifndef\s*[_]{2}[a-z0-9\_\-]+\r?\n#define\s[_]{2}[a-z0-9\_\-]+\r?\n/i
};

const nukeHeaderIncludeGuards = (content) => {
	if (!patterns.includeGuard.test(content)) return content;
	let temp = content.replace(patterns.includeGuard, '');
	const closingGuard = temp.lastIndexOf('#endif');
	if (closingGuard == -1) throw new Error('Include guard removal task failed');
	return temp.slice(0, closingGuard);
}

const included = new Set();

const loadHeaderFileTree = (filepath) => {

	if (included.has(filepath)) return '';
	included.add(filepath);

	let headerContents = fs.readFileSync(filepath).toString();
	const headerIncludes = headerContents.match(patterns.include);

	headerIncludes?.forEach(include => {
		const resolvedPath = path.join(path.dirname(filepath), include.match(patterns.path)[0]);
		console.log(resolvedPath)
		headerContents = headerContents.replace(include, loadHeaderFileTree(resolvedPath));
	});
	
	return nukeHeaderIncludeGuards(headerContents);
};

let merged = loadHeaderFileTree(path.join(process.cwd(), entryPoint));

const globalIncludes = new Set();

merged.match(patterns.globalInclude)?.forEach(include => {
	globalIncludes.add(include);
	merged = merged.replaceAll(include, '');
});

merged = '\n' + Array.from(globalIncludes.values()).join('\n') + '\n' + merged;

//	nuke comments
merged = merged.replace(/\/\/.+/g, '');

let commentStartPos = 0;
let commentSearchPos = 0;

while ((commentStartPos = merged.indexOf('/*', commentSearchPos)) != -1 && commentSearchPos < merged.length) {

	let commentEndPos = merged.indexOf('*/', commentSearchPos);
	if (commentEndPos == -1) break;
	commentEndPos += 2;

	const comment = merged.substr(commentStartPos, commentEndPos - commentStartPos);
	if (!comment.length) break;

	const charArray = Array.from(comment);
	const count = {
		stars: charArray.filter(item => item == '*').length - 2,
		newlines: charArray.filter(item => item == '\n').length
	};

	const delta = (count.newlines + 1) / (count.stars + 1);

	if (delta > 2) {
		merged = merged.replace(comment, '');
	} else commentSearchPos = commentEndPos;
}

//	nuke repeating newlines
merged = merged.replace(/\r/g, '').replace(/\n{3,}/g, '\n\n');

//	add some stuff on top
const sfi = `
/*
	lambda - an HTTP and Websocket server library
	${new Date().getFullYear()} maddsua
	https://github.com/maddsua/lambda
*/

#ifndef __LIB_MADDSUA_LAMBDA__
#define __LIB_MADDSUA_LAMBDA__
${merged}
#endif
`;

fs.writeFileSync('lambda_sfi.hpp', sfi);
