# Cepheid

A hobby programming language project by someone that likes most of C++.

## Quick Start
 - `git clone https://github.com/smarky55/cepheid.git && cd cepheid`
 - `cmake -B ./build -S .`
 - Open cepheid.sln in visual studio
 - Build
 - `./build/compiler/Debug/cepheid.exe ./examples/hello_world/src/main.cep ./examples/hello_world/hello.exe`
 - `./examples/hello_world/hello.exe`

## Prerequisites
The current incarnation of Cepheid only supports x86-64 and Windows. Support for other architectures and platforms is planned for the future.
 - [nasm](https://www.nasm.us/index.php)
 - Visual Studio 2022 (or just the build tools)

## Acknowledgements
This project was inspired by Pixeld's [Creating a Compiler](https://www.youtube.com/playlist?list=PLUDlas_Zy_qC7c5tCgTMYq2idyyT241qs) series of videos with the initial implementation closely following the patterns from the videos. The repo for his implementaiton can be found [here](https://github.com/orosmatthew/hydrogen-cpp).

Robert Nystrom's [Crafting Interpreters](https://craftinginterpreters.com/) has also been an excellent resource in understanding how to structure things on the parsing side.