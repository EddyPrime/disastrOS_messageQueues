#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "disastrOS.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"
#include "disastrOS_syscalls.h"

#define PID disastrOS_getpid()

void internal_semClose() {
  // do stuff :)

  int fd = running->syscall_args[0];

  // maybe not necessary
  if (fd < 0 || fd > MAX_NUM_SEMDESCRIPTORS_PER_PROCESS) {
    disastrOS_debug("[%d:SEMCLOSE] fd out of bounds\n", PID);
    running->syscall_retvalue = DSOS_ESEMAPHORECLOSE;
    return;
  }

  // fetching semDescriptor associated to fd
  SemDescriptor* semDes = SemDescriptorList_byFd(&running->sem_descriptors, fd);

  if (!semDes) {
    disastrOS_debug("[%d:SEMCLOSE] no semDes found\n", PID);
    running->syscall_retvalue = DSOS_ESEMAPHORECLOSE;
    return;
  }

  // retrieving semaphore from semDescriptor
  Semaphore* sem = semDes->semaphore;

  if (!sem) {
    disastrOS_debug("[%d:SEMCLOSE] no sem associated with semDes\n", PID);
    running->syscall_retvalue = DSOS_ESEMAPHORECLOSE;
    return;
  }

  List_detach(&running->sem_descriptors, (ListItem*)semDes);
  SemDescriptorPtr* semDesPtr = semDes->ptr;

  if (!semDesPtr) {
    disastrOS_debug("[%d:SEMCLOSE] no semDesPtr associated with semDes\n", PID);
    running->syscall_retvalue = DSOS_ESEMAPHORECLOSE;
    return;
  }

  List_detach(&sem->descriptors, (ListItem*)semDesPtr);
  SemDescriptor_free(semDes);
  SemDescriptorPtr_free(semDesPtr);
  running->last_sem_fd--;

  // if noone is using the semaphore anymore, semaphore can be destroyed
  if (!sem->descriptors.size) {
    disastrOS_debug("[%d:SEMCLOSE] noone is using this semaphore\n", PID);

    sem = (Semaphore*)List_detach(&semaphores_list, (ListItem*)sem);

    if (!sem) {
      disastrOS_debug("[%d:SEMCLOSE] sem not found in semaphores_list\n", PID);
      running->syscall_retvalue = DSOS_ESEMAPHORECLOSE;
      return;
    }

    Semaphore_free(sem);
    disastrOS_debug("[%d:SEMCLOSE] sem has been destroyed\n", PID);
  }

  disastrOS_debug("[%d:SEMCLOSE] tutt appo\n", PID);
  running->syscall_retvalue = 0;
}
