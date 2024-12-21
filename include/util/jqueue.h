/**
 * @file jqueue.h
 * @brief Biblioteca para manipulação de filas no Arduino.
 *
 * Este arquivo implementa uma estrutura de fila com suporte a operações seguras em
 * ambientes com interrupções, permitindo o uso em ISRs (Interrupt Service Routines).
 */

#ifndef __JQUEUE_H
#define __JQUEUE_H

#include <Arduino.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAXLENGTHJQUEUE
/**
 * @brief Define o tamanho máximo do buffer da fila.
 */
#define MAXLENGTHJQUEUE 5
#endif

/**
 * @struct jQueue_t
 * @brief Estrutura que representa uma fila.
 *
 * @param buffer Array de ponteiros para funções armazenadas na fila.
 * @param head Índice do próximo item a ser lido.
 * @param tail Índice do próximo espaço disponível para escrita.
 * @param count Número atual de itens na fila.
 */
typedef struct {
    void (*buffer[MAXLENGTHJQUEUE])();
    uint8_t head;
    uint8_t tail;
    uint8_t count;
} jQueue_t;

/**
 * @brief Tipo de ponteiro para uma estrutura de fila.
 */
typedef jQueue_t* jQueueHandle_t;

/**
 * @brief Inicializa uma fila.
 *
 * @param queue Ponteiro para a estrutura da fila.
 * @return true se a inicialização foi bem-sucedida, false caso contrário.
 */
bool jQueueCreate(jQueueHandle_t queue) {
    if (queue == nullptr) return false;
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    return true;
}

/**
 * @brief Adiciona um item à fila a partir de uma ISR.
 *
 * @param xQueue Ponteiro para a fila.
 * @param pvItemToQueue Ponteiro para a função a ser enfileirada.
 * @return true se o item foi adicionado com sucesso, false caso a fila esteja cheia.
 */
bool jQueueSendFromISR(jQueueHandle_t xQueue, void (*pvItemToQueue)()) {
    if (xQueue->count == MAXLENGTHJQUEUE) return false;
    noInterrupts();
    xQueue->buffer[xQueue->tail] = pvItemToQueue;
    xQueue->tail = (xQueue->tail + 1) % MAXLENGTHJQUEUE;
    xQueue->count++;
    interrupts();
    return true;
}

/**
 * @brief Remove um item da fila e o retorna via ponteiro.
 *
 * @param xQueue Ponteiro para a fila.
 * @param pvBuffer Ponteiro onde o item removido será armazenado.
 * @return true se o item foi removido com sucesso, false caso a fila esteja vazia.
 */
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