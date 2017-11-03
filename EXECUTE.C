#include <process.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dir.h>

int num,err,mode,para;
char cc,com[15],inf[192],drv[2],dir[192],fil[8],ext[4],sys[255],ss[255],filename[255],par[255];
char master[192],ch[3],*envs;
FILE *hand;

int error_window(int dii,int drive,int error,int rx,int ry)
{
 char info[80],chr[10];
 int x,y;

 printf("Execute encountered an error performing the function you required\n");
 printf("the information relating to the error is as follows:\n");
 switch(dii){
 case 0 : printf("Disk write protected, please unprotect disk and try again");break;
 case 1 : printf("Unknown device, you tried to access a device not recognised by DOS");break;
 case 2 : printf("Drive not ready, check disk is inserted and try again");break;
 case 3 : printf("Unknown command, not recognised by DOS");break;
 case 4 : printf("CRC error in data, the files requested have data errors in them");break;
 case 5 : printf("The directory structure requested was to long for DOS to understand");break;
 case 6 : printf("Seek error, DOS encountered an error accessing the information required");break;
 case 7 : printf("Unknown media type, the disk currently in the drive is not formatted\nto DOS");break;
 case 8 : printf("Sector not found, the disk had either an unknown format type or has lost\nsectors.");break;
 case 9 : printf("Printer out of paper, please check paper is inserted correctly.");break;
 case 10: printf("Write fault, there was an incorrectable error writing to the drive");break;
 case 11: printf("Read fault, there was an incorrectable error reading from the drive");break;
 case 12: printf("General failure, the disk is either corrupt or not formatted");break;
 case 15: printf("Invalid disk change, you have changed the disk. Please re-insert old disk");break;
 }
 printf("\n\n\nProcess aborted to avoid further errors\n\n");
 return(0);
}

int handler(int errval,int ax,int bp,int si)
{

 int faildev,err,rc;
 /*
    Return values
    IGNORE 0    RETRY 1   ABORT 2 (ABORT QUITS CURRENT DOS APP'DRACOS')

  þ ax indicates whether a disk error or
    other device error was encountered.

    If ax is nonnegative, a disk error was
    encountered. Otherwise, the error was a
    device error.

    For a disk error, ax ANDed with 0x00FF
    give the failing drive number (0 equals
    A, 1 equals B, and so on).

  þ bp and si together point to the device
    driver header of the failing driver. bp
    contains the segment address, and si the
    offset.
 */
 if (bp==0) printf("");
 if (si==0) printf("");
 if (ax>=0) err=0; else err=1;
 faildev=ax&0x00FF;
 rc=error_window(errval,faildev,err,220,200);
 return(rc);
}


void main(int argc,char *argv[])
{
 envs=getenv("DRACOS");
 err=0;
 textmode(3);
 clrscr();
 printf("Execute\nDRACOS Execute Kernel\nCopyright (C)1998 EdzUp Software\n\n");
 strcpy(inf,argv[0]);
 strcpy(inf,envs);
 strcat(inf,"\\");
 strcpy(sys,inf);
 strcat(sys,"EXECUTE.#$#");
 hand=fopen(sys,"rt");
 if (hand==NULL) err=1; else
 {
  mode=0;
  do{
   fgets(ss,255,hand);
   num=strlen(ss);
   ss[num-1]=NULL;
   if (mode==3 && strcmp(master,"")==0) strcpy(master,ss);
   if (mode==2 && strcmp(par,"")==0) strcpy(par,ss);
   if (mode==1 && strcmp(filename,"")==0) strcpy(filename,ss);
   if (strcmp("#CHILD",ss)==0) mode=1;
   if (strcmp("#PARAMETERS",ss)==0) mode=2;
   if (strcmp("#MASTER",ss)==0) mode=3;
  }while(feof(hand)==0);
  fclose(hand);
 }
 printf("Executing '%s'\nParameters '%s'\n\n",filename,par);
 strcpy(ss,filename);
 strcat(ss," ");
 strcat(ss,par);
 system(ss);
 if (num==-1){
  switch(errno){
  case E2BIG : printf("To many parameters\n");break;
  case EINVAL: printf("Invalid argument\n");break;
  case ENOENT: printf("Path or Filename not found\n");break;
  case ENOEXEC: printf("Cannot execute file\n");break;
  case ENOMEM: printf("Not enough core memory\n");break;
  }
 }
 if (err==1) printf("Cannot locate EXECUTE registry file\n");
 textmode(3);
 printf("\n:Press any key to return to %s\n",master);
 getch();
 chdir(inf);
 strcpy(ss,inf);
 strcat(ss,master);
 execvp(ss,NULL);
}