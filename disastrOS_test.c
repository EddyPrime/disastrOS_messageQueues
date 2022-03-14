#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "disastrOS.h"

#define PID disastrOS_getpid()

#define ID 153
#define FLAGS DSOS_CREATE  // bitwise or to add flags
#define COUNT 1
#define PRIORITY 0
#define NUMFUNCS 15

void (*testFunPtr)();
void (*testFunPtrArray[NUMFUNCS])();
int totalOK = 0;
int totalKO = 0;

// we need this to handle the sleep state
void sleeperFunction(void* args) {
  printf("Hello, I am the sleeper, and I sleep %d\n", disastrOS_getpid());
  while (1) {
    getc(stdin);
    disastrOS_printStatus();
  }
}

void semaphoreClean() {
  printf("[%d] Running semaphoreClean with id: %d, flags: %d, count: %d,\n",
         PID, ID, FLAGS, COUNT);

  /*
   *	in critical section, concurrent access to resources should be
   *	coordinated and no race condition should interfere with
   * 	final result of shared resources
   * 	outside critical section, everything can happen :O
   */

  int b = totalKO;

  int fd = disastrOS_semOpen(ID, FLAGS, COUNT);
  printf("semopen retvalue: %d\n", fd);
  int ret = disastrOS_semWait(fd);
  printf("semwait retvalue: %d\n", ret);

  /*start of cs*/

  printf("[%d] CS ENTERED\n", disastrOS_getpid());

  int a = totalOK;
  disastrOS_sleep(1);
  a++;
  totalOK = a;

  printf("[%d] CS EXITING\n", disastrOS_getpid());

  /*end of cs*/

  ret = disastrOS_semPost(fd);
  printf("sempost retvalue: %d\n", ret);
  fd = disastrOS_semClose(fd);
  printf("semclose retvalue: %d\n", fd);
  b++;
  totalKO = b;
  return;
}

void semaphoreOpenClose() {
  printf("[%d] Running semaphoreOpenClose with id: %d, flags: %d, count: %d,\n",
         PID, ID, FLAGS, COUNT);

  int fd = disastrOS_semOpen(ID, FLAGS, COUNT);
  printf("semopen retvalue:%d\n", fd);
  fd = disastrOS_semClose(fd);
  printf("semclose retvalue:%d\n", fd);
  return;
}

void semaphoreNoOpen() {
  printf("[%d] Running semaphoreNoOpen with id: %d, flags: %d, count: %d,\n",
         PID, ID, FLAGS, COUNT);

  int ret = disastrOS_semWait(ID);
  printf("semwait retvalue: %d\n", ret);
  ret = disastrOS_semPost(ID);
  printf("sempost retvalue: %d\n", ret);
  ret = disastrOS_semClose(ID);
  printf("semclose retvalue: %d\n", ret);
  return;
}

void semaphoreDoubleOpen() {
  printf(
      "[%d] Running semaphoreDoubleOpen with id: %d, flags: %d, count: %d,\n",
      PID, ID, FLAGS, COUNT);

  int fd1 = disastrOS_semOpen(ID, FLAGS, COUNT);
  printf("semopen retvalue: %d\n", fd1);
  int fd2 = disastrOS_semOpen(ID, FLAGS, COUNT);
  printf("semopen retvalue: %d\n", fd2);

  disastrOS_printStatus();

  int ret = disastrOS_semClose(fd1);
  printf("semclose retvalue: %d\n", ret);
  ret = disastrOS_semClose(fd2);
  printf("semclose retvalue: %d\n", ret);
  return;
}

void semaphoreBoundsException() {
  printf(
      "[%d] Running semaphoreBoundException with id: %d, flags: %d, count: "
      "%d,\n",
      PID, -1, FLAGS, COUNT);

  int fd = disastrOS_semOpen(-1, FLAGS, COUNT);
  printf("semopen retvalue: %d\n", fd);
  int ret = disastrOS_semWait(fd);
  printf("semwait retvalue: %d\n", ret);
  ret = disastrOS_semPost(fd);
  printf("sempost retvalue: %d\n", ret);
  fd = disastrOS_semClose(fd);
  printf("semclose retvalue: %d\n", fd);
  return;
}

void semaphoreReOpen() {
  printf("[%d] Running semaphoreReOpen with id: %d, flags: %d, count: %d,\n",
         PID, ID, FLAGS, COUNT);

  int fd = disastrOS_semOpen(ID, FLAGS, COUNT);
  printf("semopen retvalue:%d\n", fd);

  disastrOS_sleep(1);

  fd = disastrOS_semClose(fd);
  printf("semclose retvalue:%d\n", fd);
  fd = disastrOS_semOpen(ID, FLAGS, COUNT);
  printf("semopen retvalue:%d\n", fd);

  disastrOS_sleep(1);

  fd = disastrOS_semClose(fd);
  printf("semclose retvalue:%d\n", fd);
  return;
}

