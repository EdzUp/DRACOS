#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <graphics.h>
#include <dos.h>
#include <math.h>
#include <float.h>
#include <string.h>

struct REGPACK in; /*registers structure*/

struct wn { // WINDOWS STRUCTURE FOR 200 WINDOWS
 int tx,ty,bx,by,attr;
 char header[50];
} win[200];

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

int Gd,Gm,awin;
long avail,total;
float percent,pfree;
int mousex,mousey,but1,but2,oldmousex,oldmousey;
int cursorback[16][16]; /*background behind cursor, replaced when cursor moves*/

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


void diskfree(int drive,long avail,long total)
{
 struct dfree fatinf;

 getdfree(drive,&fatinf);
 avail=(long)fatinf.df_avail*(long)fatinf.df_bsec*(long)fatinf.df_sclus;
 total=(long)fatinf.df_total*(long)fatinf.df_bsec*(long)fatinf.df_sclus;
 percent=total/100;
 if (avail>0) pfree=avail/percent; else pfree=0;
}

void dwin(int swin)
{
 char atr[8],chr[3];
 int headx,heade,footx,footy,textx,tpos;

 /* 1 Close gadget
    2 Minimise gadget
    4 Send to back gadget
    8 Resize Gadget
   16 Vertical Scrollbar
   32 Horizontal Scrollbar
   64 Diskspace gadget
  128 Window is always on top (no matter what) */

 headx=win[swin].tx+2;
 heade=win[swin].bx;
 footx=win[swin].bx;
 footy=win[swin].by;

 strcpy(atr,"--------");
 setfillstyle(1,7);
 if (win[swin].attr-128>0) {
  win[swin].attr-=128;
  atr[7]='+';
 }
 if (win[swin].attr-64>0) {
  win[swin].attr-=64;
  atr[6]='+';
 }
 if (win[swin].attr-32>0) {
  win[swin].attr-=32;
  atr[5]='+';
 }
 if (win[swin].attr-16>0) {
  win[swin].attr-=16;
  atr[4]='+';
 }
 if (win[swin].attr-8>0) {
  win[swin].attr-=8;
  atr[3]='+';
 }
 if (win[swin].attr-4>0) {
  win[swin].attr-=4;
  atr[2]='+';
  heade-=16;
 }
 if (win[swin].attr-2>0) {
  win[swin].attr-=2;
  atr[1]='+';
  heade-=16;
 }
 if (win[swin].attr-1>0) {
  win[swin].attr-=1;
  atr[0]='+';
 }
 rectangle(win[swin].tx,win[swin].ty,win[swin].bx,win[swin].by);
 if (atr[0]=='+'){
  rectangle(win[swin].tx,win[swin].ty,win[swin].tx+15,win[swin].ty+15);
  setcolor(8);
  bar(win[swin].tx+1,win[swin].ty+1,win[swin].tx+14,win[swin].ty+14);
  rectangle(win[swin].tx+6,win[swin].ty+4,win[swin].tx+10,win[swin].ty+12);
  line(win[swin].tx+15,win[swin].ty+1,win[swin].tx+15,win[swin].ty+15);
  line(win[swin].tx+1,win[swin].ty+15,win[swin].tx+15,win[swin].ty+15);
  headx+=17;
 }
 settextstyle(0,0,1);
 setcolor(15);
 bar(headx-3,win[swin].ty+1,win[swin].bx-1,win[swin].ty+15);
 tpos=0;
 textx=headx;
 do{
  chr[0]=win[swin].header[tpos];
  chr[1]=NULL;
  outtextxy(textx,win[swin].ty+4,chr);
  textx+=8;
  tpos++;
 }while(textx<heade-20 && tpos<strlen(win[swin].header));
 if (tpos<strlen(win[swin].header)) outtextxy(textx,win[swin].ty+4,"..");
 setcolor(8);
 line(headx-3,win[swin].ty+15,heade,win[swin].ty+15);
 line(heade,win[swin].ty+1,heade,win[swin].ty+15);
 if (atr[1]=='+'){
  rectangle(heade,win[swin].ty,heade+15,win[swin].ty+15);
  rectangle(heade+2,win[swin].ty+2,heade+13,win[swin].ty+13);
  rectangle(heade+2,win[swin].ty+2,heade+7,win[swin].ty+7);
  setfillstyle(1,15);
  bar(heade+3,win[swin].ty+3,heade+6,win[swin].ty+6);
 }
 if (atr[2]=='+'){
  setfillstyle(1,15);
  bar(win[swin].bx-12,win[swin].ty+6,win[swin].bx-2,win[swin].ty+13);
  setcolor(8);
  rectangle(win[swin].bx-15,win[swin].ty+3,win[swin].bx-4,win[swin].ty+11);
  setfillstyle(1,8);
  bar(win[swin].bx-14,win[swin].ty+4,win[swin].bx-5,win[swin].ty+10);
  line(win[swin].bx-16,win[swin].ty+15,win[swin].bx,win[swin].ty+15);
 }
 if (atr[3]=='+'){
  footx-=17;
  footy-=17;
  setcolor(8);
  rectangle(footx,footy,win[swin].bx,win[swin].by);
  setfillstyle(1,7);
  bar(footx+1,footy+1,win[swin].bx-1,win[swin].by-1);
  setcolor(8);
  line(win[swin].bx-2,win[swin].by-2,footx+2,win[swin].by-2);
  line(win[swin].bx-2,win[swin].by-2,win[swin].bx-2,footy+2);
  line(win[swin].bx-2,footy+2,footx+2,win[swin].by-2);
 } else
 {
  footx-=17;
  footy-=17;
  setcolor(8);
  rectangle(footx,footy,win[swin].bx,win[swin].by);
  setfillstyle(1,7);
  bar(footx+1,footy+1,win[swin].bx-1,win[swin].by-1);
 }
 if (atr[4]=='+'){
  setcolor(8);
  rectangle(footx,win[swin].ty+16,win[swin].bx,footy);
  setfillstyle(1,7);
  bar(footx+1,win[swin].ty+16,win[swin].bx-1,footy-1);
  line(footx,footy-17,win[swin].bx,footy-17);
  line(footx,footy-34,win[swin].bx,footy-34);
  chr[0]=31;
  chr[1]=NULL;
  setcolor(15);
  settextstyle(0,0,2);
  outtextxy(footx+2,footy-15,chr);
  chr[0]=30;
  chr[1]=NULL;
  outtextxy(footx+2,footy-32,chr);
 }
 if (atr[5]=='+'){
  setcolor(8);
  rectangle(win[swin].tx,footy,footx,win[swin].by);
  setfillstyle(1,7);
  bar(win[swin].tx+1,footy+1,footx-1,win[swin].by-1);
  line(footx-17,footy,footx-17,win[swin].by);
  line(footx-34,footy,footx-34,win[swin].by);
  chr[0]=16;
  chr[1]=NULL;
  setcolor(15);
  settextstyle(0,0,2);
  outtextxy(footx-15,footy+2,chr);
  chr[0]=17;
  chr[1]=NULL;
  outtextxy(footx-32,footy+2,chr);

 }
 if (atr[6]=='+'){
  diskfree(3,avail,total);
  setfillstyle(1,7);
  bar(win[swin].tx+1,win[swin].ty+16,win[swin].tx+14,footy-1);
  settextstyle(0,0,1);
  setcolor(15);
  outtextxy(win[swin].tx+4,win[swin].ty+19,"F");
  outtextxy(win[swin].tx+4,footy-10,"E");
  setcolor(8);
  rectangle(win[swin].tx,win[swin].ty+15,win[swin].tx+15,footy);
  setfillstyle(1,4);
  bar(win[swin].tx+3,win[swin].ty+29,win[swin].tx+12,footy-14);
  setfillstyle(1,2);
  percent=((win[swin].by-31)-(win[swin].ty+29));
  percent=percent/100;
  bar(win[swin].tx+3,(footy-14),win[swin].tx+12,(footy-14)-percent*pfree);
 }
 setfillstyle(1,7);
 setcolor(15);
 rectangle(win[swin].tx,win[swin].ty,win[swin].bx,win[swin].by);
 setcolor(8);
 line(win[swin].bx,win[swin].ty+1,win[swin].bx,win[swin].by);
 line(win[swin].tx+1,win[swin].by,win[swin].bx,win[swin].by);
}

