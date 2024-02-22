# Brua2 - a dynamically typed interpreted programming language
## Data Types
There are only 8 data types: 
- bool
- char (signed, 8-bit)
- int (signed, 64-bit)
- real (double precision)
- pointer
- string
- dict (dictionary)
- func (function)

Each data type except for func supports literal values.
Most types (`bool`, `char`, `int`, `real` and `string`) use the same rules as in C++. For `pointer` type, the only recongized literal is `NULL`. For `dict` type, the only recognized literal is an empty dictionary `{}`. `func` type doesn't have literals.

Some objects may not be referenceable (for example - literals), therefore cannot be assigned a value.

### bool
`bool` values can be `true` or `false`. They are used in statements.
### char
`char` values are ASCII characters. They are returned when `string` values are indexed.
### int
`int` values represent signed 64-bit numbers. `int` is one of the only two types (the other being `real`) which support arithmetic operations. `string` indexation can only be done using `int` values.
### real
`real`  values represent double precision floating point numbers. `real` supports arithmetic operations.
### pointer
`pointer` values point to other objects. passing `pointer` values is the only way to modify variables inside of functions.
### string
`string` values represent sequences of `char` values. Each character can be accessed, but cannot be modified. However, prefix and suffix operations are supported.
### dict
`dict` is a very universal data type which contains pairs of keys and values inside of it. Each key object is assigned a value object, and no key can have two values assigned to it. Each value can be accessed by a resprectful key. **Keys are compared by value**, therefore different objects with the same value are treated as the same key inside of the `dict`. Each key and value object stored in a `dict` is a copy of its original self, therefore `pointer` type may be used to store references to values outside of the dict. 
### func
`func` stores parts of code that can be executed multiple times. Each function can access arguments that were passed during the function call. Each function may return at least one object. **Some functions may be implemented inside of the language engine**.



## Namespaces
Namespace is a virtual structure that stores information about objects created during its lifetime. It stores variables with their names, as well as many other things which are required for the language to work.

Each namespace when created may or may not be able to access its parent namespace. If a value has to be located in the namespace, it may start a search in the parent namespace if it can access it. It may repeat several times until the value is found. If the required value is not found, a runtime error happens. **Each namespace may access the first namespace**, which has its own name: namespace 0. Namespace 0 is the namespaces where all your code is executed in. Therefore, some variables (which are called global variables) created inside of it may be accessed in any part of the code.

Since namespaces may look for variables in parent namespaces (and namespace 0), variable shadowing is not supported.

Variables are objects with names. Variables are created in the most recent namespace. However, as noted above, a variable will not be created if the name given to it can be found in the accessible namespaces. Instead, the already present variable will be used.

When a namespace gets destroyed, all the objects created during its lifetime are destroyed as well.

Functions heavily rely on namespaces. If a function wants to access its arguments, it looks for them in most recent namespace. When a function is called, a new namespace is created, where all the arguments are put, and the called function runs in that namespace. **Note that only copies of the arguments are put in the namespace**.

## Syntax
Each code structure must be inside of a pair of brackets. However, **names and literals MUST be left without brackets**, or a block of invalid instructions will be created**

## Statements and expressions
If a name is recognized, it is almost always (the only exclusion being `set` statement) replaced with a value assigned to it. If such name is not found in the namespace, a runtime error happens.
### `(set A B)`
- A must be a referenceable value
- B must be a value
- The result of execution is not a value

Sets value of A being equal to B. It is implemented as replacing A with a copy of B. 

If A is a name, then a variable is modified. If a variable with such name doesn't exist, a new one is created in the most recent namespace, and gets assiged value of B.

### `(while A B)`
- `A` must be of type `bool`
- `B` is treated as a code part
- The result of execution is not a value

As long as  `A` is `true`, `B` is executed in a new namespace which is destroyed each time when `B` completes.

### `(for A B C D)`
- `A` is treated as a code part
- `B` must be of type `bool`
- `C` is treated as a code part
- `D` is treated as a code part
- The result of execution is not a value

Executes `A` once.
Then, as long as  `B` is `true`, `D` is executed in a new namespace which is destroyed each time when `D` completes.
At the end of each `D` execution, `C` gets executed,

