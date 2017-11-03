#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <string.h>

unsigned long key[10];
int num,tx,ty,erro,errors;
char filename[255];
char destination[255];
char source[255];
unsigned long signature[4];
unsigned long filesize;
unsigned char initial[6],writname[14];
char idlength;
FILE *hand,*create;

void getkeys(void)
{
 char ss[1024];
 int y,cnt;

 printf("Please enter unlock code\n");
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
 gotoxy(2,2);
 printf("                     л   ллл ллл л л лл  ллл л л л ллл");
 gotoxy(2,3);
 printf("                     л   л л л   л л л л л л л л л л л");
 gotoxy(2,4);
 printf("                     л   л л л   лл  л л л л л л л л л");
 gotoxy(2,5);
 printf("                     л   л л л   л л л л л л л л л л л");
 gotoxy(2,6);
 printf("                     ллл ллл ллл л л лл  ллл ллллл л л");
 gotoxy(2,7);
 printf("                        Copyright (C)1989-1999 EdzUp");
 gotoxy(1,10);
 window(1,9,80,25);
 textcolor(7);
 textbackground(0);
 clrscr();
 printf("Archive file %s\n",filename);
}

void signatures(void)
{
 FILE *signa;
 unsigned long pos;
 unsigned long rsignature[3];

 signa=fopen(writname,"rb");
 if (signa==NULL){
  printf("Cannot read from file\n");
 } else {
  rsignature[0]=0;
  rsignature[1]=0;
  rsignature[2]=0;
  for (pos=0;pos<filesize;pos++) rsignature[0]+=fgetc(signa);
  fseek(signa,0,SEEK_SET);
  for (pos=0;pos<filesize;pos+=2) {
   rsignature[1]+=fgetc(signa);
   fgetc(signa);
  }
  fseek(signa,0,SEEK_SET);
  for (pos=1;pos<filesize;pos+=2) {
   rsignature[2]+=fgetc(signa);
   fgetc(signa);
  }
  fseek(signa,0,SEEK_SET);
  fclose(signa);
 }
 if (signature[0]!=rsignature[0] || signature[1]!=rsignature[1] || signature[2]!=rsignature[2]){
  erro=2;
  unlink(writname);
 }
}

void undofile(void)
{
 unsigned long pos;

 FILE *signa;
 char byte,erro,initbyte[5];

 erro=0;
 signa=fopen(writname,"wb");
 if (signa==NULL){
  printf("Creation Error\n");
 } else {
  pos=0;
  strcpy(initbyte,"     ");
  do{
   byte=fgetc(create);
   for (num=0;num<10;num++){
    srand(key[num]);
    byte-=random(255);
   }
   fputc(byte,signa);
   pos++;
   if (pos<5) {
    initbyte[pos-1]=byte;
   } else
   if (pos==5){
    if (strcmp(initbyte,initial)!=0) {
     erro=1;
    }
   }
  }
  while(pos<filesize+1 && erro==0);
  fclose(signa);
  signatures();
  if (erro==0) printf("UNLOCKED\n"); else {
   if (erro==1) {
    printf("Error [0]: SIGNATURE INVALID\n");
    errors++;
   } else
   if (erro==2) printf("Error[1]: CRC Failure\n");
  }
 }
}

void getfilename(void)
{
 int err,nn;
 char atl,chr[3];
 char drive[4],dir[192],name[13],ext[5];

 err=0;
 ty=wherey();
 strcpy(writname,"");
 fread(&writname,13,1,create);
 if (strcmp(writname,"**ENDOFLOCK**")==0) {
  flushall();
  printf("\n\nArchive unlocked\n\n");
  if (errors>0) printf("Errors in unlock code given\n\n");
  exit(0);
 }
 for (nn=0;nn<strlen(writname);nn++){
  if (writname[nn]<32 || writname[nn]>127) {
   printf("Error [2]: Archive error\n");
   flushall();
   exit(0);
  }
 }
 printf("%s",writname);
 gotoxy(15,ty);
 fread(&filesize,4,1,create);
 printf("%X",filesize);
 gotoxy(24,ty);
 fread(&signature[0],4,1,create);
 printf("%X",signature[0]);
 gotoxy(34,ty);
 fread(&signature[1],4,1,create);
 printf("%X",signature[1]);
 gotoxy(44,ty);
 fread(&signature[2],4,1,create);
 printf("%X",signature[2]);
 gotoxy(54,ty);
 for (num=0;num<5;num++) initial[num]=fgetc(create);
 undofile();
}

void encrypter(void)
{
 create=fopen(filename,"rb");
 if (create==NULL){
  printf("Error : Cannot access source archive [%s]\n",filename);
  window(1,1,80,25);
  exit(1);
 }
 printf("Accessing Archive\n\n");
 printf("Filename      Size     Lock1     Lock2     Lock3\n");
 fseek(create,0,SEEK_SET);
 do{
  getfilename();
 }while(feof(create)==0);
 fclose(create);
 printf("Archive complete\n\n");
}

void main(int argc,char *argv[])
{
 if (argc<2){
  printf("Lockdown\n");
  printf("Copyright (C)1989-1999 EdzUp\n\n");
  printf("Usage : LOCKDOWN archive\n\n");
  exit(0);
 }
 errors=0;
 strcpy(filename,argv[1]);
 initdisplay();
 printf("For anyone guessing you only have between 1 and 5.421893913316961726616704406192e+623\n");
 printf("to guess,this has to be done for each byte of the archive!!!!!\n\n");
 printf("Please enter the 10-255 character combination\n\r");
 printf("the file is then encrypted using this combination\n\r\n\r");
 getkeys();
 encrypter();
}