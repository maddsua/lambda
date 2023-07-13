import fs from 'fs';
import process from 'process';

if (!process.cwd().endsWith('lambda')) {
	console.error('Run this script from project root, not wherever you currently are');
	process.exit(0);
}

const lambdaVersionHeader = fs.readFileSync('./lambda_version.hpp').toString();

const dataImported = {
	version: lambdaVersionHeader.match(/\#define\s+LAMBDA_VERSION\s+\"[\d\.]+\"/)?.[0].match(/"[\d\.]+\"$/)?.[0]?.replace(/\"/g, ''),
	product: lambdaVersionHeader.match(/\#define\s+LAMBDA_PRODUCT\s+\".+\"/)?.[0].match(/".+\"$/)?.[0]?.replace(/\"/g, '')
};

let template = fs.readFileSync('./resources/lib/template.rc').toString();

[
	[ 'version_comma', dataImported.version.replace(/\./g, ',') ],
	[ 'version', dataImported.version ],
	[ 'released', new Date().getFullYear() ],
	[ 'product', dataImported.product ],

].forEach(item => template = template.replace(new RegExp(`\\$\\{${item.at(0)}\\}`, 'g'), item.at(1)));

fs.writeFileSync('./resources/lib/lambda.rc', template);

console.log('Updated dll info:', dataImported);
