#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <string.h>
#include <graphics.h>
#include <dir.h>

struct {
char filename[13],name[30];
} filereg[1024];

struct {
int r,g,b;
} col[16];

struct {
int x,y,f,t,mt,d;
} bot[16];

struct REGPACK in; /*registers structure*/
struct ffblk ffblk;

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

long coverage;
int mousex,mousey,ma,am,num,Gd,Gm,lt,rt,curcol,et,maxfile,dedbot,fnt;
char maindir[192],helpdir[192],*s,com[255],ch;
int cursorx,cursory,but1,but2,oldmousex,oldmousey,curlink,maxlink;
int cursorback[16][16]; /*background behind cursor, replaced when cursor moves*/
FILE *hand;

void setvid(void)
{
 registerbgidriver(EGAVGA_driver);
 registerbgifont(small_font);
 fnt=0;
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

void setcolors(void)
{
 setrgbpalette(0,1,1,1); //Black (Background);
 setrgbpalette(1,31,17,0); //Brown (outlines and some detail)
 setrgbpalette(2,27,27,27);//Dark Grey
 setrgbpalette(3,0,0,40); //Dark Blue
 setrgbpalette(4,0,32,0); //Green
 setrgbpalette(5,20,20,63); //Light Blue
 setrgbpalette(20,50,0,0); //Red
 setrgbpalette(7,63,63,63); // ??
 setrgbpalette(56,63,3,1);
 setrgbpalette(57,45,16,8); //flesh
 setrgbpalette(58,32,63,0); //Green
 setrgbpalette(59,63,45,0); //Golden?
 setrgbpalette(60,60,55,0); //yellow
 setrgbpalette(61,40,40,40); //Grey
 setrgbpalette(62,63,55,40); //Bone
 setrgbpalette(63,63,63,63); //White
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

void getpalettes(void)
{
 char dir[255],fln[255];
 int err,cf,fl;

 fl=0;
 cf=0;
 strcpy(dir,helpdir);
 strcat(dir,"\\*.*");
 err=findfirst(dir,&ffblk,255);
 if (err==0){
  strcpy(filereg[cf].filename,ffblk.ff_name);
  strcpy(fln,helpdir);
  strcat(fln,"\\");
  strcat(fln,filereg[cf].filename);
  hand=fopen(fln,"rb");
  if (hand!=NULL){
   num=fgetc(hand);
   fgets(filereg[cf].name,30,hand);
   fclose(hand);
   cf++;
  }
  fl++;
  do{
   err=findnext(&ffblk);
   if (err==0) {
    strcpy(filereg[cf].filename,ffblk.ff_name);
    strcpy(filereg[cf].filename,ffblk.ff_name);
    strcpy(fln,helpdir);
    strcat(fln,"\\");
    strcat(fln,filereg[cf].filename);
    hand=fopen(fln,"rb");
    if (hand!=NULL){
     num=fgetc(hand);
     fgets(filereg[cf].name,30,hand);
     fclose(hand);
     cf++;
    }
    fl++;
   }
  }while(err==0);
  //printf("%i,%i files located\n",cf,fl);
  maxfile=cf;
 }
}

void dwindow(int tx,int ty,int bx,int by,char header[80])
{
 setfillstyle(1,7);
 bar(tx,ty,bx,by);
 settextstyle(2,0,4);
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
 outtextxy(tx+3,ty,header);
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
 settextstyle(2,0,4);
 outtextxy(3,-1,"DRACOS Palette program (C)Copyright 1998-1999 EdzUp");
 setcolor(8);
 line(1,11,640,11);
 line(640,11,640,1);
}

void showpalette(void)
{
 setcolor(15);
 for (num=0;num<16;num++){
  setfillstyle(1,num);
  bar(17+(num*38),420,17+(num*38)+38,442);
  rectangle(17+(num*38),420,17+(num*38)+38,442);
 }
}

void selectinfo(void)
{
 int slink,cl,nn,sl,ok,rt;
 char chr[3],filename[255];

 gotoxy(1,1);
 dwindow(120,120,440,370,"Select palette");
 outtextxy(123,135,"Select the pre-set palette you require");
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
 settextstyle(2,0,4);
 rectangle(232,335,327,358);
 setcolor(15);
 for (nn=cl;nn<cl+16;nn++){
  outtextxy(125,164+((nn-cl)*9),filereg[nn].name);
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
     setcolor(15);
     outtextxy(125,164+((nn-cl)*9),filereg[nn].name);
    }
   } else
   if (mousex>=125 && mousey>=312 && mousex<=435 && mousey<=325) {
    setfillstyle(1,0);
    bar(123,165,437,310);
    cl++;
    if (cl>=134) cl=134;
    for (nn=cl;nn<cl+16;nn++){
     setcolor(15);
     outtextxy(125,164+((nn-cl)*9),filereg[nn].name);
    }
   }
  }
  if (but1!=0) rt++;
  if (rt>100) rt=0;
  if (mousex>622) mousex=622;
  if (mousey>468) mousey=468;
  if (mousex!=oldmousex || mousey!=oldmousey) showcursor(mousex,mousey);
 }while(ok==0);
 if (ok==1) {
  //sl;
  strcpy(com,helpdir);
  strcat(com,"\\");
  strcat(com,filereg[sl].filename);
  hand=fopen(com,"rb");
  if (hand!=NULL){
   num=fgetc(hand);
   fgets(filereg[sl].name,num+1,hand);
   for (num=0;num<16;num++){
    col[num].r=fgetc(hand);
    col[num].g=fgetc(hand);
    col[num].b=fgetc(hand);
    setupcol(num);
   }
   fclose(hand);
  }
 }
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
 for (num=0;num<1024;num++){
  strcpy(filereg[num].filename,"");
 }
}

