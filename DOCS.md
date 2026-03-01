# Whanka Language Reference

## Program Structure

Every Whanka program must begin with `enough foreplay` and end with `I came.` followed by `regards.`

```
enough foreplay

// your code here

I came.
regards.
```

That's the law. No negotiation.

---

## Comments

Lines starting with `//` are comments. They're ignored entirely.

```
// this line does nothing, just like you
```

---

## Variables

### Declaring a variable

```
moan x is 5
moan name is "Rick"
stash secret with 42
```

`moan` and `stash ... with` both create or update a variable.

### Reassigning a variable

```
x gets 10
x gets x jacked up by 1
```

`gets` assigns a new value to an existing variable.

### Rules

- Variable names are alphanumeric plus underscores: `a-z`, `A-Z`, `0-9`, `_`
- Variables are dynamically typed (number or string)
- Using an undeclared variable triggers: **WHAT THE FUCK?!**

---

## Data Types

There are two types:

| Type | Examples |
|---|---|
| Number | `0`, `42`, `3.14`, `-7` |
| String | `"hello"`, `"FizzBuzz"` |

All numbers are double-precision floating point. Integers display without decimals (`42` not `42.000000`).

---

## Output

### yell

Prints a value followed by a newline.

```
yell "hello world"
yell 42
yell x
```

### scream

Same as `yell`. Prints value + newline.

```
scream x
scream "I'M SCREAMING"
```

Both work with strings, numbers, variables, and expressions.

---

## Input

### seduce the answer out of

Reads a line from stdin and stores it in a variable. If the input looks like a number, it's stored as a number. Otherwise it's a string.

```
seduce the answer out of name
yell name
```

---

## Arithmetic

All arithmetic operates on numbers. Using arithmetic on non-numbers triggers a type error (except `jacked up by` which concatenates strings).

| Operation | Syntax | Example |
|---|---|---|
| Addition | `X jacked up by Y` | `x jacked up by 5` |
| Subtraction | `X cut down by Y` | `x cut down by 3` |
| Multiplication | `X blown up by Y` | `x blown up by 2` |
| Division | `X split with Y` | `x split with 4` |
| Modulo | `X leftover from Y` | `x leftover from 3` |

### String concatenation

`jacked up by` also concatenates when either side is a string:

```
moan greeting is "hello " jacked up by "world"
yell greeting
// prints: hello world
```

### Division by zero

Dividing or modding by zero triggers: **YOU LIMP DICK!**

### Using arithmetic in assignment

```
moan x is 10
moan y is x jacked up by 5
x gets x blown up by 2
```

---

## Comparisons

Comparisons return `1` (true) or `0` (false).

| Operation | Syntax | Meaning |
|---|---|---|
| Greater than | `X hotter than Y` | `X > Y` |
| Less than | `X colder than Y` | `X < Y` |
| Equals | `X as hot as Y` | `X == Y` |
| Is zero | `X is frigid` | `X == 0` |

---

## Conditionals

### If statement

```
if x hotter than 10
    yell "big number"
boundaries respected.
```

### If / else

```
if x as hot as 0
    yell "zero"
hard limit:
    yell "not zero"
boundaries respected.
```

### Nested

```
if x hotter than 0
    if x colder than 100
        yell "between 1 and 99"
    boundaries respected.
boundaries respected.
```

### How it works

- The condition is any expression or comparison
- Truthy: any non-zero number, any non-empty string
- Falsy: `0`, empty string
- `hard limit:` begins the else branch
- `boundaries respected.` closes the if block (required)

---

## Loops

### Basic loop

```
moan i is 0

go on a spree:
    yell i
    i gets i jacked up by 1
busted when i hits 10
```

This prints 0 through 9.

### How it works

- `go on a spree:` starts the loop
- The body executes repeatedly
- `busted when VAR hits LIMIT` checks the break condition **after** each iteration
- The loop stops when VAR >= LIMIT

### Infinite loop protection

If a loop runs more than 1,000,000 iterations it triggers: **BLUE BALLS!**

