/*
 * CMOSUTL.C - Utilities for reading or writing CMOS
 */
unsigned char GetCMOS( char i )          /* reads value from cell i */
{ unsigned char retval;
  outportb( 0x70, i );
  retval = inportb( 0x71 );
  return retval;
}

void PutCMOS( char i, char v )          /* writes value v to cell i */
{ outportb( 0x70, i );
  outportb( 0x71, v );
}

