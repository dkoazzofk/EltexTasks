#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int value;
    int priority;
}PQElement;

typedef struct {
    PQElement* elements;
    int size;
    int capacity;
}PriorityQueue;

void initQueue(PriorityQueue* pq, int capacity){
    pq->capacity = (PQElement*)malloc(sizeof(PQElement) * capacity);
    pq->size = 0;
    pq->capacity = capacity;
}

void insert(PriorityQueue* pq, int value, int priority){
    if(pq->size >= pq->capacity){
        printf("Queue is full.\n");
        return;
    }
    pq->elements[pq->size].value = value;
    pq->elements[pq->size].priority = priority;
    pq->size++;

    for(int i = pq->size - 1; i > 0; i--){
        if(pq->elements[i].priority > pq->elements[i - 1].priority){
            PQElement temp = pq->elements[i];
            pq->elements[i] = pq->elements[i - 1];
            pq->elements[i - 1] = temp;
        } else {
            break;
        }
    }
}

PQElement takeMax(PriorityQueue* pq){
    if(pq->size = 0){
        printf("Queue is empty.\n");
        PQElement empty = {0,0};
        return empty;
    }
    PQElement max = pq->elements[0];

    for(int i = 1; i < pq->size; i++){
        pq->elements[i - 1] = pq->elements[i];
    }
    pq->size--;

    return max;
}


int main(){
    PriorityQueue pq;
    initQueue(&pq, 10);

    insert(&pq, 20, 16);
    insert(&pq, 72, 20);
    insert(&pq, 20, 13);
    insert(&pq, 20, 12);
    insert(&pq, 20, 1);
    insert(&pq, 20, 4);

    PQElement max = takeMax(&pq);
    printf("Value: %d, Priority: %d\n", max.value, max.priority);

    free(pq.elements);
    return 0;
}