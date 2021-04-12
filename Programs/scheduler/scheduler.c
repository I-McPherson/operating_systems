/**********************************************************************/
/*                                                                    */
/* Program Name: scheduler - Simulate a process scheduler             */
/* Author:       Ian McPherson                                        */
/* Installation: Pensacola Christian College, Pensacola, Florida      */
/* Course:       CS326, Operating Systems                             */
/* Date Written: March 15, 2021                                       */
/*                                                                    */
/**********************************************************************/

/**********************************************************************/
/*                                                                    */
/* I pledge this assignment is my own first time work.                */
/* I pledge I did not copy or try to copy work from the Internet.     */
/* I pledge I did not copy or try to copy work from any student.      */
/* I pledge I did not copy or try to copy work from any where else.   */
/* I pledge the only person I asked for help from was my teacher.     */
/* I pledge I did not attempt to help any student on this assignment. */
/* I understand if I violate this pledge I will receive a 0 grade.    */
/*                                                                    */
/*                                                                    */
/*                      Signed: _____________________________________ */
/*                                           (signature)              */
/*                                                                    */
/*                                                                    */
/**********************************************************************/


/**********************************************************************/
/*                                                                    */
/* This program simulates a UNIX scheduler.  It will add new          */
/* processes to a process table, while scheduling each process with   */
/* the round-robin scheduling algorithm.  Each process can be         */
/* blocked, terminated, preempted, or run depending on their states.  */
/*                                                                    */
/**********************************************************************/

#include <stdio.h>  /* scanf, printf                                  */
#include <stdlib.h> /* srand                                          */
#include <math.h>   /* abs, round                                     */

/**********************************************************************/
/*                        Symbolic Constants                          */
/**********************************************************************/
#define TERMINATE_PRIORITY   999 /* Priority of a terminated process  */
#define PROCESS_TABLE_LENGTH 10  /* Length of the process table       */
#define MAXIMUM_PID          100 /* The maximum PID of a process      */
#define RUNNING              'N' /* The running state of a process    */
#define READY                'R' /* The ready state for a process     */
#define BLOCKED              'B' /* The blocked state for a process   */

/**********************************************************************/
/*                         Global Variables                           */
/**********************************************************************/
typedef int semaphore;
semaphore mutex = 1; /* Ensures only one process may run at a time    */

/**********************************************************************/
/*                        Program Structures                          */
/**********************************************************************/
/* A definition of a process                                          */
struct process
{
   int  pid,          /* A processes' process ID                      */
        cpu_used,     /* A processes' cpu time used in ticks          */
        max_time,     /* A processes' maximum time until completed    */
        pri,          /* A processes' current priority                */
        quantum_used, /* A processes' used quantum                    */
        blk_time,     /* A processes' time until blocked              */
        wait_tks;     /* A processes' time waited in ticks            */
   char state;        /* A processes' current state                   */
};
typedef struct process PROCESS;


/**********************************************************************/
/*                        Function Prototypes                         */
/**********************************************************************/
PROCESS initialize_process(PROCESS process_table[], int count);
   /* Initializes a new process                                       */
void print_table(PROCESS process_table[], int process_amt, int pid,
                 char *name);
   /* Prints the process table                                        */
void schedule_process(PROCESS process_table[], int process_amt);
   /* Schedules a process to be run                                   */
void terminate_process(PROCESS process_table[], int count);
   /* Terminates a process that has completed execution               */
void sort_table(PROCESS process_table[], int process_amt);
   /* Sorts the process table by priority                             */
void calculate_priority(PROCESS process_table[], int count);
   /* Calculates the new priority for a process                       */
void switch_priority(PROCESS process_table[], int process_amt);
   /* Switches the negative process priorities                        */
int check_for_running(PROCESS process_table[], int process_amt);
   /* Checks if there is a currently running process                  */

