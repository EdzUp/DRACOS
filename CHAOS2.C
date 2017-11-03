#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <graphics.h>

struct {
char part,monster,owner,infection,infown,build;
char stats[20]; //M WS BS S T W I A Ld Arm Magik Fly Poison Undead ethereal
} huge map[50L][50L];

int Gd,Gm,x,y;

void initvid(void)
{
 registerbgidriver(EGAVGA_driver);
 Gd=9;
 Gm=0;
 initgraph(&Gd,&Gm,"");
}

void initmap(void)
{
 int cnt;

 for (y=0;y<100;y++){
  for (x=0;x<100;x++){
   map[x][y].part=0;
   map[x][y].monster=0;
   map[x][y].owner=-1;
   map[x][y].infection=0;
   map[x][y].infown=-1;
   map[x][y].build=0;
   for (cnt=0;cnt<20;cnt++) map[x][y].stats[cnt]=0;
  }
 }
}

void main(void)
{
 initvid();
 initmap();
}