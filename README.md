# Whanka

Whanka is a small, Turing-complete esoteric programming language with provocative English-like syntax.

The interpreter is written in C11, has no external dependencies, and builds as a single executable.

## Project Rename

This project now uses the executable name `Whanka` (uppercase `W`) across build scripts, examples, and usage output.

## Features

- Minimal C implementation
- Dynamic values (numbers and strings)
- Variables, expressions, conditionals, loops, and user-defined functions
- Simple file I/O and map-like operations
- `.aids` source file extension

## Repository Layout

- `src/` - interpreter source code
- `tests/` - sample and behavior test programs (`.aids`)
- `DOCS.md` - language reference and syntax guide
- `build.sh` / `build.bat` - cross-platform build scripts
- `Makefile` - build with `make`

## Build

### Linux / macOS / Termux

```bash
./build.sh
```

or:

```bash
make
```

### Windows

```bat
build.bat
```

### Manual Build

```bash
gcc -Wall -Wextra -std=c11 -O2 -o Whanka src/*.c -lm
```

## Run

```bash
Whanka tests/hello.aids
```

## Hello World

```text
enough foreplay

yell "hello world"

I came.
regards.
```

## Development Workflow

1. Build the interpreter.
2. Run any program from `tests/`.
3. Add or update `.aids` programs to verify language behavior.

Example:

```bash
make
./Whanka tests/fizzbuzz.aids
```

## Language Documentation

See [DOCS.md](DOCS.md) for:

- Core syntax
- Expressions and operators
- Control flow
- Functions
- Runtime errors and constraints

## License

Do whatever you want with it.
