/**********************************************************************/
/*                                                                    */
/* Program Name: driver - Simulate a hard disk driver                 */
/* Author:       Ian McPherson                                        */
/* Installation: Pensacola Christian College, Pensacola, Florida      */
/* Course:       CS326, Operating Systems                             */
/* Date Written: April 7, 2021                                        */
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
/* This program simulates a hard disk driver.  It acts as an          */
/* interface between the file system and the hard disk.  It accepts   */
/* read or write requests, transforms them from disk blocks to the    */
/* physical hard disk locations, and then instructs the disk to carry */
/* out the requests.                                                  */
/*                                                                    */
/**********************************************************************/
#include <stdio.h>  /* scanf, printf                                  */

/**********************************************************************/
/*                        Symbolic Constants                          */
/**********************************************************************/
#define SECTORS_PER_TRACK   9   /* The amount of sectors per track    */
#define TRACKS_PER_CYLINDER 2   /* The amount of tracks per cylinder  */
#define CYLINDERS_PER_DISK  40  /* The amount of cylinders in a disk  */
#define BYTES_PER_SECTOR    512 /* The amount of bytes in a sector    */
#define TRUE                1   /* True                               */
#define MOTOR_OFF          -1   /* The value for the disk being off   */
#define SENSE_CYLINDER      1   /* Sense the disk head position       */
#define SEEK_CYLINDER       2   /* Send the disk head to a cylinder   */
#define DMA_SETUP           3   /* Set the DMA registers              */
#define START_MOTOR         4   /* Start the disk motor               */
#define STATUS_MOTOR        5   /* Get the status of the disk motor   */
#define READ_DATA           6   /* Read the data from DMA registers   */
#define WRITE_DATA          7   /* Write the data to DMA registers    */
#define STOP_MOTOR          8   /* Stop the disk motor                */
#define RECALIBRATE         9   /* Recalibrate the disk heads         */

/**********************************************************************/
/*                        Program Structures                          */
/**********************************************************************/
/* The definition of a message                                        */
struct message
{
                 int operation_code;  /* The disk operation           */
                 int request_number;  /* The unique request number    */
                 int block_number;    /* The block number to be       */
                                      /* processed                    */
                 int block_size;      /* The block size in bytes      */
   unsigned long int *p_data_address; /* Points to the data block     */
};

/**********************************************************************/
/*                          Global Variables                          */
/**********************************************************************/
struct message fs_message[20];      /* The file system requests       */
struct message pending_request[20]; /* Pending file system requests   */

/**********************************************************************/
/*                        Function Prototypes                         */
/**********************************************************************/
int  disk_drive(int code, int arg1, int arg2, int arg3,
                unsigned long int p_arg4);
   /* Sends an operation request to the disk drive                    */
void send_message (struct message fs_message[]);
   /* Sends a message to the file system                              */
void convert_block(int block, int *p_cylinder, int *p_track,
                   int *p_sector);
   /* Converts a block number into its disk location                  */
int  check_for_errors(int index);
   /* Checks for errors with the current pending request              */
void sort_request_list();
   /* Sorts the pending request list into ascending order             */
void add_to_pending();
   /* Adds the file system requests to the pending request list       */
int  find_next(int block);
   /* Finds the next request to run with the elevator algorithm       */
int  count_pending();
   /* Counts the amount of pending requests                           */
void make_idle_request();
   /* Creates an idle request for the file system                     */
void remove_pending(int index);
   /* Removes the finished request from the pending request list      */

