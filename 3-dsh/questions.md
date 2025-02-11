### 1. In this assignment I suggested you use 'fgets()' to get user input in the main while loop. Why is 'fgets()' a good choice for this application?


**Answer**:  
`fgets()` is ideal because it reads input line-by-line, stopping at a newline or EOF, which aligins with the shell’s need to process commands one line at a time. Unlike `scanf()`, `fgets()` retains the newline character, allowing the program to detect and trim it, ensuring clean command parsing.

---

### 2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

**Answer**:  
Using `malloc()` avoids allocating a large fixed-size array on the stack, which could cause stack overflow. Dynamic allocation on the heap is more flexible and safer for handling potentially large input buffers.

---

### 3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

**Answer**:  
Trimming spaces ensures commands and arguments are parsed correctly. For example, `"   ls   "` would be interpreted as `ls`, while untrimmed spaces could lead to invalid executable names.  Similarly, spaces around pipes ( `"cmd1 |  cmd2"`) could split into empty or incorrect commands, causing parsing errors.

---

### 4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

**Answer**:  
- **`ls > output.txt`**: Redirects STDOUT to a file.  
  *Challenge*: Managing file descriptors to replace STDOUT without affecting other processes.  
- **`grep "error" < log.txt`**: Redirects STDIN from a file.  
  *Challenge*: Error handling if the file doesn’t exist.  
- **`cmd 2> error.log`**: Redirects STDERR to a file.  
  *Challenge*: Separating STDERR from STDOUT.

---

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

**Answer**:  
- **Redirection** binds input/output to files or devices
- **Piping** connects the STDOUT of one command to the STDIN of another.  
Piping enables inter-process communication, while redirection changes I/O sources/destinations.  

---

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

**Answer**:  
Keeping them separate allows users to process normal output (STDOUT) independently of error messages (STDERR). For example, scripts can log errors to a file while processing clean output, or suppress errors without affecting results. Mixing them could obscure critical issues in automated workflows.

---

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?


**Answer**:  
The shell should default to keeping STDOUT and STDERR separate. To merge them, provide syntax like `cmd 2>&1` (redirect STDERR to STDOUT). This allows users to decide whether to combine streams for debugging or logging, while preserving separation by default for clarity. Implementing this requires modifying file descriptors during command execution.