void semaphoreNoClose() {
  printf("[%d] Running semaphoreNoClose with id: %d, flags: %d, count: %d,\n",
         PID, ID, FLAGS, COUNT);

  int fd = disastrOS_semOpen(ID, FLAGS, COUNT);
  printf("semopen retvalue:%d\n", fd);
  return;
}

void messageQueueOpenClose() {
  printf(
      "[%d] Running messageQueueOpenClose with id: %d, flags: %d, priority: "
      "%d,\n",
      PID, ID, FLAGS, PRIORITY);

  int fd = disastrOS_mqOpen(ID, FLAGS, PRIORITY);
  printf("mqopen retvalue: %d\n", fd);
  fd = disastrOS_mqClose(fd);
  printf("mqclose retvalue:%d\n", fd);
}

void messageQueueOpenCloseUnlink() {
  printf(
      "[%d] Running messageQueueOpen with id: %d, flags: %d, priority: %d,\n",
      PID, ID, FLAGS, PRIORITY);

  int fd = disastrOS_mqOpen(ID, FLAGS, PRIORITY);
  printf("mqopen retvalue: %d\n", fd);
  disastrOS_printStatus();
  fd = disastrOS_mqClose(fd);
  printf("mqclose retvalue:%d\n", fd);
  fd = disastrOS_mqUnlink(ID);
  printf("mqunlink retvalue:%d\n", fd);
}

void messageQueueWrite() {
  printf("[%d] Running messageQueueWrite\n", PID);
  int fd = disastrOS_mqOpen(ID, FLAGS, PRIORITY);
  printf("mqopen retvalue: %d\n", fd);
  int buf_size = 32;
  int priority = PID - 2;
  char buf[buf_size];
  snprintf(buf, buf_size, "%d", PID);
  printf("mqwrite retvalue: %d\n",
         disastrOS_mqWrite(fd, buf, buf_size, priority));
  fd = disastrOS_mqClose(fd);
  printf("mqclose retvalue:%d\n", fd);
}

void messageQueueRead() {
  printf("[%d] Running messageQueueRead\n", PID);

  int fd = disastrOS_mqOpen(ID, FLAGS, PRIORITY);
  printf("mqopen retvalue: %d\n", fd);
  int buf_size = 32;
  char buf[buf_size];
  int priority = 0;
  int res = disastrOS_mqRead(fd, buf, buf_size, &priority);
  printf("mqread retvalue: %d\n", res);
  if (res)
    printf("retrieved from %d /n/t%s/n", priority, buf);
  fd = disastrOS_mqClose(fd);
  printf("mqclose retvalue:%d\n", fd);
}

void messageQueueWriteRead() {
  printf("[%d] Running messageQueueWriteRead\n", PID);
  int fd = disastrOS_mqOpen(ID, FLAGS, PRIORITY);
  printf("mqopen retvalue: %d\n", fd);
  int buf_size = 32;
  int priority;
  char buf[buf_size];
  snprintf(buf, buf_size, "%d", PID);
  int i;
  int cicle = PRIORITY;
  for (i = 0; i < cicle; i++)
    printf("mqwrite retvalue: %d\n", disastrOS_mqWrite(fd, buf, buf_size, i));
  int res = disastrOS_mqRead(fd, buf, buf_size, &priority);
  printf("mqread retvalue: %d\n", res);
  if (res)
    printf("retrieved from %d \n\t%s\n", priority, buf);
  fd = disastrOS_mqClose(fd);
  printf("mqclose retvalue:%d\n", fd);
}

void messageQueueNoOpen() {
  printf(
      "[%d] Running messageQueueNoOpen with id: %d, flags: %d, priority: %d,\n",
      PID, ID, FLAGS, PRIORITY);

  int ret = disastrOS_mqClose(ID);
  printf("mqclose retvalue: %d\n", ret);
  ret = disastrOS_mqUnlink(ID);
  printf("mqunlink retvalue: %d\n", ret);
  return;
}

void messageQueueDoubleOpen() {
  printf(
      "[%d] Running messageQueueDoubleOpen with id: %d, flags: %d, priority: "
      "%d,\n",
      PID, ID, FLAGS, PRIORITY);

  int fd1 = disastrOS_mqOpen(ID, FLAGS, PRIORITY);
  printf("mqopen retvalue:%d\n", fd1);
  int fd2 = disastrOS_mqOpen(ID, FLAGS, PRIORITY);
  printf("mqopen retvalue:%d\n", fd2);

  disastrOS_sleep(3);

  fd1 = disastrOS_mqClose(fd1);
  printf("mqclose retvalue:%d\n", fd1);
  fd2 = disastrOS_mqClose(fd2);
  printf("mqclose retvalue:%d\n", fd2);

  return;
}

