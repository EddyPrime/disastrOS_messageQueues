#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

#define PID disastrOS_getpid()

void internal_semOpen(){
  // do stuff :)

	int id 				= running->syscall_args[0];
	int flags 			= running->syscall_args[1];
	unsigned int count  = running->syscall_args[2];
	
	//checking if process is too hungry
		
	if (running->last_sem_fd + 1 > MAX_NUM_SEMDESCRIPTORS_PER_PROCESS) {
        disastrOS_debug("[%d:SEMOPEN] too many semDescriptors for me\n", PID);
        running->syscall_retvalue = DSOS_ESEMAPHOREOPEN;
        return;
		}
		
    //checking arguments consistency

    if (id < 0) {
        disastrOS_debug("[%d:SEMOPEN] id out of bounds\n", PID);
        running->syscall_retvalue = DSOS_ESEMAPHOREOPEN;
        return;
        }

    if (count < 0) {
        disastrOS_debug("[%d:SEMOPEN] negative count\n", PID);
        running->syscall_retvalue = DSOS_ESEMAPHOREOPEN;
        return;
        }

	//fetching sem associated to id
	Semaphore* sem = SemaphoreList_byId(&semaphores_list, id);

	//looking for flags
	if((flags&DSOS_CREATE)){

		//if DSOS_CREATE and semaphore does not exist, it must be created
        disastrOS_debug("[%d:SEMOPEN] DSOS_CREATE has been set\n", PID);
        
		if (sem && (flags&DSOS_EXCL)){

			//if DSOS_EXCL and semaphore exists, error

            disastrOS_debug("[%d:SEMOPEN] DSOS_EXCL has been set but sem already exists\n", PID);
			running->syscall_retvalue = DSOS_ESEMAPHOREOPEN;
			return;
		}
		else if (!sem) {
			
			//semaphore does not exist
			//it must be allocated and put in semaphores_list  

            disastrOS_debug("[%d:SEMOPEN] sem does not exist\n", PID);

            if (semaphores_list.size + 1 > MAX_NUM_SEMAPHORES) {
                disastrOS_debug("[%d:SEMOPEN] sem number out of bound\n", PID);
                running->syscall_retvalue = DSOS_ESEMAPHOREOPEN;
                return;
                }

			sem = Semaphore_alloc(id, count);

            if (!sem){
                disastrOS_debug("[%d:SEMOPEN] sem has not been alocated\n", PID);
                running->syscall_retvalue = DSOS_ESEMAPHOREOPEN;
                return;
                }

			List_insert(&semaphores_list, semaphores_list.last, (ListItem*) sem);
		}
	}

	//so tiny so big
	if (!sem){
        disastrOS_debug("[%d:SEMOPEN] sem has not been alocated\n", PID);
        running->syscall_retvalue = DSOS_ESEMAPHOREOPEN;
        return;
		}

	//having the sem, it's time to alloc descriptors
	SemDescriptor* semDes = SemDescriptor_alloc(running->last_sem_fd, sem, running);

    if (!semDes) {
        disastrOS_debug("[%d:SEMOPEN] semDes has not been alocated\n", PID);
        running->syscall_retvalue = DSOS_ESEMAPHOREOPEN;
        return;
        }

	SemDescriptorPtr* semDesPtr = SemDescriptorPtr_alloc(semDes);

    if (!semDesPtr) {
        disastrOS_debug("[%d:SEMOPEN] semDesPtr has not been alocated\n", PID);
        running->syscall_retvalue = DSOS_ESEMAPHOREOPEN;
        return;
        }

	semDes->ptr = semDesPtr;
	List_insert(&running->sem_descriptors, running->sem_descriptors.last, (ListItem*) semDes);
	List_insert(&sem->descriptors, sem->descriptors.last, (ListItem*) semDesPtr);
	running->last_sem_fd++;
    disastrOS_debug("[%d:SEMOPEN] tutt appo\n", PID);
	running->syscall_retvalue = running->last_sem_fd - 1;
}
