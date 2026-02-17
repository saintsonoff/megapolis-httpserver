# megapolis-httpserver

Lightweight asynchronous HTTP server for embedded Linux (ARMv7). Built on Boost.Beast and Boost.Asio C++23 coroutines.

## Features

- **Cooperative multitasking** — coroutine-based (`co_await`) event loop, no thread-per-connection overhead
- **Streaming file upload** — non blocking, 64 KiB chunked reads, constant memory regardless of payload size
- **Log tail** — `GET /log` returns the last 5 MiB of the log file
- **Parallel upload offloading** — `POST /upload` dispatched to `asio::thread_pool` (`hardware_concurrency - 1` threads)
- **Root enforcement** — `geteuid() == 0` check at startup; exits immediately otherwise
- **Graceful shutdown** — `SIGINT` / `SIGTERM` stops the `io_context`
- **OOP code design** - expandable code base

## Requirements

| Dependency | Version       |
|------------|---------------|
| C++        | `23`          |
| CMake      | `>= 3.31`     |
| Boost      | `>= 1.86`     |
| Platform   | Linux `armhf` (ARMv7) |

The server **requires root privileges** (`sudo` or UID `0`).

## Build

### Native (on-device)

```bash
mkdir build
cmake -S ./src -B ./build -DCMAKE_BUILD_TYPE=Release
cmake --build ./build --parallel $(nproc)
```

Binary: `./build/app/httpserver`

### Cross-compile via Docker (ARMv7)

1. Build the toolchain image (once):

```bash
docker build -t arm32v7-cppdev:latest ./toolchain/armv7/development
```

2. Create `./toolchain/armv7/building/.env` from the example:

```bash
cp ./toolchain/armv7/building/.env-example ./toolchain/armv7/building/.env
```

`.env` contents:

```env
TOOLCHAIN_IMAGE=arm32v7-cppdev:latest
BUILD_TYPE=Release
LOCAL_EXPORT_PATH=./bin_output
```

3. Run the build script:

```bash
./toolchain/armv7/building/build.sh
```

Binary: `./bin_output/httpserver`

## Usage

Start the server (port `1616`, root required):

```bash
sudo ./build/app/httpserver
```

### Endpoints

| Method | Path      | Description                                |
|--------|-----------|--------------------------------------------|
| `ANY`  | `/info`   | Health check, returns `"all ok"`           |
| `GET`  | `/log`    | Last 5 MiB of `/tmp/server_log.txt`        |
| `POST` | `/upload` | Stream file to `/tmp/` ans "Success"in end |

### Examples

Health check:

```bash
curl http://localhost:1616/info
```

```
all ok
```

Server log (tail):

```bash
curl http://localhost:1616/log
```

Upload a file:

```bash
curl -X POST -F "file=@myTestFile.txt" http://localhost:1616/upload
```

```
# after loading
Success
```

## Documentation

Requirements: `doxygen`, `graphviz`.

```bash
doxygen Doxyfile
open docs/html/index.html
```

## License

[GNU General Public License v3.0](LICENSE)