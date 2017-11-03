#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <dir.h>
#include <math.h>
#include <float.h>
#include <cmosutl.h>
#include <bcdutl.h>
#include <string.h>
#include <graphics.h>
#include <alloc.h>

#define BYTE unsigned char
#define WORD unsigned short int

struct REGPACK in;
union REGS out;

//static char time[] = "hh:mm:ss";
static char timea[] = "hh:mm:ss";
static char date[] = "dd/mm/ccyy";
static char *Floppy[] = {
 "DRACOS Virtual Floppy Device",
 "5.25-inch 360K",
 "5.25-inch 1.2M",
 "3.5-inch 720k",
 "3.5-inch 1.44M",
 "3.5-inch 2.88M"
};

int x,y,col[20],num,n2,win,dosmajor,dosminor,dosrev,doshma,network,Gd,Gm;
int hardware[32]; // first 26 is drives the next two are chip and fast ram
long avail,total,percent,pfree;
char str[80];

void windowsinfo(void)
{
 char var;

 asm mov ah,0x16;
 asm mov ax,0x1600;
 asm int 0x2F;
 asm mov al,var;
 printf(" Windows version : ");
 if (var==0x00 || var==0x80) {
  printf("None present");
  win=-1;
 } else
 if (var==0x01 || var==0xFF) {
  printf("2.x\n");
  win=2;
 }else
 {
  if (var<4) {
   printf("3.x\n");
   win=3;
  }else {
   printf("95/98 or NT");
   win=95;
  }
 }
 printf("\n");
}

void CDinfo(void)
{
 int active,start;
 char var;
 unsigned int aa;

 out.h.ah=0x11;
 out.x.ax=0x11000;
 int86(0x2F,&out,&out);

 if (out.h.al==0xFF){
  printf("CD-ROM extensions loaded\n");
  active=1;
 } else
 if (out.h.al==0x00) {
  printf("CD-ROM extensions not loaded\n");
  active=0;
 } else
 if (out.h.al==0x01) {
  printf("CD-ROM extensions not loaded, cannot be installed safely\n");
  active=-1;
 } else printf("Unknown value returned '%X'\n",var);

 if (active==1){
  out.h.ah=0x15;
  out.x.ax=0x1500;
  out.x.bx=0x0000;
  int86(0x2F,&out,&out);
  printf("%u CD-ROM drives detected\n",out.x.bx);
  printf("CD-ROM letters starting at %c:\n",'A'+out.x.cx);
  start=out.x.cx;
  do{
   out.h.ah = 0x15;
   out.x.ax = 0x150B;
   out.x.cx = start;
   int86(0x2F, &out, &out);
   if (out.x.ax==0x0000) printf(""); else
   {
    printf("");
    hardware[start]=10;
   }
   start++;
  }while (start<26);
 }
}

void dosinfo(void)
{
 int active,start;
 char val[5],var;

 out.h.ah=0x33;
 out.h.al=0x06;
 int86(0x21,&out,&out);
 dosmajor=out.h.bl;
 dosminor=out.h.bh;
 dosrev=out.h.dl;
 if (out.h.dh==0x08) doshma=0; else
 if (out.h.dh==0x10) doshma=1; else
 doshma=0;
 printf(" DOS version : %i.%i  Revision %i. ",dosmajor,dosminor,dosrev);
 if (doshma==0) printf("Resident in Chip memory\n"); else
 if (doshma==1) printf("Resident in High Memory Area\n");
 out.h.ah = 0x11;
 out.x.ax = 0x1100;
 int86(0x2F, &out, &out);
 if (out.h.al==0xFF) network=1; else
 if (out.h.al==0x00) network=0; else
 if (out.h.al==0x01) network=-1;
 if (network==1) printf(" Network support active\n"); else
 if (network==0) printf(" Network support not present\n"); else
 if (network==-1) printf(" Network support not present, unsafe to install\n");
}


void CMOSInfo(void)
{
 strcpy(timea,"--:--:--");
 *(int *)&timea[0] = BCD_ASC(GetCMOS(0x04));
 *(int *)&timea[3] = BCD_ASC(GetCMOS(0x02));
 *(int *)&timea[6] = BCD_ASC(GetCMOS(0x00));
 printf("\n System time is %s",timea);

 *(int *)&date[3] = BCD_ASC(GetCMOS(0x08));
 *(int *)&date[0] = BCD_ASC(GetCMOS(0x07));
 *(int *)&date[6] = BCD_ASC(GetCMOS(0x32));
 *(int *)&date[8] = BCD_ASC(GetCMOS(0x09));
 gotoxy(40,wherey());
 printf("System date is %s\n",date);

 hardware[26]=(GetCMOS(0x16)<<8)+(unsigned)GetCMOS(0x15);
 hardware[27]=(GetCMOS(0x18)<<8)+(unsigned)GetCMOS(0x17);
 printf(" Chip RAM installed is %uK",hardware[26]);
 gotoxy(40,wherey());
 printf("Fast RAM installed is %uK [DRACOS]\n",hardware[27]);
 hardware[0]=GetCMOS(0x10)>>4;
 hardware[1]=GetCMOS(0x10)&15;
 printf("\n Drives installed:\n");
 printf("  A: %s\n",Floppy[hardware[0]]);
 gotoxy(40,wherey()-1);
 printf("   B: %s\n",Floppy[hardware[1]]);
}

