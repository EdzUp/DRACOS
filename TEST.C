#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <graphics.h>
#include <string.h>

int Gd,Gm;

void initvid(void)
{
 registerbgidriver(EGAVGA_driver);
 registerbgifont(small_font);
 Gd=9;
 Gm=2;
 initgraph(&Gd,&Gm,"");
 settextstyle(2,0,4);
}

void button(int tx,int ty,int width,char header[20])
{
 int ctr,len,by,bx;

 bx=tx+width;
 by=tx+24;
 len=strlen(header)*6;
 ctr=((bx-tx)/2);

 setfillstyle(1,7);
 bar(tx,ty,bx,by);
 setcolor(15);
 line(tx,ty,bx,ty);
 line(tx,ty,tx,by);
 line(tx+1,ty+1,bx-2,ty+1);
 line(tx+1,ty+1,tx+1,by-2);
 setcolor(0);
 line(tx+1,by,bx,by);
 line(bx,ty+1,bx,by);
 setcolor(8);
 line(bx-1,by-1,tx+1,by-1);
 line(bx-1,by-1,bx-1,ty+1);
 setcolor(0);
 outtextxy((tx+ctr)-(len/2),ty+6,header);
}

void main(void)
{
 initvid();
 button(50,50,75,"Test Button");
}