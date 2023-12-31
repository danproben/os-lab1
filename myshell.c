// ACADEMIC INTEGRITY PLEDGE
//
// - I have not used source code obtained from another student nor
//   any other unauthorized source, either modified or unmodified.
//
// - All source code and documentation used in my program is either
//   my original work or was derived by me from the source code
//   published in the textbook for this course or presented in
//   class.
//
// - I have not discussed coding details about this project with
//   anyone other than my instructor. I understand that I may discuss
//   the concepts of this program with other students and that another
//   student may help me debug my program so long as neither of us
//   writes anything during the discussion or modifies any computer
//   file during the discussion.
//
// - I have violated neither the spirit nor letter of these restrictions.
//
//
//
// Signed:_________Daniel Proben___________ Date:____10/06/2023___

// 3460:426 Lab 1 - Basic C shell rev. 9/10/2020

/* Basic shell */

/*
 * This is a very minimal shell. It finds an executable in the
 * PATH, then loads it and executes it (using execv). Since
 * it uses "." (dot) as a separator, it cannot handle file
 * names like "minishell.h"
 *
 * The focus on this exercise is to use fork, PATH variables,
 * and execv.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>

#define MAX_ARGS 64
#define MAX_ARG_LEN 16
#define MAX_LINE_LEN 80
#define WHITESPACE " ,\t\n"

struct command_t
{
   char *name;
   int argc;
   char *argv[MAX_ARGS];
};

/* Function prototypes */
void printPrompt();
void readCommand(char *);
int parseCommand(char *, struct command_t *);

int main(int argc, char *argv[])
{
   int pid;
   int status;
   char cmdLine[MAX_LINE_LEN];
   struct command_t command;

   while (true)
   {
      printPrompt();
      /* Read the command line and parse it */
      readCommand(cmdLine);
      parseCommand(cmdLine, &command);
      command.argv[command.argc] = NULL;
      bool exitFlag = false;

      // in most cases, the command.name entered by the user can simply be replaced with the unix equivalent to be executed by the child
      switch (*command.name)
      {
      case 'C':
         // C file1 file2
         // Copy; create file2, copy all bytes of file1 to file2 without deleting file1.
         command.name = "cp";
         break;

      case 'D':
         // D file
         // Delete the named file.
         command.name = "rm";
         break;

      case 'E':
         // E comment
         // Echo; display comment on screen followed by a new line (multiple spaces/tabs may be reduced to a single space); if no argument simply issue a new prompt.
         command.name = "echo";
         if (!command.argv[1])
         {
            continue;
         }
         break;

      case 'H':
         // H
         // Help; display the user manual.
         command.name = "cat";
         command.argv[1] = "help.txt";

         break;

      case 'L':
      {
         // L
         // List the contents of the current directory
         // Skip a line
         printf("\n");

         // fork a child to handle the pwd command
         command.name = "pwd";
         if ((pid = fork()) == 0)
         {
            /* Child executing command */
            command.argv[0][0] = '\0';
            execvp(command.name, command.argv);
            // The execvp function will fail with an error and start executing below it. Handle this error here
            exit(1);
         }
         else
         {
            /* Wait for the child to terminate */
            wait(&status);
            printf("\n");

            // set the command struct for execution on the next fork on line 189
            command.name = "ls";
            command.argv[0] = "l";
         }
      }
      break;

      case 'M':
         // M file
         // Make; create the named text file by launching a text editor.
         command.name = "nano";
         break;

      case 'P':
         // Print; display the contents of the named file on screen.
         command.name = "cat";
         if (!command.argv[1])
         {
            continue;
         }
         break;

      case 'Q':
         // Quit the shell.
         command.name = "exit";
         exitFlag = true;
         command.argv[0][0] = '\0';
         break;

      case 'W':
         // Wipe; clear the screen.
         command.name = "clear";
         command.argv[0][0] = '\0';
         break;

      case 'X':
      {
         // Execute the named program.
         char destination[MAX_ARG_LEN + 2] = "./";
         strcat(destination, command.argv[1]);
         command.name = destination;
         command.argv[0][0] = '\0';
      }
      break;

      default:
         break;
      }

      // if the user entered Q, the exit flag was set to true and we can exit the loop and end the process without creating another child process
      if (exitFlag)
      {
         break;
      }

      /* Create a child process to execute the command */
      if ((pid = fork()) == 0)
      {
         /* Child executing command */
         execvp(command.name, command.argv);
         exit(EXIT_FAILURE);
      }
      else if (pid == -1)
      {
         return (EXIT_FAILURE);
      }
      else
      {
         /* Wait for the child to terminate */
         wait(&status);
      }
   }

   /* Shell termination */
   printf("\n\nshell: Terminating successfully\n");
   return 0;
}

/* End basic shell */

/* Parse Command function */

/* Determine command name and construct the parameter list.
 * This function will build argv[] and set the argc value.
 * argc is the number of "tokens" or words on the command line
 * argv[] is an array of strings (pointers to char *). The last
 * element in argv[] must be NULL. As we scan the command line
 * from the left, the first token goes in argv[0], the second in
 * argv[1], and so on. Each time we add a token to argv[],
 * we increment argc.
 */
int parseCommand(char *cLine, struct command_t *cmd)
{
   int argc;
   char **clPtr;
   /* Initialization */
   clPtr = &cLine; /* cLine is the command line */
   argc = 0;
   cmd->argv[argc] = (char *)malloc(MAX_ARG_LEN);
   /* Fill argv[] */
   while ((cmd->argv[argc] = strsep(clPtr, WHITESPACE)) != NULL)
   {
      cmd->argv[++argc] = (char *)malloc(MAX_ARG_LEN);
   }

   /* Set the command name and argc */
   cmd->argc = argc - 1;
   cmd->name = (char *)malloc(sizeof(cmd->argv[0]));
   strcpy(cmd->name, cmd->argv[0]);
   return 1;
}

/* End parseCommand function */

/* Print prompt and read command functions - Nutt pp. 79-80 */

void printPrompt()
{
   /* Build the prompt string to have the machine name,
    * current directory, or other desired information
    */
   char promptString[] = "linux(djp111)|>";
   printf("%s ", promptString);
}

void readCommand(char *buffer)
{
   /* This code uses any set of I/O functions, such as those in
    * the stdio library to read the entire command line into
    * the buffer. This implementation is greatly simplified,
    * but it does the job.
    */
   fgets(buffer, 80, stdin);
}

/* End printPrompt and readCommand */
