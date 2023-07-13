import fs, { writeFileSync } from 'fs';
import { exit } from 'process';

let project = '';

try {
	project = fs.readFileSync('./lambda_version.hpp').toString();
} catch (error) {
	console.error('Run this script from project root, not wherever you currently are');
	exit(1);
}

const dataImported = {
	version: project.match(/\#define\s+LAMBDA_VERSION\s+\"[\d\.]+\"/)?.[0].match(/"[\d\.]+\"$/)?.[0]?.replace(/\"/g, ''),
	product: project.match(/\#define\s+LAMBDA_PRODUCT\s+\".+\"/)?.[0].match(/".+\"$/)?.[0]?.replace(/\"/g, '')
};

console.log(dataImported);

const resourceTemplate = fs.readFileSync('./res/template.rc').toString();

let outputcontent = resourceTemplate;

const replaces = [
	{
		variable: 'version_comma',
		data: dataImported.version.replace(/\./g, ',')
	},
	{
		variable: 'version',
		data: dataImported.version
	},
	{
		variable: 'released',
		data: new Date().getFullYear()
	},
	{
		variable: 'product',
		data: dataImported.product
	}
];

replaces.forEach(item => outputcontent = outputcontent.replace(new RegExp(`\\$\\{${item.variable}\\}`, 'g'), item.data));

writeFileSync('./res/lambda.rc', outputcontent);
