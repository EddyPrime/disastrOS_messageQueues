#include <assert.h>
#include <stdio.h>
#include "disastrOS_mqdescriptor.h"
#include "pool_allocator.h"
#include "disastrOS_constants.h"

#define MQDESCRIPTOR_SIZE sizeof(MQDescriptor)
#define MQDESCRIPTOR_MEMSIZE (sizeof(MQDescriptor)+sizeof(int))
#define MAX_NUM_MQDESCRIPTORS (MAX_NUM_MQDESCRIPTORS_PER_PROCESS*MAX_NUM_PROCESSES)
#define MQDESCRIPTOR_BUFFER_SIZE MAX_NUM_MQDESCRIPTORS*MQDESCRIPTOR_MEMSIZE

#define MQDESCRIPTORPTR_SIZE sizeof(MQDescriptorPtr)
#define MQDESCRIPTORPTR_MEMSIZE (sizeof(MQDescriptorPtr)+sizeof(int))
#define MQDESCRIPTORPTR_BUFFER_SIZE MAX_NUM_MQDESCRIPTORS*MQDESCRIPTORPTR_MEMSIZE

static char _mq_descriptor_buffer[MQDESCRIPTOR_BUFFER_SIZE];
static PoolAllocator _mq_descriptor_allocator;

static char _mq_descriptor_ptr_buffer[MQDESCRIPTORPTR_BUFFER_SIZE];
static PoolAllocator _mq_descriptor_ptr_allocator;

void MQDescriptor_init(){
  int result=PoolAllocator_init(& _mq_descriptor_allocator,
				MQDESCRIPTOR_SIZE,
				MAX_NUM_PROCESSES,
				_mq_descriptor_buffer,
				MQDESCRIPTOR_BUFFER_SIZE);
  assert(! result);

  result=PoolAllocator_init(& _mq_descriptor_ptr_allocator,
			    MQDESCRIPTORPTR_SIZE,
			    MAX_NUM_PROCESSES,
			    _mq_descriptor_ptr_buffer,
			    MQDESCRIPTORPTR_BUFFER_SIZE);
  assert(! result);
}

MQDescriptor* MQDescriptor_alloc(int fd, MessageQueue* res, PCB* pcb) {
  MQDescriptor* d=(MQDescriptor*)PoolAllocator_getBlock(&_mq_descriptor_allocator);
  if (!d)
    return 0;
  d->list.prev=d->list.next=0;
  d->fd=fd;
  d->mq=res;
  d->pcb=pcb;
  return d;
}

int MQDescriptor_free(MQDescriptor* d) {
  return PoolAllocator_releaseBlock(&_mq_descriptor_allocator, d);
}

MQDescriptor*  MQDescriptorList_byFd(ListHead* l, int fd){
  ListItem* aux=l->first;
  while(aux){
    MQDescriptor* d=(MQDescriptor*)aux;
    if (d->fd==fd)
      return d;
    aux=aux->next;
  }
  return 0;
}

MQDescriptorPtr* MQDescriptorPtr_alloc(MQDescriptor* descriptor) {
  MQDescriptorPtr* d=PoolAllocator_getBlock(&_mq_descriptor_ptr_allocator);
  if (!d)
    return 0;
  d->list.prev=d->list.next=0;
  d->descriptor=descriptor;
  return d;
}

int MQDescriptorPtr_free(MQDescriptorPtr* d){
  return PoolAllocator_releaseBlock(&_mq_descriptor_ptr_allocator, d);
}

void MQDescriptorList_print(ListHead* l){
  ListItem* aux=l->first;
  printf("[");
  while(aux){
    MQDescriptor* d=(MQDescriptor*)aux;
    printf("(fd: %d, rid:%d)",
	   d->fd,
	   d->mq->id);
    if(aux->next)
      printf(", ");
    aux=aux->next;
  }
  printf("]");
}


void MQDescriptorPtrList_print(ListHead* l){
  ListItem* aux=l->first;
  printf("[");
  while(aux){
    MQDescriptorPtr* d=(MQDescriptorPtr*)aux;
    printf("(pid: %d, fd: %d, rid:%d)",
	   d->descriptor->fd,
	   d->descriptor->pcb->pid,
	   d->descriptor->mq->id);
    if(aux->next)
      printf(", ");
    aux=aux->next;
  }
  printf("]");
}
