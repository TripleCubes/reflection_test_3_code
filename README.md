## Reflection, test 3 is still under development. Dont use it for anything important

[Syntax and stuffs](syntax_and_stuffs.md) \
[Online compiler](https://triplecubes.github.io/reflection_test_3) \
[Online compiler repository](https://github.com/TripleCubes/reflection_test_3)

[How to build](#how-to-build) \
[How to use](#how-to-use)

# How to build
Build the makefile generator
```
g++ makefilegen.cpp -o makefilegen
```
Generate the makefile
```
makefilegen
```
Build (Im on windows, use whatever make command you have on your system)
```
mingw32-make
```

## Build web
The web build is for the online compiler. You can check the online compiler's
repository to see how the web build is used

To build, just call `makefilegen` with the `web` argument
```
makefilegen web
```
The makefile generated will use emscripten to build for web instead

# How to use
Show the compiled lua in the console, no output file
```
test3 path_to_input_file
```
Compile the input file to an output file
```
test3 path_to_input_file path_to_output_file
```

# License

This repository is under the MIT License. You can find the license file at [LICENSE](LICENSE)
