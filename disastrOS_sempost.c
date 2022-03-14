#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

#define PID disastrOS_getpid()

void internal_semPost(){
  // do stuff :)

	int fd = running->syscall_args[0];

	//maybe not necessary
	if (fd < 0 || fd > MAX_NUM_SEMDESCRIPTORS_PER_PROCESS) {
        disastrOS_debug("[%d:SEMPOST] fd out of bounds\n", PID);
        running->syscall_retvalue = DSOS_ESEMAPHOREPOST;
        return;
        }

	//retrieving semDescriptor associated to fd
	SemDescriptor* semDes = SemDescriptorList_byFd(&running->sem_descriptors, fd);

	if (!semDes) {
        disastrOS_debug("[%d:SEMPOST] no semDes found\n", PID);
        running->syscall_retvalue = DSOS_ESEMAPHOREPOST;
        return;
        }
	
	//retrieving semaphore from semDescriptor
    Semaphore* sem = semDes->semaphore;

    if (!sem) {
        disastrOS_debug("[%d:SEMPOST] no sem associated with semDes\n", PID);
        running->syscall_retvalue = DSOS_ESEMAPHOREPOST;
        return;
		}
		
	/*
	 *	if count++ turns to a positive value, schedule and decrement occur
	 * 	beacuse of the semwait 
	 * 	else, nothing left to do
	 */
	 
	if (++sem->count > 0) {
        disastrOS_debug("[%d:SEMPOST] resources have been re-established...\n", PID);
		if (sem->waiting_descriptors.size) {
			//semwait emulated
			sem->count--;
			disastrOS_debug("[%d:SEMPOST] ... and it is time to awake a brand-new process\n", PID);
			//time to choose another pcb from descriptors in waiting associated to the semaphore
			SemDescriptorPtr* semDesPtr = (SemDescriptorPtr*) List_detach(&sem->waiting_descriptors, sem->waiting_descriptors.first);
            semDes = semDesPtr->descriptor;
            PCB* awakening = semDes->pcb;

            if (!awakening) {
                disastrOS_debug("[%d:SEMPOST] no PCB associated with semDes\n", PID);
                running->syscall_retvalue = DSOS_ESEMAPHOREPOST;
                return;
                }
			
            List_detach(&waiting_list, (ListItem*) awakening);
            List_insert(&ready_list, ready_list.last, (ListItem*) awakening);
            List_insert(&sem->descriptors, sem->descriptors.last, (ListItem*) semDesPtr);
            
            disastrOS_debug("[%d:SEMPOST] %d back to fight! We meet again!\n", PID, awakening->pid);
			}
		else {
			disastrOS_debug("[%d:SEMPOST] ... but there is noone that can be awakened\n", PID);
			running->syscall_retvalue = 0;
            }
		}
	else {
		disastrOS_debug("[%d:SEMPOST] still not enough resources\n", PID);
		running->syscall_retvalue = 0;
		}
}