void setupdefaultpalette(void)
{
 // R G B
 for (num=0;num<16;num++){
  col[0].r=0;
  col[0].g=0;
  col[0].b=0;
 }
 setrgbpalette(0,0,0,0);
 col[1].b=40;
 setrgbpalette(1,0,0,40);
 col[2].g=40;
 setrgbpalette(2,0,40,0);
 col[3].r=30;
 col[3].g=40;
 col[3].b=63;
 setrgbpalette(3,30,40,63);
 col[4].r=40;
 setrgbpalette(4,40,0,0);
 col[5].r=40;
 col[5].b=40;
 setrgbpalette(5,40,0,40);
 col[6].r=40;
 col[6].g=20;
 setrgbpalette(20,40,20,0);
 col[7].r=43;
 col[7].g=43;
 col[7].b=43;
 setrgbpalette(7,43,43,43);
 col[8].r=23;
 col[8].g=23;
 col[8].b=23;
 setrgbpalette(56,23,23,23);
 col[9].b=63;
 setrgbpalette(57,0,0,63);
 col[10].g=63;
 setrgbpalette(58,0,63,0);
 col[11].r=40;
 col[11].g=55;
 col[11].b=63;
 setrgbpalette(59,40,55,63);
 col[12].r=63;
 col[12].g=20;
 col[12].b=20;
 setrgbpalette(60,63,20,20);
 col[13].r=63;
 col[13].b=63;
 setrgbpalette(61,63,0,63);
 col[14].r=63;
 col[14].g=63;
 col[14].b=10;
 setrgbpalette(62,63,63,10);
 col[15].r=63;
 col[15].g=63;
 col[15].b=63;
 setrgbpalette(63,63,63,63);
}

void sliderbars(void)
{
 setcolor(15);
 setfillstyle(1,0);
 bar(10,44,30,362);
 rectangle(9,43,31,363);
 bar(60,44,80,362);
 rectangle(59,43,81,363);
 bar(110,44,130,362);
 rectangle(109,43,131,363);
 setfillstyle(1,7);
 bar(32,44,58,362);
 bar(82,44,108,362);
 bar(10,364,145,372);
 bar(132,44,144,372);
 setfillstyle(1,15);
 bar(10,362-((col[curcol].r)*5),30,362-((col[curcol].r*5))-3);
 bar(60,362-((col[curcol].g)*5),80,362-((col[curcol].g*5))-3);
 bar(110,362-((col[curcol].b)*5),130,362-((col[curcol].b*5))-3);
 setfillstyle(1,curcol);
 bar(10,373,130,393);
 rectangle(10,373,130,393);
}

