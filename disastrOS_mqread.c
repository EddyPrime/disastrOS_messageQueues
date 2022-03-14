#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "disastrOS.h"
#include "disastrOS_mq.h"
#include "disastrOS_mqdescriptor.h"
#include "disastrOS_syscalls.h"

#define PID disastrOS_getpid()

/*
mqRead gets the oldest message with highest priority, detaching it from the
message queue related to fd and saving it in buf. Upon success, the syscall
returns the number of characters written into buf, at least 0 (for instance if
no message is stored in the queue or if the message has length 0), and *priority
will contain the value of the priority of the queue which the message has been
detached from. Current number of messages stored into the message queue will be
decreased by one. If buf_size is not enough to fit the message, an error occurs.
Upon error, a negative integer is returned and the message queue is not affected
at all.
*/

void internal_mqRead() {
  int fd = running->syscall_args[0];
  char* buf = (char*)running->syscall_args[1];
  int buf_size = running->syscall_args[2];
  int* priority = (int*)running->syscall_args[3];

  // maybe not necessary
  if (fd < 0 || fd > MAX_NUM_MQDESCRIPTORS_PER_PROCESS) {
    disastrOS_debug("[%d:MQREAD] fd out of bounds\n", PID);
    running->syscall_retvalue = DSOS_EMQREAD;
    return;
  }

  // fetching mqDescriptor associated to fd
  MQDescriptor* mqDes = MQDescriptorList_byFd(&running->mq_descriptors, fd);

  if (!mqDes) {
    disastrOS_debug("[%d:MQREAD] no mqDes found\n", PID);
    running->syscall_retvalue = DSOS_EMQREAD;
    return;
  }

  // retrieving mq from mqDescriptor
  MessageQueue* mq = mqDes->mq;

  if (!mq) {
    disastrOS_debug("[%d:MQREAD] no mq associated with mqDes\n", PID);
    running->syscall_retvalue = DSOS_EMQREAD;
    return;
  }

  if (!mq->num_messages) {
    disastrOS_debug("[%d:MQREAD] mq %d has no message to be retrieved\n", PID,
                    mq->id);
    running->syscall_retvalue = 0;
    return;
  }

  // as FIFO policy, mqread retrieves messages from the head
  int i;
  Queue* q;
  Message* m;
  for (i = 0; i < mq->priority; i++) {
    q = QueueList_byPriority(&mq->queues, i);

    if (!q) {
      disastrOS_debug(
          "[%d:MQREAD] could not retrieve queue with priority %d from mq %d\n",
          PID, i, mq->id);
      running->syscall_retvalue = DSOS_EMQREAD;
      return;
    }

    m = 0;

    if (q->messages.size) {
      disastrOS_debug(
          "[%d:MQREAD] queue with priority %d from mq %d has %d messages\n",
          PID, i, mq->id, q->messages.size);
      m = (Message*)List_detach(&q->messages, q->messages.first);
    }

    if (m && buf_size >= m->text_length) {
      mq->num_messages--;
      disastrOS_debug(
          "[%d:MQREAD] a message from queue with priority %d from mq %d has "
          "been retrieved and has %d length\n",
          PID, i, mq->id, m->text_length);
      disastrOS_debug("[%d:MQREAD] the message is: %s\n", PID, m->text);
      *buf = *m->text;
      Message_free(m);
      *priority = i;
      running->syscall_retvalue = m->text_length;
      return;
    }
  }

  disastrOS_debug("[%d:MQREAD] nothing has been retrieved from mq %d\n", PID,
                  mq->id);
  running->syscall_retvalue = 0;
}