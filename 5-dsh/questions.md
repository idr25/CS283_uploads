## 1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

Our shell uses waitpid() in a loop to wait for each child process to complete before returning to the command prompt. Without waitpid(), the shell would immediately return to accepting input while commands are still running, creating zombie processes that waste system resources and causing output from different commands to mix together unpredictably.

## 2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

After dup2(), we must close unused pipe ends because processes reading from pipes wait for all writer ends to close before receiving EOF. If pipes remain open, processes will hang waiting for input, file descriptor resources will be wasted, and data flow through multiple pipes will be disrupted. Properly closing pipes ensures correct data flow and prevents resource leaks.

## 3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

The cd command must be built-in because it needs to change the working directory of the shell process itself. As an external process, cd would only change the directory of the child process, and this change would be lost when the child exits. The shell would remain in its original directory, making the command useless. Built-ins like cd and exit need direct access to the shell's state.

## 4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

To support unlimited piped commands, replace the fixed array with a dynamically allocated array that grows using realloc() when needed, or use a linked list of commands. Trade-offs include increased complexity, potential memory fragmentation, and the need for better error handling. While this approach would be more flexible, it requires careful memory management to prevent leaks.
