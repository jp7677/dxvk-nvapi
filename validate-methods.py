import sys
import re

if sys.version_info[0] < 3:
    print("Method validation failed due to invalid Python interpreter. "
          "Please use Python 3 as your default Python interpreter.")
    sys.exit(1)

if len(sys.argv) < 3:
    print("Method validation failed due to invalid usage. "
          "Usage: python validate-methods.py <implementation1.cpp> [implementation2.cpp] <interface.cpp> <interface.h>")
    sys.exit(1)

expectedMethods = []
regex = re.compile("^ *NvAPI_Status +__cdecl +(\\w+)")
for i in range(1, len(sys.argv) - 2):
    with open(sys.argv[i]) as file:
        for line in file:
            result = regex.match(line)
            if result:
                expectedMethods.append(result.group(1))

fetch = False
foundMethods = []
regex = re.compile("^ *\\w+\\((\\w+)\\)")
with open(sys.argv[len(sys.argv) - 2]) as file:
    for line in file:
        if line.strip() == "/* End */":
            fetch = False
        if fetch:
            result = regex.match(line)
            if result:
                foundMethods.append(result.group(1))
        if line.strip() == "/* Start NVAPI methods */":
            fetch = True

availableMethods = []
regex = re.compile("^ *{ \\\"(\\w+)\\\", \\w+ },$")
with open(sys.argv[len(sys.argv) - 1]) as file:
    for line in file:
        result = regex.match(line)
        if result:
            availableMethods.append(result.group(1))

if len(expectedMethods) != len(foundMethods) or len(set(expectedMethods).intersection(foundMethods)) != len(
        expectedMethods):
    print("Method validation failed. "
          "Please make sure that all implemented NVAPI methods are listed in the `nvapi_QueryInterface` function.")
    sys.exit(1)

if not set(foundMethods).issubset(set(availableMethods)):
    print("Method validation failed. "
          "Please make sure that all implemented NVAPI methods "
          "correspond to available methods in the `nvapi_interface_table` struct.")
    sys.exit(1)

sys.exit(0)
