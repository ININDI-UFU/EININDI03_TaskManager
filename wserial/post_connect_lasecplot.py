# scripts/post_connect_lasecplot.py
# Plataforma: PlatformIO extra_scripts (post)
# Função: ler [data] kitId do platformio.ini, montar host/port e enviar CONNECT via UDP para a extensão LasecPlot (127.0.0.1:47300)

import configparser
from pathlib import Path
import socket

def _read_kit_id_from_cfg():
    """
    Lê o valor inteiro da tag 'kitId' dentro da seção [data]
    em um arquivo .ini localizado um diretório acima do script atual.
    """
    # Caminho do arquivo .ini (um diretório acima)
    ini_path = Path(__file__).resolve().parent.parent / "platformio.ini"

    # Cria o parser e lê o arquivo
    config = configparser.ConfigParser()
    config.read(ini_path, encoding="utf-8")

    # Lê o valor e converte para inteiro
    try:
        kit_id = int(config["data"]["kitId"])
        return kit_id
    except Exception as e:
        print(f"[LasecPlot] Aviso: erro ao ler kitId do platformio.ini: {e}")
        return -1

def _compute_target(kit_id: int):
    host = f"iikit{kit_id}.local"
    port = 47250 + kit_id
    return host, port

def _send_connect(host: str, port: int, control_ip="127.0.0.1", control_port=47268, timeout=0.25):
    """Envia CONNECT host:port via UDP para o controle local da extensão."""
    msg = f"CONNECT {host}:{control_port}".encode("utf-8")
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(timeout)
    try:
        sock.sendto(msg, (control_ip, port))
        print(f"[LasecPlot] CONNECT {host}:{control_port} enviado para {control_ip}:{port}")
    except Exception as e:
        # Não falha o build; apenas avisa (a extensão pode não estar ativa)
        print(f"[LasecPlot] Aviso: não consegui enviar CONNECT ({e}). A extensão está ativa?")
    finally:
        try:
            sock.close()
        except Exception:
            pass

def main():
    kit_id = _read_kit_id_from_cfg()
    host, port = _compute_target(kit_id)
    _send_connect(host, port)

# Executa automaticamente como post-script
main()
