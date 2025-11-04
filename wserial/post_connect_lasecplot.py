"""
Script pós-upload para o PlatformIO
-----------------------------------

Este script é executado automaticamente **após o upload do firmware** para o microcontrolador.

Funções principais:
1. Ler o valor `kitId` do arquivo `platformio.ini`.
2. Calcular o host e porta de destino com base nesse ID.
3. Enviar um comando UDP "CONNECT" para a extensão LasecPlot (127.0.0.1).

Autor: Josué Morais (adaptado e documentado)
"""

# Importa bibliotecas padrão
from pathlib import Path          # Para manipular caminhos de arquivos de forma segura
import configparser               # Para ler arquivos .ini
import socket                     # Para comunicação via UDP
from SCons.Script import Import   # Necessário para acessar o objeto `env` do PlatformIO

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

    # Caminho absoluto do arquivo platformio.ini (na raiz do projeto)
    ini_path = Path(env["PROJECT_DIR"]) / "platformio.ini"

    # Cria o parser e tenta ler o arquivo INI
    config = configparser.ConfigParser()
    config.read(ini_path, encoding="utf-8")

    # Tenta acessar o valor [data] → kitId
    try:
        kit_id = int(config["data"]["kitId"])
        return kit_id
    except Exception as e:
        print(f"[LasecPlot] Aviso: erro ao ler kitId do platformio.ini: {e}")
        return -1


# -------------------------------------------------------------------
# 2. Função para calcular host e porta do destino
# -------------------------------------------------------------------
def _compute_target(kit_id: int):
    """
    Usa o kitId para montar o nome de host e a porta destino.
    Exemplo: se kit_id = 3 → host = 'iikit3.local', port = 47253
    """
    host = f"iikit{kit_id}.local"
    port = 47250 + kit_id
    return host, port


# -------------------------------------------------------------------
# 3. Função para enviar o comando CONNECT via UDP
# -------------------------------------------------------------------
def _send_connect(
    host: str,
    port: int,
    control_ip="127.0.0.1",
    control_port=47268,
    timeout=0.25
):
    """
    Envia a mensagem UDP:
        CONNECT <host>:<control_port>

    Exemplo de mensagem enviada:
        CONNECT iikit3.local:47268

    Essa mensagem é enviada para a extensão LasecPlot rodando no PC.
    """

    # Monta a mensagem a ser enviada (bytes UTF-8)
    msg = f"CONNECT {host}:{control_port}".encode("utf-8")

    # Cria o socket UDP
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(timeout)

    try:
        # Envia o pacote UDP para o IP e porta especificados
        sock.sendto(msg, (control_ip, port))
        print(f"[LasecPlot] CONNECT {host}:{control_port} enviado para {control_ip}:{port}")

    except Exception as e:
        # Em caso de erro (ex: extensão não está aberta), apenas avisa
        print(f"[LasecPlot] Aviso: não consegui enviar CONNECT ({e}).")

    finally:
        # Fecha o socket de forma segura
        sock.close()


# -------------------------------------------------------------------
# 4. Função que será executada automaticamente após o upload
# -------------------------------------------------------------------
def post_upload_action(source, target, env):
    """
    Essa função é chamada pelo PlatformIO **depois que o upload termina com sucesso**.
    """

    print("[LasecPlot] Executando pós-upload...")

    # 1. Lê o kitId do platformio.ini
    kit_id = _read_kit_id_from_cfg()

    # 2. Calcula o host e porta correspondentes
    host, port = _compute_target(kit_id)

    # 3. Envia o comando CONNECT via UDP
    _send_connect(host, port)

    print("[LasecPlot] Finalizado pós-upload com sucesso.")


# -------------------------------------------------------------------
# 5. Registro da função no sistema do PlatformIO
# -------------------------------------------------------------------
# Isso diz ao PlatformIO: "depois do upload, execute post_upload_action()"
env.AddPostAction("upload", post_upload_action)
