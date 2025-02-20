## Fork and Execvp Questions and Answers

### 1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

> **Answer**: `fork` is used to create a new child process, allowing the parent process to continue running independently. If we called `execvp` directly, it would replace the current process image, terminating the original process. Using `fork` allows the parent to manage multiple child processes, handle errors, and retrieve exit statuses.

### 2. What happens if the fork() system call fails? How does your implementation handle this scenario?

> **Answer**: If `fork()` fails, it returns `-1`, indicating that the system was unable to create a new process. This typically happens due to resource limitations. My implementation handles this scenario by printing an error message using `perror()` and terminating gracefully.

### 3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

> **Answer**: `execvp()` searches for the specified command in the directories listed in the `PATH` environment variable. If the command is not found in any of these directories, `execvp()` fails and returns an error.

### 4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

> **Answer**: `wait()` ensures that the parent process waits for its child to complete execution before continuing. Without `wait()`, the child process might become a zombie process, leading to resource leaks.

### 5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

> **Answer**: `WEXITSTATUS()` extracts the exit status of a terminated child process. It is important because it allows the parent process to determine whether the child executed successfully or encountered an error.

### 6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

> **Answer**: My implementation ensures that quoted arguments are treated as a single entity instead of being split at spaces. This is necessary to correctly handle commands that include multi-word arguments, such as file paths with spaces.

### 7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

> **Answer**: I improved the parsing logic by adding better handling for special characters and quoted arguments. A key challenge was ensuring that escaped characters were correctly interpreted while maintaining robust error handling.

### 8. Linux Signals Research

#### What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

> **Answer**: Signals provide a way to asynchronously notify processes about events such as termination, suspension, or custom-defined events. Unlike other IPC mechanisms like pipes or message queues, signals are lightweight and typically used for process control rather than data exchange.

#### Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

> **Answer**:
> - **SIGKILL (9)**: Immediately terminates a process without allowing cleanup.
> - **SIGTERM (15)**: Requests a process to terminate gracefully, allowing cleanup.
> - **SIGINT (2)**: Interrupts a process, usually sent when a user presses `Ctrl+C`.

#### What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

> **Answer**: `SIGSTOP` suspends a process unconditionally. Unlike `SIGINT`, it cannot be caught or ignored because it is meant to forcibly pause a process until `SIGCONT` is received.

