#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <dir.h>
#include <io.h>
#include <string.h>
#include <process.h>
#include <cmosutl.h>
#include <bcdutl.h>

static char timea[] = "hh:mm:ss";
static char date[] = "dd/mm/ccyy";

char com[255],drvname[13];
int x,y,num;

//SYSINFO VARIABLES
union REGS out;
int hardware[40],pfree;

void windowsinfo(void)
{
 char var;

 asm mov ah,0x16;
 asm mov ax,0x1600;
 asm int 0x2F;
 asm mov al,var;

 printf("Windows version : ");
 if (var==0x00 || var==0x80) {
  printf("None present\n");
 } else
 if (var==0x01 || var==0xFF) {
  printf("Windows version : 2.x\n");
 }else
 {
  if (var<4) {
  printf("Windows version : 3.x\n");
  }else {
  printf("Windows version : 9x or NT\n");
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
  active=1;
 } else
 if (out.h.al==0x00) {
  active=0;
 } else
 if (out.h.al==0x01) {
  active=-1;
 } else printf("Unknown value returned '%X'\n",var);

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

void dosinfo(void)
{
 int active,start,dosmajor,dosminor,dosrev,doshma,network;
 char val[5],var,str[255];

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
 static char *Floppy[] = {
  "DRACOS Virtual Floppy Device",
  "5.25-inch 360K",
  "5.25-inch 1.2M",
  "3.5-inch 720k",
  "3.5-inch 1.44M",
 "3.5-inch 2.88M"
 };

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
 printf("  B: %s\n",Floppy[hardware[1]]);
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
  printf("Ready  ");
  hardware[28]=1;
 } else {
  printf("Not available  ");
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
  printf("Ready  ");
  hardware[29]=1;
 } else {
  printf("Not available  ");
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
  printf("Ready  ");
  hardware[30]=1;
 } else {
  printf("Not available  ");
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
 unsigned long total;

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
    case 5 : printf("Fixed disk\n");break;
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

void getvol(void) /* gatr +:visible -:hidden*/
{
 /* Attributes are : 0 None      1 Readonly      2 Hidden
		     4 System    8 Label        16 Directory
		    32 Archived */

 struct ffblk dki;

 int dirx,diry,err,cdirfile,fo,fatr[6],dx,a,b;
 char atr,ext[4],name[13],cds[255];

 getcwd(cds,255);
 dirx=1;
 diry=1;
 cdirfile=0;
 err=findfirst("\\*.*",&dki,FA_RDONLY|FA_ARCH|FA_HIDDEN|FA_SYSTEM|FA_DIREC|FA_LABEL);
 if (err==0) {
  do{
   strcpy(name,dki.ff_name);
   for (dx=0;dx<6;dx++) fatr[dx]=0;
   gotoxy(dirx,diry);
   atr=dki.ff_attrib;
   if (atr-32>-1){
    fatr[0]=1;
    atr-=32;
   }
   if (atr-16>-1){
    fatr[1]=1;
    atr-=16;
   }
   if (atr-8>-1){
    fatr[2]=1;
    atr-=8;
    num=-1;
    a=0;
    strcpy(drvname,"            ");
    do{
     num++;
     if (a==0) b=num; else b=num-1;

     if (name[num]!='.') drvname[b]=name[num]; else
     a=1;
    }while(num<strlen(name));
   }
   if (atr-4>-1){
    fatr[3]=1;
    atr-=4;
   }
   if (atr-2>-1){
    fatr[4]=1;
    atr-=2;
   }
   if (atr-1>-1){
    fatr[5]=1;
    atr-=1;
   }
   fo=0;
   if (fo!=1) cdirfile++;
   err=findnext(&dki);
   if (fo!=1) {
    dirx+=15;
    if (dirx>70) {
     dirx=1;
     diry++;
    }
   }
  }while(err==0);
 }
 chdir(cds);
}


void sysinfo(void)
{
 printf("\n\nDRACOS system information:\n");
 printf("--------------------------\n");
 CDinfo();
 windowsinfo();
 dosinfo();
 CMOSInfo();
 x=2;
 y=wherey();
 for (num=3;num<27;num++){
  gotoxy(x,y);
  driveinfo(num);
  y++;
 }
 cominfo();
 printf("\n");
}

void cmdlist(void)
{
 printf("\nDRACOS Dshell command listing:\n");
 printf("--------------------------------------------------------------------------------");
 printf("\nFor MS-DOS commands use <command> /? for help\n\n");
 printf("Command   Host    Function\n");
 printf("--------------------------------------------------------------------------------");
 printf("dir       MS-DOS  gives listing if directory\n");
 printf("cd        MS-DOS  change directory to one specified\n");
 printf("md        MS-DOS  makes a directory with the specified name\n");
 printf("rd        MS-DOS  removes a empty directory with the name specified\n");
 printf("ren       MS-DOS  renames a file to another name specified\n");
 printf("del       MS-DOS  deletes a file from directory\n");
 printf("copy      MS-DOS  copies a file from directory to specified location\n");
 printf("move      MS-DOS  moves a file from one location to another\n");
 printf("sys       MS-DOS  makes a disk bootable by installing system files\n");
 printf("format    MS-DOS  formats a disk to soecified paramaters\n");
 printf("type      MS-DOS  displays the contents of a file as they are read from it\n");
 printf("attrib    MS-DOS  displays or alters the attributes of a file\n");
 printf("deltree   MS-DOS  deletes the whole directory tree specified\n");
 printf("unformat  MS-DOS  unformats a disk that has had the MIRROR command used on it\n");
 printf("undelete  MS-DOS  recovers a file after it has been deleted\n");
 printf("recover   MS-DOS  **** THIS COMMAND SHOULD NEVER BE USED ****\n");
 printf("restore   MS-DOS  this command is used to restore a BACKUP file\n");
 printf("backup    MS-DOS  this command is used to backup data from a disk\n");
 printf("command   MS-DOS  creates another command shell\n");
 printf("exit       both   exits from a command shell\n");
 printf("memmaker  MS-DOS  tries to configure system files to free more memory\n");
 printf("assign    MS-DOS  allows user to assign another letter to a drive\n");
 printf("date      MS-DOS  asks the user for another date, also shows the old date\n");
 printf("time      MS-DOS  show the current time and asks for a new time\n");
 printf("doskey    MS-DOS  allows the user to use the cursor keys to view commands\n");
 printf("mode      MS-DOS  set system variables and change display mode\n");
 printf("emm386    MS-DOS  sets DOS up to use EMS emulation so programs can use EMS\n");
 printf("fdisk     MS-DOS  **** THIS PROGRAM IS DANGEROUS IN NOVICE HANDS USE WISELY ****");
 printf("                  FDISK is used to partition a physical drive into several small");
 printf("                  drives.\n");
 printf("graphics  MS-DOS  a command to allow graphics to be printed onto a printer\n");
 printf("keyb      MS-DOS  command used to set up different keyboard layouts\n");
 printf("print     MS-DOS  program to allow printing in background\n");
 printf("share     MS-DOS  allows multiple users to share files over a network\n");
 printf("smartdrv  MS-DOS  this command sets up a hard disk cache for faster accessing\n");
 printf("graftabl  MS-DOS  this is to allow extended character to be displayed\n");
 printf(":cmdlist  DRACOS  shows this list\n");
 printf(":sysinfo  DRACOS  displays a simple system specification\n");
}

void testcom(void)
{
 textcolor(7);
 textbackground(0);
 if (strcmp(com,"VER")==0 || strcmp(com,"VERSION")==0) {
  printf("\n\nDSHELL v1.0\n");
  printf("   (C)Copyright 1998-1999 EdzUp\n\n");
 } else
 if (strcmp(com,"")==0) {
 printf("\n");
 } else
 if (strcmp(com,":SYSINFO")==0){
  sysinfo();
  printf("\n");
 } else
 if (strcmp(com,":CMDLIST")==0){
  cmdlist();
  printf("\n");
 } else
 if (strcmp(com,"CLS")==0) {
  clrscr();
  gotoxy(1,2);
 } else {
  system(com);
  printf("\n\n");
 }
}

void titlebar(void)
{
 gotoxy(1,1);
 textcolor(0);
 textbackground(7);
 cprintf("  DSHELL v1.0 (C)Copyright 1998-1999 EdzUp                                      ");
 gotoxy(x,y);
}

void cursor(int one,int two)
{
 union REGS regs;

 regs.h.ah = 1;
 switch (one){
	case 0 : regs.h.ch = 0x00;break;
	case 1 : regs.h.ch = 0x01;break;
	case 2 : regs.h.ch = 0x02;break;
	case 3 : regs.h.ch = 0x03;break;
	case 4 : regs.h.ch = 0x04;break;
	case 5 : regs.h.ch = 0x05;break;
	case 6 : regs.h.ch = 0x06;break;
	case 7 : regs.h.ch = 0x07;break;
	case 8 : regs.h.ch = 0x0C;break;
 }
 switch (two){
	case 8 :
	case 0 : regs.h.cl = 0x00;break;
	case 1 : regs.h.cl = 0x01;break;
	case 2 : regs.h.cl = 0x02;break;
	case 3 : regs.h.cl = 0x03;break;
	case 4 : regs.h.cl = 0x04;break;
	case 5 : regs.h.cl = 0x05;break;
	case 6 : regs.h.cl = 0x06;break;
	case 7 : regs.h.cl = 0x07;break;
 }
 int86(0x10,&regs,&regs);
}

void main(void)
{
 char st[255],prompt[255];

 strcpy(drvname,"");
 textmode(64);
 clrscr();
 strcpy(prompt,"1>");
 textcolor(15);
 printf("\nDSHELL v1.0\n");
 printf("   (C)Copyright 1998-1999 EdzUp\n\n");
 gotoxy(1,5);
 cursor(0,7);
 getvol();
 printf("\n");
 do{
  x=wherex();
  y=wherey();
  getcwd(prompt,192);
  getvol();
  titlebar();
  strcpy(st,"[");
  strcat(st,drvname);
  strcat(st,"] ");
  strcat(st,prompt);
  strcat(st,"¯");
  textcolor(15);
  printf("%s",st);
  strcpy(com,"");
  gets(com);
  strcpy(com,strupr(com));
  if (strcmp(com,"EXIT")!=0 && strcmp(com,"exit")!=0) testcom(); else {
   cursor(6,7);
   exit(0);
  }
 }while(strcmp(com,"EXIT")!=0);
}