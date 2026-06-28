# DBMS Engine

A lightweight database management system built in C++17. The project is split into a storage engine, a TCP server, and a CLI client that can search, insert, and delete key/value records backed by a B-tree.

## Features

- B-tree based storage engine with persistence
- Buffer pool and page management layer
- TCP server for query execution
- Interactive CLI client
- Catch2-based test suite with generated datasets

## Repository Layout

- `commons/` shared types, constants, and utilities
- `database/` storage engine, server, and tests
- `client/` interactive command-line client
- `build/` generated object files

## Prerequisites

- `g++` with C++17 support
- `make`
- Python 3 for generating test data

## Build

Build both the server and client binaries:

```bash
make
```

This produces:

- `server_app`
- `client_app`

Clean generated binaries and object files:

```bash
make clean
```

## Run the Server

Start the database server with a path to the database file or directory:

```bash
./server_app <database_path> [port]
```

The current implementation accepts an optional port argument, but the server starts on port `8080`.

## Run the Client

Connect the client to the server with a host and port:

```bash
./client_app <host> <port>
```

Once connected, enter commands at the `datashil>` prompt.

### Supported Commands

Commands are terminated with a semicolon and use a simple grammar:

```text
search <key>;
insert <key> "<payload>";
delete <key>;
```

Example session:

```text
datashil> insert 42 "hello world";
datashil> search 42;
datashil> delete 42;
```

## Tests

Generate the test datasets and run the test binary with:

```bash
make generate_data
make test
```

The test suite exercises B-tree insert, search, delete, persistence, and defragmentation behavior.

## Notes

- Test data is stored under `database/tests/data/`.
- The database engine uses binary client/server messages with checksum validation.
- If you change the on-disk format or protocol, update the tests and this README together.