---

## Functions

### Defining a function

No parameters:

```
my kink is greet
    yell "hello there"
safe word.
```

With parameters:

```
my kink is add and it takes a, b
    finish with a jacked up by b
safe word.
```

### Calling a function

No arguments:

```
get freaky with greet
```

With arguments:

```
moan result is get freaky with add using 3, 4
yell result
// prints: 7
```

### Return values

`finish with EXPR` returns a value from a function.

```
my kink is double and it takes x
    finish with x blown up by 2
safe word.

moan result is get freaky with double using 21
yell result
// prints: 42
```

### Rules

- Functions must be defined before they're called
- Parameter count must match argument count or you get: **COCKBLOCKED!**
- Calling a function that doesn't exist triggers: **COCKBLOCKED!**
- Functions have their own scope but can read parent variables

---

## Flavor Text

These tokens are purely decorative and get ignored by the parser. Sprinkle them in for vibes:

- `oh god`
- `oh god yes`
- `harder`
- `no questions`
- `crap`

```
moan x is 69 oh god
yell x harder
```

Same as:

```
moan x is 69
yell x
```

---

## Error Messages

When you screw up, Whanka tells you about it:

| Situation | Error |
|---|---|
| Undeclared variable | **WHAT THE FUCK?!** "x" doesn't exist, you absolute degenerate. |
| Division by zero | **YOU LIMP DICK!** Can't divide by zero. Even a criminal knows that. |
| Infinite loop | **BLUE BALLS!** This loop won't stop. I'm cutting you off. |
| Unexpected end of file | **PREMATURE FINISH!** Hit end of file before you were done. |
| Function not found | **COCKBLOCKED!** No kink called "x" exists. |
| Wrong argument count | **COCKBLOCKED!** "x" takes 2 arg(s) but got 1. |
| Out of memory | **DRIED UP!** Out of memory. We're done here. |
| Syntax error | **WHAT THE FUCK?!** Syntax error: ... |
| Type error | **WHAT THE FUCK?!** Type error: arithmetic requires numbers, you donkey |

---

## Complete Example: FizzBuzz with Functions

```
enough foreplay

my kink is fizzbuzz and it takes n
    moan mod3 is n leftover from 3
    moan mod5 is n leftover from 5

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
            scream n
        boundaries respected.
    boundaries respected.
safe word.

moan i is 1
go on a spree:
    get freaky with fizzbuzz using i
    i gets i jacked up by 1
busted when i hits 101

I came.
regards.
```

---

## Grammar Summary

```
program     = "enough foreplay" statement* "I came." "regards."

statement   = assign | reassign | print | input | if | loop
            | func_def | func_call | return

assign      = "moan" IDENT "is" expr
            | "stash" IDENT "with" expr
reassign    = IDENT "gets" expr

print       = "yell" expr
            | "scream" expr
input       = "seduce the answer out of" IDENT

if          = "if" comparison statement* ["hard limit:" statement*]
              "boundaries respected."

loop        = "go on a spree:" statement*
              "busted when" IDENT "hits" expr

func_def    = "my kink is" IDENT ["and it takes" params]
              statement* "safe word."
params      = IDENT ("," IDENT)*

func_call   = "get freaky with" IDENT ["using" args]
args        = expr ("," expr)*

return      = "finish with" expr

expr        = primary [arith_op primary]
            | "get freaky with" IDENT ["using" args]
comparison  = expr [cmp_op expr]
            | expr "is frigid"

primary     = NUMBER | STRING | IDENT

arith_op    = "jacked up by" | "cut down by" | "blown up by"
            | "split with" | "leftover from"
cmp_op      = "hotter than" | "colder than" | "as hot as"
```

---

## Implementation

The interpreter is a classic three-stage pipeline:

1. **Lexer** — line-oriented tokenizer with multi-word keyword matching (longest match first)
2. **Parser** — recursive descent, builds an AST
3. **Interpreter** — tree-walking evaluator

Written in C11 with no external dependencies. Compiles with gcc, clang, or MSVC.