void setbot(void)
{
 int cc;

 for (num=0;num<16;num++){
  bot[num].x=random(450)+5;
  bot[num].y=random(190)+5;
  bot[num].f=random(4);
  bot[num].t=random(50)+10;
  bot[num].mt=random(50);
  bot[num].d=1;
  setfillstyle(1,num);
  bar(10+(num*16),205,10+(num*16)+8,215);
  setcolor(15);
  rectangle(10+(num*16),205,10+(num*16)+8,215);
 }
}

void example(void)
{
 int x,y,c;

 setviewport(164,166,627,388,1);
 settextstyle(0,0,3);
 for (num=0;num<16;num++){
  if (bot[num].mt==0 && bot[num].d==1) {
   if (getpixel(bot[num].x,bot[num].y)!=0 && num!=0){
    bot[num].d=0;
    dedbot++;
    if (coverage<10000){
     do{
      bot[num].x=random(450)+5;
      bot[num].y=random(190)+5;
     } while(getpixel(bot[num].x,bot[num].y)!=0);
     bot[num].f=random(4);
     bot[num].t=random(50)+10;
     bot[num].mt=random(50);
     bot[num].d=1;
     dedbot--;
    }
    if (bot[num].d==0){
     setfillstyle(1,0);
     bar(10+(num*16),205,10+(num*16)+8,215);
     setcolor(0);
     rectangle(10+(num*16),205,10+(num*16)+8,215);
    }
   }
   putpixel(bot[num].x,bot[num].y,num);
   switch(bot[num].f){
    case 0 : bot[num].y--;break;
    case 1 : bot[num].x++;break;
    case 2 : bot[num].y++;break;
    case 3 : bot[num].x--;break;
   }
   switch(bot[num].f){
    case 0 : if (getpixel(bot[num].x,bot[num].y-1)!=0) bot[num].t=0;break;
    case 1 : if (getpixel(bot[num].x+1,bot[num].y)!=0) bot[num].t=0;break;
    case 2 : if (getpixel(bot[num].x,bot[num].y+1)!=0) bot[num].t=0;break;
    case 3 : if (getpixel(bot[num].x-1,bot[num].y)!=0) bot[num].t=0;break;
   }
   if (bot[num].t<1){
    c=random(2);
    if (c==0) {
     switch(bot[num].f){
     case 0 : if (getpixel(bot[num].x-1,bot[num].y)!=0) bot[num].f++; else bot[num].f--;break;
     case 1 : if (getpixel(bot[num].x,bot[num].y-1)!=0) bot[num].f++; else bot[num].f--;break;
     case 2 : if (getpixel(bot[num].x+1,bot[num].y)!=0) bot[num].f++; else bot[num].f--;break;
     case 3 : if (getpixel(bot[num].x,bot[num].y+1)!=0) bot[num].f++; else bot[num].f--;break;
     }
    }else {
     switch(bot[num].f){
     case 0 : if (getpixel(bot[num].x+1,bot[num].y)!=0) bot[num].f--; else bot[num].f++;break;
     case 1 : if (getpixel(bot[num].x,bot[num].y+1)!=0) bot[num].f--; else bot[num].f++;break;
     case 2 : if (getpixel(bot[num].x-1,bot[num].y)!=0) bot[num].f--; else bot[num].f++;break;
     case 3 : if (getpixel(bot[num].x,bot[num].y-1)!=0) bot[num].f--; else bot[num].f++;break;
     }

    }
    bot[num].t=random(50)+10;
   }
   if (bot[num].y<2 && bot[num].f==0) {
    c=random(2);
    if (c==0) bot[num].f--; else bot[num].f++;
   }
   if (bot[num].y<2 && bot[num].f==3) {
    c=random(2);
    if (c==0) bot[num].f--; else bot[num].f++;
   }
   if (bot[num].x>461 && bot[num].f==1) {
    c=random(2);
    if (c==0) bot[num].f--; else bot[num].f++;
   }
   if (bot[num].y>200 && bot[num].f==2) {
    c=random(2);
    if (c==0) bot[num].f--; else bot[num].f++;
   }
   if (bot[num].f<0) bot[num].f=3;
   if (bot[num].f>3) bot[num].f=0;
   bot[num].mt=50;
   coverage++;
   for (y=0;y<16;y++){
    if (bot[num].x==bot[y].x && bot[num].y==bot[y].y && bot[y].d==1 && y!=num){
     bot[num].d=0;
     bot[y].d=0;
     dedbot+=2;
     setfillstyle(1,0);
     bar(10+(num*16),205,10+(num*16)+8,215);
     setcolor(0);
     rectangle(10+(num*16),205,10+(num*16)+8,215);
     setfillstyle(1,0);
     bar(10+(y*16),205,10+(y*16)+8,215);
     setcolor(0);
     rectangle(10+(y*16),205,10+(y*16)+8,215);

    }
   }
  } else bot[num].mt--;
  if (coverage>50000 || dedbot>14){
   setfillstyle(1,0);
   bar(0,0,462,221);
   coverage=0;
   dedbot=0;
   setbot();
  }
 }
 setviewport(0,0,639,479,1);
}