/**********************************************************************/
/*                          Main Function                             */
/**********************************************************************/
int main()
{
   PROCESS process_table[PROCESS_TABLE_LENGTH];
                    /* The process table                              */
   int count,       /* The index for the process table                */
       pid,         /* The pid of the current process                 */
       process_amt; /* The amount of processes in the process table   */

   /* Initializes the proces table with 5 processes                   */
   for (count = 0; count < 5; count++)
      process_table[count] = initialize_process(process_table, count);
   pid         = count;
   process_amt = count;

   /* Loops until the last proces is created                          */
   while (pid != MAXIMUM_PID)
   {
      /* Loop through the process table processing processes          */
      for (count = 0; count < process_amt; count++)
      {
         if (check_for_running(process_table, process_amt) ||
               process_table[count].cpu_used ==
                                        process_table[count].max_time ||
               process_table[count].quantum_used ==
                                        process_table[count].blk_time)
         {
            /* Prints the process table's before state                */
            sort_table(process_table, process_amt);
            print_table(process_table, process_amt, pid, "BEFORE");

            /* Schedules a ready process                              */
            if (process_table[count].state == READY)
            {
               sort_table(process_table, process_amt);
               schedule_process(process_table, process_amt);
            }

            /* Terminates a process when it is done executing         */
            if (process_table[count].cpu_used ==
                                        process_table[count].max_time &&
               process_table[count].state == RUNNING)
            {
               terminate_process(process_table, count);
               sort_table(process_table, process_amt);
               process_amt -= 1;
               mutex = 1;
            }

            /* Preempts a process that has finished its quantum       */
            if (process_table[count].quantum_used ==
                                        process_table[count].blk_time &&
               process_table[count].state == RUNNING)
            {
               if (process_table[count].blk_time == 6)
                  process_table[count].state = READY;
               else
                  process_table[count].state = BLOCKED;
               calculate_priority(process_table, count);
               process_table[count].quantum_used = 0;
               mutex = 1;
               sort_table(process_table, process_amt);
            }

            /* Prints the process table's after state                 */
            sort_table(process_table, process_amt);
            print_table(process_table, process_amt, pid, "AFTER");
         }
      }

      /* Adds a new process to the process table                      */
      if (!(rand() % 5))
      {
         if (process_amt < PROCESS_TABLE_LENGTH)
         {
            process_table[process_amt] =
                                 initialize_process(process_table, pid);
            pid += 1;
            process_amt += 1;
         }
         else
         {
            if (process_table[count].pri == TERMINATE_PRIORITY)
               process_table[PROCESS_TABLE_LENGTH - 1] =
                                 initialize_process(process_table, count);
         }
         sort_table(process_table, process_amt);
      }

      /* Runs a process when its state is set to running              */
      for (count = 0; count < process_amt; count++)
      {
         /* Unblocks each process with a 5% chance                    */
         if (!(rand() % 20) && process_table[count].state == BLOCKED &&
             process_table[count].pri != TERMINATE_PRIORITY)
         {
            process_table[count].state = READY;
         }

         /* Runs a process for each clock tick                        */
         if (process_table[count].state == RUNNING)
         {
            if (process_table[count].cpu_used <
                                        process_table[count].max_time ||
                process_table[count].quantum_used <
                                        process_table[count].blk_time)
            {
               process_table[count].cpu_used += 1;
               process_table[count].quantum_used += 1;
            }
         }
         else
            if (process_table[count].state == READY)
               process_table[count].wait_tks += 1;
      }
   }
   return 0;
}

/**********************************************************************/
/*       Initialize a new process and add to the process table        */
/**********************************************************************/
PROCESS initialize_process(PROCESS process_table[], int count)
{
   PROCESS new_process; /* A new process to be initialized            */

   new_process.pid = (count + 1);
   new_process.cpu_used = 0;
   new_process.max_time = ((rand() % 18) + 1);
   new_process.state = READY;
   new_process.pri = 0;
   new_process.quantum_used = 0;
   if (rand() % 3)
      new_process.blk_time = (rand() % 5) + 1;
   else
      new_process.blk_time = 6;
   new_process.wait_tks = 0;
   return new_process;
}

