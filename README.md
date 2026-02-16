# nanLanguage – A Simple Custom Scripting Language

## Overview

**nanLanguage** is a lightweight, beginner-friendly interpreted scripting language written in C++.
It is designed to demonstrate:

* How interpreters work
* Basic command parsing
* Variables and state handling
* Loops and recursion
* Script execution from a file

nanLanguage reads a `.txt` file and executes commands line by line.

---

# Getting Started

## Compile

```bash
g++ -std=c++17 main.cpp -o mini_lang
```

## Run

```bash
./mini_lang script.txt
```

If no file is provided:

```
Usage: mini_lang <filename.txt>
```

---

# Language Syntax

nanLanguage is line-based. Each line represents one instruction.

Blank lines are ignored.

---

# Supported Commands

---

## `print`

Prints text or a variable value.

### Print a string (must use quotes)

```
print "Hello World"
```

### Print a variable

```
print x
```

If the variable does not exist, it prints the text as-is.

---

## `set`

Creates or assigns a variable.

### Assign number

```
set x = 10
```

### Assign from another variable

```
set y = x
```

Variables are stored as integers.

---

## `add`

Adds a number to a variable.

```
add x 5
```

If `x` = 10 → now `x` = 15.

If the variable does not exist, an error is printed.

---

## `comment`

Ignores the line.

```
comment "This is ignored"
```

⚠️ Note: The command must be spelled correctly as `comment`.

---

## `loop`

Runs a block multiple times.

### Syntax

```
loop variable:count (
    commands
)
```

### Example

```
loop i:5 (
    print i
)
```

This will run from:

```
i = 0
i = 1
i = 2
i = 3
i = 4
```

The loop:

* Starts at 0
* Stops before `count`
* Automatically assigns the loop variable

Loops can contain any valid nanLanguage commands.

---

# 🧪 Example Program Explained

Below is your example script (corrected for syntax consistency):

```
comment "Print needs quotes"
comment "Add functions"
comment "Add api to AI"
comment "write to files/open files"

print "Hello World"

set x = 10
print x

add x 5

comment "We can do comments now!!"

add x 50
add x 5000

print "After adding:"
print x

print "Start Loop"

comment "Loops are from 0 to one before the specified value"

loop i:15 (
    comment "Add one to print from 1 to 15"
    set aux = i
    add aux 1
    print aux
)

print "Done"
```

---

# Step-by-Step Execution

### 🟢 1. Print Hello

```
Hello World
```

---

### 🟢 2. Set x = 10

```
x = 10
```

---

### 🟢 3. Add values to x

```
add x 5      → 15
add x 50     → 65
add x 5000   → 5065
```

Output:

```
After adding:
5065
```

---

### 🟢 4. Loop from 0 to 14

The loop runs 15 times:

```
i = 0 → aux = 1
i = 1 → aux = 2
...
i = 14 → aux = 15
```

Output:

```
1
2
3
...
15
```

---

### 🟢 5. Final Output

```
Done
```

---

# How the Interpreter Works (Internally)

The interpreter:

1. Reads the entire file into memory
2. Splits it line-by-line
3. Parses the first word as a command
4. Executes the matching behavior
5. Stores variables in a `std::map<std::string, int>`
6. Uses recursion to execute loop blocks

---

# Language Rules

| Rule                        | Description                           |
| --------------------------- | ------------------------------------- |
| Strings must use quotes     | `print "text"`                        |
| Variables are integers only | No floats or strings yet              |
| Commands are case-sensitive | `Print` ≠ `print`                     |
| Loops require parentheses   | Must open with `(` and close with `)` |

---

# Known Limitations

* No string variables
* No math expressions (`set x = 5 + 3` not supported)
* No nested parentheses validation
* No functions
* No file writing
* No conditionals (if statements)
* Weak syntax validation
* No error recovery system

---

# Some Next Steps

Here are logical improvements to evolve nanLanguage:

---

## 🔥 1. Add Conditionals

```
if x > 5 (
    print "Greater"
)
```

---

## 🔥 2. Add Functions

```
func greet (
    print "Hello"
)

call greet
```

---

## 🔥 3. Add Math Expressions

Support:

```
set x = 5 + 3 * 2
```

Requires expression parser (Shunting-yard algorithm).

---

## 🔥 4. Add String Variables

```
set name = "John"
print name
```

Would require changing variable storage to:

```cpp
std::map<std::string, std::variant<int, std::string>>
```

---

## 🔥 5. Add File IO

```
write "Hello" to file.txt
read file.txt into x
```

---

## 🔥 6. Add Error Reporting with Line Numbers

Instead of:

```
Unknown command
```

Show:

```
Line 12: Unknown command 'abc'
```

---

## 🔥 7. Add Nested Loop Validation

Currently, nested loops work but without structural validation.

A proper block parser would improve reliability.

---

## 🔥 8. Add AI API Integration

Example concept:

```
ai "Explain recursion"
```

Would connect to an API and print the result.

---

## 🔥 9. Add REPL Mode

Instead of running files:

```
mini_lang
>
```

Interactive execution.

---

## 🔥 10. Convert to Bytecode Interpreter

For performance:

* Parse once
* Generate instruction list
* Execute bytecode

---

# Educational Value

nanLanguage demonstrates:

* Tokenization
* Parsing
* Recursive execution
* State management
* Language design fundamentals
* Building interpreters in C++

It is a strong foundation for building:

* A real scripting language
* A compiler
* A VM
* A DSL (Domain Specific Language)

---

# Open Source, fell fre to modify it!

Just tell me what direction you'd like to take 🚀
