#!/bin/bash

SRV_BINARY="./server"
CLI_BINARY="./client"

SOCKET_PATH="/tmp/uppercase_socket"
teardown() {
    echo "Остановка всех компонентов..."
    kill $SRV_PID 2>/dev/null
    kill $CLI_1_PID 2>/dev/null
    kill $CLI_2_PID 2>/dev/null
    kill $CLI_3_PID 2>/dev/null
    kill $CLI_4_PID 2>/dev/null
    kill $CLI_5_PID 2>/dev/null
    wait $SRV_PID $CLI_1_PID $CLI_2_PID $CLI_3_PID $CLI_4_PID $CLI_5_PID 2>/dev/null
    rm -f "$SOCK_NAME"
    exit 0
}

trap teardown SIGINT SIGTERM

echo "Запуск серверного процесса..."
$SRV_BINARY &
SRV_PID=$!

# Ожидание появления сокета
for attempt in {1..20}; do
    if [ -S "$SOCK_NAME" ]; then
        break
    fi
    sleep 0.1
done

if [ ! -S "$SOCK_NAME" ]; then
    echo "Критическая ошибка: сокет не создан"
    kill $SRV_PID 2>/dev/null
    exit 1
fi

echo "Сервер активен. Запуск клиентских потоков..."

# Клиент 1 — интервал 0.5 сек
(
    while true; do
        echo "Client 1"
        echo "second line of 1"
        sleep 0.5
    done
) | $CLI_BINARY &
CLI_1_PID=$!

# Клиент 2 — интервал 0.7 сек
(
    while true; do
        echo "Client 2"
        echo "second line of 2"
        sleep 0.7
    done
) | $CLI_BINARY &
CLI_2_PID=$!

# Клиент 3 — интервал 0.2 сек
(
    while true; do
        echo "Client 3"
        echo "second line of 3"
        sleep 0.2
    done
) | $CLI_BINARY &
CLI_3_PID=$!

# Клиент 4 — интервал 0.4 сек
(
    while true; do
        echo "Client 4"
        echo "second line of 4"
        sleep 0.4
    done
) | $CLI_BINARY &
CLI_4_PID=$!

# Клиент 5 — интервал 0.6 сек
(
    while true; do
        echo "Client 5"
        echo "second line of 5"
        sleep 0.6
    done
) | $CLI_BINARY &
CLI_5_PID=$!

echo "Все клиенты запущены. Нажмите Ctrl+C для завершения."

wait