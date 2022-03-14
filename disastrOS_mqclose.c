#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "disastrOS.h"
#include "disastrOS_mq.h"
#include "disastrOS_mqdescriptor.h"
#include "disastrOS_syscalls.h"

#define PID disastrOS_getpid()

/*
mqClose closes the message queue referred to file descriptor fd. This means that
the process cannot access in any way the message queue unless a new open occurs.
Notice that unless the last process does not call an unlink, even if everyone
has closed the message queue, this remains alive in the system. Upon success,
mqdescriptor and mqdescriptor pointer are freed, last_mq_fd is decreased and 0
is returned. Upon error, a negative integer is returned.
*/

void internal_mqClose() {

  int fd = running->syscall_args[0];

  // maybe not necessary
  if (fd < 0 || fd > MAX_NUM_MQDESCRIPTORS_PER_PROCESS) {
    disastrOS_debug("[%d:MQCLOSE] fd out of bounds\n", PID);
    running->syscall_retvalue = DSOS_EMQCLOSE;
    return;
  }

  // fetching mqDescriptor associated to fd
  MQDescriptor* mqDes = MQDescriptorList_byFd(&running->mq_descriptors, fd);

  if (!mqDes) {
    disastrOS_debug("[%d:MQCLOSE] no mqDes found\n", PID);
    running->syscall_retvalue = DSOS_EMQCLOSE;
    return;
  }

  // retrieving mq from mqDescriptor
  MessageQueue* mq = mqDes->mq;

  if (!mq) {
    disastrOS_debug("[%d:MQCLOSE] no mq associated with mqDes\n", PID);
    running->syscall_retvalue = DSOS_EMQCLOSE;
    return;
  }

  List_detach(&running->mq_descriptors, (ListItem*)mqDes);
  MQDescriptorPtr* mqDesPtr = mqDes->ptr;

  if (!mqDesPtr) {
    disastrOS_debug("[%d:MQCLOSE] no mqDesPtr associated with mqDes\n", PID);
    running->syscall_retvalue = DSOS_EMQCLOSE;
    return;
  }

  List_detach(&mq->descriptors, (ListItem*)mqDesPtr);
  MQDescriptor_free(mqDes);
  MQDescriptorPtr_free(mqDesPtr);
  running->last_mq_fd--;
  disastrOS_debug("[%d:MQCLOSE] descriptors destroyed, file descriptor descreased\n", PID);
  running->syscall_retvalue = 0;
}
