//Dani program je portabilan za oba operacijska sustava. Dani program generira datoteku veličine 1GB. 
//Za datoteku od 1MB potrebno je vrijednost NBUFS postaviti na 1024, a za datoteku od 1KB potrebno je 
//NBUFS staviti na 1. S ovim programom nije moguće napraviti 'veliku' datoteku. Zašto?

#include <stdio.h>
#include <stdlib.h>
#define BUF_SIZE 1024
#define NBUFS 1024*1024
#define _CRT_SECURE_NO_DEPRECATE

int main()
{
  long i; char buf[BUF_SIZE];
  FILE *f;
  f = fopen( "test.txt", "wb" );
  for (i=0; i<BUF_SIZE; i++) buf[i] = 30+rand()%100;

  for (i=0; i<NBUFS; i++)
	  fwrite( buf, BUF_SIZE, 1, f );
  fclose( f );
  return 0;
}