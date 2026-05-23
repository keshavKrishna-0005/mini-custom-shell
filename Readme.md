# Custom Unix Shell

A minimal Unix-like shell implemented in C. This project focuses on understanding how command-line interpreters work at a low level, including process creation, command execution, environment handling and file redirection.

## Features

- Execute external commands using `fork()` and `execve()`
- Parse command-line arguments
- Handle environment variables
- Input/output/error file redirection
- Custom SIGINT (`Ctrl+C`) handling
- Custom utility/string functions
- Basic error handling and process management

## Example

```bash
shell> ls -l
shell> echo Hello > output.txt
shell> cat < output.txt
```

## Compilation & Usage

Compile:
```bash
make
```
Run:
```bash
./shell
```
Compile and Run:
```bash
make run
```

## Learning Outcomes

- Process management (fork, execve, wait)
- Memory management and error handling
- Understanding how shells execute commands
- Understanding file handling for standard services
- Building core utilities from scratch

## Challenges & What I Learned

While building this shell, I started with the goal of simply executing commands, but quickly realized how much complexity exists behind real Unix shells like bash or zsh.

Features such as signal handling and file redirection introduced lower-level concepts like:

- TTY (terminal) behavior
- Process creation and control
- File descriptor manipulation
- Parent/child process separation
- Signal propagation and handling

One of the most interesting parts of this project was understanding why professional shells behave the way they do.
For example, handling SIGINT correctly is not just about catching Ctrl+C, but about deciding whether the shell process or the child process should respond to it.
This project made me appreciate both the elegance and complexity of Unix systems programming.

## Notes

- This project is intended for learning and experimentation with Unix internals.
- Behavior may differ from standard shells like bash


## Author
Keshav Krishna