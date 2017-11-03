#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <string.h>
#include <dos.h>
#include <process.h>
#include <errno.h>
#include <io.h>
#include <dir.h>
#include <fcntl.h>

#include <edzupcon.h>

unsigned long key[10];
int num,tx,ty;
char filename[255];
char destination[255];
char source[255];
unsigned long signature[3];
unsigned long filesize;
char initial[6],writname[14];
char idlength;
FILE *hand,*create;

void getkeys(void)
{
 char ss[1024];
 int y,cnt;

 printf("Please enter a sentence\n");
 y=wherey();
 do{
  gotoxy(1,y);
  clreol();
  gotoxy(1,y);
  gets(ss);
 }while(strlen(ss)<10 || strlen(ss)>255);
 for (num=0;num<strlen(ss);num++) key[0]+=ss[num];
 gotoxy(1,wherey());
 printf("Generating keys [.");
 srand(key[0]);
 for (cnt=0;cnt<random(30000)+2000;cnt++) key[1]+=ss[random(strlen(ss))];
 key[1]+=key[0];
 cprintf(".");
 srand(key[1]);
 for (cnt=0;cnt<random(30000)+2000;cnt++) key[2]+=ss[random(strlen(ss))];
 key[2]+=key[1];
 cprintf(".");
 srand(key[2]);
 for (cnt=0;cnt<random(30000)+2000;cnt++) key[3]+=ss[random(strlen(ss))];
 key[3]+=key[2];
 cprintf(".");
 srand(key[3]);
 for (cnt=0;cnt<random(30000)+2000;cnt++) key[4]+=ss[random(strlen(ss))];
 key[4]+=key[3];
 cprintf(".");
 srand(key[4]);
 for (cnt=0;cnt<random(30000)+2000;cnt++) key[5]+=ss[random(strlen(ss))];
 key[5]+=key[4];
 cprintf(".");
 srand(key[5]);
 for (cnt=0;cnt<random(30000)+2000;cnt++) key[6]+=ss[random(strlen(ss))];
 key[6]+=key[5];
 cprintf(".");
 srand(key[6]);
 for (cnt=0;cnt<random(30000)+2000;cnt++) key[7]+=ss[random(strlen(ss))];
 key[7]+=key[6];
 cprintf(".");
 srand(key[7]);
 for (cnt=0;cnt<random(30000)+2000;cnt++) key[8]+=ss[random(strlen(ss))];
 key[8]+=key[7];
 cprintf(".");
 srand(key[8]);
 for (cnt=0;cnt<random(30000)+2000;cnt++) key[9]+=ss[random(strlen(ss))];
 key[9]+=key[8];
 printf(".] Complete\n");
}

void initdisplay(void)
{
 textbackground(1);
 textcolor(14);
 clrscr();
 box(1,1,80,8);
 gotoxy(2,2);
 printf("                              л   ллл ллл л л");
 gotoxy(2,3);
 printf("                              л   л л л   л л");
 gotoxy(2,4);
 printf("                              л   л л л   лл ");
 gotoxy(2,5);
 printf("                              л   л л л   л л");
 gotoxy(2,6);
 printf("                              ллл ллл ллл л л");
 gotoxy(2,7);
 printf("                        Copyright (C)1989-1999 EdzUp");
 gotoxy(1,10);
 window(1,9,80,25);
 textcolor(7);
 textbackground(0);
 clrscr();
 printf("Data file %s\n",filename);
 printf("Archive file %s\n",destination);
}

void begin(void)
{
 FILE *signa;
 char byte;

 signa=fopen(source,"rb");
 if (signa==NULL){
  printf("Read Error\n");
 } else {
  do{
   byte=fgetc(signa);
   for (num=0;num<10;num++){
    srand(key[num]);
    byte+=random(255);
   }
   fputc(byte,create);
  }while(feof(signa)==0);
  fclose(signa);
  printf("LOCKED\n");
 }
}

