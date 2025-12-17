#!/bin/bash

SRV_EXE="./server"
CLI_EXE="./client"

SOCKET_FILE="my_socket"

final_cleanup() {
    echo "Выполняется финальная очистка..."
    kill $PID_SRV 2>/dev/null
    kill $PID_CLI1 2>/dev/null
    kill $PID_CLI2 2>/dev/null
    kill $PID_CLI3 2>/dev/null
    kill $PID_CLI4 2>/dev/null
    kill $PID_CLI5 2>/dev/null
    wait $PID_SRV $PID_CLI1 $PID_CLI2 $PID_CLI3 $PID_CLI4 $PID_CLI5 2>/dev/null
    rm -f "$SOCKET_FILE"
    exit 0
}

trap final_cleanup SIGINT SIGTERM

echo "Инициализация сервера..."
$SRV_EXE &
PID_SRV=$!

# Ожидание появления файла сокета
for try_count in {1..20}; do
    if [ -S "$SOCKET_FILE" ]; then
        break
    fi
    sleep 0.1
done

if [ ! -S "$SOCKET_FILE" ]; then
    echo "Сбой: сокет не обнаружен"
    kill $PID_SRV 2>/dev/null
    exit 1
fi

echo "Сервер готов. Запуск клиентских потоков..."

# Клиент 1 — 0.5 сек
( while true; do echo "Client 1"; echo "second line of 1"; sleep 0.5; done ) | $CLI_EXE &
PID_CLI1=$!

# Клиент 2 — 0.7 сек
( while true; do echo "Client 2"; echo "second line of 2"; sleep 0.7; done ) | $CLI_EXE &
PID_CLI2=$!

# Клиент 3 — 0.2 сек
( while true; do echo "Client 3"; echo "second line of 3"; sleep 0.2; done ) | $CLI_EXE &
PID_CLI3=$!

# Клиент 4 — 0.4 сек → ИСПРАВЛЕНО: был дубль CLIENT3_PID
( while true; do echo "Client 4"; echo "second line of 4"; sleep 0.4; done ) | $CLI_EXE &
PID_CLI4=$!

# Клиент 5 — 0.6 сек
( while true; do echo "Client 5"; echo "second line of 5"; sleep 0.6; done ) | $CLI_EXE &
PID_CLI5=$!

echo "Клиенты активны. Для остановки нажмите Ctrl+C."

wait