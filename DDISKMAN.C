#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <dir.h>
#include <bcdutl.h>
#include <cmosutl.h>
#include <io.h>
#include <fcntl.h>
#include <graphics.h>
#include <errno.h>
#include <string.h>
#include <process.h>
#include <ctype.h>
#include <bios.h>

struct {
int r,g,b;
} col[16];

struct {
int tx[10],ty[10],bx[10],by[10]; //button pos
} request;

struct {
int tx,ty,bx,by,type;
char name[13];
} ficon[55];

struct {
int tx,ty,bx,by,num;
} dicon[26];

struct {
char name[13],tagged;
int  type;
} fileinf[1024];

struct ffblk dki;
struct REGPACK in; /*registers structure*/
int buf[500];

/* DEFINITIONS FOR SCREEN DISPLAY*/
int maxx,maxy,img;
int icon[100][100][2];

// DRACOS ENVIROMENT SYSTEM
char maindir[255];
char driveicon[13][26];

char dirs[192],cwd[192],st[256];
int Gd,Gm,lclick,rclick,oldl,oldr,menu,itype,oldxsize,oldysize,ln,mode,cdi;
int num,xsize,ysize,mfile,usr,cfile,cdrive,sbpt,sbpb,dsbpt,dsbpb,lefttime,righttime,oldicon;
int mousex,mousey,but1,but2,oldmousex,oldmousey,dx,dy,da[26],mdrive,activew,req;
unsigned char ch;
int rmx,rmy,rma,rmt; //right click menu
FILE *hand;

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

int x,y,n2,win,dosmajor,dosminor,dosrev,doshma,network;
int hardware[32]; // first 26 is drives the next two are chip and fast ram
long avail,total,percent,pfree;
char str[80];

