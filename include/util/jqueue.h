#ifndef __JQUEUE_H
#define __JQUEUE_H

#include <Arduino.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAXLENGTHJQUEUE
#define MAXLENGTHJQUEUE 5
#endif

typedef struct {
    void (*buffer[MAXLENGTHJQUEUE])(); // Ponteiro para o buffer da fila
    uint8_t head; // Índice do próximo item a ser lido
    uint8_t tail; // Índice do próximo espaço para escrita
    uint8_t count; // Número atual de itens na fila
} jQueue_t;

typedef jQueue_t* jQueueHandle_t;

// Função para criar uma fila
bool jQueueCreate(jQueueHandle_t queue) {
    if (queue == nullptr) return false;
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    return true;
}

// Função para enviar um item para a fila a partir de uma ISR
bool jQueueSendFromISR(jQueueHandle_t xQueue, void (*pvItemToQueue)()) {
    if (xQueue->count == MAXLENGTHJQUEUE) return false;
    noInterrupts();
    xQueue->buffer[xQueue->tail] = pvItemToQueue;
    xQueue->tail = (xQueue->tail + 1) % MAXLENGTHJQUEUE;
    xQueue->count++;
    interrupts();
    return true;
}

// Função para receber um item da fila
bool jQueueReceive(jQueueHandle_t xQueue, void (**pvBuffer)()) {
    if (xQueue->count == 0) return false; // Retorna false se a fila estiver vazia
    noInterrupts(); // Desativa interrupções para acesso seguro
    *pvBuffer = xQueue->buffer[xQueue->head]; // Atribui o item ao ponteiro fornecido
    xQueue->head = (xQueue->head + 1) % MAXLENGTHJQUEUE; // Incrementa o índice de leitura
    xQueue->count--; // Decrementa o contador de itens
    interrupts(); // Habilita interrupções novamente
    return true;
}

#endif
