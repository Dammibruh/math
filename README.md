# ami
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/5348f5a6a61746ef950e5e6e5291b562)](https://www.codacy.com/gh/dammi-i/ami/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=dammi-i/ami&amp;utm_campaign=Badge_Grade)  ![release](https://img.shields.io/badge/release-0.3.3--beta-brightgreen?style=flat-square)

An advanced math interpreter written in C++
supports math constants, functions and user defined identifiers, functions, sets, vectors, intervals

## expressions
regular maths expressions such as multiplication, addition, substraction,
division, power and the module operator.
identifiers can be builtin such as: pi, tau, e, inf, nan
or user defined by `my_iden = 5`. Function can also be builtin such as: `sqrt`, `min`, `max`...
or user defined by `func(x) -> x*2`.
Numbers can be negative/positive decimals/integers and can be written as
`1'000'000` and `1e5` for better readablity.
this project is still not yet stable, any issue or pr is appreciated

## dependencies:
- [libfmt](https://github.com/fmtlib/fmt)
- C++17

### building
```shell
$ git clone https://github.com/dammi-i/ami
$ mkdir build
$ cd build
$ cmake ..
$ make 
```
