#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char *Screen = (unsigned char *)0xB8000000L;
unsigned char *Buffer;

int vx,vy;
unsigned char ary[81][26],attr[81][26];

void PutPixel (int x, int y, char c)
{
  Buffer [(y << 8) + (y << 6) + x] = c;
}

void posn(unsigned char x, unsigned char y)
{
 asm mov ax,0x0200
 asm mov bh,0
 asm mov dh,y
 asm mov dl,x
 asm int 0x10
}

void putchr(unsigned char x,unsigned char y,char c,unsigned char a)
{

/* a=paper*16;
 a=a+ink;*/

 asm mov ah,0x09
 asm mov al,c
 asm mov bh,0
 asm mov bl,a
 asm mov cx,1
 asm int 0x10

 asm mov ax,0x0200
 asm mov bh,0
 asm mov dh,y
 asm mov dl,x
 asm int 0x10
}

void text(int x,int y,char *str,int ink,int paper)
{
 int len;

 for (len=0;len<strlen(str);len++){
  ary[x+len][y]=str[len];
  attr[x+len][y]=(paper*16)+ink;
 }
}

void show(unsigned char ary[81][26],unsigned char attr[81][26])
{
 int showx,showy;

 for (showy=0;showy<26;showy++){
  for (showx=0;showx<81;showx++){
   putchr(showx,showy,ary[showx][showy],attr[showx][showy]);
  }
 }
 gotoxy(1,1);
}

void main(void)
{
 textmode(3);
 do{
  text(random(80),random(25),"*",14,0);
  show(ary,attr);
 }while(kbhit()==0);
}