# My Simple Application TCP Client and Server

- **Unit**: Advanced Computer Communications (`CNCO3002`)
- **Study Period**: 2024 Trimester 2A
- **Degree Programme**: Bachelor of Information Technology — Awarded with Distinction by Curtin University
- **Overall Unit Mark**: 88 (High Distinction)

My Simple Application (MSA) is a C client/server assignment using TCP/IPv4 sockets and POSIX threads. Clients request arithmetic, server time, connected-client information and text files through an interactive command prompt.

## Assignment context and constraints

The assignment required two complementary programs, `server` and `client`. The server had to handle multiple simultaneous TCP sessions, with a worker thread serving each connected client. Both programs needed to validate commands, check system-call results and close their sockets. The client had to display replies and record requests and responses in a file named `log`.

The server's command-line settings constrain simultaneous clients to **1–10** and the per-session idle wait to **1–120 seconds**. Required services were `SUM(x,y)`, `TIME`, `USERS`, `FILE <filename>` and `DONE`. The report also had to explain shared structures, synchronization, compilation, execution and testing.

## Implementation

- **Server (`server.c`)**: tries ports **50065–50069** in order and binds one available port. It creates detached request-processing threads, tracks connected sockets in a linked list, and uses mutexes and a condition variable around connection-count and client-record operations. `select` implements the configured wait for the next request.
- **Client (`client.c`)**: resolves an IPv4 address or hostname, checks the initial `ALIVE` exchange, reads interactive commands and appends requests and replies to `log`. Connection and response waits use a three-second timeout.
- **Command utilities (`toolKits.c`)**: POSIX regular expressions classify valid requests; helper functions remove spaces where required and format values.
- **File transfer**: the server announces the file size, waits for a `READY` acknowledgement and sends the content. The client displays and logs the received text. A missing file receives a `404 not found` response.

### Commands

| Command | Behaviour |
|---|---|
| `SUM(2.5,3)` | Add two floating-point values on the server |
| `TIME` | Return the server's local time, formatted with `ctime` |
| `USERS` | List the IP addresses and port numbers of connected clients |
| `FILE file1` | Request a file relative to the server's working directory |
| `FILE "file name"` | Request a filename containing spaces |
| `DONE` | End the client's session after the goodbye reply |
| `TERMINATE` | Stop the server; this is an implementation extension |

The client automatically performs the `ALIVE` connection check when it connects. The included `file1`, `file2` and `file3` are transfer examples.

## Build and run

Use GCC, Make and a POSIX environment with TCP sockets, pthreads and POSIX regular expressions, such as Linux or WSL.

```bash
make CFLAGS="-Wall -pthread"
```

Start the server from this project directory so it can read the example files:

```bash
./server <max_clients> <idle_timeout_seconds>
./server 5 60
```

Read the selected port in the server's startup banner. If it reports `50065`, start the client in another terminal with:

```bash
./client 127.0.0.1 50065
```

A hostname is also accepted in place of the IPv4 address. At the `Client>` prompt, try:

```text
SUM(2,3)
TIME
USERS
FILE file1
FILE missing_file
DONE
```

Run another client in a separate terminal to exercise concurrent sessions and inspect `USERS`. The client's `log` file records the interaction. To remove compiled executables:

```bash
make clean
```
