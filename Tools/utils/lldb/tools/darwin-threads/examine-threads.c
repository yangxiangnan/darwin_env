#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mach/mach.h>
#include <time.h>
#include <sys/sysctl.h>
#include <ctype.h>
#include <libproc.h>
#include <errno.h>

/* Step through the process table, find a matching process name, return
   the pid of that matched process.
   If there are multiple processes with that name, issue a warning on stdout
   and return the highest numbered process.
   The proc_pidpath() call is used which gets the full process name including
   directories to the executable and the full (longer than 16 character) 
   executable name. */

pid_t
get_pid_for_process_name (const char *procname)
{
  int process_count = proc_listpids (PROC_ALL_PIDS, 0, NULL, 0) / sizeof (pid_t);
  if (process_count < 1)
    {
      printf ("Only found %d processes running!\n", process_count);
      exit (1);
    }

  // Allocate a few extra slots in case new processes are spawned
  int all_pids_size = sizeof (pid_t) * (process_count + 3);
  pid_t *all_pids = (pid_t *) malloc (all_pids_size);

  // re-set process_count in case the number of processes changed (got smaller; we won't do bigger)
  process_count = proc_listpids (PROC_ALL_PIDS, 0, all_pids, all_pids_size) / sizeof (pid_t);

  int i;
  pid_t highest_pid = 0;
  int match_count = 0;
  for (i = 1; i < process_count; i++)
    {
      char pidpath[PATH_MAX];
      int pidpath_len = proc_pidpath (all_pids[i], pidpath, sizeof (pidpath));
      if (pidpath_len == 0)
        continue;
      char *j = strrchr (pidpath, '/');
      if ((j == NULL && strcmp (procname, pidpath) == 0)
          || (j != NULL && strcmp (j + 1, procname)  == 0))
        {
          match_count++;
          if (all_pids[i] > highest_pid)
            highest_pid = all_pids[i];
        }
    }
  free (all_pids);

  if (match_count == 0)
    {
      printf ("Did not find process '%s'.\n", procname);
      exit (1);
    }
  if (match_count > 1)
    {
      printf ("Warning:  More than one process '%s'!\n", procname);
      printf ("          defaulting to the highest-pid one, %d\n", highest_pid);
    }
  return highest_pid;
}

/* Given a pid, get the full executable name (including directory 
   paths and the longer-than-16-chars executable name) and return
   the basename of that (i.e. do not include the directory components).
   This function mallocs the memory for the string it returns;
   the caller must free this memory. */

const char *
get_process_name_for_pid (pid_t pid)
{
  char tmp_name[PATH_MAX];
  if (proc_pidpath (pid, tmp_name, sizeof (tmp_name)) == 0)
    {
      printf ("Could not find process with pid of %d\n", (int) pid);
      exit (1);
    }
  if (strrchr (tmp_name, '/'))
    return strdup (strrchr (tmp_name, '/') + 1);
  else
    return strdup (tmp_name);
}

/* Get a struct kinfo_proc structure for a given pid.
   Process name is required for error printing.
   Gives you the current state of the process and whether it is being debugged by anyone.
   memory is malloc()'ed for the returned struct kinfo_proc
   and must be freed by the caller.  */

struct kinfo_proc *
get_kinfo_proc_for_pid (pid_t pid, const char *process_name)
{
  struct kinfo_proc *all_kinfos;
  int mib[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0 };
  size_t len;
  if (sysctl (mib, 3, NULL, &len, NULL, 0) != 0)
    {
      printf ("Could not number of processes\n");
      exit (1);
    }
  all_kinfos = (struct kinfo_proc *) malloc (len);
  if (sysctl (mib, 3, all_kinfos, &len, NULL, 0) != 0)
    {
      printf ("Could not get process infos\n");
      exit (1);
    }

  int proc_count, i;
  proc_count = len / sizeof (struct kinfo_proc);
  for (i = 0 ; i < proc_count; i++)
    if (all_kinfos[i].kp_proc.p_pid == pid)
      {
        struct kinfo_proc *kinfo = (struct kinfo_proc *) malloc (sizeof (struct kinfo_proc));
        memcpy (kinfo, &all_kinfos[i], sizeof (struct kinfo_proc));
        free ((void *) all_kinfos);
        return kinfo;
      }
  printf ("Did not find process '%s' when re-getting proc table.\n", process_name);
  exit (1);
}

/* Get the basic information (thread_basic_info_t) about a given
   thread.  
   Gives you the suspend count; thread state; user time; system time; sleep time; etc.
   The return value is a pointer to malloc'ed memory - it is the caller's
   responsibility to free it.  */

