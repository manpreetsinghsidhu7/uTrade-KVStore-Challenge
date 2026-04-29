# uTrade KV Store Challenge

An in-memory key-value store built in C++. This project acts as a Redis-like store supporting TTL (Time-To-Live) expiration, basic operations, atomicity, lists, persistence snapshots, and memory statistics. It is designed to act as a caching layer, session store, or low-latency lookup table.

## Features

- **Core KV Operations**: `SET`, `GET`, `DEL`, `KEYS`.
- **Expiration System**: Supports `EX` (seconds) on `SET`. Implements lazy expiration (keys are cleaned up upon access).
- **Atomic Operations**: `INCR` and `DECR` for atomic integer operations.
- **List Operations**: `LPUSH`, `RPUSH`, `LPOP`, and `RPOP` for basic list manipulation.
- **Persistence (Snapshots)**: `SAVE` to snapshot all non-expired keys to disk, and `LOAD` to restore them.
- **Memory Statistics**: Use `STATS` to view the total key count, the number of expired keys removed, and a memory usage estimate.

---

## Getting Started

### Prerequisites

You need a C++11 compatible compiler. 

### Compilation

Compile the project by running:
```bash
g++ main.cpp -o main
```

### Running

Execute the binary. The store reads commands via `stdin` line by line.
```bash
./main
```

---

## Supported Commands

### String Commands
- `SET key value [EX seconds]` : Set the string value of a key. Optional expiration in seconds.
- `GET key` : Get the value of a key. Returns `(nil)` if not found or expired.
- `DEL key` : Delete a key. Returns `OK` or `(nil)`.
- `TTL key` : Get time-to-live. Returns remaining seconds, `-1` if it has no expiry, or `-2` if it doesn't exist.
- `INCR key` : Increments the integer value of a key by one.
- `DECR key` : Decrements the integer value of a key by one.

### List Commands
- `LPUSH key value` : Insert value at the head of the list stored at key.
- `RPUSH key value` : Insert value at the tail of the list stored at key.
- `LPOP key` : Removes and returns the first element of the list.
- `RPOP key` : Removes and returns the last element of the list.

### Utility Commands
- `KEYS pattern*` : Find all keys matching the given pattern (prefix match).
- `STATS` : Displays total key count, removed expired keys, and estimated memory usage in bytes.
- `SAVE [filename]` : Snapshots the store to disk (default: `snapshot.db`).
- `LOAD [filename]` : Restores the store from a snapshot (default: `snapshot.db`).
- `EXIT` : Exits the CLI safely.

---

## Project Structure
- `main.cpp`: Entry point. Runs the `stdin` command parser loop.
- `store.cpp`: The core key-value store logic and operations mapped to specific commands.

## Example Interaction
```text
SET user:1 John EX 300
OK
GET user:1
John
TTL user:1
299
INCR counter
1
INCR counter
2
LPUSH mylist A
1
LPUSH mylist B
2
RPOP mylist
A
STATS
Total Keys: 3
Expired Removed: 0
Memory Used: 120 bytes
SAVE
OK
EXIT
```
