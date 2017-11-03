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
#include <errno.h>

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

char mainfile[13][30]; //DRACOS main files to install to X:\DRACOS
char iconfile[13][30]; //DRACOS icons to install to X:\DRACOS\ICONS
char colrfile[13][30]; //DRACOS palettes to install to X:\DRACOS\PALETTE

char maindir[255];
char icondir[255];
char colrdir[255];

int x,y,col[20],num,n2,win,dosmajor,dosminor,dosrev,doshma,network,Gd,Gm;
int hardware[32]; // first 26 is drives the next two are chip and fast ram
long avail,total,percent,pfree;
char str[80],installdrv[4],instdir[255],ch;

void CDinfo(void)
{
 int active,start;
 char var;
 unsigned int aa;

 out.h.ah=0x11;
 out.x.ax=0x11000;
 int86(0x2F,&out,&out);

 if (out.h.al==0xFF){
  active=1;
 } else
 if (out.h.al==0x00) {
  active=0;
 } else
 if (out.h.al==0x01) {
  active=-1;
 }

 if (active==1){
  out.h.ah=0x15;
  out.x.ax=0x1500;
  out.x.bx=0x0000;
  int86(0x2F,&out,&out);
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

void CMOSInfo(void)
{
 strcpy(timea,"--:--:--");
 *(int *)&timea[0] = BCD_ASC(GetCMOS(0x04));
 *(int *)&timea[3] = BCD_ASC(GetCMOS(0x02));
 *(int *)&timea[6] = BCD_ASC(GetCMOS(0x00));

 *(int *)&date[3] = BCD_ASC(GetCMOS(0x08));
 *(int *)&date[0] = BCD_ASC(GetCMOS(0x07));
 *(int *)&date[6] = BCD_ASC(GetCMOS(0x32));
 *(int *)&date[8] = BCD_ASC(GetCMOS(0x09));
 hardware[26]=(GetCMOS(0x16)<<8)+(unsigned)GetCMOS(0x15);
 hardware[27]=(GetCMOS(0x18)<<8)+(unsigned)GetCMOS(0x17);
 hardware[0]=GetCMOS(0x10)>>4;
 hardware[1]=GetCMOS(0x10)&15;
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

  dparm.dpDevAttr>>=1;
  if (Drive>2 && dparm.dpDevType!=0){
   if (hardware[Drive-1]!=10){
    if (hardware[Drive-1]==0) hardware[Drive-1]=dparm.dpDevType;
   }
  } else {
  }
  // printf("dpSpecFunc: %6i      dpDevType: %6i\n",dparm.dpSpecFunc,dparm.dpDevType);
  // printf("dpMediaType: %6i\n",dparm.dpMediaType);
 }
 setdisk(save);
}

void title(void)
{
 printf("DRACOS Installer\n");
 printf("Copyright (C)1998-1999 EdzUp\n");
 printf("\n");
}

void disksavail(void)
{
 printf("Drives available to install DRACOS to:\n");
 for (num=0;num<26;num++){
  if (hardware[num]==5) printf("[%c] ",num+'A');
 }
 printf("\n");
}

void installer(void)
{
 char ch;
 int ok,drve;

 textcolor(0);
 textbackground(7);
 gotoxy(1,1);
 cprintf(" DRACOS Installer  Copyright (C)1999 EdzUp                                      ");
 printf("\n\n");
 printf("When installing a directory '\\DRACOS' will be created in the root directory of\n");
 printf("the selected drive. All DRACOS files will be installed into this directory.\n\n");
 disksavail();
 printf("\nPlease select a drive to install to from the letters above: ");
 ok=1;
 do{
  ch=getch();
  if (ch>=97 && ch<=122) {
   drve=ch-97;
  } else
  if (ch>=65 && ch<=90){
   drve=ch-65;
  }
  if (hardware[drve]==5) {
   ok=0;
   printf("%c\n\n",drve+'A');
  } else ok=1;
 }while(ch!=27 && ok==1);
 printf("DRACOS Directory will be %c:\\DRACOS\n",drve+'A');
 strcpy(installdrv,"");
 ch=drve+'A';
 switch(ch){
 case 'C' : strcpy(installdrv,"C:\\");break;
 case 'D' : strcpy(installdrv,"D:\\");break;
 case 'E' : strcpy(installdrv,"E:\\");break;
 case 'F' : strcpy(installdrv,"F:\\");break;
 case 'G' : strcpy(installdrv,"G:\\");break;
 case 'H' : strcpy(installdrv,"H:\\");break;
 case 'I' : strcpy(installdrv,"I:\\");break;
 case 'J' : strcpy(installdrv,"J:\\");break;
 case 'K' : strcpy(installdrv,"K:\\");break;
 case 'L' : strcpy(installdrv,"L:\\");break;
 case 'M' : strcpy(installdrv,"M:\\");break;
 case 'N' : strcpy(installdrv,"N:\\");break;
 case 'O' : strcpy(installdrv,"O:\\");break;
 case 'P' : strcpy(installdrv,"P:\\");break;
 case 'Q' : strcpy(installdrv,"Q:\\");break;
 case 'R' : strcpy(installdrv,"R:\\");break;
 case 'S' : strcpy(installdrv,"S:\\");break;
 case 'T' : strcpy(installdrv,"T:\\");break;
 case 'U' : strcpy(installdrv,"U:\\");break;
 case 'V' : strcpy(installdrv,"V:\\");break;
 case 'W' : strcpy(installdrv,"W:\\");break;
 case 'X' : strcpy(installdrv,"X:\\");break;
 case 'Y' : strcpy(installdrv,"Y:\\");break;
 case 'Z' : strcpy(installdrv,"Z:\\");break;
 }
}

void begininstall(void)
{
 int err;
 char dracosdir[255];


 strcpy(dracosdir,"");
 strcpy(dracosdir,installdrv);
 strcat(dracosdir,"DRACOS");
 strcpy(maindir,dracosdir);
 strcat(maindir,"\\");
 printf("Creating %s\n",dracosdir);
 err=mkdir(dracosdir);
 if (err==-1){
  switch(errno){
  case ENOENT : printf("Error : Directory creation system not implemented\n");break;
  }
 }
 strcat(dracosdir,"\\ICONS");
 strcpy(icondir,dracosdir);
 printf("Creating %s\n",dracosdir);
 err=mkdir(dracosdir);
 strcpy(dracosdir,maindir);
 strcat(dracosdir,"\PALETTE");
 strcpy(colrdir,dracosdir);
 printf("Creating %s\n",dracosdir);
 err=mkdir(dracosdir);
 strcpy(dracosdir,maindir);
 strcat(dracosdir,"\LINKS");
 printf("Creating %s\n",dracosdir);
 err=mkdir(dracosdir);

}

void main(void)
{
 getcwd(instdir,255);
 textbackground(0);
 textcolor(7);
 clrscr();
 textmode(64);
 clrscr();
 CDinfo();
 clrscr();
 title();
 CMOSInfo();
 x=2;
 y=wherey();
 for (num=3;num<27;num++){
  driveinfo(num);
  x+=40;
  if (x>42){
   x=2;
   y++;
  }
 }
 printf("\n");
 clrscr();
 installer();
 begininstall();
}