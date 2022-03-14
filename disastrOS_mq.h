#pragma once
#include "disastrOS_pcb.h"
#include "linked_list.h"

typedef struct {
  ListItem list;
  int id;
  int priority;
  int num_messages;  // current number of messages into mq
  ListHead queues;
  ListHead descriptors;
} MessageQueue;

typedef struct {
  ListItem list;
  int priority;
  ListHead messages;
} Queue;

typedef struct {
  ListItem list;
  int text_length;
  char* text;
  //char text[MAX_MESSAGE_LENGTH];
} Message;

void MessageQueue_init();
void Queue_init();
void Message_init();
void String_init();

MessageQueue* MessageQueue_alloc(int id, int priority);
int MessageQueue_free(MessageQueue* mq);
Queue* Queue_alloc(int priority);
int Queue_free(Queue* q);
Message* Message_alloc(char* text, int text_length);
int Message_free(Message* m);
char* String_alloc(char string[]);
int String_free(char* string);

typedef ListHead MessageQueueList;
typedef ListHead QueueList;

MessageQueue* MessageQueueList_byId(MessageQueueList* l, int id);
Queue* QueueList_byPriority(QueueList* l, int priority);

void MessageQueueList_print(ListHead* l);