/**********************************************************************/
/*                          Main Function                             */
/**********************************************************************/
int main()
{
   int cylinder,          /* The cylinder to be seeked to             */
       index = 0,         /* The index for the pending requests       */
       track,             /* The track number of the data             */
       sector,            /* The sector number for the data           */
       motor_status = -1, /* The running status of the motor          */
       current_cylinder,  /* The cylinder the disk is currently on    */
       amt_of_idle = 0;   /* The amount of idle requests              */

   make_idle_request();

   /* Loops forever processing file system requests                   */
   while (TRUE)
   {
      /* Sends and receives requests, adding them to the pending list */
      /* and then sorts the pending list.                             */
      send_message(fs_message);
      add_to_pending();
      sort_request_list();

      /* Check whether the file system returned an idle request       */
      if (pending_request[0].operation_code == 0 || count_pending() < 1)
      {
         make_idle_request();
         amt_of_idle++;
         if (amt_of_idle >= 2)
         {
            if (motor_status != MOTOR_OFF)
            {
               disk_drive(STOP_MOTOR, 0, 0, 0, 0);
               motor_status = MOTOR_OFF;
            }
         }
      }
      else
      {
         amt_of_idle = 0;

         /* Find the next request to be processed                    */
         index = find_next(pending_request[index].block_number);

         /* Start the disk drive motor                                */
         if (motor_status == MOTOR_OFF)
         {
            disk_drive(START_MOTOR, 0, 0, 0, 0);
            while ((motor_status =
                     disk_drive(STATUS_MOTOR, 0, 0, 0, 0)) == MOTOR_OFF)
               disk_drive(START_MOTOR, 0, 0, 0, 0);
         }

         /* Sense which cylinder the hard disk is currently at        */
         current_cylinder = disk_drive(SENSE_CYLINDER, 0, 0, 0, 0);

         /* Convert the disk blocks to the hard disk location         */
         convert_block(pending_request[index].block_number, &cylinder,
            &track, &sector);

         /* Seek the disk to the current cylinder                     */
         while (current_cylinder != cylinder)
            if ((current_cylinder = disk_drive(SEEK_CYLINDER,
                                        cylinder, 0, 0, 0)) != cylinder)
               while (disk_drive(RECALIBRATE, 0, 0, 0, 0) != 0)
                  if ((current_cylinder =
                    disk_drive(SENSE_CYLINDER, 0, 0, 0, 0)) != cylinder)
                     current_cylinder =
                            disk_drive(SEEK_CYLINDER, cylinder, 0, 0, 0);

         /* Check for errors in the pending request information       */
         if (check_for_errors(index) == 0)
         {
            /* Setup the DMA registers for reading writing            */
            while(disk_drive(DMA_SETUP, sector, track,
                     pending_request[index].block_size,
                     (long)pending_request[index].p_data_address) != 0);

            /* Perform the correct request operation                  */
            switch (pending_request[index].operation_code)
            {
               /* Read the data from the disk                         */
               case 1:
                  while (disk_drive(READ_DATA, 0, 0, 0, 0) == -2);
                  break;
                /* Write the data to the disk                         */
               case 2:
                  while (disk_drive(WRITE_DATA, 0, 0, 0, 0) == -2);
                  break;
            }
            pending_request[index].operation_code = 0;
         }

         /* Fill the file system message with proper return values    */
         fs_message[0] = pending_request[index];
         pending_request[index].operation_code = 0;
         sort_request_list();
         remove_pending(index);
      }
   }

   return 0;
}

/**********************************************************************/
/*          Converts a block number into its disk location            */
/**********************************************************************/
void convert_block(int block, int *p_cylinder, int *p_track,
                   int *p_sector)
{
   /* Calculate the hard disk cylinder numbers                        */
   *p_cylinder = (int)((block - 1) / SECTORS_PER_TRACK);

   /* Calculate the hard disk track numbers                           */
   *p_track = (int)((block - 1) % SECTORS_PER_TRACK);
   if (*p_track < 4.5f)
      *p_track = 0;
   else
      *p_track = 1;

   /* Calculate the hard disk sector numbers                          */
   *p_sector = (int)(((block - 1) % SECTORS_PER_TRACK) *
                                                   TRACKS_PER_CYLINDER);
   if (*p_sector > SECTORS_PER_TRACK)
      *p_sector -= SECTORS_PER_TRACK;
   return;
}