### `(repeat A B)`
- `A` is treated as a code part
- `B` must be of type `bool`
- The result of execution is not a value

Executes `A` until `B` becomes `true`. `A` is executed in a new namespace, which gets destroyed each time when `A` completes.

### `(if A B C)`
- `A` must be of type `bool`
- `B` is treated as a code part
- `C` is treated as a code part
- The result of execution is not a value

If `A` is `true`, executes `B` in a new namespace, which gets destroyed at the end. Otherwise, execute `C` in a new namespace, which gets destroyed at the end.

### `(continue)`
When inside of a code block, stops executing any further code in the block. When inside of a loop body, stops executing any further code in the body, and begins a new loop execution. Has no effect in other places.
- The result of execution is not a value

### `(break)`
When inside of a code block, stops executing any further code in the block. When inside of a loop body, stops executing any further code in the body, and stops the whole loop. Has no effect in other places.
- The result of execution is not a value

### `(return A)`
- If present, A must be a value
- The result of execution is not a value

If inside of a function body, stops executing any further code in the body, and completes the function call. If a value `A` is present, returns it as the result of the function call. Has no effect in other places.

### `(func A)`
- `A` is treated as a code part
- The result of execution is an unreferenceable object of type `func`

Creates a new object (function) of type `func` which will execute code `A`.

### `(arg A)`
- `A` must be of type `int`
- The result of execution is a referenceable object

Returns a function argument with index `A` which was passed during the function call. Arguments are numbered in first-to-last order.

### `(call A *B)`
- `A` must be of type `func`
- `*B` represent zero or more arguments. Each argument must be a value.
- The result of execution is an unreferenceable object or not a value, depending on if the function returns an object

Calls function `A` with arguments `*B`. Creates a new namespace, where arguments `*B` are put. Then, function `A` is executed. After that, the namespace gets destroyed, and the value that function returned is taken as a result.

### `(bool A)`
- `A` must be a value
- The result of execution is an unreferenceable object of type `bool`

Returns an `bool` value which represents boolean value of `A`. Values that are `false` are:
1. `false` (type `bool`)
2. `0` (type `char`, `int`, and `real`)
3. `NULL` (type `pointer`)
4. `""` (type `string`)
5. `{}` (type `dict`)

Every other value is treated as `true`

### `(char A)`
- `A` must be a value of type `bool`, `char`, or `int`
- The result of execution is an unreferenceable object of type `char`

Returns a `char` object with value of `A` (via standard C casts)

### `(int A)`
- `A` must be a value of type `bool`, `char`, `int`, `real`, or `string`
- The result of execution is an unreferenceable object of type `int`

Returns an `int` object with value of `A` (via standard C casts). If `A` is `string`, then it parses contents of `A` and tries to recognize an integer.

### `(real A)`
- `A` must be a value of type `bool`, `char`, `int`, `real`, or `string`
- The result of execution is an unreferenceable object of type `real`

Returns a `real` object with value of `A` (via standard C casts). If `A` is `string`, then it parses contents of `A` and tries to recognize a floating point number.

### `(string A)`
- `A` must be a value
- The result of execution is an unreferenceable object of type `string`

Returns textual representation of `A`. If `A` is a function, returns `"function"`

### `(deref A)`
- `A` must be of type `pointer`, and cannot be `NULL`
- The result of execution is a referenceable object.

Returns the original object that is pointed by `A`.

### `(ref A)`
- `A` must be a referenceable value
- The result of execution is an unreferenceable object of type `pointer`.

Returns a pointer pointing to `A`

### `(inv A)`
- `A` must be of type `int`
- The result of execution is an unreferenceable object of type `int`

Returns a binary inversed copy of `A`

### `(not A)`
- `A` must be of type `bool`
- The result of execution is an unreferenceable object of type `nool`

Returns a logicaly negated copy of `A`

### `(neg A)`
- `A` must be of type `int` or `real`
- The result of execution is an unreferenceable object of the same type as `A`

Returns a negated copy of `A`

