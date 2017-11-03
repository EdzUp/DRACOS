/*
includes for this file
stdio.h,conio.h,stdlib.h,math.h,string.h
*/

unsigned char *Screen = (unsigned char *)0xA0000000L;
unsigned char *Screen2;
unsigned char *Buffer;

void VGAinit ()
{
 asm mov ah,0
 asm mov al,0x13
 asm int 0x10

  Screen2=(unsigned char *)malloc(64000);
  if (Screen2==NULL){
   printf("Video memory error\n\n");
   exit(0);
  }
  Buffer = Screen2;
}

void SVGAinit (unsigned char mode)
{
 asm mov ah,0
 asm mov al,mode
 asm int 0x10

  Screen2=(unsigned char *)malloc(307200);
  if (Screen2==NULL){
   printf("Video memory error\n\n");
   exit(0);
  }
  Buffer = Screen2;
}


//This resets the display to text mode
void Textinit ()
{
 asm mov ah,0
 asm mov al,0x03
 asm int 0x10

 free(Screen2);
}

//Sets a pixel in the buffer to a specific colour
void PutPixel (int x, int y, char c)
{
 if (x>-1 && y>-1 && x<320 && y<200) Buffer [(y << 8) + (y << 6) + x] = c;
}

void clearscreen(void)
{
 unsigned int clscnt;

 for (clscnt=0;clscnt<64000;clscnt++) Screen2[clscnt]=0;
}

void viewpage(void)
{
 memcpy(Screen,Screen2,64000);
}



//Draws a line between two points
void DrawLine (int x1, int y1, int x2, int y2, unsigned char c)
{
  int   i, deltax, deltay, numpixels,
        d, dinc1, dinc2,
        x, xinc1, xinc2,
        y, yinc1, yinc2;

  //Calculate deltax and deltay for initialisation
  deltax = abs (x2 - x1);
  deltay = abs (y2 - y1);

  //Initialize all vars based on which is the independent variable
  if (deltax >= deltay) {
    //x is independent variable
    numpixels = deltax + 1;
    d = (deltay << 1) - deltax;
    dinc1 = deltay << 1;
    dinc2 = (deltay - deltax) << 1;
    xinc1 = xinc2 = yinc2 = 1;
    yinc1 = 0;
  } else {
    //y is independent variable
    numpixels = deltay + 1;
    d = (deltax << 1) - deltay;
    dinc1 = deltax << 1;
    dinc2 = (deltax - deltay) << 1;
    xinc1 = 0;
    xinc2 = yinc1 = yinc2 = 1;
  }

  //Make sure x and y move in the right directions
  if (x1 > x2) {
    xinc1 = -xinc1;
    xinc2 = -xinc2;
  }
  if (y1 > y2) {
    yinc1 = - yinc1;
    yinc2 = - yinc2;
  }

  //Start drawing at <x1, y1>
  x = x1;
  y = y1;

  //Draw the pixels
  for (i = 0; i < numpixels; i++) {
    PutPixel (x, y, c);
    if (d < 0) {
      d += dinc1;
      x += xinc1;
      y += yinc1;
    } else {
      d += dinc2;
      x += xinc2;
      y += yinc2;
    }
  }
}

void DrawHorLine (int x1, int y, int x2, char c)
{
  int offset, length;

  //Clip to vertical screen size
  if (y < 0 || y > 200) return;

  //Swap endpoints if necessary
  if (x1 > x2) { length = x2; x2 = x1; x1 = length; }

  //Clip to horizontal screen size
  if (x2 < 0) x2=0;
  if (x1 > 319) x1=319;
  if (x1 < 0) x1 = 0;
  if (x2 > 319) x2 = 319;

  //Calculate address and length
  offset = (y << 8) + (y << 6) + x1;
  length = x2 - x1 + 1;

  //Draw the line
  memset (Buffer + offset, c, length);
}