/**********************************************************************/
/*             Checks for errors with the pending request             */
/**********************************************************************/
int check_for_errors(int index)
{
   int error_code = 0; /* The error code for an invalid request       */

   if(pending_request[index].operation_code != 1 &&
      pending_request[index].operation_code != 2)
      error_code += -1;

   if(pending_request[index].request_number <= 0)
      error_code += -2;
   
   if(pending_request[index].block_number < 1 ||
      pending_request[index].block_number >
                             (CYLINDERS_PER_DISK * TRACKS_PER_CYLINDER *
                              SECTORS_PER_TRACK / TRACKS_PER_CYLINDER))
      error_code += -4;
   
   if(((pending_request[index].block_size < 0  &&
       (pending_request[index].block_number &
                  (pending_request[index].block_number - 1)) == 0) ||
        pending_request[index].block_size > 1024))
      error_code += -8;
   
   if(pending_request[index].p_data_address < 0)
      error_code += -16;

   if(error_code < 0)
      pending_request[index].operation_code = error_code;
   return error_code;
}

/**********************************************************************/
/*        Sorts the pending request list into ascending order         */
/**********************************************************************/
void sort_request_list()
{
   struct message temp_message; /* Temporary message for swapping     */
   int            current,      /* The current pending request        */
                  next;         /* The next pending request           */

   for (current = 0 ; current < count_pending(); current++)
   {
      for (next = 0 ; next < (count_pending() - 1); next++)
      {
         if (pending_request[next].block_number >
             pending_request[next + 1].block_number)
         {
            temp_message              = pending_request[next];
            pending_request[next]     = pending_request[next + 1];
            pending_request[next + 1] = temp_message;
         }
      }
   }
   return;
}

/**********************************************************************/
/*     Adds the file system requests to the pending request list      */
/**********************************************************************/
void add_to_pending()
{
   int message_index = 0, /* The index for the file system messages   */
       index = 0;         /* The index for the pending messages       */

   while (fs_message[message_index].operation_code != 0 &&
          message_index < 20)
   {
      while (pending_request[index].operation_code != 0 &&
             index < 20)
         index++;
      pending_request[index] = fs_message[message_index];
      index++;
      message_index++;
   }
   sort_request_list();
   return;
}

/**********************************************************************/
/*     Finds the next request to run with the elevator algorithm      */
/**********************************************************************/
int find_next(int block)
{
   int index = 0; /* The index of a pending request                   */

   while((pending_request[index].block_number < block ||
          pending_request[index].operation_code == 0) &&
          index < 20)
      index++;

   if(index == 20)
   {
      index   = 0;
      block = 0;
      while((pending_request[index].block_number < block ||
             pending_request[index].operation_code == 0) &&
             index < 20)
         index++;
   }
   return index;
}

/**********************************************************************/
/* Counts the amount of pending requests in the pending request list  */
/**********************************************************************/
int count_pending()
{
   int amount_pending = 0, /* The amount of pending requests          */
       index;              /* The index for a pending request         */

   for (index = 0; index < 20; index++)
      if(pending_request[index].operation_code != 0)
         amount_pending++;
   return amount_pending;
}

/**********************************************************************/
/*            Creates an idle request for the file system             */
/**********************************************************************/
void make_idle_request()
{
   fs_message[0].operation_code = 0;
   fs_message[0].request_number = 0;
   fs_message[0].block_number = 0;
   fs_message[0].block_size = 0;
   fs_message[0].p_data_address = NULL;
   return;
}

/**********************************************************************/
/*    Removes the finished request from the pending request list      */
/**********************************************************************/
void remove_pending(int index)
{
   while (index < 19)
   {
      pending_request[index] = pending_request[index + 1];
      index++;
   }
   pending_request[19].operation_code = 0;
   pending_request[19].request_number = 0;
   pending_request[19].block_number = 0;
   pending_request[19].block_size = 0;
   pending_request[19].p_data_address = NULL;
   return;
}