void diskfree(int drive)
{
 struct dfree fatinf;

 getdfree(drive,&fatinf);
 avail=(long)fatinf.df_avail*(long)fatinf.df_bsec*(long)fatinf.df_sclus;
 total=(long)fatinf.df_total*(long)fatinf.df_bsec*(long)fatinf.df_sclus;
 percent=total/100;
 if (avail>0) pfree=avail/percent; else pfree=0;

}

void bitop(int info,short bit[8])
{
 if (info&1==1) bit[0]=1; else bit[0]=0;
 info>>1;
 if (info&1==1) bit[1]=1; else bit[1]=0;
 info>>1;
 if (info&1==1) bit[2]=1; else bit[2]=0;
 info>>1;
 if (info&1==1) bit[3]=1; else bit[3]=0;
 info>>1;
 if (info&1==1) bit[4]=1; else bit[4]=0;
 info>>1;
 if (info&1==1) bit[5]=1; else bit[5]=0;
 info>>1;
 if (info&1==1) bit[6]=1; else bit[6]=0;
 info>>1;
 if (info&1==1) bit[7]=1; else bit[7]=0;
 info>>1;

}

void cominfo(void)
{
 short bit[8];
 int status,modem,ok;

 out.h.ah = 0x03;
 out.x.dx = 0x00;
 int86(0x14, &out, &out);
 status=out.h.ah;
 bitop(status,bit);
 ok=0;
 printf("\n COM1:");
 if (bit[7]==1) ok=1;
 if (bit[3]==1) ok=1;
 if (bit[2]==1) ok=1;
 if (bit[1]==1) ok=1;
 if (ok==0) {
  printf("Ready");
  hardware[28]=1;
 } else {
  printf("Not available");
  hardware[28]=0;
 }
 out.h.ah = 0x03;
 out.x.dx = 0x01;
 int86(0x14, &out, &out);
 status=out.h.ah;
 bitop(status,bit);
 ok=0;
 gotoxy(20,wherey());
 printf("COM2:");
 if (bit[7]==1) ok=1;
 if (bit[3]==1) ok=1;
 if (bit[2]==1) ok=1;
 if (bit[1]==1) ok=1;
 if (ok==0) {
  printf("Ready");
  hardware[29]=1;
 } else {
  printf("Not available");
  hardware[29]=0;
 }

 out.h.ah = 0x03;
 out.x.dx = 0x02;
 int86(0x14, &out, &out);
 status=out.h.ah;
 bitop(status,bit);
 ok=0;
 gotoxy(40,wherey());
 printf("COM3:");
 if (bit[7]==1) ok=1;
 if (bit[3]==1) ok=1;
 if (bit[2]==1) ok=1;
 if (bit[1]==1) ok=1;
 if (ok==0) {
  printf("Ready");
  hardware[30]=1;
 } else {
  printf("Not available");
  hardware[30]=0;
 }

 out.h.ah = 0x03;
 out.x.dx = 0x03;
 int86(0x14, &out, &out);
 status=out.h.ah;
 bitop(status,bit);
 ok=0;
 gotoxy(60,wherey());
 printf("COM4:");
 if (bit[7]==1) ok=1;
 if (bit[3]==1) ok=1;
 if (bit[2]==1) ok=1;
 if (bit[1]==1) ok=1;
 if (ok==0) {
  printf("Ready\n");
  hardware[31]=1;
 } else {
  printf("Not available\n");
  hardware[31]=0;
 }
}

void netcheck(char DriveCode)
{
 int Stat,local;

 out.h.bl=DriveCode;
 out.x.ax=0x4409;
 int86(0x21,&out,&out);
 Stat=out.x.dx;

 local=-1;
 if (Stat-32768>=0){
  Stat-=32768;
  local=2;
 }
 if (Stat-16384>=0) Stat-=16384;
 if (Stat-8192>=0) Stat-=8192;
 if (Stat-4096>=0) {
  Stat-=4096;
  local=0;
 }
 if (Stat-2048>=0) Stat-=2048;
 if (Stat-1024>=0) Stat-=1024;
 if (Stat-512>=0 && local!=0) {
  Stat-=512;
  local=1;
 }
 if (Stat-256>=0) Stat-=256;
 if (Stat-128>=0) Stat-=128;
 if (Stat-64>=0) Stat-=64;
 if (Stat-32>=0) Stat-=32;
 if (Stat-16>=0) Stat-=16;
 if (Stat-8>=0) Stat-=8;
 if (Stat-4>=0) Stat-=4;
 if (Stat-2>=0) Stat-=2;
 if (Stat-1>=0) Stat-=1;
 switch(local){
 case 0 : hardware[DriveCode]=11;break; //Remote Network
 case 1 : hardware[DriveCode]=12;break; //Local shared network drive
 case 2 : hardware[DriveCode]=13;break; //Subst drive
 default: break;
 }
}