void writedetail(void)
{
 int nnn;

 fputs(writname,create);
 for (nnn=strlen(writname);nnn<13;nnn++) fputc(32,create);
 fwrite(&filesize,4,1,create);
 fwrite(&signature[0],4,1,create);
 fwrite(&signature[1],4,1,create);
 fwrite(&signature[2],4,1,create);
 for (num=0;num<5;num++) fputc(initial[num],create);
 begin();
}

int getsize(void)
{
 int handle,err;

 handle=open(source,O_RDONLY);
 if (handle==-1){
  err=-1;
  printf("Error cannot locate file\n");
 } else {
  err=0;
  filesize=filelength(handle);
  printf("%X",filesize);
  close(handle);
 }
 return err;
}

void signatures(void)
{
 FILE *signa;
 unsigned long pos;

 signa=fopen(source,"rb");
 if (signa==NULL){
  printf("Cannot read from file\n");
 } else {
  signature[0]=0;
  signature[1]=0;
  signature[2]=0;
  for (pos=0;pos<5;pos++) initial[pos]=fgetc(signa);
  fseek(signa,0,SEEK_SET);
  for (pos=0;pos<filesize;pos++) signature[0]+=fgetc(signa);
  printf("%X",signature[0]);
  fseek(signa,0,SEEK_SET);
  gotoxy(34,ty);
  for (pos=0;pos<filesize;pos+=2) {
   signature[1]+=fgetc(signa);
   fgetc(signa);
  }
  printf("%X",signature[1]);
  fseek(signa,0,SEEK_SET);
  gotoxy(44,ty);
  for (pos=1;pos<filesize;pos+=2) {
   signature[2]+=fgetc(signa);
   fgetc(signa);
  }
  fclose(signa);
 }
 printf("%X",signature[2]);
 gotoxy(54,ty);
 writedetail();
}

void getfilename(void)
{
 char drive[3],dir[192],name[9],ext[5];
 int err;

 tx=wherex();
 ty=wherey();
 fgets(source,255,hand);
 if (strcmp(source,NULL)!=0 && source[0]!=10 && source[0]!=13){
  fnsplit(source,drive,dir,name,ext);
  printf("%s%s",name,ext);
  strcpy(writname,name);
  strcat(writname,ext);
  strcpy(source,drive);
  strcat(source,dir);
  strcat(source,name);
  strcat(source,ext);
  //printf("[%s]",source);
  gotoxy(15,ty);
  err=getsize();
  if (err!=-1){
   gotoxy(24,ty);
   signatures();
  }
 }
}

void encrypter(void)
{
 create=fopen(destination,"wb");
 if (create==NULL){
  printf("Error : Cannot create destination archive [%s]\n",destination);
  window(1,1,80,25);
  exit(1);
 }
 hand=fopen(filename,"rt");
 if (hand==NULL){
  printf("Error : Cannot open data file [%s]\n",filename);
  window(1,1,80,25);
  exit(1);
 }
 printf("\nArchive file created...\n");
 printf("Accessing file list...\n\n");
 printf("Filename      Size     Lock1     Lock2     Lock3\n");
 do{
  getfilename();
 }while(feof(hand)==0);
 fclose(hand);
 fputs("**ENDOFLOCK**",create);
 fclose(create);
 printf("Archive complete\n\n");
}

void main(int argc,char *argv[])
{
 if (argc<3){
  printf("Lockup\n");
  printf("Copyright (C)1989-1999 EdzUp\n\n");
  printf("Usage : LOCKUP listfile destinationfile\n\n");
  exit(0);
 }
 strcpy(filename,argv[1]);
 strcpy(destination,argv[2]);
 initdisplay();
 cprintf("Please enter the 10-255 character combination\n\r");
 cprintf("the file is then encrypted using this combination\n\r\n\r");
 getkeys();
 encrypter();
}