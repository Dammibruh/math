# ami
A simple math interpreter written in C++
supports math constants and user defined identifiers

## expressions
regular maths expressions such as multiplication, addition, substraction,
division, power and the module operator.
identifiers can be builtin such as: pi, tau, eu(constant e), inf, nan
or user defined by `my_iden = 5;`.
Numbers can be negative/positive decimals/integers and can be written as
`1'000'000` and `1e5` for better readablity.
this project is still not yet stable, any issue or pr is appreciated

### building
```shell
$ git clone https://github.com/dammi-i/ami
$ mkdir build
$ cd build
$ cmake ..
$ make 
```

### todos
[ ] support user defined functions
[ ] fix ambiguous syntax such as `(-(-5))`
[ ] support for optional arguments
[ ] support for complex numbers
