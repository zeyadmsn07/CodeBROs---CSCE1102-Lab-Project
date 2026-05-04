# CodeBROs

A collaborative coding desktop app built for beginner programmers — specifically for people
like us who are learning C++ and wanted something better than just sharing code over Discord.

Built by Zeyad Nada, Youssef Gohar, and Omar Mohammed for CSCE1102 at AUC.

---

## What it does

You open the app, create an account, and get dropped into a party room with a code you can
share with friends. Everyone in the room sees the same code editor in real time — you type,
they see it. There's a chat panel on the side, a member list showing who's in the room, and
a tasks section with small C++ challenges you can work through together. There's also an AI
assistant you can ping directly from the chat by typing /ask followed by your question.

It's basically the tool we wished existed when we started learning C++.

---

## Tech stack

- Qt 6 — the desktop GUI
- Boost.Asio — TCP networking between clients and server
- nlohmann/json — message serialization over the wire
- GoogleTest — unit tests
- OpenRouter API — AI assistant (Nvidia Nemotron 3 super, free tier)
- CMake 3.20+ — build system
- PicoSHA2 — password hashing
- clang-format + clang-tidy — code formatting and linting

---

## Prerequisites

Make sure you have these installed before trying to build:
sudo apt install cmake build-essential git 
libgtest-dev nlohmann-json3-dev libboost-all-dev 
qt6-base-dev qt6-tools-dev libgl1-mesa-dev 
clang-format clang-tidy

---

## Building

Clone the repo and build from the root:

git clone https://github.com/zeyadmsn07/CodeBROs---CSCE1102-Lab-Project.git
cd CodeBROs---CSCE1102-Lab-Project
cmake -B build
cmake --build build

First build takes a minute. After that it's fast.

---

## AI assistant setup

The AI feature needs an API key from openrouter.ai — it's completely free, no card needed.

1. Sign up at https://openrouter.ai and grab a key from the Keys section
2. Copy the template: `cp client/config.h.template client/config.h`
3. Open `client/config.h` and paste your key where it says to
4. Rebuild: `cmake --build build`

The `config.h` file is in `.gitignore` so it never gets committed.

---

## Running it

You need two terminals — one for the server, one for the client.

**Terminal 1 — start the server:** ./build/server/server
**Terminal 2 — start the client:** ./build/client/client

The server needs to be running before anyone logs in. Once it's up, multiple clients can
connect to it at the same time.

---

## Testing across multiple machines

If you want to test with people on different laptops on the same Wi-Fi:

1. The person running the server finds their local IP: `hostname -I`
2. In `client/MainWindow.cpp`, change `"127.0.0.1"` in `goToDashboard()` to that IP
3. Rebuild the client on each machine
4. Run the server on one machine, clients on the others

Everyone connects, joins the same room code, and the real-time sync works across all of them.

---

## Running the tests

ctest --test-dir build --output-on-failure

Tests cover: user registration and auth, session persistence, party room management,
message serialization and JSON round trips, and task answer validation.

---

## Code formatting and linting

We used clang-format with Google style (4-space indent) and clang-tidy for static analysis.

---

## Project structure

 CodeBROs/
├── client/       Qt GUI — login, register, dashboard, tasks, networking
├── server/       Boost.Asio TCP server — rooms, sessions, broadcasting
├── shared/       Pure C++ logic — messages, auth, party management, tasks
├── tests/        GoogleTest unit tests
└── data/         Runtime files (users.json, sessions.json, tasks.json)

The `shared/` library has zero Qt and zero Boost dependencies by design — it compiles
independently and is used by both the client and server.

---

## What's implemented

This project is fully completed and includes the following features:

- Full login and registration with persistent accounts
- Auto-login on relaunch using saved sessions
- Room-based system where users can join specific rooms using a code
- Real-time chat between multiple clients in the same room
- Real-time code synchronization across all room members
- Live member list updates as users join and leave
- Typing indicator for active users
- Task system with multiple C++ challenges loaded from JSON
- Local code execution:
  - Code is compiled using `g++`
  - Executed safely with a timeout
  - Output is captured and validated
- Detailed task feedback including compilation errors and incorrect outputs
- AI assistant for code review and suggestions
- Multi-threaded AI requests to prevent UI freezing
- JSON-based message system for all client-server communication
- 13+ passing unit tests covering core functionality

---

## Remaining limitations

- Task validation is primarily output-based (no deep semantic analysis)
- No version control or collaborative undo history
- Conflict resolution uses a simple last-write-wins approach
- AI responses depend on external API availability
- Limited number of predefined tasks
---

## Honest note on development

This project pushed us way beyond what we'd covered in class. Boost.Asio async networking,
Qt signals/slots across threads, session persistence, real-time sync with loop guards, fnone
of this was material we had seen before. We spent a lot of time reading documentation and
trying things that didn't work before finding approaches that did.

We used AI assistance for parts that felt genuinely out of reach, particularly the
Boost.Asio architecture, the Qt/Boost thread crossing with QMetaObject, AI-API implementation, creating a pleasent design for the UI and getting the CMake build system wired correctly across four subdirectories. We didn't just copy code blindly though. We asked for explanations, pushed back when things didn't make sense to us,
and rewrote or adapted things until we understood what was actually happening.



Course: CSCE1102-06, AUC
Instructor: Dr. Abdelrahman Al-Khateeb
