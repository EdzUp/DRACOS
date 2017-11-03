#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <graphics.h>
#include <dos.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <dir.h>
#include <process.h>
#include <ctype.h>

struct {
char name[38],execute[192],param[80],icon[13],linkname[13];
int x,y,priority;
} link[100];

struct REGPACK in; /*registers structure*/
struct ffblk ffblk; //file structure
struct ffblk dki;

struct {
char name[13];
char type;
} fileinf[1050];

struct {
int r,g,b;
} col[16];

char *s,maindir[192],icondir[192],linkdir[192],com[192],ch;
int Gd,Gm,rt,lt,num,ort,icon[32][33][2],am,ma,usr,req,mousetype;
int mousex,mousey,but1,but2,but3,oldmousex,oldmousey,curlink,maxlink;
FILE *hand;

int huge Detectsvga(void)
{
 return (2);
}

void setvid(void)
{
 int err;

 registerbgidriver(EGAVGA_driver);
 registerbgifont(small_font);
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

void dwindow(int tx,int ty,int bx,int by,char header[80])
{
 setfillstyle(1,7);
 bar(tx,ty,bx,by);
 settextstyle(0,0,0);
 setcolor(8);
 line(tx+1,by,bx,by);
 line(bx,by,bx,ty+1);
 line(tx+2,ty+11,bx,ty+11);
 line(bx-1,ty+11,bx-1,ty+2);
 setcolor(15);
 line(tx,ty,bx,ty);
 line(tx,ty,tx,by);
 line(tx+1,ty+1,bx-1,ty+1);
 line(tx+1,ty+1,tx+1,ty+11);
 outtextxy(tx+3,ty+3,header);
}

void quit(void)
{
 closegraph();
 exit(0);
}

void load(char fname[192])
{
 char ch,dosicon[255];
 int  ok,xsize,ysize,itype,dx,dy,n2;

 strcpy(dosicon,icondir);
 strcat(dosicon,fname);
 hand=fopen(dosicon,"rb");
 if (hand==NULL){
  gotoxy(40,24);
  printf("Error loading file. <SPACE>");
  gotoxy(40,25);
  printf("[%s]    ",fname);
  do{
   ch=getch();
  }while(ch!=32);
 } else {
  fread(&itype,2,2,hand);
  fread(&n2,2,2,hand);
  fread(&n2,2,2,hand);
  xsize=32;
  ysize=32;
  for (dy=1;dy<ysize+1;dy++){
   for (dx=0;dx<xsize;dx++){
    fread(&icon[dx][dy][0],2,2,hand);
    fread(&icon[dx][dy][1],2,2,hand);
   }
  }
  fclose(hand);
 }
}

void placeicon(int plx,int ply,int pimg)
{
 int plxx,plyy;

 for (plyy=1;plyy<33;plyy++){
  for (plxx=0;plxx<32;plxx++){
   if (icon[plxx][plyy][pimg]!=0) putpixel(plxx+plx,plyy+ply,icon[plxx][plyy][pimg]);
  }
 }
}

void showlink(int lnk)
{
 char dosicon[255],words[20][50],cw[50],chr[5],dispwords[20][50];
 int cx,cy,cwn,sp,mw,dw;
 cwn=0;
 mw=0;
 for (cx=0;cx<20;cx++) strcpy(words[cx],"");
 for (sp=0;sp<strlen(link[lnk].name);sp++){
  chr[0]=link[lnk].name[sp];
  chr[1]=NULL;
  if (chr[0]!=' ') strcat(words[cwn],chr);
  if (chr[0]==' ') cwn++;
 }
 mw=cwn;
 for (cwn=0;cwn<20;cwn++){
  strcpy(dispwords[cwn],"");
 }
 cwn=0;
 dw=0;
 do{
  if (strlen(dispwords[dw])+strlen(words[cwn])<15 || strcmp(dispwords[dw],NULL)==0) {
   if (strcmp(dispwords[dw],NULL)==0) strcpy(dispwords[dw],words[cwn]); else
   {
    strcat(dispwords[dw]," ");
    strcat(dispwords[dw],words[cwn]);
   }
  } else
  {
   dw++;
   strcpy(dispwords[dw],words[cwn]);
  }
  cwn++;
 }while(cwn<=mw);
 mw=cwn;
 load(link[lnk].icon);
 placeicon(link[lnk].x,link[lnk].y+23,0);
 setcolor(15);
 cx=link[lnk].x-10;
 cy=link[lnk].y+34+23;
 cwn=0;
 settextstyle(2,0,4);
 do{
  cx=(link[lnk].x+16)-((strlen(dispwords[cwn])*6)/2)+1;
  outtextxy(cx,cy,dispwords[cwn]);
  cy+=10;
  cwn++;
 }while(cwn<mw);
 settextstyle(0,0,0);
}

void morebutton(int x,int y)
{
 mouseoff();
 settextstyle(0,0,0);
 setcolor(15);
 setfillstyle(1,7);
 bar(x,y,x+40,y+19);
 line(x,y,x+40,y);
 line(x,y,x,y+19);
 outtextxy(x+5,y+6,"MORE");
 setcolor(8);
 line(x+40,y+1,x+40,y+19);
 line(x+40,y+19,x+1,y+19);
 mouseon();
}

void desktoplink(int pos)
{
 int lp;
 int lx,ly;

 ly=7;
 lx=40;
 if (curlink>0) morebutton(600,24);
 if (curlink+35<maxlink) morebutton(600,459);
 for (lp=pos;lp<pos+35;lp++){
  if (strcmp(link[lp].name,"")!=0 && strcmp(link[lp].icon,"")!=0 && lp<100){
   link[lp].x=lx;
   link[lp].y=ly;
   showlink(lp);
   ly+=60;
   if (ly>400){
    lx+=120;
    ly=7;
   }
  }
 }
}

void clearmenu(void)
{
 mouseoff();
 setfillstyle(1,0);
 bar(0,24,640,480);
 desktoplink(curlink);
 mouseon();
}

void titlebar(int op)
{
 if (op==0){
  setcolor(15);
  setfillstyle(1,7);
  bar(0,0,getmaxx(),11);
  rectangle(0,0,getmaxx(),11);
  outtextxy(3,2,"DRACOS (C)Copyright 1998-1999 EdzUp");
 }
 setfillstyle(1,7);
 bar(0,12,getmaxx(),23);
 setcolor(15);
 rectangle(0,12,getmaxx(),23);
 rectangle(0,12,62,23);
 rectangle(63,12,125,23);
 rectangle(126,12,182,23);
 rectangle(183,12,getmaxx(),23);
 outtextxy(3,14," Tools   Links   Help   ");
 setcolor(8);
 line(1,11,getmaxx(),11);
 line(getmaxx(),11,getmaxx(),1);
 line(1,23,62,23);
 line(62,23,62,13);
 line(64,23,125,23);
 line(125,23,125,13);
 line(127,23,182,23);
 line(182,23,182,13);
 line(184,23,getmaxx(),23);
 line(getmaxx(),23,getmaxx(),13);
}

void loadlink(int lk, char filename[13])
{
 int mode;
 char ss[255],newfile[255];

 strcpy(newfile,linkdir);
 strcat(newfile,"\\");
 strcat(newfile,filename);
 hand=fopen(newfile,"rt");
 if (hand==NULL) {
 } else {
  strcpy(link[lk].linkname,filename);
  mode=0;
  do{
   fgets(ss,255,hand);
   num=strlen(ss);
   ss[num-1]=NULL;
   if (mode==5 && strcmp(link[lk].param,"")==0) strcpy(link[lk].param,ss);
   if (mode==4 && strcmp(link[lk].execute,"")==0) strcpy(link[lk].execute,ss);
   if (mode==3 && link[lk].priority<0) link[lk].priority=atoi(ss);
   if (mode==2 && strcmp(link[lk].icon,"")==0) strcpy(link[lk].icon,ss);
   if (mode==1 && strcmp(link[lk].name,"")==0) strcpy(link[lk].name,ss);
   if (strcmp("#ICONNAME",ss)==0) mode=1;
   if (strcmp("#ICONFILE",ss)==0) mode=2;
   if (strcmp("#PRIORITY",ss)==0) mode=3;
   if (strcmp("#EXECUTE",ss)==0) mode=4;
   if (strcmp("#PARAMETERS",ss)==0) mode=5;
   if (ss[0]=='#') strcpy(ss,"");
  }while(feof(hand)==0);
  fclose(hand);
 }
 printf("Link: [%s]\n",link[lk].name);
 printf("Linkfile: [%s]\n",link[lk].linkname);
 printf("Priority: [%i]\n",link[lk].priority);
 printf("Link Executable: [%s]\n",link[lk].execute);
 printf("Link Parameters: [%s]\n\n",link[lk].param);
}

void readlinks(void)
{
 int hand,cl,er;
 char loca[255];

 for (num=0;num<100;num++){
  strcpy(link[num].name,"");
  strcpy(link[num].linkname,"");
  strcpy(link[num].icon,"");
  strcpy(link[num].execute,"");
  strcpy(link[num].param,"");
  link[num].priority=-1;
 }
 strcpy(loca,linkdir);
 strcat(loca,"\*.*");
 strcpy(link[0].name,"DRACOS Disk manager");
 strcpy(link[0].icon,"DDISKMAN.DIS");
 link[0].x=40;
 link[0].y=7;
 link[0].priority=0;
 strcpy(link[0].execute,maindir);
 strcat(link[0].execute,"\\DDISKMAN.EXE");
 strcpy(link[0].param,"");
 strcpy(link[1].name,"DRACOS Shell");
 strcpy(link[1].icon,"DSHELL.DIS");
 link[1].x=40;
 link[1].y=67;
 link[1].priority=1;
 strcpy(link[1].execute,maindir);
 strcat(link[1].execute,"\\DSHELL.EXE");
 strcpy(link[1].param,"");
 strcpy(link[2].name,"DRACOS Help");
 strcpy(link[2].icon,"DHELP.DIS");
 link[2].x=40;
 link[2].y=127;
 link[2].priority=2;
 strcpy(link[2].execute,maindir);
 strcat(link[2].execute,"\\DHELP.EXE");
 strcpy(link[2].param,"");
 cl=3;
 printf("%s\n",loca);
 er=findfirst(loca,&ffblk,FA_RDONLY|FA_SYSTEM|FA_ARCH|FA_HIDDEN);
 if (er!=-1) {
  loadlink(cl,ffblk.ff_name);
  do{
   er=findnext(&ffblk);
   cl++;
   if (er!=-1) loadlink(cl,ffblk.ff_name);
  }while(er!=-1 && cl<99);
  maxlink=cl;
 } else {
  printf(":DRACOS >> No user links defined\n\n");
 }
}

void execute(char execfile[255],char param[255], int ops)
{
 int err;
 char locata[255];
 char drive[3],dir[192],filename[13],ext[3],workdir[192];

 mouseoff();
 if (ops==0) {
  fnsplit(execfile,drive,dir,filename,ext);
  strcpy(workdir,drive);
  strcat(workdir,dir);
  err=chdir(workdir);
  if (err==-1){
   if (strcmp(workdir,"")!=0) {
    printf("Error : [%s]\n",workdir);
    exit(0);
   }
  }
 }
 textmode(3);
 clrscr();
 strcpy(locata,maindir);
 strcat(locata,"\\EXECUTE.#$#");
 hand=fopen(locata,"wt");
 if (hand==NULL){
 } else {
  fputs("#MASTER",hand);
  fputc(10,hand);
  fputs("DRACOS.EXE",hand);
  fputc(10,hand);
  fputs("#CHILD",hand);
  fputc(10,hand);
  printf("execfile %s\n",execfile);
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
 setvid();
 Gd=9;
 Gm=2;
 initgraph(&Gd,&Gm,"");
 titlebar(0);
 readlinks();
 desktoplink(curlink);
 mouseon();
}

void textrequest(int tx,int ty,int length,char str[255],int max)
{
 char ch,tcom[255],chr[3],oc[255];
 int pos,sp,tp,bl;

 mouseoff();
 strcpy(oc,str);
 bl=(length*12)+4;
 setfillstyle(1,0);
 bar(tx,ty-1,bl,ty+9);
 tp=tx+2;
 pos=0;
 strcpy(tcom,"");
 strcpy(tcom,str);
 pos=strlen(tcom);
 setcolor(15);
 outtextxy(tx+2,ty,tcom);
 setcolor(15);
 rectangle(tp,ty+7,tp+9,ty+8);
 do{
  ch=getch();
  setfillstyle(1,0);
  bar(tx,ty-1,bl,ty+9);
  setfillstyle(1,7);
  bar(bl,ty-1,bl+10,ty+9);
  if (ch==8 && pos>0) {
   pos--;
   tcom[pos+1]=NULL;
  } else
  if (ch>31 && pos<max) {
   tcom[pos]=ch;
   tcom[pos+1]=NULL;
   pos++;
  }
  setcolor(15);
  sp=pos-length;
  if (sp<0) sp=0;
  tp=tx+2;
  if (pos>0) {
   do{
    chr[0]=tcom[sp];
    chr[1]=NULL;
    outtextxy(tp,ty,chr);
    sp++;
    tp+=8;
   }while(sp<strlen(tcom) && sp<pos);
  }
  rectangle(tp,ty+7,tp+9,ty+8);
 }while(ch!=13 && ch!=27);
 if (ch==27){
  strcpy(com,oc);
  bl=(length*12)+4;
  setfillstyle(1,0);
  bar(tx,ty-1,bl,ty+9);
  setcolor(15);
  outtextxy(tx+2,ty,oc);
 } else {
  strcpy(com,tcom);
 }
 setcolor(0);
 rectangle(tp,ty+7,tp+9,ty+8);
 setfillstyle(1,7);
 bar(bl,ty-1,bl+10,ty+9);
 mouseon();
}

void getfile(char dir[255],int filenum)
{
 int er,count;

 count=0;
 er=findfirst(dir,&ffblk,FA_RDONLY|FA_SYSTEM|FA_ARCH|FA_HIDDEN);
 if (er!=-1){
  do{
   strcpy(com,ffblk.ff_name);
   er=findnext(&ffblk);
   if (count<filenum) count++;
  } while(er!=-1 && count<filenum);
  if (com[strlen(com)-1]!='S') {
   strcpy(com,ffblk.ff_name);
  }
  if (er==-1 && count<filenum) strcpy(com,".....");
 }
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

void getfiles(char param[255]) /* gatr +:visible -:hidden*/
{
 /* Attributes are : 0 None      1 Readonly      2 Hidden
		     4 System    8 Label        16 Directory
		    32 Archived */
 int dirx,diry,err,cdirfile,fo,fatr[6],dx;
 char atr,ext[4];

 dirx=1;
 diry=1;
 for (num=0;num<26;num++){
  strcpy(ext,"[ :]");
  ext[1]=num+65;
  strcpy(fileinf[num].name,ext);
  fileinf[num].type=5; //Drive
 }
 cdirfile=26;
 err=findfirst(param,&dki,FA_RDONLY|FA_ARCH|FA_HIDDEN|FA_SYSTEM|FA_DIREC|FA_LABEL);
 if (err==0) {
  do{
   strcpy(fileinf[cdirfile].name,dki.ff_name);
   for (dx=0;dx<6;dx++) fatr[dx]=0;
   gotoxy(dirx,diry);
   atr=dki.ff_attrib;
   fileinf[cdirfile].type=0; //ordinary file
   if (atr-32>-1){
    fatr[0]=1;
    atr-=32;
   }
   if (atr-16>-1){
    fatr[1]=1;
    atr-=16;
    fileinf[cdirfile].type=1; //Directory
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
   }
   exttype(fileinf[cdirfile].name);
   if (usr==1 || usr==4) fileinf[cdirfile].type=2; //executable file
   if (fatr[1]==1) usr=0;
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
 for (num=cdirfile+1;num<1024;num++){
  strcpy(fileinf[num].name,"");
 }
}

void cdproc(void)
{

}

void browse(int usl)
{
 char cbd[255],chr[3],iconnames[13],testd[255];
 int sl,cl,ok,nn,nd,na,cdt,bdrv;

 cdt=0;
 getcwd(cbd,255);
 getfiles("*.*");
 mouseoff();
 dwindow(120,30,440,450,"Browse");
 outtextxy(123,45,"Select a file you wish to link to");
 setcolor(15);
 setfillstyle(1,0);
 bar(123,85,437,400);
 chr[0]=30;
 chr[1]=NULL;
 setcolor(15);
 settextstyle(0,0,2);
 rectangle(125,68,435,81);
 outtextxy(270,68,chr);
 chr[0]=31;
 chr[1]=NULL;
 rectangle(125,404,435,417);
 outtextxy(270,404,chr);
 settextstyle(0,0,0);
 outtextxy(257,433,"Cancel");
 rectangle(232,425,327,448);
 cl=0;
 for (nn=cl;nn<cl+35;nn++){
  if (fileinf[nn].type==1) setcolor(7); else setcolor(15);
  outtextxy(125,87+((nn-cl)*9),fileinf[nn].name);
 }
 ok=0;
 mouseon();
 do{
  oldmousex=mousex;
  oldmousey=mousey;
  mousex=mouseinfo(1);
  mousey=mouseinfo(2);
  but1=mouseinfo(3);
  but2=mouseinfo(4);
  if (but1!=0) rt++;
  if (rt>500) rt=0;
  if (cdt>0) cdt--;
  if (but1!=0 && rt==0){
   sl=-1;
   if (mousex>=123 && mousex<=437 && mousey>=85 && mousey<=400){
    sl=(mousey-85)/9;
   }
   if (sl!=-1) {
    if (fileinf[cl+sl].type==2) ok=1; else
    if (fileinf[cl+sl].type==5){
     bdrv=cl+sl;
     setdisk(bdrv);
     cl=0;
     cdt=500;
     getfiles("*.*");
     mouseoff();
     setfillstyle(1,0);
     bar(123,85,437,400);
     for (nn=cl;nn<cl+35;nn++){
      if (fileinf[nn].type==1) setcolor(7); else setcolor(15);
      outtextxy(125,87+((nn-cl)*9),fileinf[nn].name);
     }
     mouseon();
    } else
    if (fileinf[cl+sl].type==1 && cdt==0) {
     chdir(fileinf[cl+sl].name);
     cl=0;
     cdt=500;
     getfiles("*.*");
     setfillstyle(1,0);
     mouseoff();
     bar(123,85,437,400);
     for (nn=cl;nn<cl+35;nn++){
      if (fileinf[nn].type==1) setcolor(7); else setcolor(15);
      outtextxy(125,87+((nn-cl)*9),fileinf[nn].name);
     }
     mouseon();
    }
   }
   if (mousex>=232 && mousex<=327 && mousey>=425 && mousey<=448) ok=2;
   if (mousex>=125 && mousey>=68 && mousex<=435 && mousey<=81) {
    mouseoff();
    setfillstyle(1,0);
    bar(123,85,437,400);
    cl--;
    if (cl<=0) cl=0;
    for (nn=cl;nn<cl+35;nn++){
     if (fileinf[nn].type==1) setcolor(7); else setcolor(15);
     outtextxy(125,87+((nn-cl)*9),fileinf[nn].name);
    }
    mouseon();
   } else
   if (mousex>=125 && mousey>=404 && mousex<=435 && mousey<=417) {
    setfillstyle(1,0);
    mouseoff();
    bar(123,85,437,400);
    cl++;
    if (cl>=1007) cl=1007;
    for (nn=cl;nn<cl+35;nn++){
     if (fileinf[nn].type==1) setcolor(7); else setcolor(15);
     outtextxy(125,87+((nn-cl)*9),fileinf[nn].name);
    }
    mouseon();
   }
  }
  if (mousex>622) mousex=622;
  if (mousey>468) mousey=468;
 }while(ok==0);
 if (ok==1) {
  //sl;
  getcwd(testd,255);
  strcat(testd,"\\");
  strcat(testd,fileinf[cl+sl].name);
  strcpy(link[usl].execute,testd);
 }
 clearmenu();
 ma=0;
 am=0;
 chdir(cbd);
 mouseoff();
 dwindow(100,240,540,440,"");
 if (nd==0) outtextxy(103,243,"Create link"); else outtextxy(103,243,"Edit Link");
 outtextxy(123,273,"DRACOS Link name (38 characters maximum)");
 outtextxy(123,303,"Executable location and filename");
 outtextxy(123,333,"Executable file parameters");
 outtextxy(270,397,"DRACOS Icon");
 outtextxy(168,398,"Save");
 outtextxy(434,398,"Cancel");
 outtextxy(461,270,"Browse");
 setcolor(15);
 rectangle(143,387,223,417);
 rectangle(417,387,497,417);
 rectangle(450,265,520,281);
 setfillstyle(1,0);
 setcolor(15);
 bar(120,283,(43*12)+4,293);
 if (strcmp(link[usl].name,"")!=0) outtextxy(122,285,link[usl].name);
 bar(120,313,(43*12)+4,323);
 if (ok==1) outtextxy(122,315,link[usl].execute);
 bar(120,343,(43*12)+4,353);
 if (strcmp(link[usl].param,"")!=0) outtextxy(122,345,link[usl].param);
 bar(295,363,327,395);
 chr[0]=16;
 chr[1]=NULL;
 setcolor(15);
 settextstyle(0,0,2);
 outtextxy(330,373,chr);
 chr[0]=17;
 chr[1]=NULL;
 outtextxy(279,373,chr);
 load("FILE0007.DIS");
 if (strcmp(link[usl].icon,"")==0) strcpy(link[usl].icon,"FILE0007.DIS");
 placeicon(295,363,0);
 settextstyle(0,0,0);
 mouseon();
}

void createlink(int nd)
{
 int ok,na,er,lc;
 char iconnames[13],icond[255],chr[3];
 char drive[3],dir[192],fn[13],ext[5];

 strcpy(icond,icondir);
 strcat(icond,"\*.DIS");
 lc=2;
 getfile(icond,lc);
 if (strcmp(com,".....")!=0) strcpy(iconnames,com);
 if (nd==0){
  na=0;
  do{
   na++;
  }while(link[na].priority>-1 && na<100);
 } else na=nd-1;
 clearmenu();
 ma=0;
 am=0;
 if (na<100){
  mouseoff();
  dwindow(100,240,540,440,"");
  if (nd==0) outtextxy(103,243,"Create link"); else outtextxy(103,243,"Edit Link");
  outtextxy(123,273,"DRACOS Link name (38 characters maximum)");
  outtextxy(123,303,"Executable location and filename");
  outtextxy(123,333,"Executable file parameters");
  outtextxy(270,397,"DRACOS Icon");
  outtextxy(168,398,"Save");
  outtextxy(434,398,"Cancel");
  outtextxy(461,270,"Browse");
  setcolor(15);
  rectangle(143,387,223,417);
  rectangle(417,387,497,417);
  rectangle(450,265,520,281);
  setfillstyle(1,0);
  setcolor(15);
  bar(120,283,(43*12)+4,293);
  if (strcmp(link[na].name,"")!=0) outtextxy(122,285,link[na].name);
  bar(120,313,(43*12)+4,323);
  if (strcmp(link[na].execute,"")!=0) outtextxy(122,315,link[na].execute);
  bar(120,343,(43*12)+4,353);
  if (strcmp(link[na].param,"")!=0) outtextxy(122,345,link[na].param);
  strcpy(iconnames,link[na].icon);
  bar(295,363,327,395);
  chr[0]=16;
  chr[1]=NULL;
  setcolor(15);
  settextstyle(0,0,2);
  outtextxy(330,373,chr);
  chr[0]=17;
  chr[1]=NULL;
  outtextxy(279,373,chr);
  load("FILE0007.DIS");
  placeicon(295,363,0);
  settextstyle(0,0,0);
  mouseon();
  do{
   strcpy(com,"");
   ok=0;
   oldmousex=mousex;
   oldmousey=mousey;
   mousex=mouseinfo(1);
   mousey=mouseinfo(2);
   but1=mouseinfo(3);
   but2=mouseinfo(4);
   if (mousex>622) mousex=622;
   if (mousey>468) mousey=468;
   if (but1!=0 && rt==0){
    if (mousex>=450 && mousex<=520 && mousey>=265 && mousey<=281){
     browse(na);
    }
    if (mousex>=120 && mousex<=(43*12)+4 && mousey>=283 && mousey<=293){
     mouseoff();
     textrequest(120,285,43,link[na].name,38);
     strcpy(link[na].name,com);
     mouseon();
    }
    if (mousex>=120 && mousex<=(43*12)+4 && mousey>=313 && mousey<=323){
     mouseoff();
     textrequest(120,315,43,link[na].execute,192);
     strcpy(link[na].execute,com);
     mouseon();
    }
    if (mousex>=120 && mousex<=(43*12)+4 && mousey>=343 && mousey<=353){
     mouseoff();
     textrequest(120,345,43,link[na].param,80);
     strcpy(link[na].param,com);
     mouseon();
    }
    if (mousex>=279 && mousex<=295 && mousey>=373 && mousey<=389 && rt==0) {
     if (lc>0) lc--;
     getfile(icond,lc);
     if (strcmp(com,".....")!=0) strcpy(iconnames,com);
     load(iconnames);
     mouseoff();
     setfillstyle(1,0);
     bar(295,363,327,395);
     placeicon(295,363,0);
     mouseon();
    }
    if (mousex>=330 && mousex<=346 && mousey>=373 && mousey<=389 && rt==0) {
     lc++;
     getfile(icond,lc);
     if (strcmp(com,".....")!=0) strcpy(iconnames,com); else
     {
      lc--;
      getfile(icond,lc);
      if (strcmp(com,".....")!=0) strcpy(iconnames,com);
     }
     load(iconnames);
     setfillstyle(1,0);
     bar(295,363,327,395);
     placeicon(295,363,0);
    }
    if (mousex>=143 && mousex<=223 && mousey>=387 && mousey<=419){
     ok=1;
    }
    if (mousex>=417 && mousex<=497 && mousey>=387 && mousey<=419){
     ok=2;
    }
   }
   if (but1!=0) rt++; else rt=0;
  }while(ok==0);
  if (ok==2 && nd==0){
   strcpy(link[na].name,"");
   strcpy(link[na].icon,"");
   strcpy(link[na].execute,"");
   strcpy(link[na].param,"");
   strcpy(link[na].linkname,"");
   link[na].priority=-1;
  }
  if (ok==1 && nd!=1 && nd!=2 && nd!=3){
   er=0;
   if (er==0){
    fnsplit(link[na].execute,drive,dir,fn,ext);
    strcpy(icond,linkdir);
    strcat(icond,"\\");
    strcat(icond,fn);
    strcat(icond,".DLF");
    strcpy(link[na].icon,iconnames);
    hand=fopen(icond,"wt");
    if (hand!=NULL){
     fputs("#ICONNAME",hand);
     fputc(10,hand);
     fputs(link[na].name,hand);
     fputc(10,hand);
     fputs("#ICONFILE",hand);
     fputc(10,hand);
     fputs(link[na].icon,hand);
     fputc(10,hand);
     fputs("#PRIORITY",hand);
     fputc(10,hand);
     itoa(na,fn,10);
     fputs(fn,hand);
     fputc(10,hand);
     fputs("#EXECUTE",hand);
     fputc(10,hand);
     fputs(link[na].execute,hand);
     fputc(10,hand);
     fputs("PARAMETERS",hand);
     fputc(10,hand);
     fputs(link[na].param,hand);
     fclose(hand);
    } else {
     if (strcmp(link[na].execute,"")==0){
      strcpy(link[na].name,"");
      strcpy(link[na].icon,"");
      strcpy(link[na].execute,"");
      strcpy(link[na].param,"");
      strcpy(link[na].linkname,"");
      link[na].priority=-1;
     } else {
      printf("Error encountered creating link file\n");
     }
    }
   }
  }
  clearmenu();
  ma=0;
  am=0;
 }
}

void editlink(void)
{
 int slink,cl,nn,sl,ok,rt;
 char chr[3];

 mouseoff();
 dwindow(120,120,440,370,"Edit Link");
 outtextxy(123,135,"Select the link you wish to edit");
 setcolor(15);
 setfillstyle(1,0);
 bar(123,165,437,310);
 chr[0]=30;
 chr[1]=NULL;
 setcolor(15);
 settextstyle(0,0,2);
 rectangle(125,150,435,163);
 outtextxy(270,150,chr);
 chr[0]=31;
 chr[1]=NULL;
 rectangle(125,312,435,325);
 outtextxy(270,312,chr);
 cl=3;
 settextstyle(0,0,0);
 outtextxy(257,343,"Cancel");
 rectangle(232,335,327,358);
 settextstyle(2,0,4);
 for (nn=cl;nn<cl+16;nn++){
  outtextxy(125,164+((nn-cl)*9),link[nn].name);
 }
 rt=0;
 ok=0;
 mouseon();
 do{
  oldmousex=mousex;
  oldmousey=mousey;
  mousex=mouseinfo(1);
  mousey=mouseinfo(2);
  but1=mouseinfo(3);
  but2=mouseinfo(4);
  if (but1!=0 && rt==0){
   sl=-1;
   if (mousex>=122 && mousex<=436 && mousey>=164 && mousey<=173) sl=0; else
   if (mousex>=122 && mousex<=436 && mousey>=174 && mousey<=182) sl=1; else
   if (mousex>=122 && mousex<=436 && mousey>=183 && mousey<=191) sl=2; else
   if (mousex>=122 && mousex<=436 && mousey>=192 && mousey<=200) sl=3; else
   if (mousex>=122 && mousex<=436 && mousey>=201 && mousey<=209) sl=4; else
   if (mousex>=122 && mousex<=436 && mousey>=210 && mousey<=218) sl=5; else
   if (mousex>=122 && mousex<=436 && mousey>=219 && mousey<=227) sl=6; else
   if (mousex>=122 && mousex<=436 && mousey>=228 && mousey<=236) sl=7; else
   if (mousex>=122 && mousex<=436 && mousey>=237 && mousey<=245) sl=8; else
   if (mousex>=122 && mousex<=436 && mousey>=246 && mousey<=254) sl=9; else
   if (mousex>=122 && mousex<=436 && mousey>=255 && mousey<=263) sl=10; else
   if (mousex>=122 && mousex<=436 && mousey>=264 && mousey<=272) sl=11; else
   if (mousex>=122 && mousex<=436 && mousey>=273 && mousey<=281) sl=12; else
   if (mousex>=122 && mousex<=436 && mousey>=282 && mousey<=290) sl=13; else
   if (mousex>=122 && mousex<=436 && mousey>=291 && mousey<=299) sl=14; else
   if (mousex>=122 && mousex<=436 && mousey>=300 && mousey<=309) sl=15;
   if (sl!=-1) ok=1;
   if (mousex>=232 && mousex<=327 && mousey>=335 && mousey<=358) ok=2;
   if (mousex>=125 && mousey>=150 && mousex<=435 && mousey<=163) {
    setfillstyle(1,0);
    bar(123,165,437,310);
    cl--;
    if (cl<=3) cl=3;
    for (nn=cl;nn<cl+16;nn++){
     outtextxy(125,164+((nn-cl)*9),link[nn].name);
    }
   } else
   if (mousex>=125 && mousey>=312 && mousex<=435 && mousey<=325) {
    mouseoff();
    setfillstyle(1,0);
    bar(123,165,437,310);
    cl++;
    if (cl>=83) cl=83;
    for (nn=cl;nn<cl+16;nn++){
     outtextxy(125,164+((nn-cl)*9),link[nn].name);
    }
    mouseon();
   }
  }
  if (but1!=0) rt++;
  if (rt>100) rt=0;
  if (mousex>622) mousex=622;
  if (mousey>468) mousey=468;
 }while(ok==0);
 settextstyle(0,0,0);
 if (ok==1) createlink((sl+1)+cl);
 clearmenu();
 ma=0;
 am=0;
}

void deletelink(void)
{
 int slink,cl,nn,sl,ok,rt;
 char chr[3],filename[255];

 mouseoff();
 dwindow(120,120,440,370,"Delete Link");
 outtextxy(123,135,"Select the link you wish to delete");
 setcolor(15);
 setfillstyle(1,0);
 bar(123,165,437,310);
 chr[0]=30;
 chr[1]=NULL;
 setcolor(15);
 settextstyle(0,0,2);
 rectangle(125,150,435,163);
 outtextxy(270,150,chr);
 chr[0]=31;
 chr[1]=NULL;
 rectangle(125,312,435,325);
 outtextxy(270,312,chr);
 cl=3;
 settextstyle(0,0,0);
 outtextxy(257,343,"Cancel");
 rectangle(232,335,327,358);
 for (nn=cl;nn<cl+16;nn++){
  outtextxy(125,167+((nn-cl)*9),link[nn].name);
 }
 rt=0;
 ok=0;
 mouseon();
 do{
  oldmousex=mousex;
  oldmousey=mousey;
  mousex=mouseinfo(1);
  mousey=mouseinfo(2);
  but1=mouseinfo(3);
  but2=mouseinfo(4);
  if (but1!=0 && rt==0){
   sl=-1;
   if (mousex>=122 && mousex<=436 && mousey>=164 && mousey<=173) sl=0; else
   if (mousex>=122 && mousex<=436 && mousey>=174 && mousey<=182) sl=1; else
   if (mousex>=122 && mousex<=436 && mousey>=183 && mousey<=191) sl=2; else
   if (mousex>=122 && mousex<=436 && mousey>=192 && mousey<=200) sl=3; else
   if (mousex>=122 && mousex<=436 && mousey>=201 && mousey<=209) sl=4; else
   if (mousex>=122 && mousex<=436 && mousey>=210 && mousey<=218) sl=5; else
   if (mousex>=122 && mousex<=436 && mousey>=219 && mousey<=227) sl=6; else
   if (mousex>=122 && mousex<=436 && mousey>=228 && mousey<=236) sl=7; else
   if (mousex>=122 && mousex<=436 && mousey>=237 && mousey<=245) sl=8; else
   if (mousex>=122 && mousex<=436 && mousey>=246 && mousey<=254) sl=9; else
   if (mousex>=122 && mousex<=436 && mousey>=255 && mousey<=263) sl=10; else
   if (mousex>=122 && mousex<=436 && mousey>=264 && mousey<=272) sl=11; else
   if (mousex>=122 && mousex<=436 && mousey>=273 && mousey<=281) sl=12; else
   if (mousex>=122 && mousex<=436 && mousey>=282 && mousey<=290) sl=13; else
   if (mousex>=122 && mousex<=436 && mousey>=291 && mousey<=299) sl=14; else
   if (mousex>=122 && mousex<=436 && mousey>=300 && mousey<=309) sl=15;
   if (sl!=-1) ok=1;
   if (mousex>=232 && mousex<=327 && mousey>=335 && mousey<=358) ok=2;
   if (mousex>=125 && mousey>=150 && mousex<=435 && mousey<=163) {
    mouseoff();
    setfillstyle(1,0);
    bar(123,165,437,310);
    cl--;
    if (cl<=3) cl=3;
    for (nn=cl;nn<cl+16;nn++){
     outtextxy(125,167+((nn-cl)*9),link[nn].name);
    }
    mouseon();
   } else
   if (mousex>=125 && mousey>=312 && mousex<=435 && mousey<=325) {
    mouseoff();
    setfillstyle(1,0);
    bar(123,165,437,310);
    cl++;
    if (cl>=83) cl=83;
    for (nn=cl;nn<cl+16;nn++){
     outtextxy(125,167+((nn-cl)*9),link[nn].name);
    }
    mouseon();
   }
  }
  if (but1!=0) rt++;
  if (rt>100) rt=0;
  if (mousex>622) mousex=622;
  if (mousey>468) mousey=468;
  //if (mousex!=oldmousex || mousey!=oldmousey) showcursor(mousex,mousey);
 }while(ok==0);
 if (ok==1) {
  //sl;
  sl+=cl;
  strcpy(filename,linkdir);
  strcat(filename,"\\");
  strcat(filename,link[sl].linkname);
  unlink(filename);
  strcpy(link[sl].name,"");
  strcpy(link[sl].icon,"");
  strcpy(link[sl].execute,"");
  strcpy(link[sl].param,"");
  strcpy(link[sl].linkname,"");
  link[sl].priority=-1;

 }
 clearmenu();
 ma=0;
 am=0;
}


void format(void)
{
 int ok,options[4];
 char drive[5],param[40],ch;

 for (num=0;num<4;num++) options[num]=0;
 clearmenu();
 ma=0;
 am=0;
 mouseoff();
 dwindow(120,120,410,315,"Format");
 setcolor(15);
 outtextxy(231,144,"Drive   :");
 setfillstyle(1,0);
 bar(276,142,291,154);
 rt=0;
 mouseon();
 do{
  ok=0;
  oldmousex=mousex;
  oldmousey=mousey;
  mousex=mouseinfo(1);
  mousey=mouseinfo(2);
  but1=mouseinfo(3);
  but2=mouseinfo(4);
  if (mousex>622) mousex=622;
  if (mousey>468) mousey=468;
  if (but1!=0 && rt==0){
   if (mousex>=276 && mousey>=142 && mousex<=291 && mousey<=154){
    do{
     ch=getch();
    }while(ch<'a' && ch>'z' && ch<'A' && ch>'Z');
    drive[0]=ch;
    drive[1]=NULL;
    setfillstyle(1,0);
    bar(276,142,291,154);
    setcolor(15);
    outtextxy(280,144,drive);
    strcat(drive,":");
   } else
   if (mousex>=151 && mousey>=157 && mousex<=379 && mousey<=177){
    if (options[0]==1) options[0]=0; else options[0]=1;
   } else
   if (mousex>=151 && mousey>=187 && mousex<=379 && mousey<=207){
    if (options[1]==1) options[1]=0; else options[1]=1;
   } else
   if (mousex>=151 && mousey>=217 && mousex<=379 && mousey<=237){
    if (options[2]==1) options[2]=0; else options[2]=1;
   } else
   if (mousex>=151 && mousey>=247 && mousex<=379 && mousey<=267){
    options[3]++;
    if (options[3]>7) options[3]=0;
   } else
   if (mousex>=151 && mousey>=277 && mousex<=253 && mousey<=297){
    ok=1;
   } else
   if (mousex>=263 && mousey>=277 && mousex<=379 && mousey<=297){
    ok=2;
   }
   setfillstyle(1,7);
   bar(151,157,379,310);
  }
  setcolor(15);
  mouseoff();
  rectangle(151,157,379,177);
  if (options[0]==0) setcolor(15); else setcolor(8);
  outtextxy(223,164,"Full format");
  setcolor(15);
  rectangle(151,187,379,207);
  if (options[1]==0) setcolor(15); else setcolor(8);
  outtextxy(197,194,"Copy system files");
  setcolor(15);
  rectangle(151,217,379,237);
  if (options[2]==0) setcolor(15); else setcolor(8);
  outtextxy(201,224,"Test BAD sectors");
  setcolor(15);
  rectangle(151,247,379,267);
  if (ch=='a' || ch=='b' || ch=='A' || ch=='B') setcolor(15); else setcolor(8);
  if (options[3]==0) outtextxy(201,254,"Format to 1.44Mb"); else
  if (options[3]==1) outtextxy(210,254,"Format to 720K"); else
  if (options[3]==2) outtextxy(205,254,"Format to 1.2Mb"); else
  if (options[3]==3) outtextxy(210,254,"Format to 360K"); else
  if (options[3]==4) outtextxy(205,254,"Format to 2.8Mb"); else
  if (options[3]==5) outtextxy(210,254,"Format to 320K"); else
  if (options[3]==6) outtextxy(210,254,"Format to 180K"); else
  if (options[3]==7) outtextxy(210,254,"Format to 160K");
  setcolor(15);
  rectangle(151,277,253,297);
  outtextxy(194,284,"Ok");
  rectangle(263,277,379,297);
  outtextxy(298,284,"Cancel");
  mouseon();
//  if (mousex!=oldmousex || mousey!=oldmousey) showcursor(mousex,mousey);
  if (but1!=0) rt++; else rt=0;
 }while(ok==0);
 if (ok==1){
  strcpy(param,drive);
  strcat(param," ");
  if (options[0]==1) strcat(param,"/Q"); else strcat(param,"/U");
  if (options[1]==0) strcat(param,"/S");
  if (options[2]==0) strcat(param,"/C");
  if (options[3]==0) strcat(param,"/F:1.44"); else
  if (options[3]==1) strcat(param,"/F:720"); else
  if (options[3]==2) strcat(param,"/F:1.2"); else
  if (options[3]==3) strcat(param,"/F:360"); else
  if (options[3]==4) strcat(param,"/F:2.88"); else
  if (options[3]==5) strcat(param,"/F:320"); else
  if (options[3]==6) strcat(param,"/F:180"); else
  if (options[3]==7) strcat(param,"/F:160");
  execute("FORMAT.COM",param,0);
 }
 clearmenu();
 ma=0;
 am=0;
}

void copydisk(void)
{
 int from,two,ok;
 char drive[5],param[40],ch;
 mouseoff();
 dwindow(100,240,540,310,"Copy disk");
 outtextxy(118,258,"Please specify a source drive and destination drive");
 rectangle(118,270,218,300);
 rectangle(243,270,343,300);
 outtextxy(223,282,"to");
 rectangle(442,270,522,300);
 outtextxy(460,282,"Cancel");
 rectangle(345,270,440,300);
 outtextxy(385,282,"OK");
 rt=0;
 ok=0;
 from=0;
 two=0;
 outtextxy(162,282,"A:");
 outtextxy(290,282,"A:");
 mouseon();
 do{
  oldmousex=mousex;
  oldmousey=mousey;
  mousex=mouseinfo(1);
  mousey=mouseinfo(2);
  but1=mouseinfo(3);
  but2=mouseinfo(4);
  if (but1!=0){
   setfillstyle(1,7);
   bar(119,271,217,299);
   bar(244,271,342,299);
   if (mousex>=460 && mousey>=270 && mousex<=522 && mousey<=300) ok=2; else
   if (mousex>=345 && mousey>=273 && mousex<=440 && mousey<=300) ok=1; else
   if (mousex>=118 && mousey>=270 && mousex<=218 && mousey<=300 && rt==0) {
    if (from==0) from=1; else from=0;
   } else
   if (mousex>=243 && mousey>=270 && mousex<=343 && mousey<=300 && rt==0) {
    if (two==0) two=1; else two=0;
   }
   setcolor(15);
   mouseoff();
   if (from==0) outtextxy(162,282,"A:"); else outtextxy(162,282,"B:");
   if (two==0) outtextxy(290,282,"A:"); else outtextxy(290,282,"B:");
   mouseon();
  }
  if (but1!=0) rt++;
  if (rt>500) rt=0;
  if (but1==0) rt=0;
  if (mousex>622) mousex=622;
  if (mousey>468) mousey=468;
  if (but2!=0) ok=5;
 }while(ok==0);
 if (ok==1){
  if (from==0) strcpy(param,"A: "); else strcpy(param,"B: ");
  if (two==0) strcat(param,"A: "); else strcat(param,"B: ");
  strcat(param,"/V");
  execute("DISKCOPY.COM",param,0);

 }
 clearmenu();
 ma=0;
 am=0;
}

void leftclick(void)
{
 int dx,dy,err,ok;
 char str[5],execstring[255],filename[255],param[255],ch[3];

 strcpy(com,"");
 if (mousex>=0 && mousey>=11 && mousex<=61 && mousey<=22) ma=1; else
 if (mousex>=62 && mousey>=11 && mousex<=124 && mousey<=22) ma=2; else
 if (mousex>=125 && mousey>=11 && mousex<=181 && mousey<=22) ma=3; else
 if (ma==1) {
  if (mousex<0 || mousex>99 || mousey<23 || mousey>83){
   clearmenu();
   ma=0;
   am=0;
  } else {
   if (mousex>=0 && mousex<=99 && mousey>=23 && mousey<33){
    dwindow(100,240,540,310,"Execute");
    outtextxy(123,273,"Please enter you DOS command string");
    textrequest(120,283,43,com,192);
    strcpy(filename,"");
    strcpy(param,"");
    num=0;
    do{
     ch[0]=com[num];
     ch[1]=NULL;
     strcat(filename,ch);
     num++;
    }while(num<strlen(com) && ch[0]!=' ');
    printf("Filename %s\n",filename);
    if (num<strlen(com)){
     do{
      ch[0]=com[num];
      ch[1]=NULL;
      strcat(param,ch);
      num++;
     }while(num<strlen(com));
     printf("Parameters %s\n",param);
    }
    execute(filename,param,1);
   } else
   if (mousex>=0 && mousex<=99 && mousey>=33 && mousey<43){
    format();
   } else
   if (mousex>=0 && mousex<=99 && mousey>=43 && mousey<53){
    execute("C:\\COMMAND.COM","",0);
   } else
   if (mousex>=0 && mousex<=99 && mousey>=53 && mousey<63){
    copydisk();
   } else
   if (mousex>=0 && mousex<=99 && mousey>=73 && mousey<83){
    closegraph();
    exit(0);
   }
  }
 } else
 if (ma==2) {
  if (mousex<62 || mousex>162 || mousey<23 || mousey>53){
   clearmenu();
   ma=0;
   am=0;
  } else
  {
   if (mousex>=62 && mousex<=162 && mousey>=23 && mousey<33) createlink(0); else
   if (mousex>=62 && mousex<=162 && mousey>=33 && mousey<43) editlink(); else
   if (mousex>=62 && mousex<=162 && mousey>=43 && mousey<53) deletelink();
  }
 } else
 if (ma==3) {
  if (mousex<125 || mousex>225 || mousey<23 || mousey>43){
   clearmenu();
   ma=0;
   am=0;
  } else
  {
   if (mousex>=125 && mousex<=225 && mousey>=33 && mousey<43) {
    mouseoff();
    setfillstyle(1,7);
    bar(100,100,539,379);
    setcolor(15);
    line(100,100,539,100);
    line(101,101,538,101);
    line(100,100,100,379);
    line(101,101,101,111);
    outtextxy(103,103,"About");
    setcolor(8);
    line(539,101,539,379);
    line(539,379,101,379);
    line(538,102,538,111);
    line(538,111,102,111);
    setcolor(15);
    outtextxy(103,113,"DRACOS v1.0 Beta (C)Copyright 1989-1999 EdzUp");
    outtextxy(103,123,"---------------------------------------------");
    outtextxy(103,133,"Written by  :Ed Upton (EdzUp, Programming)");
    outtextxy(103,153,"Help from   :Rue Upton (Testing)");
    outtextxy(103,163,"             Rob 'its crashed again' (Testing)");
    outtextxy(103,183,"Greetings to:Wendy Hopkins");
    outtextxy(103,193,"             Jim Kyle (whos books are brilliant)");
    outtextxy(103,203,"             All at Databak Computer Services,Brighton");
    outtextxy(103,213,"             All my family and friends");
    outtextxy(103,233,"DISCLAIMER: EdzUp Software cannot be held responsible");
    outtextxy(103,243,"for any damage to any part of any computer system that");
    outtextxy(103,253,"arose from the use of this program. EdzUp Software has");
    outtextxy(103,263,"done their upmost to test this program for bugs but as");
    outtextxy(103,273,"we do not have access to every configuration possible");
    outtextxy(103,283,"we cannot guarantee that it will run perfectly on your");
    outtextxy(103,293,"machine. By running this software you accept the above");
    outtextxy(103,303,"terms.");
    settextstyle(0,0,0);
    outtextxy(282,343,"  Ok");
    rectangle(257,335,353,358);
    mouseon();
    do{
     ok=0;
     oldmousex=mousex;
     oldmousey=mousey;
     mousex=mouseinfo(1);
     mousey=mouseinfo(2);
     but1=mouseinfo(3);
     but2=mouseinfo(4);
     if (but1!=0){
      if (mousex>=257 && mousex<=353 && mousey>=335 && mousey<=358) ok=1;
     }
     if (mousex>622) mousex=622;
     if (mousey>468) mousey=468;
     //if (mousex!=oldmousex || mousey!=oldmousey) showcursor(mousex,mousey);
    }while(ok==0);
    clearmenu();
    //getimage(mousex+1,mousey+1,mousex+16,mousey+16,cursorback);
    ma=0;
    am=0;
   } else
   if (mousex>=125 && mousex<=225 && mousey>=23 && mousey<33) {
    strcpy(filename,maindir);
    strcat(filename,"\\");
    strcat(filename,"DHELP.EXE");
    execute(filename,"",0);
   };
  }
 } else
 if (mousex>=600 && mousey>=459 && mousex<=639 && mousey<=479 && curlink+35<maxlink){
  curlink+=35;
  clearmenu();
  ma=0;
  am=0;
 } else
 if (mousex>=600 && mousey>=24 && mousex<=639 && mousey<=43 && curlink>0){
  curlink-=35;
  if (curlink<0) curlink=0;
  clearmenu();
  ma=0;
  am=0;
 } else
 {
  num=0;
  do{
   err=0;
   if (mousex>=link[num].x && mousey>=link[num].y+23 && mousex<link[num].x+32 && mousey<link[num].y+55){
    if (strcmp(link[num].execute,"")!=0) {
     setcolor(15);
     rectangle(link[num].x-1,link[num].y+22,link[num].x+32,link[num].y+55);
    }
   } else err=1;
   if (err==1) num++;
  }while(err==1 && num<100);
  if (num<100) {
   if (strcmp(link[num].execute,"")!=0) execute(link[num].execute,link[num].param,0);
  }
 }
}

void rightclick(void)
{
}

void displaymenu(void)
{
 //putimage(mousex+1,mousey+1,cursorback,0);

 if (ma!=am) clearmenu();
 mouseoff();
 switch(ma){
 case 1 : { //Tools menu [Execute/Format/Shell/---/Exit]
	   setfillstyle(1,7);
	   bar(0,24,100,84);
	   setcolor(15);
	   rectangle(0,24,100,84);
	   outtextxy(2,26,"Execute");
	   outtextxy(2,36,"Format");
	   outtextxy(2,46,"Shell to DOS");
	   outtextxy(2,56,"Copy disk");
	   outtextxy(2,66,"------------");
	   outtextxy(2,76,"Exit");
	   setcolor(8);
	   line(1,84,100,84);
	   line(100,84,100,25);
	  }break;
 case 2 : { // Links menu [Create/Edit/Delete]
	   setfillstyle(1,7);
	   bar (63,24,163,53);
	   setcolor(15);
	   rectangle(63,24,163,54);
	   outtextxy(65,26,"Create link");
	   outtextxy(65,36,"Edit Link");
	   outtextxy(65,46,"Delete Link");
	   setcolor(8);
	   line(64,54,163,54);
	   line(163,54,163,25);
	  }break;
 case 3 : { // Help menu [Help/About]
	   setfillstyle(1,7);
	   bar(126,24,226,44);
	   setcolor(15);
	   rectangle(126,24,226,44);
	   outtextxy(128,26,"Help");
	   outtextxy(128,36,"About");
	   setcolor(8);
	   line(127,44,226,44);
	   line(226,44,226,25);
	  }break;
 }
 am=ma;
 titlebar(1);
 mouseon();
 //getimage(mousex+1,mousey+1,mousex+16,mousey+16,cursorback);
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
}

void keypress(void)
{
 char keys,modifier;

 keys=getch();
 switch(keys){
 case 27: exit(0);break;
 }
}

int error_window(int dii,int drive,int error,int rx,int ry)
{
 char info[80],chr[10];
 int x,y,ok;


 x=rx-100;
 y=ry;

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
 setfillstyle(1,0);
 bar(x,y,x+400,y+100);
 bar(mousex,mousey,mousex+16,mousey+16);
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

void centreclick(void)
{
 printf("CENTRE BUTTON\n");
}

void main(void)
{
 int ErrorCode;

 textmode(3);
 printf("DRACOS v1.0\n");
 printf("-----------\n");
 printf("Copyright (C)1999 EdzUp\nwritten by Ed Upton\n\n");
 mousetype=resetmouse();
 switch(mousetype){
 case 0 : printf("Single");break;
 case 2 : printf("Two");break;
 case 3 : printf("Three");break;
 case 4 : printf("Unknown mouse detected\n");break;
 }
 if (mousetype<4) printf(" button mouse detected\n");
 harderr(handler);
 s=getenv("DRACOS");
 strcpy(maindir,s);
 strcpy(icondir,s);
 strcat(icondir,"\\ICONS\\");
 strcpy(linkdir,s);
 strcat(linkdir,"\\LINKS\\");
 readlinks();
 clrscr();
 setvid();
 Gd=9;
 Gm=2;
 initgraph(&Gd,&Gm,"");
 num=graphresult();
 if (num!=grOk){
  printf("Graphics error: %s\n", grapherrormsg(num));
  printf("Press any key to halt:");
  getch();
  exit(1); /* terminate with an error code */
 }
 loadpalette();
 cleardevice();
 titlebar(0);
 /* This line displays the Enviroment variable for DRACOS,
    this allows users to install DRACOS to any directory on any drive.*/
 // printf("DRACOS Enviroment varible : '%s'\n",s);
 desktoplink(0);
 lt=0;
 rt=0;
 ma=0;
 am=0;
 curlink=0;
 mouseon();
 do{
  oldmousex=mousex;
  oldmousey=mousey;
  mousex=mouseinfo(1);
  mousey=mouseinfo(2);
  if (kbhit()!=0) keypress();
  but1=mouseinfo(3);
  but2=mouseinfo(4);
  but3=mouseinfo(5);
  if (but1!=0 || but2!=0 || but3!=0) {
   if (but1!=0 && but2!=0){
   }
   if (but1!=0) leftclick();
   if (but2!=0) rightclick();
   if (but3!=0) centreclick();
  }
  if (but1!=0) lt++; else lt=0;
  if (but2!=0) rt++; else rt=0;
  if (mousex>622) mousex=622;
  if (mousey>468) mousey=468;
  if (ma!=0 && am!=ma) displaymenu();
  if (ma!=0 && am!=ma) clearmenu();
  //if (mousex!=oldmousex || mousey!=oldmousey) showcursor(mousex,mousey);
 }while(mousex<700);
}