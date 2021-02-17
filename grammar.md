# grammar

## basic operations
basic operations can be expressed as
```js
(number) (operator) (number)

number = 1'00'00 or 1e3 or -number
operator = + or - or / or % or * or ^ 
// there is also assignement operators 
x += 1
x -= 1
x /= 1
x *= 1
x %= 1
x ^= 2
```
### logical operations
there is 2 logical operators `and` and `or` there is also Booleans, `true` or `false`
```js
true and true
false or true
false and true
true or false
not true
0 or 1
1 or 0
1 and 1
0 and 0
...
```

### comparaisons
there is also comparaison operators such as `==`, `<=`, `<`, `>`, `>=`, `!=`
```js
0 == 0
2 - 1 == 1
1 == 2 -1
5 != -4
0 <= 1
1 < 1
6 > 8
6 >= 6
```

## identifiers
idenifiers can be builtin or user defined
```js
builtins = pi or tau or e or inf or nan
or user defined by
idenifier = expression
x = 5
x = true
y = pi
```

## functions
functions can also be builtin such as
```js
log(1)
log2(1)
log10(2)
sqrt(25)
abs(-1)
min(0, 1)
max(0, 0)
gcd(0, 1)
lcm(0, 1)
cos(pi)
sin(pi)
tan(pi)
cosh(pi)
sinh(pi)
tanh(pi)
```

or user defined by
```js
func(x) -> x*2
```

## intervals
ami also supports intervals (union and intersection)
```js
5 in [0; 5]
0 in ]-inf; 0[ union ]0; inf[
10 in ]-inf; 0[ intersection ]0; inf[
```

## sets
there is also sets
```js
x = {5, 10}
y = {20, 4}
x union y
x intersection y
x == y
x != y
x - y
```
for sets only '!=', '==' and '-' are supported

## vectors
vectors are also supported
```js
x = [2, 1] // 2d vector
y = [3, 6, 1] // 3d vector
x * y // error cannot perform operations on different sizes
x * [9, 3] // performs dot product `ax.bx + ay.by`
x * 2 // multiply coordinates by 2
```

more soon™
