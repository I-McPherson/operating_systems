/**********************************************************************************/
/* This file is not indended to be compiled nor run.  It is simply pseudocode     */
/* connecting three UNIX process together with two pipes by using the UNIX system */
/* calls.  It is written with C programming language code, syntax, and semantics. */
/**********************************************************************************/

#define STD_INPUT 0 /* File descriptor for standard input */
#define STD_OUTPUT 1 /* File descriptor for standard output */
pipeline(process1, process2, process3)
/* Connects three processes with two pipes */
char *process1, *process2, *process3;
/* Points to each user command */
{
	int fd[2]; /* File descriptors for the pipe */
	pipe(&fd[0]); /* Creates the first pipe */
	if (fork() != 0) /* Creates the firsts child, and checks who is */
	/* returning from the fork() */
		{
		/* This is executed by the parent, process1 */
		close(fd[0]); /* Closes the read file descriptor because the */
		/* process1 does not need to read from the pipe */
		close(STD_OUTPUT); /* Closes the standard output to prepare for the */
		/* new output to the pipe */
		dup (fd[1]); /* Sets the output to write to the pipe */
		close(fd[1]); /* Closes the write file descriptor because it is */
		/* not needed anymore */
		execl(process1, process1, 0);
		/* Turns the process1 into the user’s commands */
		}
	else
	{
		/* This is executed by the first child, process2 */
		close(fd[1]); /* Process2 does not need to write to the pipe */
		close(STD_INPUT); /* Prepares for new standard input */
		dup (fd[0]; /* Sets the standard input to the read from the */
		/* pipe */
		close(fd[0]); /* The read pipe file descriptor is no longer */
		/* needed
		pipe(&fd[0]); /* Creates the second pipe */
		if (fork() != 0) /* Creates the second child, and checks who is */
		/* returning from the fork() */
		{
		close(fd[0]); /* Process2 does not need to read from the pipe */
		close(STD_OUTPUT);
		/* Prepares process2 for new output to the pipe */
		dup (fd[1]); /* Sets the process2 output to write to the pipe */
		close(fd[1]); /* The write file descriptor is no longer needed */
		execl(process2, process2, 0);
		/* Turns the process2 into the user’s commands */
		}
		else
		{
		/* This is executed by the second child, process3 */
		close(fd[1]); /* Process3 does not need to write to the pipe */
		close(STD_INPUT); /* Prepares for new standard input */
		dup (fd[0]); /* Sets the process3 input to come from the pipe */
		close(fd[0]); /* The read file descriptor is no longer needed */
		execl(process3, process3, 0);
		/* Turns the process3 into the user’s commands */
		}
	}
}
