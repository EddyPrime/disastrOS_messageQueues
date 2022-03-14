#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "disastrOS.h"
#include "disastrOS_mq.h"
#include "disastrOS_mqdescriptor.h"
#include "disastrOS_syscalls.h"

#define PID disastrOS_getpid()

/*
mqUnlink deletes the message queue from the system. This can occur if and only
if the message queue has no more messages to be retrieved and there is no open
descriptor. This states that a number of mqread equal to the number of messages
remaining in the message queue has to occur and each process has to close the
file descriptor related to the message queue. Obviously, priority queues are
unlinked along with the message queue itself. Upon success, 0 is returned and
the message queue will be no more alive all over the system. Thus, a call to a
mqOpen with the same id of the just exploded message queue will create a new
one. Upon error, a negative integer is returned and the message queue will live
another day.
*/

void internal_mqUnlink() {
  int id = running->syscall_args[0];

  MessageQueue* mq = MessageQueueList_byId(&messageQueues_list, id);

  if (!mq) {
    disastrOS_debug("[%d:MQUNLINK] mq %d not found in messageQueues_list\n",
                    PID, id);
    running->syscall_retvalue = DSOS_EMQUNLINK;
    return;
  }

  if (mq->num_messages) {
    disastrOS_debug(
        "[%d:MQUNLINK] mq %d has still %d messages to be retireved\n", PID, id,
        mq->num_messages);
    running->syscall_retvalue = DSOS_EMQUNLINK;
    return;
  }

  if (mq->descriptors.size) {
    disastrOS_debug(
        "[%d:MQUNLINK] mq %d has still %d descriptors to be closed\n", PID, id,
        mq->descriptors.size);
    running->syscall_retvalue = DSOS_EMQUNLINK;
    return;
  }

  mq = (MessageQueue*)List_detach(&messageQueues_list, (ListItem*)mq);

  if (!mq) {
    disastrOS_debug("[%d:MQUNLINK] mq %d could not be detached\n", PID, id);
    running->syscall_retvalue = DSOS_EMQUNLINK;
    return;
  }

  disastrOS_debug("[%d:MQUNLINK] starting unlink of %d for mq %d\n", PID,
                  mq->priority, id);
  int i;
  Queue* q;
  for (i = 0; i <= mq->priority; i++) {
    q = (Queue*)List_detach(&mq->queues, (ListItem*)mq->queues.first);
    if (q && q->messages.size == 0) {
      Queue_free(q);
      disastrOS_debug(
          "[%d:MQUNLINK] queue with priority %d of mq %d has gone\n", PID, i,
          id);
    }
  }
  MessageQueue_free(mq);
  disastrOS_debug("[%d:MQUNLINK] goodbye %d\n", PID, id);
  running->syscall_retvalue = 0;
}
