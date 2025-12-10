# Mini OS Shell in C
A fully functional miniature UNIX-like shell implemented in C with support for command parsing, execution, built‑ins, I/O redirection, piping, background jobs, environment variables, history, and undo operations. Designed using modular architecture and core data structures (stack, queue, linked list) to model real OS‑level shell behavior.


## 🚀 Overview
This project implements a lightweight shell capable of executing system commands and built‑in operations using:

* **Process creation** (`fork`, `execvp`)
* **Process control** (`waitpid`, signals)
* **Pipes & redirection** (`pipe`, `dup2`, `open`)
* **Custom parsing engine** (tokenizer + operator handling)
* **Data structures** for history, variables & job management
This shell demonstrates concepts in **Operating Systems**, **Systems Programming**, and **Compiler Front‑End parsing**.


## ✨ Key Features
### ✔ External Commands
Runs any valid system command via `execvp()`:
```
ls -l
cat file.txt
pwd
```

### ✔ Built‑ins
* `cd`, `pwd`, `exit`
* `echo` with `$VAR` expansion
* `set VAR=value`, `unset VAR`, `printvars`
* `history`
* `undo` (variable restoration)
* `jobs` (background job queue)

### ✔ Redirection
```
command > file
command >> file
command < file
```

### ✔ Pipes
```
cmd1 | cmd2
```

### ✔ Background Execution
```
sleep 5 &
```
Managed using job queue + `SIGCHLD` cleanup.

### ✔ History & Undo

* Command history implemented with a **stack**
* Undo for variable changes

### ✔ Environment Variables
Custom key–value store using a **linked list**:
```
set NAME=Vinodh
echo $NAME
```

## 📁 Project Structure

Mini-OS-Shell-in-C/
├── Makefile
├── README.md
├── include/
│   ├── parser.h
│   ├── builtins.h
│   ├── executor.h
│   ├── kv.h
│   ├── queue.h
│   ├── stack.h
├── src/
│   ├── main.c
│   ├── parser.c
│   ├── builtins.c
│   ├── executor.c
│   ├── kv.c
│   ├── queue.c
│   ├── stack.c
└── obj/ (ignored in VCS)


## ⚙️ Build & Run
### Build
```
make
```

### Run
```
./psh
```

### Clean
```
make clean
```


## 🧪 Quick Functional Test
```
pwd
ls | grep src
echo hello > a.txt
cat < a.txt
echo world >> a.txt
cat a.txt
set X=10
echo $X
unset X
sleep 3 &
jobs
history
undo
exit
```


## 🛠️ Technologies & System Calls Used
* C (GCC, POSIX compliant)
* `fork`, `execvp`, `waitpid`
* `pipe`, `dup2`, `open`, `close`
* `signal`, `SIGCHLD`
* Manual parsing/tokenization
* Dynamic memory management



## 📘 Design Highlights
### Modular Architecture

* **Parser** → tokenization and operator extraction
* **Executor** → command execution, redirection, pipes, background
* **Built‑ins** → internal shell operations
* **KV Store** → environment variables
* **Stack** → history & undo
* **Queue** → background job management

### Error Handling

* Invalid syntax (e.g., missing filenames)
* Permission issues
* Unknown commands
* Background job cleanup


## 📌 Future Enhancements

* Multi‑stage pipelines (cmd1 | cmd2 | cmd3)
* Command aliasing
* Auto‑complete (tab completion)
* Configuration file support
* Enhanced scripting capabilities


## 📄 License
This project is developed for academic and learning purposes under an open educational license.



