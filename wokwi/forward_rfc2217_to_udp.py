import serial
import socket
import configparser
from pathlib import Path

# === CONFIGURAÇÃO INICIAL ===
# Informe aqui o nome do arquivo .ini (ex.: 'config.ini')
CONFIG_FILENAME = 'wokwi.toml'

# Define caminho para o arquivo .ini na pasta anterior ao script
config_path = Path(__file__).resolve().parent / CONFIG_FILENAME


# Carrega o arquivo .ini
config = configparser.ConfigParser()
config.read(config_path)

# Lê o kitid da seção [wokwi] (padrão 0 se não existir)
kitid = config.getint('wokwi', 'rfc2217ServerPort', fallback=4000) - 4000

# Calcula a porta Teleplot: 47269 + kitid
TEP_PORT = 47269 if kitid == 0 else 47270 + kitid
# Parâmetros Teleplot
TEP_HOST = '127.0.0.1'

# === FIM DAS CONFIGURAÇÕES ===

# Conecta ao Wokwi (RFC-2217 em localhost:4000)
ser = serial.serial_for_url(
    'rfc2217://localhost:400'+str(kitid),
    baudrate=115200,
    timeout=1
)

# Flushe o buffer inicial (joga fora aquele handshake)
ser.reset_input_buffer()

# Prepara UDP para o Teleplot
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

print(f"Using config file: {config_path}")
print(f"kitid = {kitid}, forwarding RFC-2217 localhost:400{kitid} → UDP {TEP_HOST}:{TEP_PORT}")

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