import sys, re, getopt

# TODO: We might need support multiple implementation files at some point.

if len(sys.argv) != 3:
    print("Usage: python validate-methods.py <implementation.cpp> <interface.cpp>")
    sys.exit(1)

expectedMethods = []
regex = re.compile("^\ *NvAPI_Status\ +__cdecl\ +(\w+)")
with open(sys.argv[1]) as file:
    for line in file:
        result = regex.match(line)
        if result:
            expectedMethods.append(result.group(1))

fetch = False
foundMethods = []
regex = re.compile("^\ *\w+\((\w+)\)")
with open(sys.argv[2]) as file:
    for line in file:
        if line.strip() == "/* End */":
            fetch = False
        if fetch:
            result = regex.match(line)
            if result:
                foundMethods.append(result.group(1))
        if line.strip() == "/* Start NVAPI methods */":
            fetch = True

if len(expectedMethods) != len(foundMethods) or len(set(expectedMethods).intersection(foundMethods)) != len(expectedMethods):
    print("Method validation failed. Please make sure that all implemented NVAPI methods are also listed in the `nvapi_QueryInterface` method.")
    sys.exit(1)

sys.exit(0)
