import serial
import socket

# Conecta ao Wokwi (RFC-2217 em localhost:4000)
ser = serial.serial_for_url(
    'rfc2217://localhost:4000',
    baudrate=115200,
    timeout=1
)

# Flushe o buffer inicial (joga fora aquele handshake)
ser.reset_input_buffer()

# Prepara UDP para o Teleplot
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
TEP_HOST = '127.0.0.1'
TEP_PORT = 47269

print(f"Forwarding RFC-2217 localhost:4000 → UDP {TEP_HOST}:{TEP_PORT}")

try:
    while True:
        line = ser.readline()  # lê até '\n'
        if not line:
            continue
        # filtra: só linhas que começam com '>'
        if not line.startswith(b'>'):
            continue
        # remove o '>' inicial (o Teleplot UDP não precisa dele)
        payload = line.lstrip(b'>')
        sock.sendto(payload, (TEP_HOST, TEP_PORT))
except KeyboardInterrupt:
    print("Stopping proxy.")
finally:
    ser.close()
    sock.close()