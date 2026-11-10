# My Simple Application TCP Client and Server

- **Unit**: Advanced Computer Communications (`CNCO3002`)
- **Study Period**: 2024 Trimester 2A
- **Degree Programme**: Bachelor of Information Technology — Awarded with Distinction by Curtin University
- **Overall Unit Mark**: 88 (High Distinction)

My Simple Application (MSA) is a C client/server assignment using TCP/IPv4 sockets and POSIX threads. Clients request arithmetic, server time, connected-client information and text files through an interactive command prompt.

<table>
  <thead>
    <tr>
      <th colspan="6">Curtin University &bull; Bachelor of Information Technology (Awarded with Distinction &bull; CWA 82.19)</th>
    </tr>
  </thead>
  <tbody>
    <tr align="center" style="font-weight: bold; background-color: #f6f8fa;">
      <td>2022 T3A</td>
      <td>2023 T1A</td>
      <td>2023 T2A</td>
      <td>2023 T3A</td>
      <td>2024 T1A</td>
      <td>2024 T2A</td>
    </tr>
    <tr>
      <td valign="top">
        <abbr title="Fundamentals of Programming (92 HD)"><b>COMP1005</b></abbr><br>
        &bull; <a href="https://github.com/honneground/COMP1005-Vampire-Ecosystem-Simulation">Vampire Eco</a><br>        &bull; <a href="https://github.com/honneground/COMP1005-Vehicle-Emissions-Analyzer">Emissions</a><br>        <abbr title="Computer Systems (83 HD)"><font color="#8c959f">COMP2000</font></abbr><br>
        <abbr title="Linear Algebra and Statistics for Engineers (71 D)"><font color="#8c959f">MATH1015</font></abbr><br>
        <abbr title="Fundamental Concepts of Data Security (73 D)"><font color="#8c959f">ISEC2001</font></abbr>
      </td>
      <td valign="top">
        <abbr title="Data Structures and Algorithms (89 HD)"><b>COMP1002</b></abbr><br>
        &bull; <a href="https://github.com/honneground/COMP1002-StudyHelp-Academic-Network">StudyHelp</a><br>        &bull; <a href="https://github.com/honneground/COMP1002-Advanced-Data-Structures-and-Visualizer">Visualizer</a><br>        <abbr title="Unix and C Programming (90 HD)"><b>COMP1000</b></abbr><br>
        &bull; <a href="https://github.com/honneground/COMP1000-Terminal-Maze-Survival-Game">Maze Survival</a><br>        &bull; <a href="https://github.com/honneground/COMP1000-Terminal-Maze-Darkness-Game">Maze Darkness</a><br>        &bull; <a href="https://github.com/honneground/COMP1000-Bookstore-Transaction-Simulator">Bookstore</a><br>        <abbr title="Introduction to Software Engineering (95 HD)"><b>ISAD1000</b></abbr><br>
        &bull; <a href="https://github.com/honneground/ISAD1000-Software-Testing-and-Pocket-Beast-Game">Pocket Beast</a><br>        <abbr title="Integrating Indigenous Science and STEM (73 D)"><font color="#8c959f">NPSC1003</font></abbr>
      </td>
      <td valign="top">
        <abbr title="Unix Systems Programming (88 HD)"><b>COMP2002</b></abbr><br>
        &bull; <a href="https://github.com/honneground/COMP2002-Parallel-Directory-File-Calculator">Dir Calculator</a><br>        <abbr title="Operating Systems (80 HD)"><b>COMP2006</b></abbr><br>
        &bull; <a href="https://github.com/honneground/COMP2006-Customer-Queue-Concurrency-Simulation">Queue Sim</a><br>        <abbr title="Computer Communications (75 D)"><font color="#8c959f">CNCO2000</font></abbr><br>
        <abbr title="Mobile Cloud Computing (79 D)"><font color="#8c959f">CNCO3003</font></abbr>
      </td>
      <td valign="top">
        <abbr title="Database Systems (80 HD)"><b>ISYS2014</b></abbr><br>
        &bull; <a href="https://github.com/honneground/ISYS2014-Relational-Database-Management-System">FootyStats DB</a><br>        <abbr title="Quantitative Techniques for Business (87 HD)"><b>ECOM2001</b></abbr><br>
        &bull; <a href="https://github.com/honneground/ECOM2001-Econometric-Statistical-Analysis-R">Econometric R</a><br>        <abbr title="Managing Social Media Platforms (69 C)"><font color="#8c959f">MKTG2006</font></abbr><br>
        <abbr title="Computing Topics (61 C)"><font color="#8c959f">COMP2005</font></abbr>
      </td>
      <td valign="top">
        <abbr title="Financial Decision Making (93 HD)"><b>ACCT1002</b></abbr><br>
        &bull; <a href="https://github.com/honneground/ACCT1002-Financial-Decision-Calculators">Finance Tools</a><br>        <abbr title="Human Computer Interface (71 D)"><b>ICTE3002</b></abbr><br>
        &bull; <a href="https://github.com/honneground/ICTE3002-Lumen-Interaction-Design">Lumen UI</a><br>        <abbr title="Engineering Management and Professional Practice (79 D)"><b>MGMT3000</b></abbr><br>
        &bull; <a href="https://github.com/honneground/MGMT3000-Phone-Router-Repurposing-Design">Phone Router</a><br>        <abbr title="Distributed Networks (83 HD)"><font color="#8c959f">CNCO3000</font></abbr>
      </td>
      <td valign="top" style="background-color: #f1f8ff; border: 2px solid #0969da;">
        <abbr title="Capstone Computing Project 1 (94 HD)"><b>ISAD3000</b></abbr><br>
        &bull; <a href="https://github.com/honneground/ISAD3000-CurtinSG-App-Design">CurtinSG UI</a><br>        <abbr title="Capstone Computing Project 2 (95 HD)"><b>ISAD3001</b></abbr><br>
        &bull; <a href="https://github.com/honneground/ISAD3001-CurtinSG-Student-App">CurtinSG App</a><br>        <abbr title="Advanced Computer Communications (88 HD)"><b>CNCO3002</b></abbr><br>
        &bull; <b>&#128073; Socket Server</b><br>        <abbr title="Accounting - The Fundamentals of Business (93 HD)"><font color="#8c959f">ACCT2013</font></abbr>
      </td>
    </tr>
  </tbody>
</table>


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

---

## Portfolio note

This Git repository was created retrospectively from the original project code, with assistance from OpenAI Codex and Antigravity in curating the repository and writing this README for publication. The initial commit timestamp is backdated to the original coursework submission date, followed by the final release commit.