//Draws a filled circle with the specified centerpoint and radius
void DrawSolidCircle (int centerx, int centery, int radius, char colour)
{
  int line, position;
  int x, y;
  long sqrad;

  //No radius is a single pixel
  if (radius < 1) {
    PutPixel (centerx, centery, colour);
    return;
  }

  //Square radius
  sqrad = radius * radius;

  //Center line
  DrawHorLine (centerx - radius, centery, centerx + radius, colour);

  //Rest of the circle
  for (line = 1; line < radius; line++) {
    //Calculate position
    position = sqrt (sqrad - (line * line));

    //Draw horizontal lines, taking advantage of circle's perfect symmetry
    DrawHorLine (centerx - position, centery - line, centerx + position, colour);
    DrawHorLine (centerx - position, centery + line, centerx + position, colour);
  }
}

void Capture(int tx,int ty,int bx,int by,char *array)
{
 int xsize,ysize,xcount,ycount;
 unsigned char dot;
 unsigned int pos;
 xsize=bx-tx;
 ysize=by-ty;

 pos=0;
 for (ycount=ty;ycount<by;ycount++){
  for (xcount=tx;xcount<bx;xcount++){
   dot=Screen2[(ycount*320)+xcount];
   array[pos]=dot;
   pos++;
  }
 }
}

void drawbox(int tx,int ty,int bx,int by,int col,int fill)
{
 int ysize;

 DrawLine(tx,ty,bx-1,ty,col);
 DrawLine(tx,ty,tx,by-1,col);
 DrawLine(tx,by-1,bx-1,by-1,col);
 DrawLine(bx-1,ty,bx-1,by-1,col);
 if (fill!=-1){
  for (ysize=ty+1;ysize<by-1;ysize++) DrawLine(tx+1,ysize,bx-2,ysize,fill);
 }
}

void paste(int tx,int ty,int width,int height,int deltax,int deltay,char *array)
{
 int xsize,ysize,pstx,psty;

 for (ysize=0;ysize<height;ysize++){
  for (xsize=0;xsize<width;xsize++){
   if (deltax>1 || deltay>1){
    if ((xsize*deltax)+tx<320 && (ysize*deltay)+ty<200 && (xsize*deltax)+tx>-1 && (ysize*deltay)+ty>-1){
     drawbox((xsize*deltax)+tx,(ysize*deltay)+ty,(xsize*deltax)+(tx+deltax),(ysize*deltay)+(ty+deltay),array[(width*ysize)+xsize],array[(width*ysize)+xsize]);
    }
   } else
   {
    if (xsize+tx>-1 && ysize+ty>-1 && xsize+tx<320 && ysize+ty<200){
     PutPixel(xsize+tx,ysize+ty,array[(width*ysize)+xsize]);
    }
   }
  }
 }
}

void SetDAC (unsigned char DAC, unsigned char R, unsigned char G, unsigned char B)
{
  outportb (0x3C8, DAC);
  outportb (0x3C9, R);
  outportb (0x3C9, G);
  outportb (0x3C9, B);
}

