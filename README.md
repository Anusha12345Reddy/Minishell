# Minishell

📘 MiniShell Project – Explanation

The MiniShell is a custom-built Unix-like shell implemented in C that mimics the behavior of standard shells like `bash`. It provides a command-line interface where users can execute built-in and external commands, run background processes, handle signals, and work with pipes — all while maintaining an interactive, responsive environment.

This shell supports parsing and executing user input in real time, distinguishing between internal commands (like `cd`, `pwd`, `exit`, `echo`) and external programs. It also features piping (`|`), signal handling (`Ctrl+C`, `Ctrl+Z`), and a basic job control system to manage background and foreground processes.

Key features include:

- Execution of built-in and external commands
- Customizable shell prompt using `PS1=`
- Support for piping (`cat file.txt | grep word`)
- Job control: `bg`, `fg`, `jobs` for background/foreground management
- Signal handling (`SIGINT`, `SIGTSTP`, `SIGCHLD`) for clean process control
- Maintains a job list using linked lists to track background processes
- Displays process ID and command name for managed jobs

The shell is built from scratch using system calls like `fork()`, `execvp()`, `waitpid()`, and signal APIs. It handles input parsing manually, tokenizes arguments, and uses modular functions to ensure clarity, reusability, and easy extension.

🔹 What This Project Showcases:

- Low-level systems programming in a Linux environment
- Mastery of process control, signal handling, and inter-process communication
- Parsing and tokenizing user input with error handling
- Building real-time interactive command-line tools from the ground up
