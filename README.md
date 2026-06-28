# Hostel Room Allotment System

A terminal-based hostel management system written in C, built as a Problem-Based Learning (PBL) project. Features a color-coded CLI interface, binary file persistence, XOR-encrypted admin credentials, a full audit trail, and an optional built-in HTTP server that serves a web dashboard — all in standard C89.

---

## Features

**Student Management**
- Register students with full personal, academic, parent, and guardian details
- Search by student ID, name, or room number
- Filter students by course or branch

**Room Allotment**
- Allot rooms across 6 configurable hostel blocks (Single / Double / Triple occupancy)
- Transfer students between rooms or blocks
- Vacate rooms and toggle maintenance status

**Reports**
- View all allotted students or vacant rooms
- List fee defaulters (pending balance)
- Occupancy statistics per block

**Admin Panel**
- Role-based access: Guest (read-only) vs. Admin (full control)
- XOR-encrypted admin password stored in `config.dat`
- Configurable room count and fee structure per room type
- Full system reset with confirmation gate

**Audit Trail**
- Every write operation is logged to `audit.log` with timestamp and admin username

**Web Dashboard**
- Built-in HTTP server (`server.c`) launches on port 8080
- Serves a self-contained HTML/CSS/JS frontend embedded in `web_assets.h`
- REST-like API endpoints for student and room data

---

## Project Structure

```
hostel management PBL/
├── main.c            # Entry point, menu routing, session management
├── auth.c / auth.h   # Login, password change, config management, system reset
├── student.c / .h    # Student registration and data handling
├── allotment.c / .h  # Room allotment logic
├── room.c / .h       # Room management (vacate, transfer, maintenance)
├── reports.c / .h    # Search and reporting functions
├── audit.c / .h      # Audit logging
├── server.c / .h     # Built-in HTTP server and API handlers
├── utils.c / .h      # UI helpers, input sanitization, color formatting
├── common.h          # Shared structs (Student, Room, SystemConfig), constants
├── web_assets.h      # Embedded frontend (HTML/CSS/JS as C string literals)
├── Makefile          # Cross-platform build (Windows + Linux/macOS)
├── students.dat      # Binary student database (auto-generated)
├── rooms.dat         # Binary room database (auto-generated)
├── config.dat        # System config with encrypted admin credentials
└── audit.log         # Append-only audit trail
```

---

## Build & Run

**Requirements:** GCC (or any C89-compliant compiler), Make

**Linux / macOS**
```bash
make
./hostel_system
```

**Windows**
```bash
make
hostel_system.exe
```

A pre-built Windows binary (`hostel_system.exe`) is included if you want to run it directly.

**Clean build artifacts**
```bash
make clean
```

---

## Default Credentials

On first run, the system auto-initializes with default admin credentials. Change the password immediately via **Settings & Admin Panel → Change Admin Password**.

---

## Data Storage

All data is persisted in binary flat files (`students.dat`, `rooms.dat`, `config.dat`). No external database or library dependencies — the entire system is self-contained. Admin passwords are XOR-obfuscated before being written to disk.

---

## Web Dashboard

From the main menu, select **Option 6 — Launch Web Dashboard** to start the HTTP server on `http://localhost:8080`. The frontend is fully embedded in the binary (via `web_assets.h`) and requires no separate hosting.

---

## Tech Stack

| Layer | Details |
|---|---|
| Language | C (C89 / ANSI C) |
| Build | GCC, Make (cross-platform) |
| Persistence | Binary flat files |
| Networking | POSIX sockets (Linux/macOS) / Winsock2 (Windows) |
| Frontend | HTML/CSS/JS embedded as C string literals |
| Security | XOR encryption for credentials, role-based access control |

---

## Team

Developed as a PBL project under the Department of Computer Science & Engineering.  
*Indian Dynasty Blocks — RVCE*