void LoadBMP (char *FileName)
{
  //BMP Header structure
  struct BMPHeader {
    #ifdef __DJGPP
    unsigned short bfType __attribute__((packed));
    long           bfSize __attribute__((packed));
    long           bfReserved __attribute__((packed));
    long           bfOffBits __attribute__((packed));
    long           biSize __attribute__((packed));
    long           biWidth __attribute__((packed));
    long           biHeight __attribute__((packed));
    unsigned short biPlanes __attribute__((packed));
    unsigned short biBitCount __attribute__((packed));
    long           biCompression __attribute__((packed));
    long           biSizeImage __attribute__((packed));
    long           biXPelsPerMeter __attribute__((packed));
    long           biYPelsPerMeter __attribute__((packed));
    long           biClrUsed __attribute__((packed));
    long           biClrImportant __attribute__((packed));
    #else
    unsigned short bfType;
    long           bfSize,
                   bfReserved,
                   bfOffBits,
                   biSize,
                   biWidth,
                   biHeight;
    unsigned short biPlanes,
                   biBitCount;
    long           biCompression,
                   biSizeImage,
                   biXPelsPerMeter,
                   biYPelsPerMeter,
                   biClrUsed,
                   biClrImportant;
    #endif
  } Header;
  //File handle
  FILE *BMPFile;
  //Required for palette
  unsigned char c, Palette[256][4];
  //USed for loading into video memory
  unsigned int offset, lines, paddedWidth;

  //Check to see whether the file exists and can be opened
  BMPFile = fopen (FileName, "rb");
  if (BMPFile == NULL)  {
    strcat (FileName,".BMP");
    BMPFile = fopen (FileName, "rb");
    if (BMPFile == NULL) {
      printf ("Couldn't open file.");
      return;
    }
  }

  //Read in header information
  fread (&Header, 54, 1, BMPFile);

  //Check to see whether we can display it
  if (Header.bfType != 19778 || Header.bfReserved != 0 || Header.biPlanes !=1) {
    //Not a valid bitmap file - don't display
    fclose (BMPFile);
    return;
  }
  if (Header.biCompression != 0) {
    //Compressed file - don't display
    fclose (BMPFile);
    return;
  }
  if (Header.biBitCount != 8) {
    //Other than 8-bit colour - don't display
    fclose (BMPFile);
    return;
  }
  if (Header.biWidth > 320 || Header.biHeight > 200) {
    //Too large - don't display
    fclose (BMPFile);
    return;
  }

  //Load in the palette
  fread (&Palette, 1024, 1, BMPFile);

  //Set appropriate position to display graphics data
  offset = (100 + (Header.biHeight >> 1)) * 320 + 160 - (Header.biWidth >> 1);

  //We've read no lines so far
  lines = 0;

  //Pad line length to 4bytes
  paddedWidth = Header.biWidth & 0xFFFC;
  if (Header.biWidth != paddedWidth) paddedWidth += 4;

  //Decode and display graphics
  while (lines < Header.biHeight) {
    //Read next line
    fread (Screen + offset, paddedWidth, 1, BMPFile);
    //Move up one line on the screen
    offset -= 320;
    //increase amount of lines read
    lines++;
  }
  fclose (BMPFile);
}

unsigned char getpixelinfo(unsigned int x,unsigned int y)
{
 char dot;

 asm mov ah,0x0D
 asm mov cx,x
 asm mov dx,y
 asm int 0x10
 asm mov dot,al

 return dot;
}

void capturebitmap(void)
{
 unsigned int pos;
 int cbx,cby;

 pos=0;
 for (cby=0;cby<200;cby++){
  for (cbx=0;cbx<320;cbx++){
   Screen2[pos]=getpixelinfo(cbx,cby);
   pos++;
  }
 }
}

void savebitmapdata(char filename[255])
{
 FILE *handel;
 unsigned int pos;

 handel=fopen(filename,"wb");
 if (handel==NULL){
  printf("Cannot create bitmap data file for saving\n\n");
  exit(0);
 }
 for (pos=0;pos<64000;pos++) fputc(Screen2[pos],handel);
 fclose(handel);
}

void loadbitmapdata(char filename[255])
{
 FILE *handel;
 unsigned int pos;

 handel=fopen(filename,"rb");
 if (handel==NULL){
  printf("Cannot create bitmap data file for reading\n\n");
  exit(0);
 }
 for (pos=0;pos<64000;pos++) Screen2[pos]=fgetc(handel);
 fclose(handel);
}

