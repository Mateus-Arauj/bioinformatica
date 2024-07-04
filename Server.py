import socket
import threading


def handle_broadcast_client(client_socket, sequence1, sequence2, results):
    try:
        request = f"seq1:{sequence1};seq2:{sequence2}"
        client_socket.send(request.encode())
        response = client_socket.recv(4096).decode()
        results.append(response)
    except Exception as e:
        print(f"Error communicating with client: {e}")
    finally:
        client_socket.close()

def start_broadcast_server(sequence1, sequence2):
    broadcast_results = []

    def broadcast_to_clients():
        broadcast_server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        broadcast_server.bind(("0.0.0.0", 65432))
        broadcast_server.listen(5)
        print("Broadcast Server listening on port 65432")

        while len(broadcast_results) < 3:  # Esperar 3 respostas
            client_socket, addr = broadcast_server.accept()
            client_handler = threading.Thread(target=handle_broadcast_client, args=(client_socket, sequence1, sequence2, broadcast_results))
            client_handler.start()

        broadcast_server.close()

    broadcast_thread = threading.Thread(target=broadcast_to_clients)
    broadcast_thread.start()
    broadcast_thread.join()

    return broadcast_results

# Função para enviar o melhor resultado para o servidor Java
def send_best_result_to_java_server(best_result):
    java_server_ip = "127.0.0.1"  
    java_server_port = 65433      
    delimiter = "<END>"

    try:
        java_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        java_socket.connect((java_server_ip, java_server_port))
        java_socket.send((best_result + delimiter).encode())
        java_socket.close()
    except Exception as e:
        print(f"Error sending best result to Java server: {e}")


def handle_client(client_socket):
    request = client_socket.recv(1024).decode()
    print(f"Received request: {request}")
    sequences = request.split(";")
    sequence1 = sequences[0]
    sequence2 = sequences[1]


    broadcast_results = start_broadcast_server(sequence1, sequence2)

    for result in broadcast_results:
        print(f"Received from client: {result}")

    # Selecionar o melhor resultado
    def extract_score(result):
        parts = result.split(";")
        nw_score_part = next((part for part in parts if part.startswith("Needleman;AlignmentScore")), None)
        if nw_score_part is not None:
            nw_score = int(nw_score_part.split(":")[1])
            return nw_score
        return float('-inf')  # Retornar uma pontuação mínima se não encontrado

    best_result = max(broadcast_results, key=extract_score)
    print(f"Best result: {best_result}")

    send_best_result_to_java_server(best_result)

    client_socket.close()

def start_server():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind(("0.0.0.0", 65431))
    server.listen(5)
    print("Python Server listening on port 65431")

    while True:
        client_socket, addr = server.accept()
        client_handler = threading.Thread(target=handle_client, args=(client_socket,))
        client_handler.start()

if __name__ == "__main__":
    start_server()
