import socket
import time
import select

HOST = '0.0.0.0'
PORT = 3333

def main():
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind((HOST, PORT))
sock.listen(1)
print(f"Server listening on port {PORT}...")

```
conn, addr = sock.accept()
print(f"Connection accepted from {addr}")

bytes_received = 0
start_time = time.time()

try:
    while True:
        ready_to_read, _, _ = select.select([conn], [], [], 1.0)

        if ready_to_read:
            data = conn.recv(4096)
            if not data:
                print("Connection closed by client")
                break
            bytes_received += len(data)

        elapsed = time.time() - start_time
        if elapsed >= 10.0:
            throughput = bytes_received / elapsed
            print(f"Throughput last {elapsed:.1f} seconds: {throughput:.2f} B/s")
            bytes_received = 0
            start_time = time.time()

except KeyboardInterrupt:
    print("Server manually interrupted")

finally:
    conn.close()
    sock.close()
```

if __name__ == "__main__":
main()