void smalltext(int c,int tx,int ty,char str[80])
{
 /*
  This system give a 80 x 33 character screen
 */
 int ttx,tty,len;

 ttx=tx-1;
 tty=ty-1;
 {
  for (len=0;len<strlen(str);len++){
	 if (ttx>-1 && ttx<318 && tty>-1 && tty<198){
	 switch(toupper(str[len])){
	  case 65 : {
		     PutPixel(ttx+2,tty+1,c);
		     DrawLine(ttx+1,tty+2,ttx+1,tty+5,c);
		     DrawLine(ttx+3,tty+2,ttx+3,tty+5,c);
		     PutPixel(ttx+2,tty+3,c);
		    }break;
	  case 66 : {
		     DrawLine(ttx+1,tty+1,ttx+1,tty+5,c);
		     PutPixel(ttx+2,tty+1,c);
		     PutPixel(ttx+2,tty+3,c);
		     PutPixel(ttx+2,tty+5,c);
		     PutPixel(ttx+3,tty+2,c);
		     PutPixel(ttx+3,tty+4,c);
		    }break;
	  case 67 : {
		     DrawLine(ttx+1,tty+2,ttx+1,tty+4,c);
		     PutPixel(ttx+2,tty+1,c);
		     PutPixel(ttx+2,tty+5,c);
		     PutPixel(ttx+3,tty+2,c);
		     PutPixel(ttx+3,tty+4,c);
		    }break;
	  case 68 : {
		     DrawLine(ttx+1,tty+1,ttx+1,tty+5,c);
		     DrawLine(ttx+3,tty+2,ttx+3,tty+4,c);
		     PutPixel(ttx+2,tty+1,c);
		     PutPixel(ttx+2,tty+5,c);
		    }break;
	  case 69 : {
		     DrawLine(ttx+1,tty+1,ttx+1,tty+5,c);
		     DrawLine(ttx+1,tty+1,ttx+3,tty+1,c);
		     DrawLine(ttx+1,tty+5,ttx+3,tty+5,c);
		     PutPixel(ttx+2,tty+3,c);
		    }break;
	  case 70 : {
		     DrawLine(ttx+1,tty+1,ttx+1,tty+5,c);
		     DrawLine(ttx+1,tty+1,ttx+3,tty+1,c);
		     PutPixel(ttx+2,tty+3,c);
		    }break;
	  case 71 : {
		     DrawLine(ttx+1,tty+2,ttx+1,tty+4,c);
		     DrawLine(ttx+2,tty+1,ttx+3,tty+1,c);
		     PutPixel(ttx+2,tty+5,c);
		     DrawLine(ttx+3,tty+3,ttx+3,tty+4,c);
		    }break;
	  case 72 : {
		     DrawLine(ttx+1,tty+1,ttx+1,tty+5,c);
		     DrawLine(ttx+3,tty+1,ttx+3,tty+5,c);
		     PutPixel(ttx+2,tty+3,c);
		    }break;
	  case 73 : {
		     DrawLine(ttx+2,tty+1,ttx+2,tty+4,c);
		     DrawLine(ttx+1,tty+1,ttx+3,tty+1,c);
		     DrawLine(ttx+1,tty+5,ttx+3,tty+5,c);
		    }break;
	  case 74 : {
		     DrawLine(ttx+3,tty+1,ttx+3,tty+4,c);
		     DrawLine(ttx+1,tty+1,ttx+3,tty+1,c);
		     DrawLine(ttx+1,tty+5,ttx+2,tty+5,c);
		    }break;
	  case 75 : {
		     DrawLine(ttx+1,tty+1,ttx+1,tty+5,c);
		     DrawLine(ttx+1,tty+3,ttx+3,tty+5,c);
		     DrawLine(ttx+1,tty+3,ttx+3,tty+1,c);
		    }break;
	  case 76 : {
		     DrawLine(ttx+1,tty+1,ttx+1,tty+5,c);
		     DrawLine(ttx+1,tty+5,ttx+3,tty+5,c);
		    }break;
	  case 77 : {
		     DrawLine(ttx+1,tty+1,ttx+1,tty+5,c);
		     DrawLine(ttx+3,tty+1,ttx+3,tty+5,c);
		     PutPixel(ttx+2,tty+2,c);
		    }break;
	  case 78 : {
		     DrawLine(ttx+1,tty+1,ttx+1,tty+5,c);
		     DrawLine(ttx+3,tty+1,ttx+3,tty+5,c);
		     PutPixel(ttx+2,tty+1,c);
		    }break;
	  case 48 :
	  case 79 : {
		     DrawLine(ttx+1,tty+2,ttx+1,tty+4,c);
		     DrawLine(ttx+3,tty+2,ttx+3,tty+4,c);
		     PutPixel(ttx+2,tty+1,c);
		     PutPixel(ttx+2,tty+5,c);
		    }break;
	  case 80 : {
		     DrawLine(ttx+1,tty+1,ttx+1,tty+5,c);
		     PutPixel(ttx+2,tty+1,c);
		     PutPixel(ttx+2,tty+3,c);
		     PutPixel(ttx+3,tty+2,c);
		    }break;
	  case 81 : {
		     DrawLine(ttx+1,tty+2,ttx+1,tty+4,c);
		     DrawLine(ttx+3,tty+2,ttx+3,tty+4,c);
		     PutPixel(ttx+2,tty+1,c);
		     PutPixel(ttx+2,tty+5,c);
		     DrawLine(ttx+3,tty+5,ttx+2,tty+4,c);
		    }break;
	  case 82 : {
		     DrawLine(ttx+1,tty+1,ttx+1,tty+5,c);
		     PutPixel(ttx+2,tty+1,c);
		     PutPixel(ttx+2,tty+3,c);
		     PutPixel(ttx+3,tty+2,c);
		     DrawLine(ttx+2,tty+3,ttx+3,tty+5,c);
		    }break;
	  case 83 : {
		     DrawLine(ttx+2,tty+1,ttx+3,tty+1,c);
		     PutPixel(ttx+1,tty+2,c);
		     PutPixel(ttx+2,tty+3,c);
		     PutPixel(ttx+3,tty+4,c);
		     DrawLine(ttx+1,tty+5,ttx+2,tty+5,c);
		    }break;
	  case 84 : {
		     DrawLine(ttx+1,tty+1,ttx+3,tty+1,c);
		     DrawLine(ttx+2,tty+2,ttx+2,tty+5,c);
		    }break;
	  case 85 : {
		     DrawLine(ttx+1,tty+1,ttx+1,tty+5,c);
		     DrawLine(ttx+3,tty+1,ttx+3,tty+5,c);
		     PutPixel(ttx+2,tty+5,c);
		    }break;
	  case 86 : {
		     DrawLine(ttx+1,tty+1,ttx+1,tty+4,c);
		     DrawLine(ttx+3,tty+1,ttx+3,tty+4,c);
		     PutPixel(ttx+2,tty+5,c);
		    }break;
	  case 87 : {
		     DrawLine(ttx+1,tty+1,ttx+1,tty+5,c);
		     DrawLine(ttx+3,tty+1,ttx+3,tty+5,c);
		     PutPixel(ttx+2,tty+4,c);
		    }break;
	  case 88 : {
		     DrawLine(ttx+1,tty+1,ttx+1,tty+2,c);
		     DrawLine(ttx+1,tty+4,ttx+1,tty+5,c);
		     DrawLine(ttx+3,tty+1,ttx+3,tty+2,c);
		     DrawLine(ttx+3,tty+4,ttx+3,tty+5,c);
		     PutPixel(ttx+2,tty+3,c);
		    }break;
	  case 89 : {
		     DrawLine(ttx+1,tty+1,ttx+1,tty+3,c);
		     DrawLine(ttx+3,tty+1,ttx+3,tty+3,c);
		     DrawLine(ttx+2,tty+3,ttx+2,tty+5,c);
		    }break;
	  case 90 : {
		     DrawLine(ttx+1,tty+1,ttx+3,tty+1,c);
		     DrawLine(ttx+1,tty+5,ttx+3,tty+5,c);
		     PutPixel(ttx+3,tty+2,c);
		     PutPixel(ttx+2,tty+3,c);
		     PutPixel(ttx+1,tty+4,c);
		    }break;
	  case 49 : {
		     DrawLine(ttx+2,tty+1,ttx+2,tty+4,c);
		     DrawLine(ttx+1,tty+5,ttx+3,tty+5,c);
		     PutPixel(ttx+1,tty+1,c);
		    }break;
	  case 50 : {
		     DrawLine(ttx+1,tty+1,ttx+2,tty+1,c);
		     DrawLine(ttx+1,tty+5,ttx+3,tty+5,c);
		     PutPixel(ttx+3,tty+2,c);
		     PutPixel(ttx+2,tty+3,c);
		     PutPixel(ttx+1,tty+4,c);
		    }break;
	  case 51 : {
		     DrawLine(ttx+1,tty+1,ttx+2,tty+1,c);
		     DrawLine(ttx+1,tty+5,ttx+2,tty+5,c);
		     PutPixel(ttx+3,tty+2,c);
		     PutPixel(ttx+2,tty+3,c);
		     PutPixel(ttx+3,tty+4,c);
		    }break;
	  case 52 : {
		     DrawLine(ttx+3,tty+1,ttx+3,tty+5,c);
		     DrawLine(ttx+1,tty+4,ttx+3,tty+4,c);
		     DrawLine(ttx+1,tty+3,ttx+3,tty+1,c);
		    }break;
	  case 53 : {
		     DrawLine(ttx+1,tty+1,ttx+3,tty+1,c);
		     DrawLine(ttx+1,tty+5,ttx+2,tty+5,c);
		     PutPixel(ttx+3,tty+4,c);
		     DrawLine(ttx+1,tty+3,ttx+2,tty+3,c);
		     PutPixel(ttx+1,tty+2,c);
		    }break;
	  case 54 : {
		     PutPixel(ttx+2,tty+1,c);
		     DrawLine(ttx+1,tty+2,ttx+1,tty+4,c);
		     PutPixel(ttx+2,tty+5,c);
		     PutPixel(ttx+2,tty+3,c);
		     PutPixel(ttx+3,tty+4,c);
		    }break;
	  case 55 : {
		     DrawLine(ttx+1,tty+1,ttx+3,tty+1,c);
		     PutPixel(ttx+3,tty+2,c);
		     DrawLine(ttx+2,tty+3,ttx+2,tty+5,c);
		    }break;
	  case 56 : {
		     PutPixel(ttx+2,tty+1,c);
		     PutPixel(ttx+2,tty+3,c);
		     PutPixel(ttx+2,tty+5,c);
		     PutPixel(ttx+1,tty+2,c);
		     PutPixel(ttx+1,tty+4,c);
		     PutPixel(ttx+3,tty+2,c);
		     PutPixel(ttx+3,tty+4,c);
		    }break;
	  case 57 : {
		     PutPixel(ttx+2,tty+5,c);
		     PutPixel(ttx+2,tty+1,c);
		     PutPixel(ttx+1,tty+2,c);
		     PutPixel(ttx+2,tty+3,c);
		     DrawLine(ttx+3,tty+2,ttx+3,tty+4,c);
		    }break;
	  case 46 : PutPixel(ttx+2,tty+5,c);break;
	  case 47 : {
		     PutPixel(ttx+3,tty+1,c);
		     PutPixel(ttx+2,tty+2,c);
		     PutPixel(ttx+1,tty+3,c);
		     PutPixel(ttx,tty+4,c);
		     PutPixel(ttx-1,tty+5,c);
		    } break;
	  case 92 : {
		     PutPixel(ttx-1,tty+1,c);
		     PutPixel(ttx,tty+2,c);
		     PutPixel(ttx+1,tty+3,c);
		     PutPixel(ttx+2,tty+4,c);
		     PutPixel(ttx+3,tty+5,c);
		    } break;
	  case 42 : {
		     DrawLine(ttx+1,tty+3,ttx+3,tty+3,c);
		     PutPixel(ttx+1,tty+1,c);
		     PutPixel(ttx+3,tty+1,c);
		     PutPixel(ttx+2,tty+2,c);
		     PutPixel(ttx+2,tty+4,c);
		     PutPixel(ttx+1,tty+5,c);
		     PutPixel(ttx+3,tty+5,c);
		    }break;
	  case 33 : {
		     DrawLine(ttx+2,tty+1,ttx+2,tty+3,c);
		     PutPixel(ttx+2,tty+5,c);
		    }break;
	  case 58 : {
		     PutPixel(ttx+2,tty+2,c);
		     PutPixel(ttx+2,tty+4,c);
		    }break;
	  case 43 : {
		     DrawLine(ttx+2,tty+2,ttx+2,tty+4,c);
		     DrawLine(ttx+1,tty+3,ttx+3,tty+3,c);
		    }break;
	  case 61 : {
		     DrawLine(ttx+1,tty+2,ttx+3,tty+2,c);
		     DrawLine(ttx+1,tty+4,ttx+3,tty+4,c);
		    }break;
	  case 45 : {
		     DrawLine(ttx+1,tty+3,ttx+3,tty+3,c);
		    }break;
	  case 63 : {
		     DrawLine(ttx+1,tty+1,ttx+2,tty+1,c);
		     PutPixel(ttx+3,tty+2,c);
		     DrawLine(ttx+1,tty+3,ttx+2,tty+3,c);
		     PutPixel(ttx+1,tty+5,c);
		    }break;
	  case 59 : {
		     PutPixel(ttx+2,tty+2,c);
		     PutPixel(ttx+2,tty+4,c);
		     PutPixel(ttx+1,tty+5,c);
		    }break;
	  case 44 : {
		     PutPixel(ttx+2,tty+4,c);
		     PutPixel(ttx+1,tty+5,c);
		    }break;
	  case 41 : {
		     PutPixel(ttx+1,tty+1,c);
		     PutPixel(ttx+1,tty+5,c);
		     DrawLine(ttx+2,tty+2,ttx+2,tty+4,c);
		    }break;
	  case 40 : {
		     PutPixel(ttx+3,tty+1,c);
		     PutPixel(ttx+3,tty+5,c);
		     DrawLine(ttx+2,tty+2,ttx+2,tty+4,c);
		    }break;
	  case 91 : {
		     DrawLine(ttx+2,tty+1,ttx+2,tty+5,c);
		     PutPixel(ttx+3,tty+1,c);
		     PutPixel(ttx+3,tty+5,c);
		    }break;
	  case 93 : {
		     PutPixel(ttx+1,tty+1,c);
		     DrawLine(ttx+2,tty+1,ttx+2,tty+5,c);
		     PutPixel(ttx+1,tty+5,c);
		    }break;
	  case 39 : {
		     PutPixel(ttx+2,tty+1,c);
		     PutPixel(ttx+2,tty+2,c);
		    }break;
	 }
	 }
	 ttx=ttx+4;
  }
 }
}