void textrequest(int tx,int ty,int length,char str[255],int max)
{
 char ch,tcom[255],chr[3],oc[255];
 int pos,sp,tp,bl;

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
}

void startup(void)
{
 dwindow(0,12,150,400,"RGB sliders");
 dwindow(0,401,640,480,"Color selector");
 dwindow(151,12,640,400,"Example DRACOS window");
 setfillstyle(1,0);
 setcolor(15);
 bar(153,24,638,398);
 bar(10,44,30,362);
 rectangle(9,43,31,363);
 bar(60,44,80,362);
 rectangle(59,43,81,363);
 bar(110,44,130,362);
 rectangle(109,43,131,363);
 settextstyle(0,0,2);
 outtextxy(13,27,"R");
 outtextxy(63,27,"G");
 outtextxy(113,27,"B");
 settextstyle(0,0,0);
 rectangle(20,445,90,472);
 outtextxy(39,455,"Save");
 rectangle(110,445,180,472);
 outtextxy(129,455,"Load");
 rectangle(200,445,270,472);
 outtextxy(209,455,"Default");
 rectangle(290,445,360,472);
 outtextxy(313,455,"Use");
 rectangle(552,445,622,472);
 outtextxy(565,455," Quit ");
 settextstyle(2,0,4);
 setfillstyle(1,1);
 bar(183,32,600,54);
 setfillstyle(1,0);
 rectangle(183,32,600,54);
 outtextxy(242,31,"Direct Replacement Auto Configurable Operating Shell");
 outtextxy(323,41,"Palette Adjustment program");
 outtextxy(162,64,"Please note all adjustments to the palette will affect the display as a whole and");
 outtextxy(162,74,"not just this example window. This is to allow you 'the user' to get a better feel");
 outtextxy(162,84,"of the set of colours you would like to use.");
 setcolor(14);
 outtextxy(162,114,"Please use the colour bar at the bottom of the screen, you will be able to see");
 outtextxy(162,124,"your adjustments more clearly  (unless you have set a load of colours to the same");
 outtextxy(162,134,"Red, Green and Blue values).");
 dwindow(162,154,629,390,"DRACOS example");
 setfillstyle(1,0);
 bar(164,166,627,388);
 example();
}

void savepalette(void)
{
 char name[30],filename[13],ch[2];
 int len;

 strcpy(com,"");
 dwindow(100,230,540,310,"Save palette");
 outtextxy(123,253,"What would you like your palette to be called?");
 outtextxy(123,263,"Maximum 30 characters.");
 settextstyle(0,0,0);
 textrequest(120,283,43,com,30);
 if (strcmp(com,"")!=0){
  strcpy(name,com);
  strcpy(filename,"");
  len=strlen(name);
  if (len>8) len=8;
  for (num=0;num<len;num++){
   ch[0]=name[num];
   ch[1]=NULL;
   switch(ch[0]){
   case '\\' : ch[0]='~';break;
   case '*' : ch[0]='~';break;
   case ':' : ch[0]='~';break;
   case '?' : ch[0]='~';break;
   case '.' : ch[0]='~';break;
   case ' ' : ch[0]='~';break;
   }
   strcat(filename,ch);
  }
  gotoxy(1,1);
  strcat(filename,".DPF");
  strcpy(com,helpdir);
  strcat(com,"\\");
  strcat(com,filename);
  hand=fopen(com,"wb");
  if (hand!=NULL){
   fputc(strlen(name),hand);
   fputs(name,hand);
   for (num=0;num<16;num++){
    fputc(col[num].r,hand);
    fputc(col[num].g,hand);
    fputc(col[num].b,hand);
   }
   fclose(hand);
  }
 }
 getpalettes();
 startup();
}