int shiftstate(int a)
{
 int keybit;

/*
  Bit  Function
  0    Right Shift
  1    Left Shift
  2    Ctrl key pressed
  3    Alt Pressed
  4    Scrl Lock on
  5    Num Lock on
  6    Caps Lock on
  7    Insert on
*/
 if (a<8){
  keybit=bioskey(2);
  keybit>>=a;
  return keybit&1;
 } else
 {
  printf("Error in Shift state test\n");
  exit(0);
 }
 return -1;
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

void CMOSInfo(void)
{
 strcpy(timea,"--:--:--");
 *(int *)&timea[0] = BCD_ASC(GetCMOS(0x04));
 *(int *)&timea[3] = BCD_ASC(GetCMOS(0x02));
 *(int *)&timea[6] = BCD_ASC(GetCMOS(0x00));
// printf("\n System time is %s",timea);

 *(int *)&date[3] = BCD_ASC(GetCMOS(0x08));
 *(int *)&date[0] = BCD_ASC(GetCMOS(0x07));
 *(int *)&date[6] = BCD_ASC(GetCMOS(0x32));
 *(int *)&date[8] = BCD_ASC(GetCMOS(0x09));
 gotoxy(40,wherey());
// printf("System date is %s\n",date);

 hardware[26]=(GetCMOS(0x16)<<8)+(unsigned)GetCMOS(0x15);
 hardware[27]=(GetCMOS(0x18)<<8)+(unsigned)GetCMOS(0x17);
// printf(" Chip RAM installed is %uK",hardware[26]);
 gotoxy(40,wherey());
// printf("Fast RAM installed is %uK [DRACOS]\n",hardware[27]);
 hardware[0]=GetCMOS(0x10)>>4;
 hardware[1]=GetCMOS(0x10)&15;
// printf("\n Drives installed:\n");
// printf(" A: %s\n",Floppy[hardware[0]]);
 switch(hardware[0]){
 case 0 : hardware[0]=14;break;
 case 1 : hardware[0]=0;break;
 case 2 : hardware[0]=1;break;
 case 3 : hardware[0]=2;break;
 case 4 : hardware[0]=7;break;
 case 5 : hardware[0]=9;break;
 }
// printf(" B: %s\n",Floppy[hardware[1]]);
 switch(hardware[1]){
 case 0 : hardware[1]=14;break;
 case 1 : hardware[1]=0;break;
 case 2 : hardware[1]=1;break;
 case 3 : hardware[1]=2;break;
 case 4 : hardware[1]=7;break;
 case 5 : hardware[1]=9;break;
 }

}

void diskfree(int drive)
{
 struct dfree fatinf;

 getdfree(drive,&fatinf);
 if (fatinf.df_sclus!=0xFFFF){
  avail=(long)fatinf.df_avail*(long)fatinf.df_bsec*(long)fatinf.df_sclus;
  total=(long)fatinf.df_total*(long)fatinf.df_bsec*(long)fatinf.df_sclus;
  percent=total/100;
  if (avail>0) pfree=avail/percent; else pfree=0;
 } else hardware[drive]=-1;

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
    netcheck(Drive-1);
    switch (dparm.dpDevType){
    case 0 : printf("5.25 360K\n");break;
    case 1 : printf("1.2M 5.25\n");break;
    case 2 : printf("720K 3.5\n");break;
    case 5 : {
	      printf("Fixed disk\n");
	      netcheck(Drive-1);
	      diskfree(Drive);
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
 } else {
  printf(" %c: Not available\n",'@'+Drive);
  hardware[Drive-1]=-1;
 }
 setdisk(save);
}

void mouseon(void)
{
 asm{
  mov ax, 0x0001
  int 0x33
 }
}

void mouseoff(void)
{
 asm{
  mov ax, 0x0002
  int 0x33
 }
}

unsigned char resetmouse(void){
 unsigned int mouse_type;
 unsigned int driver_status;
 unsigned char return_value;

 asm{
 mov ax, 0x0000
 int 0x33
 mov driver_status, ax
 mov mouse_type, bx
 }

 if (driver_status==0xFFFF) {
  switch (mouse_type) {
  case 0x0000: return_value = 0; break;//1
  case 0x0002: return_value = 2; break;//2
  case 0x0003: return_value = 3; break;//3
  case 0xFFFF: return_value = 4; break;//?
  }
 }else{
  printf("Error:No mouse detected\n");//duff
  printf("\nDRACOS requires a mouse to function correctly with the correct driver\n\n");
  exit(0);
 }
 return return_value;
}

int mouseinfo(int rd)
{
 in.r_ax = 3;
 intr(0x33, &in);
 switch(rd){
 case 1 : return(in.r_cx);
 case 2 : return(in.r_dx);
 case 3 : return(in.r_bx & 1); //left button
 case 4 : return(in.r_bx & 2); //right button
 case 5 : return(in.r_bx & 4); //middle button
 }
 return(-1);
}

void setmodes(void)
{
 int err,x,y;

}

void dracosbackground(void)
{
 int image[1024];
 int xx,yy;

 setfillstyle(1,7);
 setcolor(15);
 rectangle(0,0,maxx,11);
 bar(1,1,maxx-1,10);
 setcolor(8);
 line(1,11,maxx,11);
 line(maxx,1,maxx,11);
 setcolor(15);
 outtextxy(2,2,"DRACOS Disk Manager Copyright (C)1998 EdzUp");
 outtextxy(600,2,"Quit");
 line(589,1,589,11);
}

void titlebar(void)
{
 setfillstyle(1,7);
 setcolor(15);
 rectangle(0,0,maxx,11);
 bar(1,1,maxx-1,10);
 setcolor(8);
 line(1,11,maxx,11);
 line(maxx,1,maxx,11);
 setcolor(15);
 outtextxy(2,2,"DRACOS Disk Manager Copyright (C)1998 EdzUp");
 outtextxy(600,2,"Quit");
 line(589,1,589,11);
}

void convertmain(void)
{
 char newdir[255],ac[2];

 strcpy(newdir,"");
 for (num=0;num<strlen(maindir);num++){
  ac[0]=maindir[num];
  ac[1]=NULL;
  strcat(newdir,ac);
  if (ac[0]==92) strcat(newdir,"\\");
 }
}

void load(char fname[192])
{
 char ch;
 int  ok;

 hand=fopen(fname,"rb");

 if (hand==NULL){
  gotoxy(40,24);
  printf("Error loading file. <SPACE>");
  gotoxy(40,25);
  printf("%s    ",fname);
  do{
   ch=getch();
  }while(ch!=32);
 } else {
  fread(&itype,2,2,hand);
  fread(&n2,2,2,hand);
  fread(&n2,2,2,hand);
  xsize=32;
  ysize=32;
  for (dy=0;dy<ysize;dy++){
   for (dx=0;dx<xsize;dx++){
    fread(&icon[dx][dy][0],2,2,hand);
    fread(&icon[dx][dy][1],2,2,hand);
   }
  }
  fclose(hand);
 }
}

void getdrives(void) //ASUME THE SYSTEM HAS A A: AND A VIRTUAL B:
{
 int drv,dm,cr,disk,save;

 CDinfo();
 CMOSInfo();
 gotoxy(1,wherey());
 for (num=3; num<26;num++) driveinfo(num);
 mdrive=0;
 for (num=0;num<26;num++) da[num]=0;
 do{
  if (hardware[mdrive]>0) da[mdrive]=1;
  mdrive++;
 }while(mdrive<26);
 mdrive=25;
 do{
  if (hardware[mdrive]<1) mdrive--;
 }while(hardware[mdrive]<1 && mdrive>0);
 for (num=0;num<mdrive+1;num++){
  switch(hardware[num]){
  case 3 :
  case 1 :
  case 0 : strcpy(driveicon[num],"DISK2.DIS");break;
  case 7 :
  case 9 :
  case 2 : strcpy(driveicon[num],"DISK0.DIS");break;
  case 12:
  case 5 : strcpy(driveicon[num],"DISK1.DIS");break;
  case 6 : strcpy(driveicon[num],"DISK5.DIS");break;
  case 8 : strcpy(driveicon[num],"DISK6.DIS");break;
  case 10: strcpy(driveicon[num],"DISK3.DIS");break;
  case 11: strcpy(driveicon[num],"DISKNET.DIS");break;
  case 13:
  default: strcpy(driveicon[num],"DISK4.DIS");break;
  }
 }
}

void placeicon(int plx,int ply,int pimg)
{
 int plxx,plyy;

 for (plyy=0;plyy<ysize;plyy++){
  for (plxx=0;plxx<xsize;plxx++){
   if (icon[plxx][plyy][pimg]!=0) putpixel(plxx+plx,plyy+ply,icon[plxx][plyy][pimg]);
  }
 }
}

int examine(char st[255])
{
 // 0 is ok   -1 is an error
 int err,dnd;

 strcpy(st,strupr(st));
 err=strlen(st);
 st[err-2]=NULL;
 if (strcmp("#MAINDIR",st)==0) mode=1; else
 if (strcmp("#DRIVEICON",st)==0) mode=5; else
 if (st[0]==';') {} else
 if (strcmp("#EXITDIR",st)==0) mode=2; else
 switch(mode){
 case 1 : {
	   printf("Main DRACOS directory '%s' ",st);
	   if (strcmp(maindir,st)!=0) {
	    printf("Host setting incorrect\n");
	    strcpy(maindir,st);
	   } else printf("OK\n");
	  }break;
 case 2 : break;
 case 3 : break;
 case 4 : break;
 case 5 : if (strlen(st)>0 && cdi<26){
	   printf("Drive %i icon '%s'\n",cdi,st);
	   strcpy(driveicon[cdi],st);
	   cdi++;
	  } else
	  if (strlen(st)==0 && cdi<26) {
	   strcpy(driveicon[cdi],"DISKU.DIS");
	   cdi++;
	  }break;
 case 6 : break;
 default: printf("[CLI] Unknown command '%s'\n",st);
	  return(-1);
 }
 return(0);
}

void readold(void)
{
 char ch,ok,jj,rs[40],pro,frac,com[255],locat[255];
 long size;
 int bp,re,aa,err,prob;
 char *s;

 s=getenv("DRACOS");
 strcpy(maindir,s);
 strcpy(locat,maindir);
 mode=0;
 cdi=0;
}


void exttype(char filename[255])
{
 int ex;
 char ext[5],drv[5],dir[192],fn[8];

 num=0;
 usr=0;
 fnsplit(filename,drv,dir,fn,ext);
 if (strcmp(ext,".EXE")==0 || strcmp(ext,".COM")==0) usr=1; else
 if (strcmp(ext,".TXT")==0 || strcmp(ext,".1ST")==0 || strcmp(ext,".DOC")==0) usr=2; else
 if (strcmp(ext,".OVL")==0 || strcmp(ext,".DLL")==0 || strcmp(ext,".VXD")==0) usr=3; else
 if (strcmp(ext,".BAT")==0) usr=4; else
 if (strcmp(ext,".BAK")==0) usr=5; else
 if (strcmp(ext,".SYS")==0) usr=6; else
 if (strcmp(ext,".INI")==0 || strcmp(ext,".PIF")==0 || strcmp(ext,".ICO")==0) usr=7; else
 if (strcmp(ext,".ZIP")==0 || strcmp(ext,".LHA")==0 || strcmp(ext,".ARJ")==0) usr=10; else
 if (strcmp(ext,".#$#")==0 || strcmp(ext,".#I#")==0) usr=8; else
 if (strcmp(ext,".DIS")==0 || strcmp(ext,".DWS")==0 || strcmp(ext,".DPS")==0) usr=8; else usr=9;
 if (strcmp(filename,"DRACOS.#$#")==0) usr=8;
}

void getfiles(char param[255],char gatr[6]) /* gatr +:visible -:hidden*/
{
 /* Attributes are : 0 None      1 Readonly      2 Hidden
		     4 System    8 Label        16 Directory
		    32 Archived */
 int dirx,diry,err,cdirfile,fo,fatr[6];
 char atr,ext[4];

 dirx=1;
 diry=1;
 cdirfile=0;
 err=findfirst(param,&dki,FA_RDONLY|FA_ARCH|FA_HIDDEN|FA_SYSTEM|FA_DIREC|FA_LABEL);
 if (err==0) {
  do{
   strcpy(fileinf[cdirfile].name,dki.ff_name);
   fileinf[cdirfile].tagged=0;
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
   for (num=0;num<6;num++){
    if (gatr[num]=='-' && fatr[num]==1) fo=1;
   }
   exttype(fileinf[cdirfile].name);
   if (fatr[1]==1) usr=0;
   fileinf[cdirfile].type=usr;
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
 cfile=0;
 for (num=cdirfile+1;num<1024;num++){
  strcpy(fileinf[num].name,"N?A");
 }
 mfile=cdirfile;
}

void closegadget(int clx,int cly)
{
}

void drivesetup(void)
{
 setfillstyle(1,7);
 bar(0,12,120,23);
 setfillstyle(1,0);
 bar(0,24,120,480);
 setcolor(15);
 line(0,12,120,12);
 line(0,12,0,480);
 outtextxy(2,14,"Drives Free");
 line(1,479,119,479);
 line(119,479,119,23);
 setcolor(8);
 line(120,13,120,480);
 line(1,22,120,22);
 line(1,480,120,480);
}

void scrollbar(int cur,int maxf)
{
 int per,count,cc,sby;

 if (maxf>45){
  cc=maxf;
  count=0;
  do{
   count++;
   cc-=45;
  }while(cc>45);
  count++;
  setfillstyle(1,7);
  bar(618,25,638,478);
  setfillstyle(1,0);
  bar(620,45,636,458);
  per=(413/count);
  setfillstyle(1,7);
  cc=0;
  sby=46;
  gotoxy(1,1);
  if (cc<cur){
   do{
    cc+=45;
    sby+=per;
   }while(cc<cur);
  }
  bar(621,sby,635,(sby+per));
  setcolor(8);
  line(636,45,620,45);
  line(620,45,620,459);
  line(638,25,638,478);
  line(638,478,619,478);
  line(622,sby+per,635,sby+per);
  line(635,sby+per,635,sby);
  setcolor(15);
  line(618,25,638,25);
  line(618,25,618,478);
  line(636,46,636,458);
  line(636,458,621,458);
  line(635,sby,621,sby);
  line(621,sby,621,sby+per);
  sbpt=sby;
  sbpb=sby+per;
 } else {
  sbpt=0;
  sbpb=0;
 }
}

void filewindow(void)
{
 getcwd(cwd,192);

 setfillstyle(1,7);
 bar(121,12,640,23);
 setfillstyle(1,0);
 bar(121,24,640,480);
 setcolor(15);
 line(121,12,640,12);
 line(121,12,121,480);
 line(122,479,639,479);
 line(639,479,639,23);
 setcolor(8);
 line(122,22,640,22);
 line(640,13,640,480);
 line(121,480,640,480);
 setcolor(15);
 outtextxy(123,14,cwd);
 closegadget(121,12);
 scrollbar(cfile,mfile);
}

void displaysetup(void)
{
 int sx,sy;
 char dd[3];

 setcolor(14);
 outtextxy(10,20,"Main D.R.A.C.O.S. directory");
 setcolor(15);
 rectangle(10,30,630,40);
 outtextxy(12,32,maindir);
 sx=10;
 sy=54;
 setcolor(14);
 outtextxy(10,44,"Drive Icon");
 setcolor(15);
 for (num=0;num<26;num++){
  strcpy(dd," ");
  dd[0]=num+65;
  strcat(dd,":");
  outtextxy(sx,sy,dd);
  rectangle(sx+20,sy-2,sx+118,sy+8);
  outtextxy(sx+22,sy,driveicon[num]);
  sy+=10;
  if (sy>480){
   sx+=180;
   sy=44;
  }
 }
}

void drivescrollbar(int cur,int maxf)
{
 int per,count,cc,sby;

 if (maxf>8){
  cc=maxf;
  count=0;
  do{
   count++;
   cc-=9;
  }while(cc>9);
  count++;
  setfillstyle(1,7);
  bar(100,25,120,478);
  setfillstyle(1,0);
  bar(102,45,118,458);
  per=(413/count);
  setfillstyle(1,7);
  cc=0;
  sby=46;
  gotoxy(1,1);
  if (cc<cur){
   do{
    cc+=9;
    sby+=per;
   }while(cc<cur);
  }
  bar(101,sby,119,(sby+per));
  setcolor(8);
  line(103,45,117,45);
  line(102,45,102,459);
  line(120,25,120,478);
  line(103,478,117,478);
  line(103,sby+per,117,sby+per);
  line(117,sby+per,117,sby);
  setcolor(15);
  line(101,25,119,25);
  line(100,25,100,478);
  line(118,46,118,458);
  line(118,458,102,458);
  line(103,sby,117,sby);
  line(103,sby,103,sby+per);
  dsbpt=sby;
  dsbpb=sby+per;
 } else {
  dsbpt=0;
  dsbpb=0;
 }
}


void displaydrive(void)
{
 int fdy,dn,nf,fx,ix;
 char fst[3],locat[192];

 for (num=0;num<26;num++){
  dicon[num].tx=-1;
  dicon[num].ty=-1;
  dicon[num].bx=-1;
  dicon[num].by=-1;
 }
 strcpy(fst," :");
 fdy=35;
 ix=0;
 setcolor(15);
 for (nf=cdrive;nf<=mdrive;nf++){
  if (da[nf]==1){
   fst[0]=nf+65;
   fst[2]=NULL;
   strcpy(locat,maindir);
   strcat(locat,"\\ICONS\\");
   strcat(locat,driveicon[nf]);
   load(locat);
   if (nf<=mdrive){
    dicon[ix].tx=60-(xsize/2);
    dicon[ix].ty=fdy;
    dicon[ix].bx=60+(xsize/2);
    dicon[ix].by=fdy+ysize;
    dicon[ix].num=nf;
    if (fdy<440) placeicon(60-(xsize/2),fdy,0);
    outtextxy(55,fdy+(ysize+2),fst);
   }
   fdy+=(ysize+10)+6;
   ix++;
  }
 }
 drivescrollbar(cdrive,mdrive);
}

void displayfile(void)
{
 int fdx,fdy,nf,fx,ix;
 char fst[5],lname[13],locat[192];


 for (num=0;num<55;num++){
  ficon[num].tx=-1;
  ficon[num].ty=-1;
  ficon[num].bx=-1;
  ficon[num].by=-1;
 }
 ix=0;
 fdx=113;
 fdy=25;
 setcolor(15);
 settextstyle(2,0,4);
 for (nf=cfile;nf<=cfile+44;nf++){
  strcpy(lname,"FILE0000.DIS");
  switch(fileinf[nf].type){
  case 0 : lname[7]='9';break;
  case 1 : lname[7]='0';break;
  case 2 : lname[7]='1';break;
  case 3 : lname[7]='2';break;
  case 4 : lname[7]='3';break;
  case 5 : lname[7]='4';break;
  case 6 : lname[7]='5';break;
  case 7 : lname[7]='6';break;
  case 8 : lname[7]='7';break;
  default: lname[7]='8';break;
  case 9 : lname[7]='8';break;
  case 10: lname[6]='1';break;
  }
  strcpy(locat,maindir);
  strcat(locat,"\\ICONS\\");
  strcat(locat,lname);

  if (nf<mfile){ //mfile

   load(locat);

   oldicon=fileinf[nf].type;
   ficon[ix].tx=fdx+50-(xsize/2);
   ficon[ix].ty=fdy;
   ficon[ix].bx=fdx+50+(xsize/2);
   ficon[ix].by=fdy+ysize;
   strcpy(ficon[ix].name,fileinf[nf].name);
   ficon[ix].type=fileinf[nf].type;
   if (strcmp(fileinf[nf].name,"N?A")!=0 && strcmp(fileinf[nf].name,"")!=0){
    placeicon(fdx+50-(xsize/2),fdy,0);
    if (fileinf[nf].tagged==0) outtextxy(fdx+48-((strlen(fileinf[nf].name)*5)/2),fdy+(ysize+2),fileinf[nf].name); else
    {
     outtextxy(fdx+48-((strlen(fileinf[nf].name)*5)/2),fdy+(ysize+2),fileinf[nf].name);
     outtextxy(fdx+49-((strlen(fileinf[nf].name)*5)/2),fdy+(ysize+2),fileinf[nf].name);
     rectangle(fdx+46-((strlen(fileinf[nf].name)*5)/2),fdy+(ysize+3),fdx+48+((strlen(fileinf[nf].name)*7)/2),fdy+(ysize+13));
    }
   }
  }
  ix++;
  if (xsize<32) xsize=32;
  fdx+=xsize+65;
  if (fdx>540){
   fdx=113;
   if (ysize<32) ysize=32;
   fdy+=(ysize+10)+6;
  }
 }
 settextstyle(0,0,0);
}

void displayicon(int icn,int tp)
{
 char locat[255],lname[13];

 // tp= 0 drive icon   1= file icon
 // icn= number of file
 strcpy(locat,maindir);
 strcat(locat,"\\ICONS\\");
 switch(tp){
 case 0 : {
	   strcat(locat,driveicon[icn]);
	   load(locat);
	   placeicon(dicon[icn].tx,dicon[icn].ty,1);
	  }break;
 case 1 : {
	   strcpy(lname,"FILE0000.DIS");
	   switch(ficon[icn].type){
	   case 0 : lname[7]='9';break;
	   case 1 : lname[7]='0';break;
	   case 2 : lname[7]='1';break;
	   case 3 : lname[7]='2';break;
	   case 4 : lname[7]='3';break;
	   case 5 : lname[7]='4';break;
	   case 6 : lname[7]='5';break;
	   case 7 : lname[7]='6';break;
	   case 8 : lname[7]='7';break;
	   default: lname[7]='8';break;
	   case 9 : lname[7]='8';break;
	   case 10: lname[6]='1';break;
	   }
	   strcat(locat,lname);
	   load(locat);
	   placeicon(ficon[icn].tx,ficon[icn].ty,1);
	  }break;

 }
}

void execute(char execfile[255],char param[255])
{
 int err;
 char locata[255];

 textmode(3);
 clrscr();
 strcpy(locata,maindir);
 strcat(locata,"\\EXECUTE.#$#");
 hand=fopen(locata,"wt");
 if (hand==NULL){
 } else {
  fputs("#MASTER",hand);
  fputc(10,hand);
  fputs("DDISKMAN.EXE",hand);
  fputc(10,hand);
  fputs("#CHILD",hand);
  fputc(10,hand);
  fputs(execfile,hand);
  fputc(10,hand);
  fputs("#PARAMETERS",hand);
  fputc(10,hand);
  fputs(param,hand);
  fputc(10,hand);
  fclose(hand);
 }
 strcpy(locata,maindir);
 strcat(locata,"\\EXECUTE.EXE");
 execvp(locata,NULL);
 setmodes();
 dracosbackground();
 getcwd(cwd,192);
 getfiles("*.*","++---+");
 drivesetup();
 displaydrive();
 filewindow();
 displayfile();
}

void leftclick()
{
 int nn,err,count,cc,per,spec,spec2;

 mouseoff();
 spec=shiftstate(0); //Right
 spec2=shiftstate(1); //Left
 lefttime=500;
 if (mousex>588 && mousey>-1 && mousex<640 && mousey<12){
  closegraph();
  exit(0);
 } else {
  if (mousex>100 && mousey>45 && mousex<120 && mousey<457){
  if (mousey<dsbpt){
   if (cdrive>0) cdrive-=8;
   if (cdrive<0) cdrive=0;
   drivesetup();
   displaydrive();
   filewindow();
   displayfile();
  } else
   if (mousey>dsbpb){
    if (cdrive<mdrive-8) cdrive+=8;
    if (cdrive>mdrive) cdrive=mdrive;
    drivesetup();
    displaydrive();
    filewindow();
    displayfile();
   } else drivescrollbar(cdrive,mdrive);
   if (mousex>621) mousex=621;
  }
 }
 if (mousey>12){
  if (mousex<121){
   activew=0;
   for (nn=0;nn<26;nn++){
    if (mousex>=dicon[nn].tx && mousey>=dicon[nn].ty && mousex<=dicon[nn].bx && mousey<=dicon[nn].by){
     displayicon(nn,0);
     setdisk(dicon[nn].num);
     getcwd(cwd,192);
     getfiles("*.*","++---+");
     filewindow();
     displayfile();
     drivesetup();
     displaydrive();
    }
   }
  } else
  {
   activew=1;    //DRIVE SCROLLBAR
   if (mousex>620 && mousey>45 && mousex<640 && mousey<457){
    if (mousey<sbpt){
     if (cfile>0) cfile-=45;
     if (cfile<0) cfile=0;
     filewindow();
     displayfile();
    } else
    if (mousey>sbpb){
     if (cfile<mfile-45) cfile+=45;
     if (cfile>mfile) cfile=mfile;
     filewindow();
     displayfile();
    } else scrollbar(cfile,mfile);
    if (mousex>621) mousex=621;
   } else {
    for (nn=0;nn<55;nn++){ //File icon clicking
     if (mousex>=ficon[nn].tx && mousey>=ficon[nn].ty && mousex<=ficon[nn].bx && mousey<=ficon[nn].by){
      if (ficon[nn].type==0){
       displayicon(nn,1);
       err=chdir(ficon[nn].name);
       if (err==0){
	getcwd(cwd,192);
	getfiles("*.*","++---+");
       }else mfile=0;
       filewindow();
       displayfile();
      } else
      if (ficon[nn].type==1 || ficon[nn].type==4){
       if (spec==0 && spec2==0) execute(ficon[nn].name,""); else
       if (fileinf[cfile+nn].tagged==0) fileinf[cfile+nn].tagged=1; else
       if (fileinf[cfile+nn].tagged==1) fileinf[cfile+nn].tagged=0;
       filewindow();
       displayfile();
      } else
      {
       if (spec==0 && spec2==0) {
	setfillstyle(1,0);
	bar(ficon[nn].tx,ficon[nn].ty,ficon[nn].bx,ficon[nn].by);
	displayicon(nn,1);
	delay(50);
	bar(ficon[nn].tx,ficon[nn].ty,ficon[nn].bx,ficon[nn].by);
       } else
       if (fileinf[cfile+nn].tagged==0) fileinf[cfile+nn].tagged=1; else
       if (fileinf[cfile+nn].tagged==1) fileinf[cfile+nn].tagged=0;
       filewindow();
       displayfile();

      }
     }
    }
   }
  }
 }
 mouseon();
}

void rightmenu(int type)
{
 /*
 0 file menu
 1 screen menu
 2 drive menu
 */
 rmx=mousex;
 rmy=mousey;
 rmt=type;

 mouseoff();
 if (rmx+100>639) rmx=mousex-100;
 if (rmy+100>479) rmy=mousey-100;
 setfillstyle(1,7);
 bar(rmx,rmy,rmx+99,rmy+99);
 setcolor(15);
 rectangle(rmx,rmy,rmx+99,rmy+99);
 setcolor(8);
 line(rmx+1,rmy+99,rmx+99,rmy+99);
 line(rmx+99,rmy+1,rmx+99,rmy+99);
 setcolor(15);
 switch(type){
 case 0 : {
	   outtextxy(rmx+2,rmy+2,"Copy");
	   outtextxy(rmx+2,rmy+12,"Rename");
	   outtextxy(rmx+2,rmy+22,"Move");
	   outtextxy(rmx+2,rmy+32,"Attributes");
	   outtextxy(rmx+2,rmy+42,"Properties");
	   outtextxy(rmx+2,rmy+52,"Send to");
	   outtextxy(rmx+2,rmy+62,"Link to");
	   outtextxy(rmx+2,rmy+82,"Delete");
	  }break;
 case 1 : {
	   outtextxy(rmx+2,rmy+2,"Rename");
	   outtextxy(rmx+2,rmy+12,"Run Defrag");
	   outtextxy(rmx+2,rmy+22,"Run Scandisk");
	   outtextxy(rmx+2,rmy+32,"Run Chkdsk");
	   outtextxy(rmx+2,rmy+42,"Run Compress");
	   outtextxy(rmx+2,rmy+52,"Properties");
	   outtextxy(rmx+2,rmy+62,"");
	   outtextxy(rmx+2,rmy+82,"Format");
	  }break;
 case 2 : {
	   outtextxy(rmx+2,rmy+2,"New Folder");
	   outtextxy(rmx+2,rmy+12,"New Text file");
	   outtextxy(rmx+2,rmy+22,"New DPROG file");
	   outtextxy(rmx+2,rmy+32,"Palette");
	   outtextxy(rmx+2,rmy+42,"Execute");
	   outtextxy(rmx+2,rmy+52,"Properties");
	   outtextxy(rmx+2,rmy+62,"About");
	   outtextxy(rmx+2,rmy+82,"Help");
	  }break;
 }
 mouseon();
}

void rightclick(void)
{
 int filloc,nn;

 filloc=-1;
 righttime=500;
 for (nn=0;nn<55;nn++){ //File icon clicking
  if (mousex>=ficon[nn].tx && mousey>=ficon[nn].ty && mousex<=ficon[nn].bx && mousey<=ficon[nn].by){
   filloc=nn;
  }
 }
 if (filloc!=-1 && strcmp(ficon[filloc].name,".")!=0 && strcmp(ficon[filloc].name,"..")!=0) { //located a file (right clicked on a file)
  rightmenu(0);
 }
}

void keypress(void)
{
 int ch;

 ch=getch();
 if (!ch){
  ch=getch();
 }
 switch(ch){
 case 27 : closegraph();exit(0);break;
 }
}

int error_window(int dii,int drive,int error,int rx,int ry)
{
 char info[80],chr[10];
 int x,y,ok;


 x=rx-100;
 y=ry;

 mouseoff();
 setfillstyle(1,7);
 setcolor(15);
 rectangle(x,y,x+400,y+100);
 rectangle(x+1,y+1,x+399,y+99);
 setfillstyle(1,7);
 bar(x+2,y+2,x+398,y+98);
 setcolor(8);
 line(x+1,y+100,x+400,y+100);
 line(x+400,y+1,x+400,y+100);
 line(x+2,y+99,x+399,y+99);
 line(x+399,y+2,x+399,y+99);
 setcolor(15);
 setfillstyle(1,1);
 bar(x+2,y+2,x+398,y+12);
 settextstyle(0,0,1);
 if (error>=0) outtextxy(x+5,y+5,"Disk error"); else
	       outtextxy(x+5,y+5,"Device error");
 outtextxy(x+5,y+15,"An error has occured accessing");
 strcpy(info,"drive [");
 chr[0]=drive+65;
 chr[1]=':';
 chr[2]=NULL;
 strcat(info,chr);
 strcat(info,"]");
 outtextxy(x+254,y+15,info);
 gotoxy(1,1);
 switch(dii){
 case 0 : outtextxy(x+5,y+30,"Disk write protected, please unprotect disk and");break;
 case 1 : outtextxy(x+5,y+30,"Unknown device");break;
 case 2 : outtextxy(x+5,y+30,"Drive not ready, check disk is inserted and try");break;
 case 3 : outtextxy(x+5,y+30,"Unknown command");break;
 case 4 : outtextxy(x+5,y+30,"CRC error in data");break;
 case 5 : outtextxy(x+5,y+30,"Incorrect length of drive request structure");break;
 case 6 : outtextxy(x+5,y+30,"Seek error");break;
 case 7 : outtextxy(x+5,y+30,"Unknown media type");break;
 case 8 : outtextxy(x+5,y+30,"Sector not found");break;
 case 9 : outtextxy(x+5,y+30,"Printer out of paper, please check paper is");break;
 case 10: outtextxy(x+5,y+30,"Write fault");break;
 case 11: outtextxy(x+5,y+30,"Read fault");break;
 case 12: outtextxy(x+5,y+30,"General failure");break;
 case 15: outtextxy(x+5,y+30,"Invalid disk change");break;
 }
 switch(dii){
 case 0 : outtextxy(x+5,y+40,"try again");break;
 case 1 : break;
 case 2 : outtextxy(x+5,y+40,"again");break;
 case 3 : break;
 case 8 :
 case 4 : outtextxy(x+5,y+40,"please check disks data integrity");break;
 case 5 : break;
 case 7 :
 case 10:
 case 11:
 case 6 : outtextxy(x+5,y+40,"please check drive and try again");break;
 case 9 : outtextxy(x+5,y+40,"inserted correctly");break;
 case 12: outtextxy(x+5,y+40,"please format disk and try again");break;
 case 15: outtextxy(x+5,y+40,"please re-insert previously removed disk");break;
 }

 setfillstyle(1,0);
 outtextxy(x+44,y+70,"Retry");
 outtextxy(x+141,y+70,"Cancel");
 rectangle(x+28,y+63,x+98,y+85);
 rectangle(x+128,y+63,x+198,y+85);
 setcolor(14);
 outtextxy(x+44,y+70,"R");
 outtextxy(x+141,y+70,"C");
 req=1;
 mouseon();
 do{
  if (kbhit()!=0) ch=toupper(getch());
  ok=0;
  oldmousex=mousex;
  oldmousey=mousey;
  mousex=mouseinfo(1);
  mousey=mouseinfo(2);
  but1=mouseinfo(3);
  but2=mouseinfo(4);
  if (but1!=0){
   if (mousex>=x+128 && mousex<=x+198 && mousey>=y+63 && mousey<=y+85) ok=1; else
   if (mousex>=x+28 && mousex<=x+98 && mousey>=y+63 && mousey<=y+85) ok=2;
  }
  if (mousex>622) mousex=622;
  if (mousey>468) mousey=468;
 }while(ch!=82 && ch!=67 && ok==0);
 mouseoff();
 setfillstyle(1,0);
 bar(x,y,x+400,y+100);
 bar(mousex,mousey,mousex+16,mousey+16);
 mouseon();
 if (ch==82 || ok==2) return(1); else return(0);
}

int handler(int errval,int ax,int bp,int si)
{

 int faildev,err,rc;
 /*
    Return values
    IGNORE 0    RETRY 1   ABORT 2 (ABORT QUITS CURRENT DOS APP'DRACOS')

  þ ax indicates whether a disk error or
    other device error was encountered.

    If ax is nonnegative, a disk error was
    encountered. Otherwise, the error was a
    device error.

    For a disk error, ax ANDed with 0x00FF
    give the failing drive number (0 equals
    A, 1 equals B, and so on).

  þ bp and si together point to the device
    driver header of the failing driver. bp
    contains the segment address, and si the
    offset.
 */
 if (bp==0) printf("");
 if (si==0) printf("");
 if (ax>=0) err=0; else err=1;
 faildev=ax&0x00FF;
 rc=error_window(errval,faildev,err,220,200);
 return(rc);
}

void setupcol(int clr)
{
 switch(clr){
 case 0 : setrgbpalette(0,col[0].r,col[0].g,col[0].b);break;
 case 1 : setrgbpalette(1,col[1].r,col[1].g,col[1].b);break;
 case 2 : setrgbpalette(2,col[2].r,col[2].g,col[2].b);break;
 case 3 : setrgbpalette(3,col[3].r,col[3].g,col[3].b);break;
 case 4 : setrgbpalette(4,col[4].r,col[4].g,col[4].b);break;
 case 5 : setrgbpalette(5,col[5].r,col[5].g,col[5].b);break;
 case 6 : setrgbpalette(20,col[6].r,col[6].g,col[6].b);break;
 case 7 : setrgbpalette(7,col[7].r,col[7].g,col[7].b);break;
 case 8 : setrgbpalette(56,col[8].r,col[8].g,col[8].b);break;
 case 9 : setrgbpalette(57,col[9].r,col[9].g,col[9].b);break;
 case 10: setrgbpalette(58,col[10].r,col[10].g,col[10].b);break;
 case 11: setrgbpalette(59,col[11].r,col[11].g,col[11].b);break;
 case 12: setrgbpalette(60,col[12].r,col[12].g,col[12].b);break;
 case 13: setrgbpalette(61,col[13].r,col[13].g,col[13].b);break;
 case 14: setrgbpalette(62,col[14].r,col[14].g,col[14].b);break;
 case 15: setrgbpalette(63,col[15].r,col[15].g,col[15].b);break;
 }
}

void loadpalette(void)
{
 char dr[255],palid[255];
 int la,pc;

 strcpy(palid,"                                                  ");
 strcpy(dr,maindir);
 strcat(dr,"\\");
 strcat(dr,"DRACOS.DPF");
 hand=fopen(dr,"rb");
 if (hand!=NULL){
  printf("Reading palette\n");
  la=fgetc(hand);
  for (num=0;num<la;num++){
   pc=fgetc(hand);
   palid[num]=pc;
  }
  printf("Palette id : %s\n",palid);
  for (num=0;num<16;num++){
   col[num].r=fgetc(hand);
   col[num].g=fgetc(hand);
   col[num].b=fgetc(hand);
  }
  fclose(hand);
  for (num=0;num<16;num++) setupcol(num);
 } else
 {
  printf("No Palette defined using default palette\n");
 }
 cleardevice();
}


void main(void)
{
 int err;

 harderr(handler);
 registerbgidriver(EGAVGA_driver);
 registerbgifont(small_font);
 Gd=9; /*9*/
 Gm=2; /*2*/
 initgraph(&Gd,&Gm,"");
 if (kbhit()!=0){
  do{
   ch=getch();
  }while(kbhit()!=0);
 }
 ch=0;
 cfile=0;
 oldicon=-1;
 cdrive=0;
 printf("DRACOS Disk Manager Copyright (C)1998-1999 EdzUp\n");
 detectgraph(&Gd,&Gm);
 switch(Gd){
 case 1 : printf("CGA");break;
 case 2 : printf("MCGA");break;
 case 3 : printf("EGA");break;
 case 4 : printf("EGA 64K");break;
 case 5 : printf("Mono EGA");break;
 case 6 : printf("IBM 8514");break;
 case 7 : printf("Hercules Monochrome");break;
 case 8 : printf("AT&T 6300");break;
 case 9 : printf("VGA");break;
 case 10: printf("IBM 3270");break;
 default: printf("Unknown");break;
 }
 printf(" adapter detected\n\n");
 getdrives();
 readold();
 clrscr();
 err=graphresult();
 if (err!=grOk){
  printf("Internal graphics error\n\n");
  printf("Graphics error: %s\n", grapherrormsg(err));
  printf("Press any key to halt:");
  getch();
  exit(1); /* terminate with an error code */
 }
 maxx=getmaxx();
 maxy=getmaxy();
 loadpalette();
 dracosbackground();
 img=0;
 lefttime=0;
 righttime=0;
 itype=0;
 xsize=32;
 ysize=32;
 oldxsize=xsize;
 oldysize=ysize;
 lclick=0;
 rclick=0;
 mousex=mouseinfo(1);
 mousey=mouseinfo(2);
 oldmousex=mousex;
 oldmousey=mousey;
 menu=0;
 getfiles("*.*","++---+");
 drivesetup();
 filewindow();
 displayfile();
 displaydrive();
 mouseon();
 do{
  oldmousex=mousex;
  oldmousey=mousey;
  mousex=mouseinfo(1);
  mousey=mouseinfo(2);
  but1=mouseinfo(3);
  but2=mouseinfo(4);
  if (but1!=0 && lefttime==0) {
   leftclick();
  }
  if (but2!=0 && righttime==0) {
   rightclick();
  }
  if (lefttime>0) lefttime--;
  if (righttime>0) righttime--;
  if (kbhit()!=0) keypress();
  if (mousex>621) mousex=621;
  if (mousey>468) mousey=468;
  setcolor(15);
 }while(ch!=255);
 mouseoff();
}