import re
import datetime
import os
import sys


# check for correct directory
if not os.getcwd().endswith("res"):
	print("The paths for this script are relative to `res/` directory, so `cd res` and run this script over there")
	sys.exit(0)


# read contents of lambda_version.hpp
versionHeaderFile = open('../lambda_version.hpp', 'r')
versionHeaderContent = versionHeaderFile.read()
versionHeaderFile.close()


# replace values
insertVariables = [
	{  "var": "released", "data":  str(datetime.date.today().year) }
]


# lambda version
versionLine = re.search("\#define\s+LAMBDA_VERSION\s+\"[\d\.]+\"", versionHeaderContent)
versionMatch = re.search("\"[\d\.]+\"$", versionLine.group()).group()
version = re.sub(r'\"', '', versionMatch)

insertVariables.append({ "var": "version", "data":  version })
insertVariables.append({ "var": "version_comma", "data":  re.sub(r'\.', ',', version) })


# lambda name
productLine = re.search("\#define\s+LAMBDA_PRODUCT\s+\".+\"", versionHeaderContent)
productMatch = re.search("\".+\"$", productLine.group()).group()
productName = re.sub(r'\"', '', productMatch)

insertVariables.append({ "var": "product", "data":  productName })


# generate rc file from the template
templateFile = open('template.rc', 'r')
template = templateFile.read()
templateFile.close()

for item in insertVariables:
	template = re.sub(r"\$\{" + item["var"] + "\}", item["data"], template)

# write results
rcFile = open("lambda.rc", 'w')
rcFile.write(template)
rcFile.close()

print(">>> rc file generated")
