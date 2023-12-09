#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED

#include "scheduler.h"

#include <assert.h>
#include <curses.h>
#include <ucontext.h>

#include "util.h"

// This is an upper limit on the number of tasks we can create.
#define MAX_TASKS 128

// This is the size of each task's stack memory
#define STACK_SIZE 65536

// define all the states
#define READY 0
#define WAITING 1
#define SLEEPING 2
#define WAITINPUT 3
#define EXIT 4
// This struct will hold the all the necessary information for each task
typedef struct task_info {
  // This field stores all the state required to switch back to this task
  ucontext_t context;

  // This field stores another context. This one is only used when the task
  // is exiting.
  ucontext_t exit_context;

  // TODO: Add fields here so you can:
  //   a. Keep track of this task's state.
  int state;
  //   b. If the task is sleeping, when should it wake up?
  size_t sleep_time;
  //   c. If the task is waiting for another task, which task is it waiting for?
  task_t wait_for_task;
  //   d. Was the task blocked waiting for user input? Once you successfully
  //      read input, you will need to save it here so it can be returned.
  int input;

} task_info_t;

int current_task = 0;          //< The handle of the currently-executing task
int num_tasks = 1;             //< The number of tasks created so far
task_info_t tasks[MAX_TASKS];  //< Information for every task
void findReadyTask();
/**
 * Initialize the scheduler. Programs should call this before calling any other
 * functiosn in this file.
 */
void scheduler_init() {
  // TODO: Initialize the state of the scheduler
  task_info_t main_task;
  main_task.state = READY;
  main_task.sleep_time = 0;
  main_task.wait_for_task = 0;
  main_task.input = ERR;
  tasks[0] = main_task;
  int status = getcontext(&tasks[0].context);
  if (status == -1){
    perror("not initialized");
  }

}

/**
 * This function will execute when a task's function returns. This allows you
 * to update scheduler states and start another task. This function is run
 * because of how the contexts are set up in the task_create function.
 */
void task_exit() {
  // TODO: Handle the end of a task's execution here
  tasks[current_task].state = EXIT;
  findReadyTask();
}

/**
 * Create a new task and add it to the scheduler.
 *
 * \param handle  The handle for this task will be written to this location.
 * \param fn      The new task will run this function.
 */
void task_create(task_t* handle, task_fn_t fn) {
  // Claim an index for the new task
  int index = num_tasks;
  num_tasks++;

  // Set the task handle to this index, since task_t is just an int
  *handle = index;
  tasks[index].state = READY;
  tasks[index].sleep_time = 0;
  tasks[index].wait_for_task = 0;
  tasks[index].input = ERR;
  // We're going to make two contexts: one to run the task, and one that runs at the end of the task
  // so we can clean up. Start with the second

  // First, duplicate the current context as a starting point
  getcontext(&tasks[index].exit_context);

  // Set up a stack for the exit context
  tasks[index].exit_context.uc_stack.ss_sp = malloc(STACK_SIZE);
  tasks[index].exit_context.uc_stack.ss_size = STACK_SIZE;

  // Set up a context to run when the task function returns. This should call task_exit.
  makecontext(&tasks[index].exit_context, task_exit, 0);

  // Now we start with the task's actual running context
  getcontext(&tasks[index].context);

  // Allocate a stack for the new task and add it to the context
  tasks[index].context.uc_stack.ss_sp = malloc(STACK_SIZE);
  tasks[index].context.uc_stack.ss_size = STACK_SIZE;

  // Now set the uc_link field, which sets things up so our task will go to the exit context when
  // the task function finishes
  tasks[index].context.uc_link = &tasks[index].exit_context;

  // And finally, set up the context to execute the task function
  makecontext(&tasks[index].context, fn, 0);
}

/**
Searches through the list of tasks until the next ready task and switchs context
to the ready task.
*/
void findReadyTask() {
  int old_task = current_task;
  int i = current_task;

  // runs until a ready task is found
  while (true) {
    i++;
    if (i == num_tasks) { // if we reach the end of tasks array start over
      i = 0;
    }
    if (tasks[i].state == WAITINPUT) {// if the task is waiting for input, get input and swap to this task
      int input = getch();
      if (input != ERR) {
        tasks[i].input = input;
        tasks[i].state = READY;
        current_task = i;
        break;
      }
    }
    else if (tasks[i].state == WAITING && tasks[tasks[i].wait_for_task].state == EXIT) {// if the task is waiting and the task it is waiting for has exited, then switch to this task
      tasks[i].state = READY;
      current_task = i;
      break;
    }
    else if (tasks[i].state == SLEEPING && time_ms() >= tasks[i].sleep_time) { // if the task is sleeping and it has slept the correct amount then switch to it run the task
      tasks[i].state = READY;
      current_task = i;
      break;
    }
    else if (tasks[i].state == READY) {// if this is a ready task switch to it
      current_task = i;
      break;
    }
  }
  swapcontext(&tasks[old_task].context, &tasks[current_task].context);
}
/**
 * Wait for a task to finish. If the task has not yet finished, the scheduler should
 * suspend this task and wake it up later when the task specified by handle has exited.
 *s
 * \param handle  This is the handle produced by task_create
 */
void task_wait(task_t handle) {
  // TODO: Block this task until the specified task has exited.
  tasks[current_task].state = WAITING;
  tasks[current_task].wait_for_task = handle;
  findReadyTask();
}

/**
 * The currently-executing task should sleep for a specified time. If that time is larger
 * than zero, the scheduler should suspend this task and run a different task until at least
 * ms milliseconds have elapsed.
 *
 * \param ms  The number of milliseconds the task should sleep.
 */
void task_sleep(size_t ms) {
  // TODO: Block this task until the requested time has elapsed.
  // Hint: Record the time the task should wake up instead of the time left for it to sleep. The
  // bookkeeping is easier this way.
  tasks[current_task].state = SLEEPING;
  tasks[current_task].sleep_time = time_ms() + ms;
  findReadyTask();
}

/**
 * Read a character from user input. If no input is available, the task should
 * block until input becomes available. The scheduler should run a different
 * task while this task is blocked.
 *
 * \returns The read character code
 */
int task_readchar() {
  // TODO: Block this task until there is input available.
  // To check for input, call getch(). If it returns ERR, no input was available.
  // Otherwise, getch() will returns the character code that was read.
  tasks[current_task].state = WAITINPUT;
  findReadyTask();
  if(tasks[current_task].input == ERR){
    return ERR;
  }else{
    return tasks[current_task].input;
  }
}
