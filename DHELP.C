#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <dir.h>
#include <graphics.h>
#include <string.h>

struct {
char info[50],filename[13];
} filereg[100];

struct {      // information contained within the help file
char fline[85];
} fileinfo[400];

struct REGPACK in; /*registers structure*/

int cursor[16][16]={15,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		    15,15,15,15,0,0,0,0,0,0,0,0,0,0,0,0,
		    0,15,15,15,15,15,0,0,0,0,0,0,0,0,0,0,
		    0,15,15,15,15,15,15,15,0,0,0,0,0,0,0,0,
		    0,0,15,15,15,15,15,15,15,15,0,0,0,0,0,0,
		    0,0,15,15,15,15,15,0,0,0,0,0,0,0,0,0,
		    0,0,0,15,15,15,15,15,0,0,0,0,0,0,0,0,
		    0,0,0,15,15,0,15,15,15,0,0,0,0,0,0,0,
		    0,0,0,0,15,0,0,15,15,15,0,0,0,0,0,0,
		    0,0,0,0,15,0,0,0,15,15,15,0,0,0,0,0,
		    0,0,0,0,0,0,0,0,0,15,15,15,0,0,0,0,
		    0,0,0,0,0,0,0,0,0,0,15,15,15,0,0,0,
		    0,0,0,0,0,0,0,0,0,0,0,15,15,0,0,0,
		    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

int mousex,mousey,ma,am,num,Gd,Gm,lt,rt;
char maindir[192],helpdir[192],*s,com[255],ch;
int cursorx,cursory,but1,but2,oldmousex,oldmousey,curlink,maxlink;
int cursorback[16][16]; /*background behind cursor, replaced when cursor moves*/
FILE *hand;

void setvid(void)
{
 registerbgidriver(EGAVGA_driver);
}

int mouseinfo(int rd)
{
 in.r_ax = 3;
 intr(0x33, &in);
 switch(rd){
 case 1 : return(in.r_cx);
 case 2 : return(in.r_dx);
 case 3 : return(in.r_bx & 1);
 case 4 : return(in.r_bx & 2);
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

void showcursor(int x,int y)
{
 int ccx,ccy;
 int xpos,ypos;

 putimage(oldmousex+1,oldmousey+1,cursorback,0);
 getimage(x+1,y+1,x+16,y+16,cursorback);
 for (ccy=0;ccy<16;ccy++){
  for (ccx=0;ccx<16;ccx++){
   if (cursor[ccx][ccy]!=0 && x<623) putpixel(ccx+x+1,ccy+y+1,cursor[ccx][ccy]);
  }
 }
}

void morebutton(int x,int y)
{
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
}

void quit(void)
{
 closegraph();
 exit(0);
}

void titlebar(void)
{
 setcolor(15);
 setfillstyle(1,7);
 bar(0,0,640,11);
 rectangle(0,0,640,11);
 outtextxy(3,2,"DRACOS Help (C)Copyright 1998-1999 EdzUp");
 setcolor(8);
 line(1,11,640,11);
 line(640,11,640,1);
}

void clearmenu(void)
{
}

void selectinfo(void)
{
 int slink,cl,nn,sl,ok,rt;
 char chr[3],filename[255];

 dwindow(120,120,440,370,"Select Help");
 outtextxy(123,135,"Select the help you require");
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
 cl=0;
 settextstyle(0,0,0);
 outtextxy(257,343,"Cancel");
 rectangle(232,335,327,358);
 for (nn=cl;nn<cl+16;nn++){
  outtextxy(125,167+((nn-cl)*9),filereg[nn].info);
 }
 rt=0;
 ok=0;
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
    if (cl<=0) cl=0;
    for (nn=cl;nn<cl+16;nn++){
     outtextxy(125,167+((nn-cl)*9),filereg[nn].info);
    }
   } else
   if (mousex>=125 && mousey>=312 && mousex<=435 && mousey<=325) {
    setfillstyle(1,0);
    bar(123,165,437,310);
    cl++;
    if (cl>=134) cl=134;
    for (nn=cl;nn<cl+16;nn++){
     outtextxy(125,167+((nn-cl)*9),filereg[nn].info);
    }
   }
  }
  if (but1!=0) rt++;
  if (rt>100) rt=0;
  if (mousex>622) mousex=622;
  if (mousey>468) mousey=468;
  if (mousex!=oldmousex || mousey!=oldmousey) showcursor(mousex,mousey);
 }while(ok==0);
 exit(0);
 if (ok==1) {
  //sl;
 }
 clearmenu();
 ma=0;
 am=0;
 getimage(mousex+1,mousey+1,mousex+16,mousey+16,cursorback);
}

void keypress(void)
{
 ch=getch();
 switch(ch){
 case 27 : exit(0);break;
 }
}

void readinfofile(void)
{
 for (num=0;num<100;num++){
  strcpy(filereg[num].info,"");
  strcpy(filereg[num].filename,"");
 }
 strcpy(filereg[0].info,"About DRACOS");
 strcpy(filereg[0].filename,"");
 strcpy(filereg[1].info,"About EdzUp Software");
 strcpy(filereg[1].filename,"");
 strcpy(filereg[2].info,"DISCLAIMER");
 strcpy(filereg[2].filename,"");
 strcpy(filereg[3].info,"Licence Agreement");
 strcpy(filereg[3].filename,"");

}

void main(void)
{
 printf("DRACOS DHELP\n");
 printf("(C)Copyright 1999 EdzUp\n\n");
 s=getenv("DRACOS");
 getcwd(com,192);
 num=chdir(s);
 if (num!=0) {
  printf("Fatal error encountered\n\n");
  printf("Please check AUTOEXEC.BAT for the following line:\n");
  printf("SET DRACOS=<DRACOS INSTALLED DIRECTORY>\n\n");
  printf("For example if DRACOS is installed in C:\DRACOS it would read as follows;\n");
  printf("SET DRACOS=C:\DRACOS\n\n");
  printf("If the line is incorrect or missing please replace the line and reboot\n");
  printf("the machine.\n\n");
  exit(1);
 }
 printf("Boot successful, DRACOS main directory located and confirmed\n\n");
 strcpy(maindir,s);
 strcpy(helpdir,maindir);
 strcat(helpdir,"\\HELP");
 readinfofile();
 clrscr();
 setvid();
 Gd=9;
 Gm=2;
 initgraph(&Gd,&Gm,"");
 titlebar();
 lt=0;
 rt=0;
 ma=0;
 am=0;
 curlink=0;
 do{
  selectinfo();
  oldmousex=mousex;
  oldmousey=mousey;
  mousex=mouseinfo(1);
  mousey=mouseinfo(2);
  but1=mouseinfo(3);
  but2=mouseinfo(4);
  if (kbhit()!=0) keypress();
  if (but1!=0) lt++; else lt=0;
  if (but2!=0) rt++; else rt=0;
  if (mousex<0) mousex=0;
  if (mousey<0) mousey=0;
  if (mousex>622) mousex=622;
  if (mousey>468) mousey=468;
  if (mousex!=oldmousex || mousey!=oldmousey) showcursor(mousex,mousey);
 }while(mousex<700);
}