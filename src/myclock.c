/* FILE:   "myclock.c"
 * 
 */

#include <time.h>

/* gestisci i dettagli dovuti alle diverse librerie */
#if __GNUC__  &&  __MSDOS__
#include <limits.h>
#include <sys/times.h>
#define times(v) ((v)->tms_utime=time(0),(v)->tms_stime=0)
#define HZ 1.0

#elif VAXC || __TURBOC__ || __MSDOS__

#include <limits.h>		/* da rivedere */
#define times(v) ((v)->tms_utime=clock(),(v)->tms_stime=0)
#define HZ (double)CLK_TCK
struct tms
{
  time_t          tms_utime, tms_stime;
};
#else				/* Unix environment */

#include <sys/times.h>
#ifndef HZ
#define HZ 60.0
#endif
#define ULONG_MAX (~(0L))
#endif				/* VAX || TURBO */

static unsigned int prev_time = 0;
static unsigned char first_call = 1;

unsigned int    my_clock()
{
  /* locals */
  unsigned int    pres_time;
  unsigned int    diff_time;
  struct tms      t;

  if (first_call == 1)
  {
    first_call = 0;
    times(&t);
    prev_time = (t.tms_utime + t.tms_stime);
    diff_time = 0;
  }
  else
  {
    times(&t);
    pres_time = (t.tms_utime + t.tms_stime);
    diff_time = pres_time - prev_time;
    prev_time = pres_time;
  }				/* else */
  return (diff_time * 1000.0 / HZ);
}				/* my_clock () */