void quit(void)
{
 closegraph();
 exit(0);
}

void leftclick(void)
{
 int dx,dy;
 char str[5];

 if (mousex>win[awin].tx+1 && mousey>win[awin].ty+1 && mousex<win[awin].tx+14 && mousey<win[awin].ty+14){
  quit();
 }
}

void rightclick(void)
{
}

void keypress(void)
{
 char ch;

 ch=getch();
 switch(ch){
 case 45 : quit();break;
 }
}

void testcol(void)
{
 int tsx,tsp;

 textmode(19);
 tsx=0;
 for (tsp=0;tsp<256;tsp++){
  setcolor(tsp);
  line(tsx,0,tsx,320);
  tsx++;
 }
 exit(0);
}

void main(void)
{
 clrscr();
 setvid();
 Gd=9;
 Gm=2;
 initgraph(&Gd,&Gm,"");
 win[0].tx=100;
 win[0].ty=100;
 win[0].bx=200;
 win[0].by=375;
 strcpy(win[0].header,"DRACOS window 1");
 win[0].attr=128;
 awin=0;
 dwin(0);
 do{
  oldmousex=mousex;
  oldmousey=mousey;
  mousex=mouseinfo(1);
  mousey=mouseinfo(2);
  but1=mouseinfo(3);
  but2=mouseinfo(4);
  if (but1!=0 || but2!=0) {
   if (but1!=0 && but2!=0){
   }
   if (but1!=0) leftclick();
   if (but2!=0) rightclick();
  }
  if (mousex>622) mousex=622;
  if (mousey>468) mousey=468;
  if (mousex!=oldmousex || mousey!=oldmousey) showcursor(mousex,mousey);
 }while(mousex<700);

}