#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

#define PID disastrOS_getpid()

void internal_semWait(){
  // do stuff :)

	int fd = running->syscall_args[0];

	//maybe not necessary	
	if (fd < 0 || fd > MAX_NUM_SEMDESCRIPTORS_PER_PROCESS) {
        disastrOS_debug("[%d:SEMWAIT] fd out of bounds\n", PID);
        running->syscall_retvalue = DSOS_ESEMAPHOREWAIT;
        return;
        }
	
	//fetching semDescriptor associated to fd
	SemDescriptor* semDes = SemDescriptorList_byFd(&running->sem_descriptors, fd);

	if (!semDes) {
        disastrOS_debug("[%d:SEMWAIT] no semDes found\n", PID);
        running->syscall_retvalue = DSOS_ESEMAPHOREWAIT;
        return;
        }

	//retrieving semaphore from semDescriptor
    Semaphore* sem = semDes->semaphore;

    if (!sem) {
        disastrOS_debug("[%d:SEMWAIT] no sem associated with semDes\n", PID);
        running->syscall_retvalue = DSOS_ESEMAPHOREWAIT;
        return;
		}
	
	/*
	 * decrease occurs only if count > 0
	 * if count <= 0, decrease will occur on eventual reawekening
	 */
	
	if (sem->count > 0) {
		disastrOS_debug("[%d:SEMWAIT] resources are enough\n", PID);
		sem->count--;
		running->syscall_retvalue = 0;
		}
	else {
		disastrOS_debug("[%d:SEMWAIT] not enough resources, scheduling\n", PID);
		
		/*
		 * need to move semdescriptor associated to sem waiting_descriptors
		 * from descriptors so that sempost can awake someone
		 */
		 
		SemDescriptorPtr* semDesPtr = semDes->ptr;

		if (!semDesPtr) {
			disastrOS_debug("[%d:SEMWAIT] no semDesPtr found\n", PID);
			running->syscall_retvalue = DSOS_ESEMAPHOREWAIT;
			return;
        }

		semDesPtr = (SemDescriptorPtr*) List_detach(&sem->descriptors, (ListItem*) semDesPtr);

		if (!semDesPtr) {
			disastrOS_debug("[%d:SEMWAIT] no semDesPtr detached\n", PID);
			running->syscall_retvalue = DSOS_ESEMAPHOREWAIT;
			return;
        }

		List_insert(&sem->waiting_descriptors, sem->waiting_descriptors.last, (ListItem*) semDesPtr);
		//time to choose another pcb to become running
		PCB* newRunning = (PCB*) List_detach(&ready_list, ready_list.first);

		if (!newRunning) {
			disastrOS_debug("[%d:SEMWAIT] no other process ready to run\n", PID);
			running->syscall_retvalue = DSOS_ESEMAPHOREWAIT;
			return;
			}
		
		//time to let "old running" sleep
		List_insert(&waiting_list, waiting_list.last, (ListItem*) running);
		running = newRunning;
		}
}
