/* LD_PRELOAD library which launches gdb "just-in-time" in response to a process SIGSEGV-ing
 * Compile with:
 *
 * gcc -g -fpic -shared -nostartfiles -o jitdbg.so jitdbg.c
 *
 * then put in LD_PRELOAD before running process, e.g.:
 *
 * LD_PRELOAD=~/scripts/jitdbg.so defective_executable
 */

#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>


void gdb(int sig) {
  if(sig == SIGSEGV || sig == SIGABRT)
    {
      pid_t cpid = fork();
      if(cpid == -1)
        return;   // fork failed, we can't help, hope core dumps are enabled...
      else if(cpid != 0)
        {
          // Parent
          prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY, 0, 0, 0);  // allow any process to ptrace us
          raise(SIGSTOP);  // wait for child's gdb invocation to pick us up
        }
      else
        {
          // Child - now try to exec gdb in our place attached to the parent

          // Avoiding using libc since that may already have been stomped, so building the
          // gdb args the hard way ("gdb dummy PID"), first copy
          char cmd[100];
          const char* stem = "gdb _dummy_process_name_                   ";  // 18 trailing spaces to allow for a 64 bit proc id
          const char*s = stem;
          char* d = cmd;
          while(*s)
            {
            *d++ = *s++;
            }
          *d-- = '\0';
          char* hexppid = d;

          // now backfill the trailing space with the hex parent PID - not
          // using decimal for fear of libc maths helper functions being dragged in
          pid_t ppid = getppid();
          while(ppid)
            {
              *hexppid = ((ppid & 0xF) + '0');
              if(*hexppid > '9')
                *hexppid += 'a' - '0' - 10;
              --hexppid;
              ppid >>= 4;
            }
          *hexppid-- = 'x';   // prefix with 0x
          *hexppid = '0';
          // system() isn't listed as safe under async signals, nor is execlp,
          // or getenv. So ideally we'd already have cached the gdb location, or we
          // hardcode the gdb path, or we accept the risk of re-entrancy/library woes
          // around the environment fetch...
          execlp("mate-terminal", "mate-terminal", "-e", cmd, (char*) NULL);
        }
    }
}

void _init() {
  signal(SIGSEGV, gdb);
  signal(SIGABRT, gdb);
}
