"""
Script pós-upload para o PlatformIO
-----------------------------------

Este script é executado automaticamente **após o upload do firmware** para o microcontrolador.

Funções principais:
1. Ler o valor `kitId` do arquivo `platformio.ini`.
2. Calcular o host e porta do dispositivo com base nesse ID.
3. Enviar um comando UDP de controle para a extensão LasecPlot (rodando em 127.0.0.1),
   usando o protocolo de UI baseado em prefixo ':':

       :CONNECT:<host>:<porta_do_dispositivo>\n

Autor: Josué Morais (adaptado e documentado)
"""

from pathlib import Path          # Manipulação de caminhos
import configparser               # Leitura de arquivos .ini
import socket                     # Comunicação UDP
from SCons.Script import Import   # Acessar o objeto `env` do PlatformIO

# Importa o ambiente de build do PlatformIO (variável `env`)
Import("env")


# -------------------------------------------------------------------
# 1. Função para ler o valor kitId do platformio.ini
# -------------------------------------------------------------------
def _read_kit_id_from_cfg():
    """
    Lê o valor inteiro 'kitId' da seção [data] no arquivo platformio.ini.
    Retorna o kitId como inteiro, ou -1 se houver erro.
    """

    ini_path = Path(env["PROJECT_DIR"]) / "platformio.ini"

    config = configparser.ConfigParser()
    config.read(ini_path, encoding="utf-8")

    try:
        kit_id = int(config["data"]["kitId"])
        return kit_id
    except Exception as e:
        print(f"[LasecPlot] Aviso: erro ao ler kitId do platformio.ini: {e}")
        return -1


# -------------------------------------------------------------------
# 2. Função para calcular host e porta do dispositivo
# -------------------------------------------------------------------
def _compute_target(kit_id: int):
    """
    Usa o kitId para montar o nome de host e a porta do dispositivo.
    Exemplo: se kit_id = 3 → host = 'iikit3.local', device_port = 47253
    """
    host = f"iikit{kit_id}.local"
    device_port = 47250 + kit_id
    return host, device_port


# -------------------------------------------------------------------
# 3. Função para enviar o comando de controle via UDP
# -------------------------------------------------------------------
def _send_connect(
    host: str,
    device_port: int,
    control_ip: str = "127.0.0.1",
    vscode_udp_port: int = 47268,
    timeout: float = 0.25,
):
    """
    Envia para a extensão LasecPlot (rodando no PC) uma mensagem UDP
    usando o protocolo de controle de UI (prefixo ':').

    Formato da mensagem:

        :CONNECT:<host>:<device_port>\n

    Exemplo:

        :CONNECT:iikit3.local:47253
    """

    # Monta a mensagem no formato combinado com a webview
    msg_str = f":CONNECT:{host}:{device_port}\n"
    msg = msg_str.encode("utf-8")

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(timeout)

    try:
        # Envia para o servidor UDP da extensão (porta fixa do VSCode)
        sock.sendto(msg, (control_ip, vscode_udp_port))
        print(
            f"[LasecPlot] '{msg_str.strip()}' enviado para "
            f"{control_ip}:{vscode_udp_port}"
        )

    except Exception as e:
        print(f"[LasecPlot] Aviso: não consegui enviar comando de controle ({e}).")

    finally:
        sock.close()


# -------------------------------------------------------------------
# 4. Função que será executada automaticamente após o upload
# -------------------------------------------------------------------
def post_upload_action(source, target, env):  # type: ignore[override]
    """
    Essa função é chamada pelo PlatformIO **depois que o upload termina com sucesso**.
    """

    print("[LasecPlot] Executando pós-upload...")

    # 1. Lê o kitId do platformio.ini
    kit_id = _read_kit_id_from_cfg()
    if kit_id < 0:
        print("[LasecPlot] kitId inválido. Ignorando envio de comando.")
        return

    # 2. Calcula host e porta do dispositivo
    host, device_port = _compute_target(kit_id)

    # 3. Envia o comando de controle para a extensão
    _send_connect(host, device_port)

    print("[LasecPlot] Finalizado pós-upload com sucesso.")


# -------------------------------------------------------------------
# 5. Registro da função no sistema do PlatformIO
# -------------------------------------------------------------------
env.AddPostAction("upload", post_upload_action)