void usepalette(void)
{
 char name[30],filename[13],ch[2];
 int len;

 strcpy(name,"DRACOS.DPF");
 gotoxy(1,1);
 strcpy(com,maindir);
 strcat(com,"\\");
 strcat(com,name);
 hand=fopen(com,"wb");
 if (hand!=NULL){
  fputc(strlen(name),hand);
  fputs(name,hand);
  for (num=0;num<16;num++){
   fputc(col[num].r,hand);
   fputc(col[num].g,hand);
   fputc(col[num].b,hand);
  }
  fclose(hand);
 }
 getpalettes();
 startup();
}


void leftbutton(void)
{
 if (lt==0){
  if (mousex>=20 && mousey>=445 && mousex<=90 && mousey<=472){
   savepalette();
  } else
  if (mousex>=110 && mousey>=445 && mousex<=180 && mousey<=472){
   selectinfo();
   startup();
   showpalette();
   sliderbars();
   getimage(mousex+1,mousey+1,mousex+16,mousey+16,cursorback);
  } else
  if (mousex>=290 && mousey>=445 && mousex<=360 && mousey<=472){
   usepalette();
  } else
  if (mousex>=552 && mousey>=445 && mousex<=622 && mousey<=472){
   closegraph();
   exit(0);
  } else
  if (mousex>=17 && mousey>=420 && mousex<=625 && mousey<=442){
   curcol=(mousex-17)/38;
   sliderbars();
  } else
  if (mousex>=200 && mousey>=445 && mousex<=270 && mousey<=472){
   setupdefaultpalette();
   showpalette();
   sliderbars();
  } else
  if (mousex>=10 && mousey>=44 && mousex<=30 && mousey<=362){
   col[curcol].r=63-((mousey-44)/5);
   putimage(oldmousex+1,oldmousey+1,cursorback,0);
   setupcol(curcol);
   sliderbars();
   getimage(mousex+1,mousey+1,mousex+16,mousey+16,cursorback);
  } else
  if (mousex>=60 && mousey>=44 && mousex<=80 && mousey<=362){
   col[curcol].g=63-((mousey-44)/5);
   putimage(oldmousex+1,oldmousey+1,cursorback,0);
   setupcol(curcol);
   sliderbars();
   getimage(mousex+1,mousey+1,mousex+16,mousey+16,cursorback);
  } else
  if (mousex>=110 && mousey>=44 && mousex<=130 && mousey<=362){
   col[curcol].b=63-((mousey-44)/5);
   putimage(oldmousex+1,oldmousey+1,cursorback,0);
   setupcol(curcol);
   sliderbars();
   getimage(mousex+1,mousey+1,mousex+16,mousey+16,cursorback);
  }
 }
}

void main(void)
{
 printf("DRACOS Palette program\n");
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
 strcat(helpdir,"\\PALETTE");
 readinfofile();
 getpalettes();
 coverage=0;
 dedbot=0;
 clrscr();
 setvid();
 Gd=9;
 Gm=2;
 initgraph(&Gd,&Gm,"");
 settextstyle(fnt,0,0);
 for (num=0;num<16;num++){
  bot[num].x=random(450)+5;
  bot[num].y=random(190)+5;
  bot[num].f=random(4);
  bot[num].t=random(50)+10;
  bot[num].mt=random(50);
  bot[num].d=1;
 }
 setupdefaultpalette();
 titlebar();
 startup();
 lt=0;
 rt=0;
 ma=0;
 am=0;
 curlink=0;
 curcol=11;
 showpalette();
 sliderbars();
 et=15;
 do{
  et--;
  if (et<0) et=15;
  oldmousex=mousex;
  oldmousey=mousey;
  mousex=mouseinfo(1);
  mousey=mouseinfo(2);
  but1=mouseinfo(3);
  but2=mouseinfo(4);
  example();
  if (kbhit()!=0) keypress();
  if (but1!=0) leftbutton();
  if (but1!=0) rt++;
  if (mousex<0) mousex=0;
  if (mousey<0) mousey=0;
  if (mousex>622) mousex=622;
  if (mousey>468) mousey=468;
  if (mousex!=oldmousex || mousey!=oldmousey) showcursor(mousex,mousey);
 }while(mousex<700);
}
