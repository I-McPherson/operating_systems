/**********************************************************************/
/*                                                                    */
/* Program Name: scheduler - Convert block numbers to hard disk       */
/*                           locations                                */
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
/* This program converts a fily system block number into a disk       */
/* drive's cylinder, track, and sector numbers and prints them to a   */
/* table.                                                             */
/*                                                                    */
/**********************************************************************/
#include <stdio.h>  /* printf                                         */

/**********************************************************************/
/*                        Symbolic Constants                          */
/**********************************************************************/
#define SECTORS_PER_TRACK   9   /* The amount of sectors per track    */
#define TRACKS_PER_CYLINDER 2   /* The amount of tracks per cylinder  */
#define CYLINDERS_PER_DISK  40  /* The amount of cylinders per disk   */
#define BYTES_PER_SECTOR    512 /* The amount of bytes per sector     */
#define TOTAL_BLOCKS        SECTORS_PER_TRACK * CYLINDERS_PER_DISK
                                /* The total amount of file system    */
                                /* blocks                             */

/**********************************************************************/
/*                        Function Prototypes                         */
/**********************************************************************/
void convert_block(int block, int *p_cylinder, int *p_track,
                   int *p_sector);
   /* Converts a block number into the corresponding disk geometry    */

/**********************************************************************/
/*                           Main Function                            */
/**********************************************************************/
int main()
{
   int block,    /* The file system block number                      */
       cylinder, /* The cylinder number on the disk                   */
       sector,   /* The sector number on the disk                     */
       track;    /* The track number on the disk                      */

   /* Prints the table column headings                                */
   printf(  "   Block   Cylinder   Track   Sector");
   printf("\n   -----   --------   -----   ------");

   /* Loops for all disk blocks, calculating the hard disk locations  */
   for (block = 1; block <= TOTAL_BLOCKS; block++)
   {
      convert_block(block, &cylinder, &track, &sector);
      printf("\n   %4d    %4d       %3d    %4d", block, cylinder,
         track, sector);
   }
   return 0;
}

/**********************************************************************/
/* Convert a block number into ts cylinder, track, and sector numbers */
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
   *p_sector = (((block - 1) % SECTORS_PER_TRACK) *
                                                   TRACKS_PER_CYLINDER);
   if (*p_sector > SECTORS_PER_TRACK)
      *p_sector -= SECTORS_PER_TRACK;
   return;
}
