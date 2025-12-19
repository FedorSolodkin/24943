#include <stdio.h>
#define PRADDR(A)       printf(#A " at %p\n", &A)
extern int etext, edata, end;
static char s = 'S';
int a, b = 1;

int main(int argc, char *argv[])
{
  void sub1(int);
  static int c, d = 1;
  char m, n = 'n';

  printf("main at %p and sub1 at %p\n", main, sub1);
  printf("end of text segment at %p\n", &etext);
  printf("end of initialized data at %p\n", &edata);
  printf("end of uninitialized data at %p\n", &end);
  PRADDR(a);              PRADDR(c);
  PRADDR(m);              PRADDR(n);
  PRADDR(argc);           PRADDR(argv);
  for (b = 0; b <argc; b++)
      printf("argv[%d] at %p\n", b, &argv[b]);
  sub1(c);
}

void sub1(int p)
{
  static int t;
  char v;
  PRADDR(t);     PRADDR(p);     PRADDR(v);
}