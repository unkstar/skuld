
#if defined(WIN32)

#include <process.h>
#include <windows.h>
int getProcessorCount()
{
  SYSTEM_INFO sysinfo;
  GetSystemInfo( &sysinfo );

  return sysinfo.dwNumberOfProcessors;
}

#elif defined(__linux__) || defined(__sun) || defined(sun) || defined(_AIX)

#include <unistd.h>
int getProcessorCount()
{
  return sysconf( _SC_NPROCESSORS_ONLN );
}
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__DragonFly__) || defined(__NetBSD__) || defined(__OpenBSD__)
#include <sys/sysctl.h>
int getProcessorCount()
{
  int numCPU = -1;
  int mib[4];
  size_t len = sizeof(numCPU);

  /*  set the mib for hw.ncpu */
  mib[0] = CTL_HW;
  mib[1] = HW_AVAILCPU;  // alternatively, try HW_NCPU;

  /*  get the number of CPUs from the system */
  sysctl(mib, 2, &numCPU, &len, NULL, 0);

  if( numCPU < 1 )
  {
    mib[1] = HW_NCPU;
    sysctl( mib, 2, &numCPU, &len, NULL, 0 );

    if( numCPU < 1 )
    {
      numCPU = 1;
    }
  }
  return numCPU;
}
#elif defined(hpux) || defined(__hpux)
int getProcessorCount()
{
  return mpctl(MPC_GETNUMSPUS, NULL, NULL);
}
#else
int getProcessorCount()
{
  return sysconf( _SC_NPROC_ONLN );
}
#endif