/**********************************************************************/
/*             Print out each process and its information             */
/**********************************************************************/
void print_table(PROCESS process_table[], int process_amt, int pid,
                 char *name)
{
   int count; /* The index for each process in the process table      */

   printf(  " %s SCHEDULING CPU:  Next PID = %2d,", name, pid + 1);
   printf( "  Number of Processes = %2d", process_amt);
   printf("\n PID   CPU Used   MAX Time   STATE   PRI   QUANTUM USED");
   printf("   BLK TIME   WAIT TKS");

   for (count = 0; count < process_amt; count++)
   {
      if (process_table[count].pri != TERMINATE_PRIORITY)
      {
         printf("\n %3d   %5d        %3d        %1c     %3d       %3d",
               process_table[count].pid, process_table[count].cpu_used,
               process_table[count].max_time, process_table[count].state,
               process_table[count].pri,
               process_table[count].quantum_used);
         printf("            %1d         %3d",
               process_table[count].blk_time,
               process_table[count].wait_tks);
      }
   }
   printf("\n\n");
   return;
}

/**********************************************************************/
/*                         Schedule a process                         */
/**********************************************************************/
void schedule_process(PROCESS process_table[], int process_amt)
{
   int count; /* The index for each process in the process table      */

   for (count = 0; count < process_amt; count++)
   {
      if (process_table[count].state == READY &&
          process_table[count].pri != TERMINATE_PRIORITY &&
          mutex == 1)
      {
         mutex = 0;
         process_table[count].state = RUNNING;
      }
   }
   return;
}

/**********************************************************************/
/*          Terminate a process that has finished executing           */
/**********************************************************************/
void terminate_process(PROCESS process_table[], int count)
{
   process_table[count].pri = TERMINATE_PRIORITY;
   process_table[count].state = BLOCKED;
   return;
}

/**********************************************************************/
/*            Sort the process table according to priority            */
/**********************************************************************/
void sort_table(PROCESS process_table[], int process_amt)
{
   PROCESS temp_process; /* Temporary process for swapping            */
   int before,           /* The before process                        */
       after;            /* The after process                         */

   switch_priority(process_table, process_amt);
   for (before = 0 ; before < process_amt - 1; before++)
   {
      for (after = 0 ; after < process_amt - before - 1; after++)
      {
         if (process_table[after].pri > process_table[after + 1].pri)
         {
            temp_process             = process_table[after];
            process_table[after]     = process_table[after + 1];
            process_table[after + 1] = temp_process;
         }
      }
   }
   switch_priority(process_table, process_amt);
   return;
}

/**********************************************************************/
/*                   Calculate a processes priority                   */
/**********************************************************************/
void calculate_priority(PROCESS process_table[], int count)
{
   int new_pri; /* The new priority for a process                     */

   new_pri = (int)(float)((process_table[count].pri +
                              process_table[count].quantum_used) * .5f);
   if (process_table[count].state == BLOCKED)
      new_pri = -(new_pri);
   process_table[count].pri = new_pri;
   return;
}


/**********************************************************************/
/*              Switch a processes priority for sorting               */
/**********************************************************************/
void switch_priority(PROCESS process_table[], int process_amt)
{
   int count; /* The index for each process in the process table      */

   for (count = 0; count < process_amt; count++)
   {
      switch (process_table[count].pri)
      {
         case -1:
            process_table[count].pri = -5;
            break;
         case -2:
            process_table[count].pri = -4;
            break;
         case -4:
            process_table[count].pri = -2;
            break;
         case -5:
            process_table[count].pri = -1;
            break;
      }
   }
   return;
}

/**********************************************************************/
/*           Checks whether there are any running processes           */
/**********************************************************************/
int check_for_running(PROCESS process_table[], int process_amt)
{
   int count,      /* The index for each process in the process table */
       is_running; /* The value for when there is a running process   */

   for (count = 0; count < process_amt; count++)
   {
      if (process_table[count].state == RUNNING)
      {
         is_running = 0; /* THIS MEANS PROCESS IS RUNNING */
         break;
      }
      else
         is_running = 1;
   }
   return is_running;
}
