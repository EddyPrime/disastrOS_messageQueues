#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "disastrOS.h"
#include "disastrOS_mq.h"
#include "disastrOS_mqdescriptor.h"
#include "disastrOS_syscalls.h"

#define PID disastrOS_getpid()

/*
mqOpen opens the message queue identified by id and gives it the ability
to manage |priority| priority queues. Priorities will go from 0 (maximum) to
priority (minimum). With non flags specified, if the message queue exists, it
will be opened, otherwise, an error occurs. Following flags are allowed
- if DSOS_CREATE is specified and the message queue does not exist, the message
queue will be created;
- if DSOS_EXCL is specified along with DSOS_CREATE, the creation of the message
queue il be forced and, if it exists, an error occurs. If DSOS_EXCL is specified
without DSOS_CREATE, an error occurs.
Upon success, a descriptor of the message queue for the process is created and
the matching file descriptor is returned. Upon error, a negative integer is
returned.
*/

// int id, int flags, int priority

void internal_mqOpen() {
  
  int id = running->syscall_args[0];
  int flags = running->syscall_args[1];
  int priority = running->syscall_args[2];

  // checking if process is too hungry

  if (running->last_mq_fd + 1 > MAX_NUM_MQDESCRIPTORS_PER_PROCESS) {
    disastrOS_debug("[%d:MQOPEN] too many semDescriptors for me\n", PID);
    running->syscall_retvalue = DSOS_EMQOPEN;
    return;
  }

  // checking arguments consistency

  if (id < 0) {
    disastrOS_debug("[%d:MQOPEN] id out of bounds\n", PID);
    running->syscall_retvalue = DSOS_EMQOPEN;
    return;
  }

  // fetching mq associated to id
  MessageQueue* mq = MessageQueueList_byId(&messageQueues_list, id);

  // looking for flags
  if ((flags & DSOS_CREATE)) {
    // if DSOS_CREATE and semaphore does not exist, it must be created
    disastrOS_debug("[%d:MQOPEN] DSOS_CREATE has been set\n", PID);

    if (mq && (flags & DSOS_EXCL)) {
      // if DSOS_EXCL and mq exists, error

      disastrOS_debug(
          "[%d:MQOPEN] DSOS_EXCL has been set but sem already exists\n", PID);
      running->syscall_retvalue = DSOS_EMQOPEN;
      return;
    } else if (!mq) {
      // mq does not exist
      // it must be allocated and put in messagequeues_list

      disastrOS_debug("[%d:MQOPEN] mq does not exist\n", PID);

      if (messageQueues_list.size + 1 > MAX_NUM_MQS) {
        disastrOS_debug("[%d:MQOPEN] mq number out of bound\n", PID);
        running->syscall_retvalue = DSOS_EMQOPEN;
        return;
      }

      if (priority < 0 || priority > MAX_NUM_QUEUES) {
        disastrOS_debug("[%d:MQOPEN] negative priority\n", PID);
        running->syscall_retvalue = DSOS_EMQOPEN;
        return;
      }

      mq = MessageQueue_alloc(id, priority);

      if (!mq) {
        disastrOS_debug("[%d:MQOPEN] mq has not been allocated\n", PID);
        running->syscall_retvalue = DSOS_EMQOPEN;
        return;
      }

      int i;
      Queue* q;
      disastrOS_debug("[%d:MQOPEN] inserting %d queue in mq %d\n", PID,
                      priority, mq->id);
      for (i = 0; i <= priority; i++) {
        q = Queue_alloc(i);
        if (!q) {
          disastrOS_debug("[%d:MQOPEN] q %d has not been allocated in mq %d\n",
                          PID, i, id);
          running->syscall_retvalue = DSOS_EMQOPEN;
          return;
        }
          disastrOS_debug("[%d:MQOPEN] mq %d welcomes q %d\n",
                          PID, id, i);
      List_insert(&mq->queues, messageQueues_list.last, (ListItem*)q);
      }

      disastrOS_debug("[%d:MQOPEN] inserting mq %d in messageQueues_list\n",
                      PID, mq->id);
      List_insert(&messageQueues_list, messageQueues_list.last, (ListItem*)mq);
    }
  }

  // so tiny so big
  if (!mq) {
    disastrOS_debug("[%d:MQOPEN] mq has not been allocated\n", PID);
    running->syscall_retvalue = DSOS_EMQOPEN;
    return;
  }

  // having the mq, it's time to alloc descriptors
  disastrOS_debug("[%d:MQOPEN] allocating descriptor for mq %d\n", PID, mq->id);
  MQDescriptor* mqDes = MQDescriptor_alloc(running->last_mq_fd, mq, running);

  if (!mqDes) {
    disastrOS_debug("[%d:MQOPEN] mqDes has not been allocated\n", PID);
    running->syscall_retvalue = DSOS_EMQOPEN;
    return;
  }

  disastrOS_debug(
      "[%d:MQOPEN] allocating descriptor_ptr for mq %d descriptor\n", PID,
      mq->id);
  MQDescriptorPtr* mqDesPtr = MQDescriptorPtr_alloc(mqDes);

  if (!mqDesPtr) {
    disastrOS_debug("[%d:MQOPEN] mqDesPtr has not been allocated\n", PID);
    running->syscall_retvalue = DSOS_EMQOPEN;
    return;
  }

  mqDes->ptr = mqDesPtr;
  List_insert(&running->mq_descriptors, running->mq_descriptors.last,
              (ListItem*)mqDes);
  List_insert(&mq->descriptors, mq->descriptors.last, (ListItem*)mqDesPtr);
  running->last_mq_fd++;
  disastrOS_debug("[%d:MQOPEN] tutt appo\n", PID);
  running->syscall_retvalue = running->last_mq_fd - 1;
}
