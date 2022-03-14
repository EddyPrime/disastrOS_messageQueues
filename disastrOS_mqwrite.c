#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "disastrOS.h"
#include "disastrOS_mq.h"
#include "disastrOS_mqdescriptor.h"
#include "disastrOS_syscalls.h"

#define PID disastrOS_getpid()

/*
mqWrite puts a message in the queue with priority priority of the message queue
related to fd. If the priority is not a valid one the process will be punished
and the system call will throw the message down to the last priority. Notice:
the biggest priority is always 0 while the highest priority is the one shown in
the message queue struct. Moreovre, each queue is a queue, so it applies a FIFO
policy. Current number of messages stored into the message queue will be
increased by one. If this icrease makes the message queue overcome the maximum
number of messages allowed, an error occurs. If buf_size overcomes the maximum
size allowed for a message, an error occurs. If buf has something wrong, an
error occurs. Upon success, the effective priority related to the queue the
message has been inserted in is returned. Upon error, a negative integer is
returned and the message queue is not affected at all.
*/

void internal_mqWrite() {

  int fd = running->syscall_args[0];
  char* buf = (char*)running->syscall_args[1];
  int buf_size = running->syscall_args[2];
  int priority = running->syscall_args[3];

  disastrOS_debug("[%d:MQWRITE] building message with %s\n", PID, buf);

  // check total num messages over any message queue
  if (0) {
    disastrOS_debug("[%d:MQWRITE] fd out of bounds\n", PID);
    running->syscall_retvalue = DSOS_EMQWRITE;
    return;
  }

  // maybe not necessary
  if (fd < 0 || fd > MAX_NUM_MQDESCRIPTORS_PER_PROCESS) {
    disastrOS_debug("[%d:MQWRITE] fd out of bounds\n", PID);
    running->syscall_retvalue = DSOS_EMQWRITE;
    return;
  }

  if (!buf) {
    disastrOS_debug("[%d:MQWRITE] buf null\n", PID);
    running->syscall_retvalue = DSOS_EMQWRITE;
    return;
  }

  if (buf_size < 0 || buf_size > MAX_MESSAGE_LENGTH) {
    disastrOS_debug("[%d:MQWRITE] buf_size out of bounds\n", PID);
    running->syscall_retvalue = DSOS_EMQWRITE;
    return;
  }

  // fetching mqDescriptor associated to fd
  MQDescriptor* mqDes = MQDescriptorList_byFd(&running->mq_descriptors, fd);

  if (!mqDes) {
    disastrOS_debug("[%d:MQWRITE] no mqDes found\n", PID);
    running->syscall_retvalue = DSOS_EMQWRITE;
    return;
  }

  // retrieving mq from mqDescriptor
  MessageQueue* mq = mqDes->mq;

  if (!mq) {
    disastrOS_debug("[%d:MQWRITE] no mq associated with mqDes\n", PID);
    running->syscall_retvalue = DSOS_EMQWRITE;
    return;
  }

  if (mq->num_messages + 1 > MAX_NUM_MESSAGES_PER_MQ) {
    disastrOS_debug("[%d:MQWRITE] mq %d can receive no more messages\n", PID,
                    mq->id);
    running->syscall_retvalue = 0;
    return;
  }

  if (priority < 0 || priority > mq->priority - 1) {
    disastrOS_debug(
        "[%d:MQWRITE] mq %d has normalized priority from %d to %d\n", PID,
        mq->id, priority, mq->priority - 1);
    priority = mq->priority;
  }

  // as FIFO policy, mqwrite inserts messages from the tails
  Queue* q = (Queue*)QueueList_byPriority(&mq->queues, priority);

  if (!q) {
    disastrOS_debug(
        "[%d:MQWRITE] could not retrieve queue with priority %d from mq %d\n",
        PID, priority, mq->id);
    running->syscall_retvalue = DSOS_EMQWRITE;
    return;
  }

  Message* m = Message_alloc(buf, buf_size);

  if (!m) {
    disastrOS_debug("[%d:MQWRITE] m has not been allocated\n", PID);
    running->syscall_retvalue = DSOS_EMQWRITE;
    return;
  }

  List_insert(&q->messages, q->messages.last, (ListItem*)m);
  mq->num_messages++;
  disastrOS_debug(
      "[%d:MQWRITE] mq %d welcomes a new message with priority %d\n", PID,
      mq->id, priority);
  running->syscall_retvalue = priority;
}