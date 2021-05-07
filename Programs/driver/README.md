#### This program is a simulation of a floppy/hard disk combination driver.  It is written with the C programming language.

The compiler used is gcc and the compilation instructions are these:

```$gcc -Wall -ansi -c driver.c```

```$gcc -odriver driver.o filesys.o disk.o -lm```

The program can be run within the Linux terminal with this command:

```./driver```

Inside of the driver, you have the option to run it to a certain point, where it will stop. You may enter
anything from 0 (manually step through all) to 32767 (run all the way through).  To have it run infinitely,
you may enter 40000.  In this mode, it will only show errors and warnings.

After that, you have the option to input a speed from 0 to 5000.  THIS SPEED IS NOT LINEAR.  I recommend
running it at 3850, that is the maximum speed to be able to monitor what is happening while it is moving quickly.
  
 Thank you for looking at my work!
