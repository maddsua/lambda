import re
import glob
import os
import sys

if not os.getcwd().endswith("deps"):
	print("The paths for this script are relative to `deps/` directory, so `cd deps` and run this script over there")
	sys.exit(0)

warningMsg = [
	'#	This is an automatically generated file',
	'#	Don\'t edit it directly - it may be overwritten by gen.py',
	'#	Modify template.mk to add something on top, or gen.py to change automatic imports'
]

sources = [
	{
		"target": "libbrotli-static.a",
		"patterns": [
			"brotli/c/common/*.c",
			"brotli/c/dec/*.c",
			"brotli/c/enc/*.c",
		],
		"out": "temp/brotli/"
	},
	{
		"target": "libz-static.a",
		"patterns": [
			"zlib/*.c"
		],
		"out": "temp/zlib/"
	}
]

makefileInsertContent = []

# find sources and normalize paths
for source in sources:

	objectFiles = []
	objectsSubtasks = []

	for pattern in source["patterns"]:

		for fileItem in glob.glob(pattern):

			filePathNormalized = re.sub(r'\.\/', '', re.sub(r'[\\\/]+', '/', fileItem))

			sourceObject = source["out"] + re.sub(r'.+[\/]', '', re.sub(r'\.c$', '.o', filePathNormalized))
			objectFiles.append(sourceObject)

			subtask = sourceObject + ": " + filePathNormalized
			subtask += "\n\t" + "$(CC) -c " + filePathNormalized + " -o " + sourceObject + " $(FLAGS)\n"
			objectsSubtasks.append(subtask)

	sourceMakefileEntry = source["target"] + ": " + ' '.join(objectFiles) + "\n"
	sourceMakefileEntry += "\tar rvs " + source["target"] + " " + source["out"] + "*.o\n"

	makefileInsertContent.append(sourceMakefileEntry + "\n" + '\n'.join(objectsSubtasks))

# put all the stuff into makefile
with open('template.mk', 'r') as makefileTemplate:
	makefileTemplate = makefileTemplate.read()
	makefileTemplate = re.sub(r'\$\{sources\}', '\n'.join(makefileInsertContent), makefileTemplate)
	makefileTemplate = re.sub(r'\$\{warningmsg\}', '\n'.join(warningMsg), makefileTemplate)
	
	makefileResult = open("makefile", 'w')
	makefileResult.write(makefileTemplate)
	makefileResult.close()

print(">>> makefile generated")