### `(mult A B)`
- `A` must be of type `int` or `real`
- `B` must be of type `int` or `real`
- The result of execution is an unreferenceable object of the smallest fitting type based on `A` and `B`

Returns a multiplication result of `A` and `B`

### `(div A B)`
- `A` must be of type `int` or `real`
- `B` must be of type `int` or `real`
- The result of execution is an unreferenceable object of the smallest fitting type based on `A` and `B`

Returns a division result of `A` and `B`

### `(rem A B)`
- `A` must be of type `int` or `real`
- `B` must be of type `int` or `real`
- The result of execution is an unreferenceable object of the smallest fitting type based on `A` and `B`

Returns a division remainder result of `A` and `B`

### `(add A B)`
- `A` must be of type `int` or `real`
- `B` must be of type `int` or `real`
- The result of execution is an unreferenceable object of the smallest fitting type based on `A` and `B`

Returns an addition result of `A` and `B`

### `(sub A B)`
- `A` must be of type `int` or `real`
- `B` must be of type `int` or `real`
- The result of execution is an unreferenceable object of the smallest fitting type based on `A` and `B`

Returns a subtraction result of `A` and `B`

### `(shl A B)`
- `A` must be of type `int`
- `B` must be of type `int`
- The result of execution is an unreferenceable object of type `int`

Returns a binary left shift result of `A` and `B`

### `(shr A B)`
- `A` must be of type `int`
- `B` must be of type `int`
- The result of execution is an unreferenceable object of type `int`

Returns a binary right shift result of `A` and `B`

### `(lt A B)`
- `A` must be of type `int` or `real`
- `B` must be of type `int` or `real`
- The result of execution is an unreferenceable object of type `bool`

Returns a less-then result of `A` and `B`

### `(gt A B)`
- `A` must be of type `int` or `real`
- `B` must be of type `int` or `real`
- The result of execution is an unreferenceable object of type `bool`

Returns a greater-then result of `A` and `B`

### `(le A B)`
- `A` must be of type `int` or `real`
- `B` must be of type `int` or `real`
- The result of execution is an unreferenceable object of type `bool`

Returns a less-then-or-equal-to result of `A` and `B`

### `(ge A B)`
- `A` must be of type `int` or `real`
- `B` must be of type `int` or `real`
- The result of execution is an unreferenceable object of type `bool`

Returns a greater-then-or-equal-to result of `A` and `B`

### `(eq A B)`
- `A` must be a value
- `B` must be a value
- The result of execution is an unreferenceable object of type `bool`

Returns `true` if values of `A` and `B` are equal, otherwise returns `false`

### `(neq A B)`
- `A` must be a value
- `B` must be a value
- The result of execution is an unreferenceable object of type `bool`

Returns `true` if values of `A` and `B` are not equal, otherwise returns `false`

### `(and A B)`
- `A` must be of type `int`
- `B` must be of type `int`
- The result of execution is an unreferenceable object of type `int`

Returns binary and of `A` and `B`

### `(xor A B)`
- `A` must be of type `int`
- `B` must be of type `int`
- The result of execution is an unreferenceable object of type `int`

Returns binary xor of `A` and `B`

### `(or A B)`
- `A` must be of type `int`
- `B` must be of type `int`
- The result of execution is an unreferenceable object of type `int`

Returns binary xor of `A` or `B`

### `(conj A B)`
- `A` must be of type `bool`
- `B` must be of type `bool`
- The result of execution is an unreferenceable object of type `bool`

Returns logical and of `A` or `B`

### `(disj A B)`
- `A` must be of type `bool`
- `B` must be of type `bool`
- The result of execution is an unreferenceable object of type `bool`

Returns logical or of `A` or `B`

### `([d] A B)`
- `A` must be a referenceable object of type `dict`
- `B` must be a value
- The result of execution is a referenceable object

Returns value that is assigned key `B` in dictionary `A`. If not found, runtime error happens.

### `([dn] A)`
- `A` must be a referenceable object of type `dict`
- The result of execution is an unreferenceable object of type `int`

Returns number of elements in dictionary `A`

### `([d?] A B)`
- `A` must be a referenceable object of type `dict`
- `B` must be a value
- The result of execution is an unreferenceable object of type `bool`

