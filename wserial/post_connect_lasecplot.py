# scripts/post_connect_lasecplot.py
# Plataforma: PlatformIO extra_scripts (post)
# Função: ler [data] kitId do platformio.ini, montar host/port e enviar CONNECT via UDP para a extensão LasecPlot (127.0.0.1:47300)

Import("env")

import socket
import sys

def _read_kit_id_from_cfg():
    """Lê kitId da seção [data] do platformio.ini via API do PlatformIO."""
    try:
        cfg = env.GetProjectConfig()
        # Se existir a seção [data] com kitId, usa; senão, fallback=0
        kit = cfg.get("data", "kitId", fallback="0")
        kit = int(str(kit).strip())
        if kit < 0:
            kit = 0
        return kit
    except Exception as e:
        print(f"[LasecPlot] Aviso: erro ao ler kitId do platformio.ini: {e}")
        return 0

def _compute_target(kit_id: int):
    host = f"inindkit{kit_id}.local"
    port = 47250 + kit_id
    return host, port

def _send_connect(host: str, port: int, control_ip="127.0.0.1", control_port=47300, timeout=0.25):
    """Envia CONNECT host:port via UDP para o controle local da extensão."""
    msg = f"CONNECT {host}:{port}".encode("utf-8")
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(timeout)
    try:
        sock.sendto(msg, (control_ip, control_port))
        print(f"[LasecPlot] CONNECT {host}:{port} enviado para {control_ip}:{control_port}")
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
