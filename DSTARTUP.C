#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <graphics.h>
#include <string.h>
#include <dos.h>

char image[64][64];
int Gd,Gm,num;
int x,y,logox,logoy,logosize;

void setupimage(void)
{
 strcpy(image[0], "                                                                 ");
 strcpy(image[1], "                                                                 ");
 strcpy(image[2], "                                                                 ");
 strcpy(image[3], "                                                                 ");
 strcpy(image[4], "                                                                 ");
 strcpy(image[5], "                                                                 ");
 strcpy(image[6], "                                                                 ");
 strcpy(image[7], "                                                                 ");
 strcpy(image[8], "                                                                 ");
 strcpy(image[9], "                                                                 ");
 strcpy(image[10],"                                                                 ");
 strcpy(image[11],"                                                                 ");
 strcpy(image[12],"                                                                 ");
 strcpy(image[13],"                                                                 ");
 strcpy(image[14],"            OOOOOOOOOOO                                          ");
 strcpy(image[15],"          OOOOOOOOOOOOOOOO                                       ");
 strcpy(image[16],"        OOOOOOOOOOOOOOOOOOOOO                                    ");
 strcpy(image[17],"       OOOOOOOOOOOOOOOOOOOOOOOO                                  ");
 strcpy(image[18],"      OOOOOOOOOOOOOOOOOOOOOOOOOOO                                ");
 strcpy(image[19],"     OOOOOOOOOOOOOOOOOOOOOOOOOOOOO                               ");
 strcpy(image[20],"    OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO                              ");
 strcpy(image[21],"    OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO                             ");
 strcpy(image[22],"    OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO                            ");
 strcpy(image[23],"   OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO                           ");
 strcpy(image[24],"   OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO                          ");
 strcpy(image[25],"   OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO                         ");
 strcpy(image[26],"  OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO                        ");
 strcpy(image[27],"  OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO                       ");
 strcpy(image[28],"  OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO                      ");
 strcpy(image[29],"  OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO                      ");
 strcpy(image[30],"  OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO                     ");
 strcpy(image[31],"  OOOOOOOOOOOOO OOOOOOOOOOOOOOOOOOOOOOOOOOOO                     ");
 strcpy(image[32]," OOOOOOOOOOO       OOOOOOOOOOOOOOOOOOOOOOOOOO                    ");
 strcpy(image[33]," OOOOOOOOOO         OOOOOOOOOOOOOOOOOOOOOOOOOO E                 ");
 strcpy(image[34]," OOOOOOOOO             OOOOOOOOOOOOOEOOOOOOOOOEEE                ");
 strcpy(image[35]," OOOOOOOOO              OOOOOOOOOOOEEEOOOOOOOEEEEE               ");
 strcpy(image[36]," OOOOOOOO                OOOOOOOOOOEEEEOOOPPPEEEEEE              ");
 strcpy(image[37]," OOOOOOOO                 OOOOOOOOOOEEEEOPPPPPEEEEEE             ");
 strcpy(image[38],"OOOOOOOO                   OOOOOOOOOOEEEEPHPPPPEEEEEE            ");
 strcpy(image[39],"OOOOOOOO                    OOOOOOOOOOEEEEHHPPPPEEEEEE           ");
 strcpy(image[40],"OOOOOOOO                     OOOOOOOOOIEEEEHHPPPPEEEEEE          ");
 strcpy(image[41],"OOOOOOO                       OOOOOOOIIHEEEEHHPPPPEEEEE          ");
 strcpy(image[42],"OOOOOOO                          OOOIIIHHEEEEHHPPPPEEEE          ");
 strcpy(image[43],"OOOOOOO                           OOIIIIHHEEEEHHPPPPEEE          ");
 strcpy(image[44],"OOOOOO                              EIIIIHHEEEEHHPPPPEE          ");
 strcpy(image[45],"OOOOOO                             EEEIIIIHHEEEEHHPPPPE          ");
 strcpy(image[46],"OOOOOO                            EEEEEIIIIHHEEEEHHPPPP          ");
 strcpy(image[47],"OOOOO                             EEEEEEIIIIHHEEEHHHPPPP         ");
 strcpy(image[48],"OOOOO                              EEEEEEIIIIHHEHHHHHPPPP        ");
 strcpy(image[49],"OOOOO                               EEEEEEIIIIHHHHHHHHPPPP       ");
 strcpy(image[50],"OOOO                                 EEEEEEIIIIHHHHHHHHPPPP      ");
 strcpy(image[51],"OOOO                                  EEEEEEIIIIHHHHHHHHPPPP     ");
 strcpy(image[52],"OOO                                    EEEEEEIIIIHHHHHHHHPPPP    ");
 strcpy(image[53],"OOO                                     EEEEEEIIIIHHHHHHHHPPP    ");
 strcpy(image[54],"OO                                             IIIIHHHHHHHHPPP   ");
 strcpy(image[55],"OO                                              IIIIHHHHHHHPPP   ");
 strcpy(image[56],"OO                                               IIIIHHHHHHHPPP  ");
 strcpy(image[57],"OO                                                IIIIHHHHHHPPP  ");
 strcpy(image[58],"O                                                  IIIIHHHHHHPP  ");
 strcpy(image[59],"O                                                   IIIIHHHHHPP  ");
 strcpy(image[60],"O                                                    IIIIHHHHPP  ");
 strcpy(image[61],"O                                                     IIIIIHHHP  ");
 strcpy(image[62],"                                                        IIIIIHP  ");
 strcpy(image[63],"                                                          IIIIP  ");

}

void showimage(int px,int py,int size)
{
 for (x=0;x<64;x++){
  for (y=0;y<64;y++){
   if (image[y][x]!=32) setfillstyle(1,image[y][x]-65); else setfillstyle(1,0);
   bar(px+(x*size),py+(y*size),px+(x*size)+size,py+(y*size)+size);
  }
 }
}

void main(void)
{
 logox=303;
 logoy=184;
 logosize=1;
 delay(1);
 setupimage();
 registerbgidriver(EGAVGA_driver);
 registerbgifont(small_font);
 Gd=9;
 Gm=2;
 initgraph(&Gd,&Gm,"");
 showimage(100,-50,7);
 settextstyle(2,0,10);
 setcolor(15);
 outtextxy(240,400,"DRACOS");
 settextstyle(2,0,4);
 outtextxy(227,440,"Copyright (C)1998-1999 EdzUp");
 delay(3000);
 closegraph();
}