void DrawPoly(int numpoints,int *array,int col,int fill)
{
 int cp,cx,cy;

 cx=array[0];
 cy=array[1];
 for (cp=2;cp<numpoints*2;cp+=2){
  DrawLine(cx,cy,array[cp],array[cp+1],col);
  cx=array[cp];
  cy=array[cp+1];
 }
 DrawLine(cx,cy,array[0],array[1],col);
}

//Draws a line between two points
int Distance (int x1, int y1, int x2, int y2)
{
  int   i, deltax, deltay, numpixels,
	d, dinc1, dinc2,
	x, xinc1, xinc2,
	y, yinc1, yinc2;

  //Calculate deltax and deltay for initialisation
  deltax = abs (x2 - x1);
  deltay = abs (y2 - y1);

  //Initialize all vars based on which is the independent variable
  if (deltax >= deltay) {
    //x is independent variable
    numpixels = deltax + 1;
    d = (deltay << 1) - deltax;
    dinc1 = deltay << 1;
    dinc2 = (deltay - deltax) << 1;
    xinc1 = xinc2 = yinc2 = 1;
    yinc1 = 0;
  } else {
    //y is independent variable
    numpixels = deltay + 1;
    d = (deltax << 1) - deltay;
    dinc1 = deltax << 1;
    dinc2 = (deltax - deltay) << 1;
    xinc1 = 0;
    xinc2 = yinc1 = yinc2 = 1;
  }

  //Make sure x and y move in the right directions
  if (x1 > x2) {
    xinc1 = -xinc1;
    xinc2 = -xinc2;
  }
  if (y1 > y2) {
    yinc1 = - yinc1;
    yinc2 = - yinc2;
  }

  //Start drawing at <x1, y1>
  x = x1;
  y = y1;

  return numpixels;
}