void driveinfo(int Drive)
{
 char far *fcb1;

 struct{
  unsigned char dpSpecFunc; /*drive parm structure */
  unsigned char dpDevType;
  unsigned int  dpDevAttr;
  unsigned int  dpCylinders;
  unsigned char dpMediaType;
  unsigned int  dpBytesPerSec;
  unsigned char dpSecPerClust;
  unsigned int  dpResSectors;
  unsigned char dpFATs;
  unsigned int  dpRootDirEnts;
  unsigned int  dpSectors;
  unsigned char dpMedia;
  unsigned int  dpFATsecs;
  unsigned int  dpSecPerTrack;
  unsigned int  dpHeads;
  unsigned long dpHiddenSecs;
  unsigned long dpHugeSectors;
  unsigned int  dpSkip1[3]; /*unused words to test*/
  unsigned int  dpTrkLayout; /* used only by 0840h*/
 } dparm;
 unsigned int segptr,offptr;
 int save,disks,disk,removable,changeline;

 /* save original drive */
 save = getdisk();

 setdisk(Drive-1);
 if (Drive-1 == getdisk()) {
  dparm.dpSpecFunc = 1; /*get current info */

  //fcb1 = MK_FP(_psp, 0x5C ); /*use FCB trick to get drive */
  //Drive = *fcb1;
  segptr = FP_SEG( (void far *)&dparm );
  offptr = FP_OFF( (void far *)&dparm );

  asm mov bx,Drive;
  asm mov cx,0x0860;
  asm mov dx,offptr;
  asm mov ax,segptr;
  asm push ds;
  asm mov ds,ax;
  asm mov ax,0x440D;
  asm int 0x21;
  asm pop ds;

  if (Drive!=0) printf(" %c: ",'@'+Drive); else printf(" ?: ");
  if (dparm.dpDevAttr&1==1) removable=0; else removable=1;
  dparm.dpDevAttr>>=1;
  if (Drive>2 && dparm.dpDevType!=0){
   if (hardware[Drive-1]!=10){
    switch (dparm.dpDevType){
    case 0 : printf("5.25 360K\n");break;
    case 1 : printf("1.2M 5.25\n");break;
    case 2 : printf("720K 3.5\n");break;
    case 5 : {
	      printf("Fixed disk ");
	      diskfree(Drive);
	      printf("[%ldMb, %ld% Free]\n",(total/1024)/1024,pfree);
	     };break;
    case 6 : printf("Tape drive\n");break;
    case 7 : printf("1.44 floppy\n");break;
    case 8 : printf("Read/write optical drive\n");break;
    case 9 : printf("2.88M floppy\n");break;
    default: {
	     if (removable==1) {
	      if (hardware[Drive-1]!=10) {
	       netcheck(Drive-1);
	       switch(hardware[Drive-1]){
	       case 11 : printf("Remote network drive\n");break;
	       case 12 : printf("Local network drive\n");break;
	       case 13 : printf("SUBST drive\n");break;
	       default : printf("Removable device [ID%i]\n",hardware[Drive-1]);
	       }
	      } else
	       printf("CD-ROM Drive\n");
	     } else
	      printf("Unknown device\n");
	    };break;
    }
    if (hardware[Drive-1]==0) hardware[Drive-1]=dparm.dpDevType;
   } else printf("CD-ROM Drive\n");
  } else {
   if (hardware[Drive-1]!=10) printf("Removable media device ID %i\n",hardware[Drive-1]); else
    printf("CD-ROM Drive\n");
  }
  // printf("dpSpecFunc: %6i      dpDevType: %6i\n",dparm.dpSpecFunc,dparm.dpDevType);
  // printf("dpMediaType: %6i\n",dparm.dpMediaType);
 } else printf(" %c: Not available\n",'@'+Drive);
 setdisk(save);
}

void title(void)
{
 printf("SysInfo\n");
 printf("Copyright (C)1998-1999 EdzUp\n");
 printf("\n");
}

void main(void)
{
 textmode(64);
 CDinfo();
 clrscr();
 title();
 windowsinfo();
 dosinfo();
 CMOSInfo();
 x=2;
 y=wherey();
 for (num=3;num<27;num++){
  gotoxy(x,y);
  driveinfo(num);
  x+=40;
  if (x>42){
   x=2;
   y++;
  }
 }
 printf("\n");
 cominfo();
}