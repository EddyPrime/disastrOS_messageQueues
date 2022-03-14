#pragma once
#include "linked_list.h"
#include "disastrOS_pcb.h"
#include "disastrOS_mq.h"


struct MQDescriptorPtr;

typedef struct MQDescriptor {
  ListItem list;
  PCB* pcb;
  MessageQueue* mq;
  int fd;
  struct MQDescriptorPtr* ptr;
} MQDescriptor;

typedef struct MQDescriptorPtr {
  ListItem list;
  ListItem DescriptorPtr;
  MQDescriptor* descriptor;
} MQDescriptorPtr;

void MQDescriptor_init();
MQDescriptor* MQDescriptor_alloc(int fd, MessageQueue* res, PCB* pcb);
int MQDescriptor_free(MQDescriptor* d);
MQDescriptor*  MQDescriptorList_byFd(ListHead* l, int fd);
void MQDescriptorList_print(ListHead* l);

MQDescriptorPtr* MQDescriptorPtr_alloc(MQDescriptor* descriptor);
int MQDescriptorPtr_free(MQDescriptorPtr* d);
void MQDescriptorPtrList_print(ListHead* l);