Returns `true` is key `B` is present in the dictionary `A`, otherwise returns `false`

### `([d+] A B C)`
- `A` must be a referenceable object of type `dict`
- `B` must be a value
- `C` must be a value
- The result of execution is not a value

Inserts pair (`B`, `C`) into dictionary `A`. If a pair with the same key already exists, it gets replaced by the new one.

### `([d-] A B)`
- `A` must be a referenceable object of type `dict`
- `B` must be a value
- The result of execution is not a value

Removes pair with key `B` from dictionary `A`. If such pair is not found, nothing happens.

### `([dk] A)`
- `A` must be a referenceable object of type `dict`
- The result of execution is an unreferenceable value of type `dict`

Returns a dictionary which has all the keys that are present in `A`. They are given their keys, which are numbers from 0 to the size of `A` minus 1.

### `([dv] A)`
- `A` must be a referenceable object of type `dict`
- The result of execution is an unreferenceable value of type `dict`

Returns a dictionary which has all the values that are stored in `A`. They are given their keys, which are numbers from 0 to the size of `A` minus 1.

### `([dc] A)`
- `A` must be a referenceable object of type `dict`
- The result of execution is not a value

Removes all contents from dictionary `A`

### `([s] A B)`
- `A` must be a referenceable object of type `string`
- `B` must be a value of type `int`
- The result of execution is an unreferenceable object of type `char`

Returns character in string `A` at index `B`. Note, that the returned character is a copy of the original one.

### `([s+] A B)`
- `A` must be a referenceable object of type `string`
- `A` must be a value of type `string`
- The result of execution is not a value

Adds string `B` to the end of string `A`, modifying `A`

### `([+s] A B)`
- `A` must be a referenceable object of type `string`
- `A` must be a value of type `string`
- The result of execution is not a value

Adds string `B` to the beginning of string `A`, modifying `A`

### `([s-] A B)`
- `A` must be a referenceable object of type `string`
- `A` must be a value of type `int`
- The result of execution is not a value

Removes `B` characters from the end of string `A`

### `([-s] A B)`
- `A` must be a referenceable object of type `string`
- `A` must be a value of type `int`
- The result of execution is not a value

Removes `B` characters from the beginning of string `A`

### `(*A)`
- `*A` is treated as zero or more instructions
- The result of execution is not a value

Creates a block of code

## Builtin functions
Builtin functions can be easily added in `src/predefined.cpp` file.
### print
- takes zero or more arguments

Prints all its arguments to the standard output stream. Doesn't add any spaces or newline character.

### println
- takes zero or more arguments

Prints all its arguments to the standard output stream. Doesn't add any spaces, but adds a newline character at the end.

### gettimems
- takes zero arguments

Returns the amout of time passed since the beginning of the epoch. Returns time in milliseconds.

### sleep
- takes one `int` argument

Sleeps for the given amount of milliseconds.

### exit
- takes one `int` argument

Exits with return code being its argument.

### assert
- takes one `bool` argument an one `string` argument

Checks if the first given argument is `true`. If it's `false`, a runtime error happens and the `string` message is printed.

### randint
- takes one `int` argument

Returns a random non-negative `int` number not greater than it's argument.

### sin
- takes one `int` or `real` argument

Returns sin of the given value.

### cos
- takes one `int` or `real` argument

Returns cos of the given value.

### abs
- takes one `int` or `real` argument

Returns absolute of the given value.

### clearterminal
- takes zero arguments

Clears all text from the terminal. *Added only for a ping-ping game*

### getch
- takes zero arguments

Returns a character read without blocking the standard input stream. *Added only for a ping-ping game*


## Tests, Programs
In `tests` directory I prepared some programs that are supposed to check if the language works correctly. I've also included one program that checks the speed of some instructions. All tests may be run with a single command: `bash runtests.sh`

In `programs` directory I prepared a single terminal-based game "ping-pong", which should run in any modern terminal of enough size.

## Running code
To run the language on a code file, simple pass it to the `bash run.sh` command as an argument.

## Hello world!
    (call println "Hello world!")