thread_basic_info_t
get_thread_basic_info (thread_t thread)
{
  kern_return_t kr;
  integer_t *thinfo = (integer_t *) malloc (sizeof (integer_t) * THREAD_INFO_MAX);
  mach_msg_type_number_t thread_info_count = THREAD_INFO_MAX;
  kr = thread_info (thread, THREAD_BASIC_INFO,
                    (thread_info_t) thinfo, &thread_info_count);
  if (kr != KERN_SUCCESS)
    {
      printf ("Error - unable to get basic thread info for a thread\n");
      exit (1);
    }
  return (thread_basic_info_t) thinfo;
}

/* Get the thread identifier info (thread_identifier_info_data_t) 
   about a given thread. 
   Gives you the system-wide unique thread number; the pthread identifier number 
*/

thread_identifier_info_data_t 
get_thread_identifier_info (thread_t thread)
{
  kern_return_t kr;
  thread_identifier_info_data_t tident;
  mach_msg_type_number_t tident_count = THREAD_IDENTIFIER_INFO_COUNT;
  kr = thread_info (thread, THREAD_IDENTIFIER_INFO,
                    (thread_info_t) &tident, &tident_count);
  if (kr != KERN_SUCCESS)
    {
      printf ("Error - unable to get thread ident for a thread\n");
      exit (1);
    }
  return tident;
}

/* Get the current pc value for a given thread.  */

uint64_t
get_current_pc (thread_t thread, int *wordsize)
{
  kern_return_t kr;

#if defined (__x86_64__) || defined (__i386__)
  x86_thread_state_t gp_regs;
  mach_msg_type_number_t gp_count = x86_THREAD_STATE_COUNT;
  kr = thread_get_state (thread, x86_THREAD_STATE,
                         (thread_state_t) &gp_regs, &gp_count);
  if (kr != KERN_SUCCESS)
    {
      printf ("Error - unable to get registers for a thread\n");
      exit (1);
    }

  if (gp_regs.tsh.flavor == x86_THREAD_STATE64)
    {
      *wordsize = 8;
      return gp_regs.uts.ts64.__rip;
    }
  else
    {
      *wordsize = 4;
      return gp_regs.uts.ts32.__eip;
    }
#endif

#if defined (__arm__)
  arm_thread_state_t gp_regs;
  mach_msg_type_number_t gp_count = ARM_THREAD_STATE_COUNT;
  kr = thread_get_state (thread, ARM_THREAD_STATE,
                         (thread_state_t) &gp_regs, &gp_count);
  if (kr != KERN_SUCCESS)
    {
      printf ("Error - unable to get registers for a thread\n");
      exit (1);
    }
  return gp_regs.__pc;
  *wordsize = 4;
#endif

}

/* Get the proc_threadinfo for a given thread.
   Gives you the thread name, if set; current and max priorities.  
   Returns 1 if successful
   Returns 0 if proc_pidinfo() failed
*/

int
get_proc_threadinfo (pid_t pid, uint64_t thread_handle, struct proc_threadinfo *pth)
{
  pth->pth_name[0] = '\0';
  int ret = proc_pidinfo (pid, PROC_PIDTHREADINFO, thread_handle,
                          pth, sizeof (struct proc_threadinfo));
  if (ret != 0)
    return 1;
  else
    return 0;
}

