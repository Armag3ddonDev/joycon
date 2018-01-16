[![Build status](https://ci.appveyor.com/api/projects/status/2khd706ci4l2cgio?svg=true)](https://ci.appveyor.com/project/al3xst/joycon) [![Build Status](https://travis-ci.org/Armag3ddonDev/joycon.svg?branch=master)](https://travis-ci.org/Armag3ddonDev/joycon)

# joycon

This project is focused around creating a driver for the Joy-Cons from the Nintendo Switch. A driver working under Windows and Unix.

We want to support most functionalities from the Joy-Cons, this includes all buttons, the analog sticks, Rumble, NFC and IR.

At the moment we are concentrating on providing a safe and easy-to-use interface for all available functions of the Joy-Cons. Currently we are mapping the bluetooth communication.


Our implementation is based on the discoveries from https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering. Thanks to dekuNukem and everyone who contributed to the reverse engineering of the Joy-Cons!

We are using the HIDAPI (https://github.com/signal11/hidapi) library to communicate with the Joy-Cons (cross platform!)

### Requirements
- A C++14 Compiler (gcc 5.0 and later) (MSVC: It may work with 2015, but we are using the 2017 edition)
- HIDAPI library (Linux: Most distributions provide the libhidapi-dev package)
- cmake (or MSVC for Windows)
