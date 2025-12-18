#!/bin/bash

SRV_BINARY="./server"
CLI_BINARY="./client"
SOCKET_PATH="/tmp/socket_solodkin_v1"

# Функция очистки
teardown() {
    echo "Остановка всех компонентов..."
    # Убиваем процессы, если они запущены
    [ -n "$SRV_PID" ] && kill $SRV_PID 2>/dev/null
    [ -n "$CLI_1_PID" ] && kill $CLI_1_PID 2>/dev/null
    [ -n "$CLI_2_PID" ] && kill $CLI_2_PID 2>/dev/null
    [ -n "$CLI_3_PID" ] && kill $CLI_3_PID 2>/dev/null
    [ -n "$CLI_4_PID" ] && kill $CLI_4_PID 2>/dev/null
    [ -n "$CLI_5_PID" ] && kill $CLI_5_PID 2>/dev/null
    
    # Ждем завершения
    wait 2>/dev/null
    
    # Удаляем сокет
    rm -f "$SOCKET_PATH"
    echo "Готово."
    exit 0
}

trap teardown SIGINT SIGTERM

echo "Запуск серверного процесса..."
$SRV_BINARY &
SRV_PID=$!

# Ожидание появления сокета
echo "Ожидание создания сокета $SOCKET_PATH..."
for attempt in {1..20}; do
    if [ -S "$SOCKET_PATH" ]; then
        break
    fi
    sleep 0.1
done

if [ ! -S "$SOCKET_PATH" ]; then
    echo "Критическая ошибка: сокет не создан за отведенное время"
    kill $SRV_PID 2>/dev/null
    exit 1
fi

echo "Сервер активен. Запуск клиентских потоков..."

# Клиент 1 — интервал 0.5 сек
(
    while true; do
        echo "Client 1 line 1"
        echo "Client 1 line 2"
        sleep 0.5
    done
) | $CLI_BINARY &
CLI_1_PID=$!

# Клиент 2 — интервал 0.7 сек
(
    while true; do
        echo "Client 2 line 1"
        echo "Client 2 line 2"
        sleep 0.7
    done
) | $CLI_BINARY &
CLI_2_PID=$!

# Клиент 3 — интервал 0.2 сек
(
    while true; do
        echo "Client 3 line 1"
        echo "Client 3 line 2"
        sleep 0.2
    done
) | $CLI_BINARY &
CLI_3_PID=$!

# Клиент 4 — интервал 0.4 сек
(
    while true; do
        echo "Client 4 line 1"
        echo "Client 4 line 2"
        sleep 0.4
    done
) | $CLI_BINARY &
CLI_4_PID=$!

# Клиент 5 — интервал 0.6 сек
(
    while true; do
        echo "Client 5 line 1"
        echo "Client 5 line 2"
        sleep 0.6
    done
) | $CLI_BINARY &
CLI_5_PID=$!

echo "Все клиенты запущены. Нажмите Ctrl+C для завершения."

wait