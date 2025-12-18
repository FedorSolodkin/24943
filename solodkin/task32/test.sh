#!/bin/bash

# Удаляем старые бинарники
rm -f server client

echo "Компиляция..."
# Для Solaris нужны -lsocket -lnsl, для AIO нужен -lrt
gcc -o server server.c -lsocket -lnsl -lrt 2>/dev/null || gcc -o server server.c -lrt
gcc -o client client.c -lsocket -lnsl 2>/dev/null || gcc -o client client.c

if [ ! -f "./server" ] || [ ! -f "./client" ]; then
    echo "Ошибка компиляции!"
    exit 1
fi

SERVER="./server"
CLIENT="./client"
SOCKET_PATH="my_socket"

# Функция завершения
cleanup() {
    echo ""
    echo "--- Завершаем работу ---"
    # Убиваем сервер
    [ -n "$SERVER_PID" ] && kill $SERVER_PID 2>/dev/null
    
    # Убиваем всех клиентов (группой)
    pkill -P $$ client 2>/dev/null
    
    wait 2>/dev/null
    rm -f "$SOCKET_PATH"
    echo "Все очищено."
    exit 0
}

trap cleanup SIGINT SIGTERM

# Удаляем старый сокет перед запуском
rm -f "$SOCKET_PATH"

# Запускаем сервер 
echo "Запуск сервера..."
$SERVER &
SERVER_PID=$!

# Ждём пока сокет появится 
echo "Ожидание сокета..."
for i in {1..50}; do
    if [ -S "$SOCKET_PATH" ]; then
        break
    fi
    sleep 0.1
done

if [ ! -S "$SOCKET_PATH" ]; then
    echo "Ошибка: сервер не создал сокет"
    kill $SERVER_PID 2>/dev/null
    exit 1
fi

echo "Сервер запущен (PID $SERVER_PID). Запускаем клиентов..."

# Клиент 1 
(
    while true; do
        echo "Client 1 line A"
        echo "Client 1 line B"
        sleep 0.5
    done
) | $CLIENT &

# Клиент 2 
(
    while true; do
        echo "Client 2 line A"
        echo "Client 2 line B"
        sleep 0.7
    done
) | $CLIENT &

# Клиент 3
(
    while true; do
        echo "Client 3 line A"
        echo "Client 3 line B"
        sleep 0.2
    done
) | $CLIENT &

# Клиент 4
(
    while true; do
        echo "Client 4 line A"
        echo "Client 4 line B"
        sleep 0.4
    done
) | $CLIENT &

# Клиент 5 
(
    while true; do
        echo "Client 5 line A"
        echo "Client 5 line B"
        sleep 0.6
    done
) | $CLIENT &

echo "Клиенты запущены. Нажмите Ctrl+C для остановки."

wait