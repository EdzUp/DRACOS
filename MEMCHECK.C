#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>

long memsize,*area;
int err,y,pattern;
int xmssize;


void testmem(void)
{
 long pos;
 int w,r,num;
 err=0;

 pos=0;
 do{
  gotoxy(1,y);
  printf("Testing %luK",pos/1024);
  num=0;
  do{
   w=random(255);
   area[pos]=w;
   r=area[pos];
   if (w!=r) err=1; else err=0;
   num++;
  }while(num<10 && err==0);
  printf("\n");
  pos++;
 }while(pos<=memsize && kbhit()==0 && err==0);
 if (kbhit()!=0) printf("Test skipped\n\n"); else {
  gotoxy(1,y+2);
  if (err==0) printf("Memory passed\n"); else printf("Memory failed at Byte %lu\n",pos);
 }
}

void capturemem(void)
{
 memsize=655360;
 err=0;
 y=wherey();
 do{
  gotoxy(1,y);
  printf("Allocating %lu Bytes",memsize);
  if ((area = malloc(memsize))==NULL) {
   err=1;
   printf(" : Failed\n");
  } else err=0;
  memsize--;
 }while(err==1);
 printf(" : Ok    \n");
 gotoxy(1,y-1);
 printf("\nAllocated %lu Bytes (%luK) Chip memory\n\n",memsize,memsize/1024);
 y=wherey();
}

void main(void)
{
 clrscr();
 randomize();
 printf("\nDRACOS Chip memory checker\n");
 printf("Copyright (C)1998 EdzUp Software\n\n");
 printf("Press any key to skip memory check\n\n");
 capturemem();
 testmem();
}