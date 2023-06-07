import re

filename = 'db.c'
with open(filename, 'r') as f:
    content = f.read()

# pattern = r'^\w+\s+\**\s+(\w+)\((.*)\)\s*\{'
pattern = r'^\s*\w+\s+\**\s+(\w+)\((.*)\)\s*\{'
# pattern = r'^\s*(?:(?:struct|unsigned|signed|const|volatile)\s+)+([\w\s*]+)\**\s*(\w+)\([^\)]*\)\s*\{'
# pattern = r'^\s*(?:(?:struct|unsigned|signed|const|volatile)\s+)*([\w\s*]+)\**\s*(\w+)\([^\)]*\)\s*\{'
# 
# pattern = r'^\s*(?:(?:static)\s+)*([\w\s*]+)\**\s+(\w+)\((.*)\)\s*\{'
pattern = r'^\s*((struct\s+)?\w+\s*\**)\s+(\w+)\((.*)\)\s*\{'
# pattern = r'^\s*(?:(?:[\w\s*]+)\s+)*([\w\s*]+)\**\s+(\w+)\((.*)\)\s*\{'
# pattern = r'^\s*(?:(?:struct|union|enum|\w+\s*\**) +)?([\w\s*]+)\**\s+(\w+)\((.*)\)\s*\{'
# pattern = r'^\s*(?:(?:struct|union|enum|\w+\s*\**)\s+)?([\w\s*]+)\**\s+(\w+)\((.*)\)\s*\{'
pattern = r'(\w+)'

matches = re.findall(pattern, content, re.MULTILINE)

headername = filename + '.h'
with open(headername, 'w') as f:
    for match in matches:
        print(match)
        func_return_type, func_name, func_args = match[0], match[1], match[2]
        f.write(func_return_type.strip(' ') + ' ' + func_name + '(')
        pattern = r'(\w+\s*\**\s*\w+)'
        args = re.findall(pattern, func_args)

        for i, arg in enumerate(args):
            f.write(arg)
            if i != len(args) - 1:
                f.write(', ')
        f.write(');\n')
