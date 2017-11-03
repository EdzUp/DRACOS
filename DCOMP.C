#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>

char file1[255],file2[255],ch1,ch2,ch;
char chin[255];
FILE *in,*out;
int  val,ln;
unsigned long pos,che;

void main(void)
{
 printf("DRACOS Compare program (DCOMP.EXE) v1.1\n");
 printf("Copyright (C)1998-1999 EdzUp\n\n");

 printf("Please enter the files you wish to compare:\n");
 printf("File 1 :");
 gets(file1);
 printf("File 2 :");
 gets(file2);
 if (strcmp(file1,file2)==0) {
  printf("The filenames are identical, please try another file\n");
  exit(0);
 }
 printf("Do you wish to look for a specific byte that has changed [Y]es or [N]o?");
 do{
  ch=getch();
 }while(ch!='y' && ch!='Y' && ch!='n' && ch!='N');
 if (ch=='Y' || ch=='y'){
  printf("%c\n\n",ch);
  printf("Please enter the DECIMAL value for the character to search for\n");
  gets(chin);
  val=atoi(chin);
 } else {
  val=-1;
  printf("%c\n",ch);
 }
 pos=0;
 che=0;
 ln=0;
 in=fopen(file1,"rb");
 if (in==NULL){
  printf("Error accessing file '%s'\n",file1);
  exit(0);
 }
 out=fopen(file2,"rb");
 if (out==NULL){
  printf("Error accessing file '%s'\n",file2);
  fclose(in);
 }
 pos=0;
 che=0;
 ln=0;
 do{
  ch1=fgetc(in);
  ch2=fgetc(out);
  if (ch1!=ch2){
   if (val==-1 || ch1==val){
    printf("Alteration. From");
    gotoxy(18,wherey());
    if (ch1<16) printf("0%X",ch1); else printf("%X",ch1);
    gotoxy(22,wherey());
    if (ch2<16) printf("to 0%X",ch2); else printf("to %X",ch2);
    gotoxy(30,wherey());
    printf("Location : Dec [%lu]",pos);
    gotoxy(58,wherey());
    printf("Hex [%X]",pos);
    printf("\n");
    ln++;
    che++;
    if (ln>22){
     printf("Press ENTER to continue or ESC to quit\n");
     do{
      ch=getch();
     }while(ch!=13 && ch!=27);
     if (ch==27){
      fclose(in);
      fclose(out);
      exit(0);
     }
     ln=0;
    }
   }
  }
  pos++;
 }while(feof(in)==0 && feof(out)==0);
 if (feof(in)==0) printf("Reached end of file '%s'\n",file1);
 if (feof(out)==0) printf("Reached end of file '%s'\n",file2);
 fclose(in);
 fclose(out);
}