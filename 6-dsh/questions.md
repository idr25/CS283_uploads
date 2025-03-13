## 1. Explain the protocol design choices you made for the remote shell communication.

I used null bytes to terminate client messages and EOF characters to mark the end of server responses. This approach works well with C string handling and clearly marks message boundaries in the TCP stream. Special commands like exit and stop-server are detected and handled directly at the protocol level.

## 2. What challenges did you face when implementing the remote shell, and how did you overcome them?

The biggest challenge was handling partial TCP message receives. I solved this by implementing a loop that keeps reading until finding the EOF character. File descriptor redirection between sockets and pipes was also tricky. I carefully managed resource cleanup to prevent memory leaks and zombie processes.

## 3. How did you test your implementation? What test cases did you find most useful?

I wrote BATS scripts to test basic and piped commands. The most useful tests verified that piped commands like "ls | grep" worked correctly over the network. I also tested built-in commands and sequential client connections to ensure the server remained stable.

## 4. If you implemented the multi-threaded server (extra credit), explain your implementation and any challenges you faced.

I implemented threading using pthread_create() to handle each client in a separate thread. I used pthread_detach() for automatic cleanup when clients disconnect. The main challenge was ensuring thread safety for shared resources. I solved this by minimizing shared state and using proper mutex locks where needed.

## 5. What improvements would you make to your implementation if you had more time?

I would add authentication for security, implement a thread pool for better performance, and add command history. Better error recovery for network failures would make the system more robust. A more comprehensive logging system would also help with debugging.
