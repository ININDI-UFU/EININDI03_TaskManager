"""
Script pós-upload para o PlatformIO
-----------------------------------

Executado automaticamente após o upload do firmware.

1) Lê [data] kitId e [lasecplot] udpPort do platformio.ini
2) Calcula:
   - host = f"iikit{kitId}.local"
3) Envia para a extensão LasecPlot (127.0.0.1:udpPort):
       :CONNECT:<host>:<47268>\n
4) Atualiza AppData/Code/User/settings.json com lasecplot.udpPort = <udpPort>

Autor: Josué Morais (ajustes e correções)
"""

from pathlib import Path
import configparser
import socket
from SCons.Script import Import  # Acessar o objeto `env` do PlatformIO
import os, json, shutil, re

# Importa o ambiente de build do PlatformIO (variável `env`)
Import("env")

# --- Configuráveis ---------------------------------------------------
# CHAVE da setting da extensão (ajuste se sua extensão usar outro nome)
VSCODE_KEY_CMD_PORT = "lasecplot.udpPort"

# Porta fixa do servidor de comando da extensão (no PC)
VSCODE_CONTROL_IP = "127.0.0.1"
VSCODE_DEVICE_PORT = 47268

# Defaults sensatos
DEFAULT_KIT_ID = 0
DEFAULT_UDP_PORT = 47269

# --- Utilidades JSONC ------------------------------------------------
def _strip_jsonc(text: str) -> str:
    # remove // e /* */ preservando strings
    def _repl(m):
        s = m.group(0)
        return "" if s.startswith("/") else s
    pattern = r'("(?:\\.|[^"\\])*")|(/\*[\s\S]*?\*/|//[^\n\r]*)'
    return re.sub(pattern, lambda m: m.group(1) or "", text)

def _load_jsonc(path: Path) -> dict:
    if not path.exists():
        return {}
    try:
        return json.loads(_strip_jsonc(path.read_text(encoding="utf-8")) or "{}")
    except Exception:
        return {}

def _ensure_dir(p: Path):
    p.mkdir(parents=True, exist_ok=True)

# --- 1) Leitura do platformio.ini -----------------------------------
def _read_cfg(project_dir: Path):
    """
    Retorna (kit_id:int, cmd_udp_port:int) a partir do platformio.ini
    - kitId em [data]
    - udpPort em [lasecplot]
    """
    ini_path = project_dir / "platformio.ini"
    cfg = configparser.ConfigParser()
    cfg.read(ini_path, encoding="utf-8")

    # kitId
    kit_id = DEFAULT_KIT_ID
    if cfg.has_section("data") and cfg.has_option("data", "kitId"):
        try:
            kit_id = cfg.getint("data", "kitId")
        except ValueError:
            print("[LasecPlot] Aviso: 'kitId' inválido, usando 0.")

    # udpPort (porta base de comando/config do projeto/extensão)
    udp_port = DEFAULT_UDP_PORT
    if cfg.has_section("lasecplot") and cfg.has_option("lasecplot", "udpPort"):
        try:
            udp_port = cfg.getint("lasecplot", "udpPort")
        except ValueError:
            print(f"[LasecPlot] Aviso: 'udpPort' inválido, usando {DEFAULT_UDP_PORT}.")

    return kit_id, udp_port

# --- 2) Host do dispositivo -----------------------------------------
def _compute_host(kit_id: int) -> str:
    # Ajuste o prefixo se quiser "inindkit" em vez de "iikit"
    return f"iikit{kit_id}.local"

# --- 3) Envio do CONNECT --------------------------------------------
def _send_connect(host: str, control_port : int, control_ip: str = VSCODE_CONTROL_IP,
                  device_port: int = VSCODE_DEVICE_PORT, timeout: float = 0.25):
    msg_str = f":CONNECT:{host}:{device_port}\n"
    data = msg_str.encode("utf-8")

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(timeout)
    try:
        sock.sendto(data, (control_ip, control_port))
        print(f"[LasecPlot] '{msg_str.strip()}' enviado para {control_ip}:{control_port}")
    except Exception as e:
        print(f"[LasecPlot] Aviso: falha ao enviar comando de controle ({e}).")
    finally:
        sock.close()

# --- 4) Atualiza settings do User Setting -------------------------------
def _update_user_settings_udp_port(project_dir: Path, udp_port: int):
    vscode_dir = project_dir / ".vscode"
    appdata = Path(os.environ.get("APPDATA", ""))
    settings_path = appdata / "Code" / "User" / "settings.json"
    _ensure_dir(vscode_dir)

    current = _load_jsonc(settings_path)
    # >>> Se você quiser gravar a PORTA DO DISPOSITIVO nas settings,
    # troque 'udp_port' por 'device_port' onde você chamar esta função.
    if current.get(VSCODE_KEY_CMD_PORT) != udp_port:
        if settings_path.exists():
            shutil.copy2(settings_path, settings_path.with_suffix(".settings.json.bak"))
        current[VSCODE_KEY_CMD_PORT] = udp_port
        settings_path.write_text(json.dumps(current, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
        print(f"[LasecPlot] {VSCODE_KEY_CMD_PORT} atualizado para {udp_port} em {settings_path}")
    else:
        print(f"[LasecPlot] {VSCODE_KEY_CMD_PORT} já está em {udp_port}; nada a fazer.")

# --- 5) Hook pós-upload ---------------------------------------------
def post_upload_action(source, target, env):  # type: ignore[override]
    print("[LasecPlot] Executando pós-upload...")

    project_dir = Path(env["PROJECT_DIR"])
    kit_id, udp_port = _read_cfg(project_dir)

    host = _compute_host(kit_id)

    # Atualiza settings do workspace com a porta base de comando
    _update_user_settings_udp_port(project_dir, udp_port)

    # Envia CONNECT para a extensão (no PC)
    _send_connect(host, udp_port)

    print("[LasecPlot] Finalizado pós-upload com sucesso.")

# Registro do hook
env.AddPostAction("upload", post_upload_action)