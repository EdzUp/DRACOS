#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <float.h>
#include <math.h>


struct time aa;
struct time bb;
int a;
float speed;

void testcpu(void)
{
 /*   4.77Mhz XT = 14/15
      8Mhz XT = 33/34
      16Mhz 286 = 110/111 */

 gettime(&aa);
 a=0;
 do{
  gettime(&bb);
 }while(aa.ti_hund==bb.ti_hund);
 gettime(&aa);
 do{
  gettime(&bb);
  a++;
 }while(aa.ti_hund==bb.ti_hund);
 printf("Processor cycles : %i\n",a);
 speed=(a/14)*4.77;
 printf("Estimated Mhz in comparison to 4.77Mhz XT : %2.2fMhz\n",speed);
 speed=(a/34)*8;
 printf("Estimated Mhz in comparison to 8Mhz XT    : %2.2fMhz\n",speed);
 speed=(a/111)*16;
 printf("Estimated Mhz in comparison to 16Mhz 286  : %2.2fMhz\n",speed);
}

void main(void)
{
 textmode(3);
 printf("DRACOS CPU speed test\n");
 printf("Copyright (C)1998 EdzUp Software\n\n");
 testcpu();
}