void messageQueueBoundsException() {
  printf(
      "[%d] Running messageQueueBoundException with id: %d, flags: %d, "
      "priority: "
      "%d,\n",
      PID, -1, FLAGS, PRIORITY);

  int fd = disastrOS_mqOpen(-1, FLAGS, PRIORITY);
  printf("mqopen retvalue: %d\n", fd);
  fd = disastrOS_mqClose(fd);
  printf("mqclose retvalue: %d\n", fd);
  fd = disastrOS_mqUnlink(ID);
  printf("mqunlink retvalue:%d\n", fd);
  return;
}

void messageQueueReOpen() {
  printf(
      "[%d] Running messageQueueReOpen with id: %d, flags: %d, priority: "
      "%d,\n",
      PID, ID, FLAGS, PRIORITY);

  int fd = disastrOS_mqOpen(ID, FLAGS, PRIORITY);
  printf("mqopen retvalue:%d\n", fd);

  disastrOS_sleep(3);

  fd = disastrOS_mqClose(fd);
  printf("mqclose retvalue:%d\n", fd);
  fd = disastrOS_mqOpen(ID, FLAGS, PRIORITY);
  printf("mqopen retvalue:%d\n", fd);

  disastrOS_sleep(3);

  fd = disastrOS_mqClose(fd);
  printf("mqclose retvalue:%d\n", fd);
  return;
}

void messageQueueNoClose() {
  printf("[%d] Running mqNoClose with id: %d, flags: %d, priority: %d,\n",
         PID, ID, FLAGS, PRIORITY);

  int fd = disastrOS_semOpen(ID, FLAGS, PRIORITY);
  printf("mqopen retvalue:%d\n", fd);
  return;
}

void childFunction(void* args) {
  printf("Hello, I am the child function %d\n", disastrOS_getpid());
  printf("I will iterate a bit, before terminating\n");
  /*
  int type=0;
  int mode=0;
  int fd=disastrOS_openResource(disastrOS_getpid(),type,mode);
  printf("fd=%d\n", fd);
  printf("PID: %d, terminating\n", disastrOS_getpid());

  for (int i=0; i<(disastrOS_getpid()+1); ++i){
    printf("PID: %d, iterate %d\n", disastrOS_getpid(), i);
    disastrOS_sleep((20-disastrOS_getpid())*5);
  }
  */

  testFunPtr();

  disastrOS_exit(disastrOS_getpid() + 1);
}

void initFunction(void* args) {
  disastrOS_printStatus();
  printf("hello, I am init and I just started\n");
  disastrOS_spawn(sleeperFunction, 0);

  printf("I feel like to spawn 10 nice threads\n");
  int alive_children = 0;
  int i;
  for (i = 0; i < 10; ++i) {
    int type = 0;
    int mode = DSOS_CREATE;
    printf("mode: %d\n", mode);
    printf("opening resource (and creating if necessary)\n");
    int fd = disastrOS_openResource(i, type, mode);
    printf("fd=%d\n", fd);
    disastrOS_spawn(childFunction, 0);
    alive_children++;
  }

  disastrOS_printStatus();
  int retval;
  int pid;
  while (alive_children > 0 && (pid = disastrOS_wait(0, &retval)) >= 0) {
    disastrOS_printStatus();
    printf("initFunction, child: %d terminated, retval:%d, alive: %d \n", pid,
           retval, alive_children);
    --alive_children;
  }
  printf("totalOK %d\n", totalOK);
  printf("totalKO %d\n", totalKO);
  printf("shutdown!");
  disastrOS_shutdown();
}

int main(int argc, char** argv) {
  char* logfilename = 0;

  testFunPtrArray[0] = semaphoreClean;
  testFunPtrArray[1] = semaphoreOpenClose;
  testFunPtrArray[2] = semaphoreNoOpen;
  testFunPtrArray[3] = semaphoreDoubleOpen;
  testFunPtrArray[4] = semaphoreBoundsException;
  testFunPtrArray[5] = semaphoreReOpen;
  testFunPtrArray[6] = semaphoreNoClose;
  testFunPtrArray[7] = messageQueueOpenClose;
  testFunPtrArray[8] = messageQueueOpenCloseUnlink;
  testFunPtrArray[9] = messageQueueWriteRead;
  testFunPtrArray[10] = messageQueueNoOpen;
  testFunPtrArray[11] = messageQueueDoubleOpen;
  testFunPtrArray[12] = messageQueueBoundsException;
  testFunPtrArray[13] = messageQueueReOpen;
  testFunPtrArray[14] = messageQueueNoClose;

  testFunPtr = messageQueueOpenClose;

  if (argc > 1) {
    testFunPtr = (*testFunPtrArray[atoi(argv[1]) % NUMFUNCS]);
  }

  if (argc > 2) {
    logfilename = argv[2];
  }
  // we create the init process processes
  // the first is in the running variable
  // the others are in the ready queue

  printf("the function pointer is: %p", childFunction);
  // spawn an init process
  printf("start\n");
  disastrOS_start(initFunction, 0, logfilename);
  return 0;
}
