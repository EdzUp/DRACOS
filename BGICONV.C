#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>

FILE *hand,*h2;
char ch;
unsigned long byte;

void main(int argc,char *argv[])
{
 printf("BGI to lib converter\n");
 printf("Copyright (C)1999 EdzUp\n\n");
 if (argc<3){
  printf("Usage : BGICONV source target\n\n");
  exit(0);
 }
 printf("Source: %s\n",argv[1]);
 printf("Target: %s\n",argv[2]);
 hand=fopen(argv[1],"rb");
 if (hand==NULL){
  printf("Error : Cannot access file specified\n\n");
  exit(1);
 }
 h2=fopen(argv[2],"wb");
 if (h2==NULL){
  printf("Error : Cannot create destination library\n\n");
  exit(1);
 }
 byte=0;
 do{
  ch=fgetc(hand);
  ch=~ch+byte;
  printf("%i\n",ch);
  fputc(ch,h2);
  byte++;
 }while(feof(hand)==0);
 printf("File %s has been converted to %s\n\n",argv[1],argv[2]);
 fclose(hand);
 fclose(h2);
}