#include "disastrOS_mq.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "disastrOS_descriptor.h"
#include "linked_list.h"
#include "pool_allocator.h"

#define MQ_SIZE sizeof(MessageQueue)
#define MQ_MEMSIZE (sizeof(MessageQueue) + sizeof(int))
#define MQ_BUFFER_SIZE MAX_NUM_MQS* MQ_MEMSIZE

#define QUEUE_SIZE sizeof(Queue)
#define QUEUE_MEMSIZE (sizeof(Queue) + sizeof(int))
#define QUEUE_BUFFER_SIZE MAX_NUM_QUEUES* QUEUE_MEMSIZE

#define MESSAGE_SIZE sizeof(Message)
#define MESSAGE_MEMSIZE (sizeof(Message) + sizeof(int))
#define MESSAGE_BUFFER_SIZE MAX_NUM_MESSAGES* MESSAGE_MEMSIZE

static char _mqs_buffer[MQ_BUFFER_SIZE];
static PoolAllocator _mqs_allocator;

static char _queues_buffer[QUEUE_BUFFER_SIZE];
static PoolAllocator _queues_allocator;

static char _messages_buffer[MESSAGE_BUFFER_SIZE];
static PoolAllocator _messages_allocator;

void MessageQueue_init() {
  int result = PoolAllocator_init(&_mqs_allocator, MQ_SIZE, MAX_NUM_MQS,
                                  _mqs_buffer, MQ_BUFFER_SIZE);
  assert(!result);
}

void Queue_init() {
  int result =
      PoolAllocator_init(&_queues_allocator, QUEUE_SIZE, MAX_NUM_QUEUES,
                         _queues_buffer, QUEUE_BUFFER_SIZE);
  assert(!result);
}

void Message_init() {
  int result =
      PoolAllocator_init(&_messages_allocator, MESSAGE_SIZE, MAX_NUM_MESSAGES,
                         _messages_buffer, MESSAGE_BUFFER_SIZE);
  assert(!result);
}

MessageQueue* MessageQueue_alloc(int id, int priority) {
  MessageQueue* r = (MessageQueue*)PoolAllocator_getBlock(&_mqs_allocator);
  if (!r)
    return 0;
  r->list.prev = r->list.next = 0;
  r->id = id;
  r->priority = priority;
  r->num_messages = 0;
  List_init(&r->queues);
  List_init(&r->descriptors);
  return r;
}

Queue* Queue_alloc(int priority) {
  Queue* r = (Queue*)PoolAllocator_getBlock(&_queues_allocator);
  if (!r)
    return 0;
  r->list.prev = r->list.next = 0;
  r->priority = priority;
  List_init(&r->messages);
  return r;
}

Message* Message_alloc(char* text, int text_length) {
  Message* r = (Message*)PoolAllocator_getBlock(&_messages_allocator);
  if (!r)
    return 0;
  r->text = text;
  r->list.prev = r->list.next = 0;
  r->text_length = text_length;
  return r;
}

int MessageQueue_free(MessageQueue* r) {
  assert(r->descriptors.first == 0);
  assert(r->descriptors.last == 0);
  return PoolAllocator_releaseBlock(&_mqs_allocator, r);
}

int Queue_free(Queue* r) {
  assert(r->messages.first == 0);
  assert(r->messages.last == 0);
  return PoolAllocator_releaseBlock(&_queues_allocator, r);
}

int Message_free(Message* r) {
  return PoolAllocator_releaseBlock(&_messages_allocator, r);
}

MessageQueue* MessageQueueList_byId(MessageQueueList* l, int id) {
  ListItem* aux = l->first;
  while (aux) {
    MessageQueue* r = (MessageQueue*)aux;
    if (r->id == id)
      return r;
    aux = aux->next;
  }
  return 0;
}

Queue* QueueList_byPriority(QueueList* l, int priority) {
  ListItem* aux = l->first;
  while (aux) {
    Queue* r = (Queue*)aux;
    if (r->priority == priority)
      return r;
    aux = aux->next;
  }
  return 0;
}

void Message_print(Message* r) {
  printf("[Message] length: %d ", r->text_length);
  printf("text: %s\n", r->text);
}

void MessageList_print(ListHead* l) {
  if (!l->size) {
    printf("no messages\n");
    return;
  }
  ListItem* aux = l->first;
  printf("{\n");
  while (aux) {
    Message* r = (Message*)aux;
    printf("\t");
    Message_print(r);
    if (aux->next)
      printf(",");
    printf("\n");
    aux = aux->next;
  }
  printf("}\n");
}

void Queue_print(Queue* r) {
  printf("[Priority Queue] priority: %d\n", r->priority);
  MessageList_print(&r->messages);
}

void QueueList_print(ListHead* l) {
  ListItem* aux = l->first;
  printf("{\n");
  while (aux) {
    Queue* r = (Queue*)aux;
    printf("\t");
    Queue_print(r);
    if (aux->next)
      printf(",");
    printf("\n");
    aux = aux->next;
  }
  printf("}\n");
}

void MessageQueue_print(MessageQueue* r) {
  printf("[Message Queue] id: %d, priority: %d, num_messages: %d \n", r->id,
         r->priority, r->num_messages);
  QueueList_print(&r->queues);
  DescriptorPtrList_print(&r->descriptors);
}

void MessageQueueList_print(ListHead* l) {
  ListItem* aux = l->first;
  printf("{\n");
  while (aux) {
    MessageQueue* r = (MessageQueue*)aux;
    printf("\t");
    MessageQueue_print(r);
    if (aux->next)
      printf(",");
    printf("\n");
    aux = aux->next;
  }
  printf("}\n");
}
