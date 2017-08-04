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

void print_signal(int signal)
{
  printf ("[gdb_on_seg]: Got signal: ");
  switch (signal)
  {
    case SIGHUP:
      printf ("SIGHUP");
      break;
    case SIGILL:
      printf ("SIGILL");
      break;
    case SIGFPE:
      printf ("SIGFPE");
      break;

    case SIGBUS:
      printf ("SIGBUS");
      break;
    case SIGSEGV:
      printf ("SIGSEGV");
      break;
    case SIGPIPE:
      printf ("SIGPIPE");
      break;

    case SIGSTKFLT:
      printf ("SIGSTKFLT");
      break;
    case SIGSYS:
      printf ("SIGSYS");
      break;

    default:
      printf ("UNDEFINED: Added signal but not to the print function(?)");
      break;
  }
  printf (", number %d \n", signal);
}

void start_gdb()
{
  // allow any process to ptrace us
  prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY, 0, 0, 0);

  pid_t child_process_id = fork(); //returns -1 on failure, 0 for child process
  if(child_process_id == -1)
  {
    printf ("[gdb_on_seg]: Fork failed.. failing\n");
    return;
  }

  if(child_process_id != 0)
  {
    // In parent process, weird.. C stuff
    // Hang process so gdb can do stuff
    raise(SIGSTOP);
  }
  else
  {
    // In forked child process, running gdb here
    // printf ("[gdb_on_seg]: In gdb process\n");

    pid_t parent_process_id = getppid();
    unsigned int pidLength = sizeof(pid_t);
    char parent_process_id_string[pidLength];
    snprintf(parent_process_id_string, 10, "%d", parent_process_id);
    printf ("[gdb_on_seg]: Tring to attach to parent process pid: %s \n", parent_process_id_string);

    // -ex executes commands in gdb after start. Commands explanation:
    // "set pagination off": Don't ask for enter press to continue showing more entries
    // "thread apply all bt": Print backtrace for all processes
    // "q": Quit, but still will wait for user confirmation ("y")
    execlp("gdb", "gdb","--silent", "-p", parent_process_id_string, "-ex", "set pagination off",
           "-ex", "thread apply all bt", "-ex", "q", (char*) NULL);
  }
}

void print_signal_and_start_gdb(int signal)
{
  print_signal(signal);
  start_gdb();
}

void catch_exit(int exit_code, void* arg)
{
  if (exit_code < 0)
  {
    printf ("[gdb_on_seg]: Caught negative exit code %d \n", exit_code);
    start_gdb();
  }
}

void _init()
{
  // from http://man7.org/linux/man-pages/man7/signal.7.html#Standard%20signals
  // and http://www.comptechdoc.org/os/linux/programming/linux_pgsignals.html
  // and https://people.cs.pitt.edu/~alanjawi/cs449/code/shell/UnixSignals.htm

  signal(SIGHUP, print_signal_and_start_gdb);    // Hangup detected
  signal(SIGILL, print_signal_and_start_gdb);    // Illegal Instruction
  signal(SIGFPE, print_signal_and_start_gdb);    // Floating-point exception

  signal(SIGBUS, print_signal_and_start_gdb);    // Bus error (bad memory access)
  signal(SIGSEGV, print_signal_and_start_gdb);   // Invalid memory reference
  signal(SIGPIPE, print_signal_and_start_gdb);   // Broken pipe: write to pipe with no readers

  signal(SIGSTKFLT, print_signal_and_start_gdb); // Stack fault
  signal(SIGSYS, print_signal_and_start_gdb);    // Bad system call (SVr4)

  void* unused;
  on_exit(catch_exit, unused);
}
