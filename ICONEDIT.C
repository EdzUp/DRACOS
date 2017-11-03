#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <dir.h>
#include <io.h>
#include <fcntl.h>
#include <graphics.h>

struct ffblk dki;

/* IconEdit DEFINITIONS FOR SCREEN DISPLAY*/
int maxx,maxy,img;
int icon[100][100][2];
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



char dirs[192],*s,maindir[255],icondir[255];
int Gd,Gm,col,col2,lclick,rclick,oldl,oldr,menu,itype,oldxsize,oldysize;
int cursorback[16][16]; /*background behind cursor, replaced when cursor moves*/
int num,pi,byte,xsize,ysize,xbx,xby,itype;
int mousex,mousey,but1,but2,oldmousex,oldmousey,dx,dy;
char ch,name[8];

int mouseinfo(int rd)
{
 struct REGPACK in;

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

void setmodes(void)
{
 int err,x,y;

 Gd=9; /*9*/
 Gm=2; /*2*/
 registerbgidriver(EGAVGA_driver);
 initgraph(&Gd,&Gm,"");
 maxx=getmaxx();
 maxy=getmaxy();
}

void dracosbackground(void)
{
 int image[1024];
 int xx,yy;

 getimage(0,0,31,31,&image);
 for (yy=0;yy<480;yy+=32){
  for (xx=0;xx<640;xx+=32){
   putimage(xx,yy,&image,0);
  }
 }
 setfillstyle(1,7);
 setcolor(15);
 rectangle(0,0,maxx,11);
 bar(1,1,maxx-1,10);
 setcolor(8);
 line(1,11,maxx,11);
 line(maxx,1,maxx,11);
 setcolor(15);
 outtextxy(2,2,"DRACOS IconEdit Copyright (C)1998 EdzUp Software");
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
 outtextxy(2,2,"DRACOS IconEdit Copyright (C)1998 EdzUp Software");

}

void palettesetup(void)
{
 int cc;

 for (cc=0;cc<16;cc++){
  setcolor(15);
  rectangle((cc*16)+10,440,(cc*16)+26,456);
  setfillstyle(1,cc);
  bar((cc*16)+11,441,(cc*16)+25,455);
 }
}

void imagedisplay(int xs,int ys)
{
 int xd,yd,xp,yp,ix,iy;

 setcolor(15);
 rectangle(298,28,623,353);
 rectangle(299,29,622,352);
 setcolor(8);
 line(299,352,622,352);
 line(298,353,623,353);
 line(623,353,623,28);
 line(622,29,622,352);
 setcolor(7);
 xd=320/xs;
 yd=320/ys;
 ix=0;
 iy=0;
 for (yp=30;yp<350;yp+=yd){
  for (xp=300;xp<620;xp+=xd){
   if (ix<xsize && iy<ysize){
    rectangle(xp,yp,xp+xd,yp+yd);
   }
   ix++;
  }
  iy++;
  ix=0;
 }
 rectangle(105,302,106+xs,303+ys);
}

void displayimage(void)
{
 int xd,yd,xp,yp,ix,iy;

 setfillstyle(1,0);
 bar(298,28,640,360);
 bar(105,302,206,403);
 setcolor(15);
 rectangle(298,28,623,353);
 rectangle(299,29,622,352);
 setcolor(8);
 line(299,352,622,352);
 line(298,353,623,353);
 line(623,353,623,28);
 line(622,29,622,352);
 setcolor(7);
 xd=320/xsize;
 yd=320/ysize;
 ix=0;
 iy=0;
 for (yp=30;yp<350;yp+=yd){
  for (xp=300;xp<620;xp+=xd){
   if (ix<xsize && iy<ysize){
    setfillstyle(1,icon[ix][iy][img]);
    bar(xp,yp,xp+xd,yp+yd);
    rectangle(xp,yp,xp+xd,yp+yd);
    putpixel(106+ix,303+iy,icon[ix][iy][img]);
   }
   ix++;
  }
  ix=0;
  iy++;
 }
 rectangle(105,302,106+xsize,303+ysize);
}

void dracostitle(int x,int y)
{
 settextstyle(0,0,2);
 setcolor(15);
 rectangle(x,y,x+200,y+100);
 rectangle(x+1,y+1,x+199,y+99);
 setfillstyle(1,7);
 bar(x+2,y+2,x+198,y+98);
 setcolor(8);
 outtextxy(x+10,y+10,"  IconEdit");
 line(x+1,y+100,x+200,y+100);
 line(x+200,y+1,x+200,y+100);
 line(x+2,y+99,x+199,y+99);
 line(x+199,y+2,x+199,y+99);
 setcolor(7);
 outtextxy(x+9,y+9,"  IconEdit");
 setcolor(15);
 outtextxy(x+8,y+8,"  IconEdit");
 settextstyle(0,0,1);
 outtextxy(x+9,y+40,"Copyright (C)1998 EdzUp");
 outtextxy(x+16,y+60,"Written by : Ed Upton");
 outtextxy(x+48,y+80,"Press any key");
 setfillstyle(1,0);
 getch();
 bar(0,12,maxx,maxy);
}

void showcursor(int x,int y)
{
 int ccx,ccy;
 int xpos,ypos;

 putimage(oldmousex+1,oldmousey+1,cursorback,0);
 if (mousex>299 && mousey>29 && mousex<622 && mousey<352){
  xpos=(mousex-299)/xbx;
  ypos=(mousey-29)/xby;
  gotoxy(40,25);
  printf("Xpos:%i  Ypos:%i  X:%i,Y:%i  ",xpos,ypos,xsize,ysize);
  if (xpos<xsize && ypos<ysize){
   setfillstyle(1,icon[xpos][ypos][img]);
   bar((xpos*xbx)+300,(ypos*xby)+30,((xpos*xbx)+300)+xbx,((ypos*xby)+30)+xby);
   setcolor(7);
   rectangle((xpos*xbx)+300,(ypos*xby)+30,((xpos*xbx)+300)+xbx,((ypos*xby)+30)+xby);
  }
 }
 getimage(x+1,y+1,x+16,y+16,cursorback);
 for (ccy=0;ccy<16;ccy++){
  for (ccx=0;ccx<16;ccx++){
   if (cursor[ccx][ccy]!=0 && x<623) putpixel(ccx+x+1,ccy+y+1,cursor[ccx][ccy]);
  }
 }
}

void easybuttons(void)
{
 setfillstyle(1,7);
 setcolor(15);
 bar(60,94,250,112);
 rectangle(60,94,250,112);
 bar(60,114,250,132);
 rectangle(60,114,250,132);
 bar(60,134,250,152);
 rectangle(60,134,250,152);
 bar(60,154,250,172);
 rectangle(60,154,250,172);
 bar(60,174,250,192);
 rectangle(60,174,250,192);
 bar(60,194,250,212);
 rectangle(60,194,250,212);
 bar(60,214,250,232);
 rectangle(60,214,250,232);
 bar(60,234,250,252);
 rectangle(60,234,250,252);
 bar(60,274,250,292);
 rectangle(60,274,250,292);
 outtextxy(86,100,"    Image Type");
 outtextxy(90,120,"   Other Image   ");
 setcolor(8);
 outtextxy(90,140,"      WIDTH      ");
 outtextxy(87,160,"      HEIGHT     ");
 setcolor(15);
 outtextxy(90,180,"       New");
 outtextxy(86,200,"       Load");
 outtextxy(86,220,"       Save");
 outtextxy(82,240,"       About");
 outtextxy(86,280,"       X-it");
 setcolor(14);
 outtextxy(86,100,"    I");
 outtextxy(90,120,"   O");
 setcolor(6);
 outtextxy(90,140,"      W");
 outtextxy(87,160,"      H");
 setcolor(14);
 outtextxy(90,180,"       N");
 outtextxy(86,200,"       L");
 outtextxy(86,220,"       S");
 outtextxy(82,240,"       A");
 outtextxy(86,280,"       X");

}

void about(void)
{
 setfillstyle(1,7);
 settextstyle(0,0,2);
 setcolor(15);
 bar(60,94,250,292);
 rectangle(60,94,250,292);
 rectangle(61,95,249,291);
 setcolor(8);
 outtextxy(94,108,"ICONEDIT");
 setcolor(7);
 outtextxy(92,106,"ICONEDIT");
 setcolor(15);
 outtextxy(90,104,"ICONEDIT");
 settextstyle(0,0,0);
 outtextxy(64,130,"Copyright (C)1998 EdzUp");
 outtextxy(64,150,"         ABOUT         ");
 outtextxy(64,160," Written by : Ed Upton ");
 outtextxy(64,170," Idea by    : Ed Upton ");
 outtextxy(64,190,"This program is part of");
 outtextxy(64,200,"the  -DRACOS- Operating");
 outtextxy(64,210,"Shell, and is not to be");
 outtextxy(64,220,"sold seperatly  without");
 outtextxy(64,230,"permission.");
 outtextxy(64,250,"E-Mail:");
 outtextxy(64,260,"    dracosed@yahoo.com ");
 outtextxy(64,281,"Press SPACE to continue");
 do{
  ch=getch();
 }while(ch!=32);
 setfillstyle(1,0);
 bar(60,94,250,292);
 easybuttons();
}

int security(void)
{
 int sec;
 char ch;

 sec=0;
 if (itype<0 || itype>2) sec=1;
 if (xsize<16 || xsize>32) sec=1;
 if (ysize<16 || ysize>32) sec=1;
 gotoxy(40,25);
 if (sec==1) {
  printf("Error loading <SPACE>");
  do{
   ch=getch();
  }while(ch!=32);
 }
 return(sec);
}

void loadcursor(char cn[13])
{
 char ch;
 int fxsize,fysize,ftype,ok;
 FILE *hand;

 hand=fopen(cn,"rb");
 if (hand!=NULL){
  fread(&itype,2,2,hand);
  fread(&xsize,2,2,hand);
  fread(&ysize,2,2,hand);
  for (dy=0;dy<ysize;dy++){
   for (dx=0;dx<xsize;dx++){
    fread(&icon[dx][dy][0],2,2,hand);
    fread(&icon[dx][dy][1],2,2,hand);
   }
  }
  if (itype==1) {
   for (dy=0;dy<16;dy++){
    for (dx=0;dx<16;dx++){
     cursor[dx][dy]=icon[dx][dy][0];
    }
   }
  }
 }
 for (dy=0;dy<100;dy++){
  for (dx=0;dx<100;dx++){
   icon[dx][dy][0]=0;
   icon[dx][dy][1]=0;
  }
 }
}

void load(void)
{
 char fname[8],ch,icond[255];
 int fxsize,fysize,ftype,ok;
 FILE *hand;

 gotoxy(40,24);
 printf("LOAD:                              ");
 gotoxy(40,25);
 printf("                                   ");
 gotoxy(45,24);
 gets(name);
 strcpy(fname,name);
 switch(itype){
 case 0 : strcat(fname,".DIS");break;
 case 1 : strcat(fname,".DPS");break;
 case 2 : strcat(fname,".DWS");break;
 }
 strcpy(icond,icondir);
 strcat(icond,fname);
 hand=fopen(icond,"rb");
 if (hand!=NULL){
  fread(&itype,2,2,hand);
  fread(&xsize,2,1,hand);
  fread(&ysize,2,1,hand);
  for (dy=0;dy<ysize;dy++){
   for (dx=0;dx<xsize;dx++){
    fread(&icon[dx][dy][1],2,2,hand);
    fread(&icon[dx][dy][0],2,2,hand);
   }
  }
  displayimage();
  xbx=(622-299)/xsize;
  xby=(352-29)/ysize;
 } else {
  gotoxy(40,25);
  printf("Error loading file '%s'. <SPACE>",fname);
  do{
   ch=getch();
  }while(ch!=32);
 }
}

void save(void)
{
 char fname[192],ch;
 int fxsize,fysize,ftype;
 FILE *hand;

 gotoxy(40,24);
 printf("SAVE:                              ");
 gotoxy(40,25);
 printf("                                   ");
 gotoxy(45,24);
 gets(name);
 strcpy(fname,name);
 switch(itype){
 case 0 : strcat(fname,".DIS");break;
 case 1 : strcat(fname,".DPS");break;
 case 2 : strcat(fname,".DWS");break;
 }

 hand=fopen(fname,"wb");
 if (hand!=NULL){
  fwrite(&itype,2,2,hand);
  fwrite(&xsize,2,2,hand);
  fwrite(&ysize,2,2,hand);
  for (dy=0;dy<xsize;dy++){
   for (dx=0;dx<ysize;dx++){
    fwrite(&icon[dx][dy][0],2,2,hand);
    fwrite(&icon[dx][dy][1],2,2,hand);
   }
  }
 } else {
  gotoxy(40,25);
  printf("Error saving file '%s'. <SPACE>",fname);
  do{
   ch=getch();
  }while(ch!=32);
 }
}

void quit(void)
{
  gotoxy(40,24);
  printf("QUIT : Are you sure (Y/N):         ");
  gotoxy(40,25);
  printf("                                   ");
  do{
   ch=getch();
   ch=toupper(ch);
  }while(ch!=78 && ch!=89);
  if (ch==89) {
   closegraph();
   printf("ICONEDIT: Thank you for using IconEdit...\n\n");
   exit(0);
  }
}

void imagetype(void)
{
  itype++;
  if (itype>2) itype=0;
  switch(itype){
  case 0 : xsize=32;
	   ysize=32;break;
  case 1 : xsize=16;
	   ysize=16;break;
  case 2 : xsize=32;
	   ysize=32;break;
  }
  if (itype!=0) img=0;
  xbx=(622-299)/xsize;
  xby=(352-29)/ysize;
  displayimage();
}

void other(void)
{
  if (img==0) img=1; else img=0;
  displayimage();
}

void newicon(void)
{
  gotoxy(40,24);
  printf("NEW : Are you sure (Y/N):          ");
  gotoxy(40,25);
  printf("                                   ");
  do{
   ch=getch();
   ch=toupper(ch);
  }while(ch!=78 && ch!=89);
  if (ch==89) {
   for (dy=0;dy<100;dy++){
    for (dx=0;dx<100;dx++){
     icon[dx][dy][0]=0;
     icon[dx][dy][1]=0;
    }
   }
   displayimage();
  }
}

void width(void)
{
 char str[5];

  gotoxy(40,24);
  printf("WIDTH  :                           ");
  gotoxy(40,25);
  printf("                                   ");
  gotoxy(49,24);
  gets(str);
  xsize=atoi(str);
  if (xsize<16 || xsize>32) xsize=oldxsize;
  if (itype==0 || itype==2) xsize=32; else xsize=16;
  oldxsize=xsize;
  displayimage();
  xbx=(622-299)/xsize;
}

void height(void)
{
 char str[5];

  gotoxy(40,24);
  printf("HEIGHT :                           ");
  gotoxy(40,25);
  printf("                                   ");
  gotoxy(49,24);
  gets(str);
  ysize=atoi(str);
  if (ysize<16 || ysize>32) ysize=oldysize;
  if (itype==0 || itype==2) xsize=32; else xsize=16;
  oldysize=ysize;
  displayimage();
  xby=(352-29)/ysize;
}

void copyicon(void)
{
 int f,t;
 if (img==0) {
  f=1;
  t=0;
 } else {
  f=0;
  t=1;
 }
 for (dy=0;dy<ysize;dy++){
  for (dx=0;dx<xsize;dx++){
   icon[dx][dy][t]=icon[dx][dy][f];
  }
 }
 displayimage();
}

void leftclick(void)
{
 int dx,dy;
 char str[5];
 if (mousex>=349 && mousey>=440 && mousex<=391 && mousey<=456) {
  copyicon();
 } else
 if (mousex>=305 && mousey>=440 && mousex<=347 && mousey<=456) {
  for (dy=0;dy<ysize;dy++){
   for (dx=0;dx<xsize;dx++){
    if (icon[dx][dy][img]==col2) icon[dx][dy][img]=col;
   }
  }
  displayimage();
 } else
 if (mousex>=10 && mousey>=440 && mousex<=25 && mousey<456) col=0; else
 if (mousex>=26 && mousey>=440 && mousex<=42 && mousey<456) col=1; else
 if (mousex>=43 && mousey>=440 && mousex<=58 && mousey<456) col=2; else
 if (mousex>=59 && mousey>=440 && mousex<=74 && mousey<456) col=3; else
 if (mousex>=75 && mousey>=440 && mousex<=90 && mousey<456) col=4; else
 if (mousex>=91 && mousey>=440 && mousex<=106 && mousey<456) col=5; else
 if (mousex>=107 && mousey>=440 && mousex<=122 && mousey<456) col=6; else
 if (mousex>=123 && mousey>=440 && mousex<=138 && mousey<456) col=7; else
 if (mousex>=139 && mousey>=440 && mousex<=154 && mousey<456) col=8; else
 if (mousex>=155 && mousey>=440 && mousex<=170 && mousey<456) col=9; else
 if (mousex>=171 && mousey>=440 && mousex<=186 && mousey<456) col=10; else
 if (mousex>=187 && mousey>=440 && mousex<=202 && mousey<456) col=11; else
 if (mousex>=203 && mousey>=440 && mousex<=218 && mousey<456) col=12; else
 if (mousex>=219 && mousey>=440 && mousex<=234 && mousey<456) col=13; else
 if (mousex>=235 && mousey>=440 && mousex<=250 && mousey<456) col=14; else
 if (mousex>=251 && mousey>=440 && mousex<=266 && mousey<456) col=15; else
 if (mousex>=60 && mousey>=94 && mousex<=250 && mousey<=112){
  imagetype();
 } else
 if (mousex>=60 && mousey>=114 && mousex<=250 && mousey<=132 && itype==0) {
  other();
 } else
 if (mousex>=60 && mousey>=134 && mousex<=250 && mousey<=152 && itype==0) {
  if (itype==3) width(); //itype 0 is icons for 32-100 size icons
 } else
 if (mousex>=60 && mousey>=154 && mousex<=250 && mousey<=172 && itype==0) {
  if (itype==3) height();
 } else
 if (mousex>=60 && mousey>=174 && mousex<=250 && mousey<=192){
  newicon();
 } else
 if (mousex>=60 && mousey>=194 && mousex<=250 && mousey<=212){
  load();
 } else
 if (mousex>=60 && mousey>=214 && mousex<=250 && mousey<=232){
  save();
 } else
 if (mousex>=60 && mousey>=234 && mousex<=250 && mousey<=252){
  about();
 } else
 if (mousex>=60 && mousey>=274 && mousex<=250 && mousey<=292){
  quit();
 }
}

void rightclick(void)
{
 if (mousex>=10 && mousey>=440 && mousex<=25 && mousey<456) col2=0; else
 if (mousex>=26 && mousey>=440 && mousex<=42 && mousey<456) col2=1; else
 if (mousex>=43 && mousey>=440 && mousex<=58 && mousey<456) col2=2; else
 if (mousex>=59 && mousey>=440 && mousex<=74 && mousey<456) col2=3; else
 if (mousex>=75 && mousey>=440 && mousex<=90 && mousey<456) col2=4; else
 if (mousex>=91 && mousey>=440 && mousex<=106 && mousey<456) col2=5; else
 if (mousex>=107 && mousey>=440 && mousex<=122 && mousey<456) col2=6; else
 if (mousex>=123 && mousey>=440 && mousex<=138 && mousey<456) col2=7; else
 if (mousex>=139 && mousey>=440 && mousex<=154 && mousey<456) col2=8; else
 if (mousex>=155 && mousey>=440 && mousex<=170 && mousey<456) col2=9; else
 if (mousex>=171 && mousey>=440 && mousex<=186 && mousey<456) col2=10; else
 if (mousex>=187 && mousey>=440 && mousex<=202 && mousey<456) col2=11; else
 if (mousex>=203 && mousey>=440 && mousex<=218 && mousey<456) col2=12; else
 if (mousex>=219 && mousey>=440 && mousex<=234 && mousey<456) col2=13; else
 if (mousex>=235 && mousey>=440 && mousex<=250 && mousey<456) col2=14; else
 if (mousex>=251 && mousey>=440 && mousex<=266 && mousey<456) col2=15;
}

void showcol(void)
{
 setcolor(15);
 rectangle(268,440,284,456);
 setfillstyle(1,col);
 bar(269,441,283,455);
 setcolor(15);
 rectangle(286,440,302,456);
 setfillstyle(1,col2);
 bar(287,441,301,455);
 setcolor(15);
 rectangle(305,440,347,456);
 outtextxy(311,444,"FILL");
 setcolor(15);
 rectangle(349,440,391,456);
 outtextxy(355,444,"COPY");

}


void drawfunction(void)
{
 int xpos,ypos;

 if (mousex>299 && mousey>29 && mousex<622 && mousey<352){
  xpos=(mousex-299)/xbx;
  ypos=(mousey-29)/xby;
  if (but1!=0){
   if (xpos<xsize && ypos<ysize){
    icon[xpos][ypos][img]=col;
    putpixel(106+xpos,303+ypos,icon[xpos][ypos][img]);
   }
  }
  if (but2!=0){
   if (xpos<xsize && ypos<ysize){
    icon[xpos][ypos][img]=col2;
    putpixel(106+xpos,303+ypos,icon[xpos][ypos][img]);
   }
  }
 }
}

void keypress(void)
{
 char ch;

 ch=getch();
 switch(ch){
 case 45: quit();break;
 case 49: newicon();break;
 case 23: imagetype();break;
 case 24: other();break;
 case 38: load();break;
 case 31: save();break;
 case 30: about();break;
 case 17: width();break;
 case 35: height();break;
 }
}

void main(void)
{
 printf("DRACOS IconEdit Copyright (C)1998 EdzUp\n");
 printf("Written by Ed Upton\n");
 s=getenv("DRACOS");
 if (strcmp(s,NULL)==0) {
  printf("Enviroment variable not set, program aborted\n\n");
  exit(2);
 }
 strcpy(maindir,s);
 strcpy(icondir,s);
 strcat(icondir,"\\ICONS\\");
 setmodes();
 dracosbackground();
 dracostitle(220,200);
 img=0;
 itype=0;
 xsize=32;
 ysize=32;
 oldxsize=xsize;
 oldysize=ysize;
 xbx=(622-299)/xsize;
 xby=(352-29)/ysize;
 lclick=0;
 rclick=0;
 palettesetup();
 imagedisplay(xsize,ysize);
 mousex=mouseinfo(1);
 mousey=mouseinfo(2);
 oldmousex=mousex;
 oldmousey=mousey;
 easybuttons();
 getimage(mousex+1,mousey+1,mousex+17,mousey+17,cursorback);
 showcursor(mousex,mousey);
 col=15;
 col2=0;
 menu=0;
 do{
  oldmousex=mousex;
  oldmousey=mousey;
  mousex=mouseinfo(1);
  mousey=mouseinfo(2);
  but1=mouseinfo(3);
  but2=mouseinfo(4);
  gotoxy(40,24);
  printf("Button 1:%i  Button2:%i  X:%i  Y:%i",but1,but2,mousex,mousey);
  gotoxy(10,3);
  printf("Image type : ");
  switch(itype){
  case 0 : printf("Icon      ");break;
  case 1 : printf("Pointer   ");break;
  case 2 : printf("Wallpaper ");break;
  }
  gotoxy(10,4);
  if (img==0) printf("Front Image"); else printf("Back Image ");
  if (but1!=0) {
   leftclick();
   drawfunction();
  }
  if (but2!=0) {
   rightclick();
   drawfunction();
  }
  if (kbhit()!=0) keypress();
  showcol();
  if (mousex>622) mousex=622;
  if (mousey>468) mousey=468;
  if (mousex!=oldmousex || mousey!=oldmousey) showcursor(mousex,mousey);
 }while(kbhit()!=257);
}