#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>

struct fatinfo fatinf;

/*
struct fatinfo {
  char  fi_sclus;  // sectors per cluster
  char  fi_fatid;  // the FAT id byte
  int   fi_nclus;  // number of clusters
  int   fi_bysec;  // bytes per sector
};
*/

int image,num;

void main(int argc,char *argv[])
{
 textmode(3);
 clrscr();
 printf("DRACOS DCOPY\n");
 printf("v0.1 (C)Copyright 1999 EdzUp Software\n\n");
 if (argc<2){
  printf("USAGE : DCOPY [drive1] [drive2] [[/IMAGE] FILENAME] \n\n");
  printf("   drive1     This is the source drive letter (i.e. A for A:)\n");
  printf("   drive2     This is the destination drive letter\n\n");
  printf("   IMAGE      This will create a disk image of the drive1\n");
  printf("   FILENAME   This is the full path and filename of the new image\n");
  exit(0);
 }
 printf("Copy from %s\n",argv[1]);
 printf("Copy to %s\n",argv[2]);
 image=0;
/* for (num=0;num<argc;num++){
  printf("%i:%s!!!\n",num,argv[num]);
 }*/
 if (strcmp(argv[3],"IMAGE")==0 || strcmp(argv[3],"image")==0) {
  printf("Transfer Disk to IMAGE\n");
  printf("Image file %s\n",argv[4]);
  printf("Image from drive %s:\n",argv[1]);
  image=1;
 }
 absread
 printf("drive #%i\n",argv[1][0]-64);
 getfatd(argv[1][0]-64,&fatinf);
 printf("Drive layout for drive %i\n",argv[1][0]-64);
 printf("%i Disk FAT ID byte\n",fatinf.fi_fatid);
 printf("%5d sectors per cluster\n",fatinf.fi_sclus);
 printf("%5d number of clusters\n",fatinf.fi_nclus);
 printf("%5d Bytes per sector\n",fatinf.fi_bysec);
 printf("  sectors per cluster %5d\n", fatinf.fi_sclus);
 printf("   number of clusters %5d\n", fatinf.fi_nclus);
 printf("     bytes per sector %5d\n", fatinf.fi_bysec);

}