# Custom Unix Shell

A Unix-like shell implemented in C to explore how command-line interpreters work internally. The project focuses on process creation, command execution, signal handling, job control, pipelines, redirections, and command parsing.

## Features

### Command Execution

- Execute external commands using fork() and execve()
- Pass command-line arguments to programs
- Environment variable support
- Process synchronization using wait() / waitpid()

### Shell Operators
- Command pipelines using |
- Conditional execution using &&
- Conditional execution using ||

### Input / Output Redirection
- Standard input redirection (<)
- Standard output redirection (>)
- Standard error redirection (2>)

### Signals & Utilities
- SIGINT (Ctrl+C) handling
- Tokenization of user input
- Custom utility/string functions
- Error handling and reporting

## Architecture

```text
                 User Input
                      │
                Tokenizer (Lexer)
                      │
                   Parser
                      │
                   Job Control
                      │
            Pipeline and Redirection
          ┌───────────┼───────────┐
          │                       │
     Built-in               External Commands
     Commands     

```

## Example

```bash
[shell]> ls -l | grep ".c" 
[shell]> make && ./shell 
[shell]> cat input.txt | sort | uniq 
[shell]> gcc main.c -o app || echo "Compilation failed" 
[shell]> echo Hello > output.txt
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
- Unix signals and terminal behavior
- File descriptor manipulation
- Pipes and inter-process communication (IPC)
- Job control fundamentals
- Command parsing and shell design

## Challenges & What I Learned

Building a shell revealed how much work happens behind every command entered into a terminal.

What initially seemed like "run a program and wait for it" quickly expanded into understanding:

- Signal delivery
- Pipe creation and synchronization
- File descriptor inheritance
- Parsing operator precedence
- Foreground vs background execution

## Notes

- This project is intended for learning and experimentation with Unix internals.
- Behavior may differ from Bash, Zsh, or other production shells.
- Designed as an educational implementation rather than a full POSIX-compliant shell.

