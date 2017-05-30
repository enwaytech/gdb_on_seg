/* LD_PRELOAD library which launches gdb "just-in-time" in response to a process SIGSEGV-ing
 *
 * Usage:
 * LD_PRELOAD=<path to build folder>/gdb_on_seg.so executable
 */

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>
#include <stdio.h>

void start_gdb(int sig)
{
  // printf ("[gdb_on_seg]: Got sig: %d \n", sig);
  if(sig == SIGSEGV)
  {
    // allow any process to ptrace us
    prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY, 0, 0, 0);

    pid_t cpid = fork();
    if(cpid == -1)
    {
      printf ("[gdb_on_seg]: Fork failed.. failing\n");
      return;   // fork failed, we can't help, hope core dumps are enabled...
    }

    if(cpid != 0)
    {
      // In parent process, weird.. C stuff
      // Hang process so gdb can do stuff
      raise(SIGSTOP);
    }
    else
    {
      // In forked process, running gdb here
      // printf ("[gdb_on_seg]: In gdb process\n");

      pid_t ppid = getppid();
      unsigned int pidLength = sizeof(pid_t);
      char ppid_string[pidLength];
      snprintf(ppid_string, 10, "%d", ppid);
      printf ("[gdb_on_seg]: Tring to attach to ppid: %s \n", ppid_string);
      execlp("gdb", "gdb", "-p", ppid_string, "-ex", "set pagination off", "-ex", "thread apply all bt",
             "-ex", "q", (char*) NULL);
    }
  }
}

void _init() {
  signal(SIGSEGV, start_gdb);
}