int
main (int argc, char **argv)
{
  kern_return_t kr;
  task_t task;
  thread_t thread;
  pid_t pid = 0;
  char *procname = NULL;
  int arg_is_procname = 0;
  int do_loop = 0;
  int verbose = 0;
  mach_port_t mytask = mach_task_self ();

  if (argc != 2 && argc != 3 && argc != 4)
    {
      printf ("Usage: tdump [-l] [-v] pid/procname\n");
      exit (1);
    }
  
  if (argc == 3 || argc == 4)
    {
      int i = 1;
      while (i < argc - 1)
        {
          if (strcmp (argv[i], "-l") == 0)
            do_loop = 1;
          if (strcmp (argv[i], "-v") == 0)
            verbose = 1;
          i++;
        }
    }

  char *c = argv[argc - 1];
  if (*c == '\0')
    {
      printf ("Usage: tdump [-l] [-v] pid/procname\n");
      exit (1);
    }
  while (*c != '\0')
    {
      if (!isdigit (*c))
        {
          arg_is_procname = 1;
          procname = argv[argc - 1];
          break;
        }
      c++;
    }

  if (arg_is_procname && procname)
    {
      pid = get_pid_for_process_name (procname);
    }
  else
    {
      errno = 0;
      pid = (pid_t) strtol (argv[argc - 1], NULL, 10);
      if (pid == 0 && errno == EINVAL)
        {
          printf ("Usage: tdump [-l] [-v] pid/procname\n");
          exit (1);
        }
    }

  const char *process_name = get_process_name_for_pid (pid);

  // At this point "pid" is the process id and "process_name" is the process name
  // Now we have to get the process list from the kernel (which only has the truncated
  // 16 char names)

  struct kinfo_proc *kinfo = get_kinfo_proc_for_pid (pid, process_name);

  printf ("pid %d (%s) is currently ", pid, process_name);
  switch (kinfo->kp_proc.p_stat) {
    case SIDL: printf ("being created by fork"); break;
    case SRUN: printf ("runnable"); break;
    case SSLEEP: printf ("sleeping on an address"); break;
    case SSTOP: printf ("suspended"); break;
    case SZOMB: printf ("zombie state - awaiting collection by parent"); break;
    default: printf ("unknown");
  }
  if (kinfo->kp_proc.p_flag & P_TRACED)
    printf (" and is being debugged.");
  free ((void *) kinfo);

  printf ("\n");

  kr = task_for_pid (mach_task_self (), pid, &task);
  if (kr != KERN_SUCCESS)
    {
      printf ("Error - unable to task_for_pid()\n");
      exit (1);
    }

  struct timespec *rqtp = (struct timespec *) malloc (sizeof (struct timespec));
  rqtp->tv_sec = 0;
  rqtp->tv_nsec = 150000000;

  int loop_cnt = 1;
  do
    {
      int i;
      if (do_loop)
        printf ("Iteration %d:\n", loop_cnt++);
      thread_array_t thread_list;
      mach_msg_type_number_t thread_count;

      kr = task_threads (task, &thread_list, &thread_count);
      if (kr != KERN_SUCCESS)
        {
          printf ("Error - unable to get thread list\n");
          exit (1);
        }
      printf ("pid %d has %d threads\n", pid, thread_count);
      if (verbose)
        printf ("\n");

      for (i = 0; i < thread_count; i++)
        {
          thread_basic_info_t basic_info = get_thread_basic_info (thread_list[i]);

          thread_identifier_info_data_t identifier_info = get_thread_identifier_info (thread_list[i]);
          
          int wordsize;
          uint64_t pc = get_current_pc (thread_list[i], &wordsize);

          printf ("thread #%d, unique tid %lld, suspend count is %d, ", i,
                  identifier_info.thread_id,
                  basic_info->suspend_count);
          if (wordsize == 8)
            printf ("pc 0x%016llx, ", pc);
          else
            printf ("pc 0x%08llx, ", pc);
          printf ("run state is ");
          switch (basic_info->run_state) {
            case TH_STATE_RUNNING: puts ("running"); break;
            case TH_STATE_STOPPED: puts ("stopped"); break;
            case TH_STATE_WAITING: puts ("waiting"); break;
            case TH_STATE_UNINTERRUPTIBLE: puts ("uninterruptible"); break;
            case TH_STATE_HALTED: puts ("halted"); break;
            default: puts ("");
          }
          if (verbose)
            {
              printf ("           ");
              printf ("mach thread #0x%4.4x ", (int) thread_list[i]);
              printf ("pthread handle id 0x%llx ", (uint64_t) identifier_info.thread_handle);

              struct proc_threadinfo pth;
              int proc_threadinfo_succeeded = get_proc_threadinfo (pid, identifier_info.thread_handle, &pth);

              if (proc_threadinfo_succeeded && pth.pth_name[0] != '\0')
                printf ("thread name '%s' ", pth.pth_name);

              printf ("\n           ");
              printf ("user %d.%06ds, system %d.%06ds", 
                              basic_info->user_time.seconds, basic_info->user_time.microseconds, 
                              basic_info->system_time.seconds, basic_info->system_time.microseconds);
              if (basic_info->cpu_usage > 0)
                {
                  float cpu_percentage = basic_info->cpu_usage / 10.0;
                  printf (", using %.1f%% cpu currently", cpu_percentage);
                }
              if (basic_info->sleep_time > 0)
                printf (", this thread has slept for %d seconds", basic_info->sleep_time);

              printf ("\n           ");
              printf ("scheduling policy %d", basic_info->policy);

              if (basic_info->flags != 0)
                {
                  printf (", flags %d", basic_info->flags);
                  if ((basic_info->flags | TH_FLAGS_SWAPPED) == TH_FLAGS_SWAPPED)
                    printf (" (thread is swapped out)");
                  if ((basic_info->flags | TH_FLAGS_IDLE) == TH_FLAGS_IDLE)
                    printf (" (thread is idle)");
                }
               if (proc_threadinfo_succeeded)
                 printf (", current pri %d, max pri %d", pth.pth_curpri, pth.pth_maxpriority);

              printf ("\n\n");
            }

          free ((void *) basic_info);
        }
      if (do_loop)
        printf ("\n");
      vm_deallocate (mytask, (vm_address_t) thread_list, 
                         thread_count * sizeof (thread_act_t));
      nanosleep (rqtp, NULL);
    } while (do_loop);
  
  vm_deallocate (mytask, (vm_address_t) task, sizeof (task_t));
  free ((void *) process_name);

  return 0;
}
