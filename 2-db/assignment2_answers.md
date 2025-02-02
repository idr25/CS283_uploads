## Assignment 2 Answers

#### Directions  
Please answer the following questions and submit in your repo for the second assignment. Keep answers short and concise.

---

### 1. **Externalizing `get_student` Function Design**  
> **Answer**:  
Yes, externalizing `get_student` into its own function is a good design strategy as it centralizes the logic for retrieving student records. This makes the code easier to maintain, test, and debug.

---

### 2. **Issue with Returning a Local Stack Variable**  
> **ANSWER:**  
The code returns a pointer to a stack-allocated variable `student`. When the function exits, the stack memory for `student` is reclaimed, leaving the returned pointer pointing to invalid memory which causes crashes and data corruption making this implementation unsafe.

---

### 3. **Heap Allocation with `malloc` in `get_student`**  
> **ANSWER:**  
Works but risks memory leaks if the caller forgets to `free`. Adds complexity by forcing the caller to manage heap memory.  

---

### 4. **File Storage Analysis**  

#### **File Size Differences (`ls` vs. `du`)**  
- **Why `ls` reports 128 bytes, 256 bytes, and 4160 bytes?**  
  > **ANSWER:**  
  `ls` shows the logical file size, which depends on the maximum byte offset written. For ID=1 (offset `1*64=64`), the file extends to 128 bytes (2 records). For ID=3 (offset `3*64=192`), it grows to 256 bytes (4 records). For ID=64 (offset `64*64=4096`), the file expands to 4160 bytes (65 records × 64 bytes).

- **Why disk usage (`du`) remains 4K until ID=64?**  
  > **ANSWER:**  
  Filesystems allocate storage in fixed-size blocks. Skipped IDs create holes that do not consume disk blocks. Writing up to ID=63 stays within the first 4K block. ID=64 requires a second block, increasing `du` to 8K.

- **Why adding ID=99999 results in 6,400,000 bytes (`ls`) but only 12K (`du`)?**  
  > **ANSWER:**  
  The file’s logical size becomes `99999×64=6,399,936` bytes, but only the blocks containing actual student records are stored. The rest are holes, so du counts only 3 allocated 4K blocks or 12K total.
