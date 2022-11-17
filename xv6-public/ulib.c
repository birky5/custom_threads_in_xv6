#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"

void* userStack;

char*
strcpy(char *s, const char *t)
{
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
    ;
  return os;
}

int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}

uint
strlen(const char *s)
{
  int n;

  for(n = 0; s[n]; n++)
    ;
  return n;
}

void*
memset(void *dst, int c, uint n)
{
  stosb(dst, c, n);
  return dst;
}

char*
strchr(const char *s, char c)
{
  for(; *s; s++)
    if(*s == c)
      return (char*)s;
  return 0;
}

char*
gets(char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return buf;
}

int
stat(const char *n, struct stat *st)
{
  int fd;
  int r;

  fd = open(n, O_RDONLY);
  if(fd < 0)
    return -1;
  r = fstat(fd, st);
  close(fd);
  return r;
}

int
atoi(const char *s)
{
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0';
  return n;
}

void*
memmove(void *vdst, const void *vsrc, int n)
{
  char *dst;
  const char *src;

  dst = vdst;
  src = vsrc;
  while(n-- > 0)
    *dst++ = *src++;
  return vdst;
}

void lock_init(lock_t *mutex) {
	// 0 -> lock is available, 1 -> held
	mutex->flag = 0;
}

void lock_acquire(lock_t *mutex) {
	while (xchg(&mutex->flag, 1) != 0);
  //while (xchg(&(mutex->flag), 1) != 0)
   // ; // atomically spin and wait while the lock is currently acquired

  mutex->flag = 1; // set the flag to indicate it is locked
}

void lock_release(lock_t *mutex) {
	xchg(&mutex->flag, 0);
  mutex->flag = 0; // set flag back to 0 to "unlock" it
}

int thread_create(void (*start_routine)(void *, void *), void *arg1, void *arg2) {
  void* initalStack = malloc(4096); // page is 4kb or 4096
  if (initalStack == 0) { return -1; } // if malloc failed then we return -1

  // Need to check page alignment
  if ((uint)initalStack % 4096) {
    userStack = initalStack + (4096 - (uint)initalStack % 4096);
  } else {
    userStack = initalStack;
  }
  // above if else statement checks if the stack is page aligned,
  // if it is not it adds the amount to the pointer to get it page aligned

  //return clone(start_routine, arg1, arg2, initalStack);
  return clone(start_routine, arg1, arg2, userStack);
}

// Per the project spec: which calls the underlying join() system
// call, frees the user stack, and then returns. It returns the
// waited-for PID (when successful), -1 otherwise.
int thread_join() {
  // join returns the process ID or -1, and this function also
  // returns the process ID or -1, so no if statement needed
  int processReturn = join(&userStack);
  if (processReturn != -1) {
    free(userStack);
  }
  return processReturn;
}
