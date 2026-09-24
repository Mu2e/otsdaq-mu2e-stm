#!/usr/bin/env python3
"""
CPU-efficient TCP receiver -> binary file writer (Linux/Python).

Design goals:
- Accept a single TCP connection and stream all bytes to a file.
- Minimize Python overhead:
  * Use a large fixed recv buffer and avoid per-chunk allocations.
  * Write using a buffered file object and pre-allocated bytearray views.
  * Use selectors (epoll on Linux) to avoid busy waiting.
- Optional: set large socket buffers and TCP_NODELAY off (Nagle on).
- Optional: periodic throughput logging.

Usage:
  python tcp_sink.py --bind 0.0.0.0 --port 9000 --out data.bin
  python tcp_sink.py --bind 127.0.0.1 --port 9000 --out data.bin --log-rate 2

Notes:
- This writes raw bytes exactly as received (no framing).
- For maximal throughput, run on CPython 3.10+ on Linux and use fast storage.
"""

import argparse
import os
import selectors
import socket
import sys
import time

def set_sock_opts(sock: socket.socket, sndbuf: int, rcvbuf: int) -> None:
    # Large buffers help keep the pipeline full (kernel may double values)
    if rcvbuf:
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, rcvbuf)
    if sndbuf:
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, sndbuf)

    # Nagle ON for throughput efficiency (TCP_NODELAY = 0)
    sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 0)

    # Keepalive can help detect dead peers; default system params are often fine.
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)

def serve_once(bind_ip: str,
               port: int,
               out_path: str,
               recv_chunk: int,
               file_buffer: int,
               rcvbuf: int,
               backlog: int,
               log_rate_s: float,
               fsync_each: bool) -> None:
    # Create listening socket
    lsock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    lsock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    # SO_REUSEPORT can help in multi-worker designs; here we use one acceptor.
    try:
        lsock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
    except OSError:
        pass

    # Bind/listen
    lsock.bind((bind_ip, port))
    lsock.listen(backlog)
    # Blocking accept is fine (only once). Then we use selectors for the stream.
    print(f"Listening on {bind_ip}:{port} ...", flush=True)

    conn, addr = lsock.accept()
    try:
        print(f"Accepted connection from {addr[0]}:{addr[1]}", flush=True)

        # Tune connected socket
        set_sock_opts(conn, sndbuf=0, rcvbuf=rcvbuf)

        # Non-blocking + selectors: CPU efficient under variable traffic
        conn.setblocking(False)
        sel = selectors.DefaultSelector()
        sel.register(conn, selectors.EVENT_READ)

        # Pre-allocate a receive buffer to avoid per-iteration allocations.
        # We'll recv_into it and write the slice actually received.
        buf = bytearray(recv_chunk)
        view = memoryview(buf)

        # Open output file with large userspace buffer
        # buffering=file_buffer uses a large stdio buffer to reduce syscalls.
        with open(out_path, "wb", buffering=file_buffer) as f:
            # Throughput counters
            total = 0
            window_bytes = 0
            t0 = time.monotonic()
            t_last = t0

            while True:
                events = sel.select(timeout=1.0)
                if not events:
                    # Periodic rate log even if idle
                    if log_rate_s > 0:
                        now = time.monotonic()
                        if now - t_last >= log_rate_s:
                            dt = now - t_last
                            gbps = (8.0 * window_bytes) / dt / 1e9 if dt > 0 else 0.0
                            print(f"Rate: {gbps:.3f} Gbit/s | Total: {total} bytes", flush=True)
                            window_bytes = 0
                            t_last = now
                    continue

                for key, _ in events:
                    sock = key.fileobj  # the conn
                    try:
                        n = sock.recv_into(view)
                    except BlockingIOError:
                        continue
                    except ConnectionResetError:
                        print("Connection reset by peer.", flush=True)
                        return

                    if n == 0:
                        print("Peer closed connection (EOF).", flush=True)
                        return

                    # Write only received bytes (no copy: memoryview slice)
                    f.write(view[:n])

                    total += n
                    window_bytes += n

                    if fsync_each:
                        # Usually VERY expensive; only enable if you truly need it.
                        f.flush()
                        os.fsync(f.fileno())

                # Rate logging (rate-limited)
                if log_rate_s > 0:
                    now = time.monotonic()
                    if now - t_last >= log_rate_s:
                        dt = now - t_last
                        gbps = (8.0 * window_bytes) / dt / 1e9 if dt > 0 else 0.0
                        print(f"Rate: {gbps:.3f} Gbit/s | Total: {total} bytes", flush=True)
                        window_bytes = 0
                        t_last = now

    finally:
        try:
            conn.close()
        except Exception:
            pass
        try:
            lsock.close()
        except Exception:
            pass

def main() -> int:
    ap = argparse.ArgumentParser(description="CPU-efficient TCP sink to binary file")
    ap.add_argument("--bind", default="0.0.0.0", help="Bind IP (default: 0.0.0.0)")
    ap.add_argument("--port", type=int, required=True, help="Listen port")
    ap.add_argument("--out", required=True, help="Output binary file path")
    ap.add_argument("--recv-chunk", type=int, default=1 << 20,
                    help="Bytes per recv_into (default: 1 MiB)")
    ap.add_argument("--file-buffer", type=int, default=8 << 20,
                    help="Buffered file write size (default: 8 MiB)")
    ap.add_argument("--rcvbuf", type=int, default=16 << 20,
                    help="SO_RCVBUF in bytes (default: 16 MiB)")
    ap.add_argument("--backlog", type=int, default=256, help="listen() backlog (default: 256)")
    ap.add_argument("--log-rate", type=float, default=2.0,
                    help="Log avg throughput every N seconds; 0 disables (default: 2)")
    ap.add_argument("--fsync-each", action="store_true",
                    help="fsync after every write (VERY slow; usually disable)")
    args = ap.parse_args()

    # Ensure output directory exists
    out_dir = os.path.dirname(os.path.abspath(args.out))
    if out_dir and not os.path.isdir(out_dir):
        os.makedirs(out_dir, exist_ok=True)

    serve_once(
        bind_ip=args.bind,
        port=args.port,
        out_path=args.out,
        recv_chunk=args.recv_chunk,
        file_buffer=args.file_buffer,
        rcvbuf=args.rcvbuf,
        backlog=args.backlog,
        log_rate_s=args.log_rate,
        fsync_each=args.fsync_each,
    )
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
