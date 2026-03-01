# Whanka

A Turing-complete esoteric programming language with passive-aggressive, sexually-charged, crime-themed English-like syntax.

Written in C. No dependencies. Single binary.

## Quick Start

```bash
# Build
./build.sh        # Linux / macOS / Termux
build.bat          # Windows

# Run
whanka hello.aids
```

## Hello World

```
enough foreplay

yell "hello world, you degenerate"

I came.
regards.
```

## FizzBuzz

```
enough foreplay

moan i is 1

go on a spree:
    moan mod3 is i leftover from 3
    moan mod5 is i leftover from 5

    if mod3 as hot as 0
        if mod5 as hot as 0
            yell "FizzBuzz"
        hard limit:
            yell "Fizz"
        boundaries respected.
    hard limit:
        if mod5 as hot as 0
            yell "Buzz"
        hard limit:
            scream i
        boundaries respected.
    boundaries respected.

    i gets i jacked up by 1
busted when i hits 101

I came.
regards.
```

## Building From Source

Requires any C11 compiler (gcc, clang, MSVC).

```bash
# Using make
make

# Using build script (also installs to PATH)
./build.sh

# Manual
gcc -Wall -Wextra -std=c11 -O2 -o whanka src/*.c -lm
```

## Documentation

See [DOCS.md](DOCS.md) for the full language reference.

## File Extension

All Whanka source files use the `.aids` extension.

## License

Do whatever you want with it.
