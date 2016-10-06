/*
 * Copyright (c) 2008 Serge 
 * All Rights Reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <cpm.h>
#include "dsk.h"
#include "idebdos.h"
#include "integer.h"
#include "fdisk.h"

extern int in;				/* Source File Descriptor */
extern int out;				/* Destination File Descriptor */
extern int byte_tx;			/* Bytes Transfered */
extern BYTE buffer_in[sector_size];	/* Source Buffer */
extern BYTE buffer_out[sector_size];    /* Destination Buffer */
extern BOOL AltairDos;

static BYTE ZBootLoader[]={
0xC3, 0x08, 0x00, 0xEE, 0x00, 0x00, 0x00, 0x00, 0x0E, 0xFF, 0x11, 0x1F, 0xF3, 0xAF, 0x12, 0x0C, 
0x79, 0xFE, 0x04, 0x28, 0x5F, 0x07, 0x07, 0x07, 0x07, 0xC6, 0xC2, 0x26, 0x01, 0x6F, 0x3E, 0x52, 
0xBE, 0x20, 0xEA, 0x23, 0x23, 0x23, 0x23, 0xE5, 0xD9, 0xE1, 0x11, 0x04, 0x00, 0x01, 0x08, 0x00, 
0xED, 0xB0, 0x21, 0x07, 0x00, 0x1E, 0x20, 0xCD, 0x34, 0xF8, 0xC0, 0xD9, 0x21, 0x00, 0xEE, 0x3E, 
0xC3, 0xBE, 0x20, 0xC9, 0x3E, 0x66, 0x06, 0x1F, 0x86, 0x23, 0x10, 0xFC, 0xBE, 0x20, 0xBE, 0x23, 
0x3A, 0x01, 0xEE, 0xFE, 0x20, 0x20, 0x03, 0x21, 0xB0, 0x00, 0xC5, 0x06, 0x10, 0xCD, 0xA6, 0x00, 
0x23, 0x10, 0xFA, 0x21, 0x04, 0x00, 0x0E, 0x08, 0xED, 0xB0, 0xC1, 0x79, 0x12, 0x13, 0x21, 0x9B, 
0x01, 0x34, 0x18, 0x99, 0x3A, 0x1F, 0xF3, 0xB7, 0xC8, 0x21, 0x07, 0x00, 0x11, 0xEC, 0x00, 0xCD, 
0x34, 0xF8, 0xC0, 0x21, 0xC0, 0x00, 0x11, 0x00, 0xF2, 0xD5, 0x01, 0xF6, 0x00, 0xED, 0xB0, 0x21, 
0x36, 0xEE, 0x11, 0xF6, 0xF2, 0x06, 0x14, 0x23, 0xCD, 0xA6, 0x00, 0x2B, 0xCD, 0xA6, 0x00, 0x23, 
0x23, 0x10, 0xF4, 0xAF, 0x12, 0xC9, 0x7E, 0xFE, 0x60, 0x38, 0x02, 0xE6, 0x5F, 0x12, 0x13, 0xC9, 
0x43, 0x50, 0x2F, 0x4D, 0x20, 0x38, 0x30, 0x20, 0x4E, 0x4F, 0x20, 0x4C, 0x41, 0x42, 0x45, 0x4C, 
0xD3, 0xF9, 0x06, 0x41, 0x21, 0xDD, 0xF2, 0xCD, 0xCD, 0xF2, 0x36, 0x0C, 0xCD, 0xBA, 0xF2, 0x3A, 
0xDC, 0xF2, 0xCD, 0x15, 0xF8, 0xCD, 0xBA, 0xF2, 0x21, 0x1F, 0xF3, 0x0E, 0x01, 0x7E, 0xB7, 0x28, 
0x27, 0xCD, 0xBA, 0xF2, 0x3A, 0xDA, 0xF2, 0xB9, 0x3E, 0x7F, 0xF5, 0xCC, 0xC5, 0xF2, 0xCD, 0xC3, 
0xF2, 0x06, 0x10, 0xCD, 0xCD, 0xF2, 0xCD, 0xC3, 0xF2, 0xF1, 0xCC, 0xC5, 0xF2, 0x11, 0x19, 0x00, 
0x19, 0x0C, 0x3A, 0xDB, 0xF2, 0xB9, 0x30, 0xD5, 0x11, 0x00, 0x18, 0xD5, 0xCD, 0x1B, 0xF8, 0xD1, 
0x21, 0xDC, 0xF2, 0x3C, 0x20, 0x0A, 0x1B, 0x7A, 0xB3, 0x20, 0xF0, 0x35, 0x3E, 0x0E, 0x20, 0x0F, 
0x36, 0x05, 0x21, 0xDA, 0xF2, 0xFE, 0x1A, 0x20, 0x09, 0x3E, 0x01, 0xBE, 0x30, 0x01, 0x35, 0xC3, 
0x02, 0xF2, 0xFE, 0x1B, 0x20, 0x09, 0x3A, 0xDB, 0xF2, 0xBE, 0x28, 0xF3, 0x34, 0x18, 0xF0, 0xFE, 
0x0E, 0x20, 0xEC, 0x46, 0x21, 0x14, 0xF3, 0x11, 0x19, 0x00, 0x19, 0x10, 0xFD, 0x72, 0x23, 0x72, 
0x1E, 0x04, 0x19, 0x1E, 0x20, 0x3E, 0x01, 0xD3, 0xF9, 0xE5, 0xCD, 0x34, 0xF8, 0xE1, 0xC0, 0x11, 
0x05, 0x00, 0x19, 0x7E, 0x0F, 0x0F, 0x0F, 0xB1, 0x77, 0x11, 0x0C, 0x00, 0x01, 0x66, 0x09, 0x7E, 
0x12, 0x81, 0x4F, 0x2B, 0x1B, 0x10, 0xF8, 0x12, 0xC7, 0xC9, 0x3E, 0x0D, 0xCD, 0xC5, 0xF2, 0x3E, 
0x0A, 0x18, 0x02, 0x3E, 0x20, 0xC5, 0xE5, 0xCD, 0x0F, 0xF8, 0xE1, 0xC1, 0xC9, 0xE5, 0x7E, 0xB7, 
0x28, 0x06, 0xCD, 0xC5, 0xF2, 0x23, 0x10, 0xF6, 0xE1, 0xC9, 0x01, 0x00, 0x05, 0x1F, 0x53, 0x45, 
0x4C, 0x45, 0x43, 0x54, 0x20, 0x42, 0x4F, 0x4F, 0x54, 0x20, 0x50, 0x41, 0x52, 0x54, 0x49, 0x54, 
0x49, 0x4F, 0x4E, 0x3A, 0x0D, 0x0A  };

/* ========================= Screen Menu interface ====================== */

char program_name[]="FDISK v2.1 - Fixed Disks Partition Manager. Type \'FDISK -?\' for help";
char copyleft[]="Public Domain Software by Serge.";

char title[30];
char option[5][30];
char mainmenu[]=", M=main_menu";
char master[]="Master";
char slave[]="Slave";
char completed[]="completed";
char failed[]="failed";
char blank[]="";
int  maximum_number_of_options=0;

BYTE ParType;
DWORD DriveSize[2], dw1, dw2, dw3;
int  CurrentDrive=0;
extern unsigned char TotalDrives;
extern char* TextPartType(BYTE pt);
int  TotalPartitions=0;

char scheme1[80],scheme2[80],scheme3[80];

void GotoXY(int x, int y)		/* VT52 display */
{
  bdos(2, 27); bdos(2, 'Y'); bdos(2, y+32); bdos(2, x+32);
}

void ClearScreen()			/* VT52 display */
{
  if (! AltairDos)
    bdos(2, 26);			/* other VT52 displays - clear & home */ 
  bdos(2, 27);   bdos(2, 'E');    
  bdos(2, 27);   bdos(2, 'H'); 
  if ((AltairDos) && (!(*PIOBYTE & 3))) {	/* if console device = TTY: */
    bdos(2, 27);			
    bdos(2, 'J');			/* other VT52 displays - clear to the end of screen */ 
  }
}

void ClrEoln()				/* VT52 display */
{
   bdos(2, 27);   bdos(2, 'K');   
}

int Inkey()
{
  return (int)bdos(1,0);		/* getchar(); */
}

void PrintInkey(char* msg)
{
  printf("\n%s. Press any key", msg);
  Inkey();
}

void PrintCentered(int y, char *text)
{
  register int x=39-strlen(text)/2;
  GotoXY(x,y);
  printf(text);
}

void CheckIDE()
{
  DriveSize[0]=DriveSize[1]=0;
  if ((disk_ioctl(0, GET_SECTOR_COUNT, &DriveSize[0]) == RES_OK) && 
      (DriveSize[0]>1)) 
    TotalDrives++;
  else
    CurrentDrive=1; 
  if ((disk_ioctl(1, GET_SECTOR_COUNT, &DriveSize[1]) == RES_OK) &&
      (DriveSize[1]>1)) 
    TotalDrives++;
  if (TotalDrives<1)
    { printf("\nNo IDE drives or IDEBDOS driver V2.x not installed\n");
      exit (-1); }
}

int nn;
int xpos;
int kk, off;
char ch;
DWORD ParStart;		/* Partition Start, 1-st Mb */ 
DWORD ParSize;		/*Partition size in Mb */

typedef struct s_ptnfo {
  char  partype;
  DWORD parstart;
  DWORD parsize;
  int gsize;
  int ssize;
  char	parMK[10];
} t_ptnfo, *p_ptnfo; 

t_ptnfo ptnfo[9];

int DrawPartition(char ch, int bpos, int bsize)
{
  register int i;
  int epos=bpos+bsize;

  if (bpos<2) bpos=2;
  if (epos>78) epos=78;

  scheme1[bpos]=scheme1[epos]='+';
  for (i=bpos+1; i<epos; i++) scheme2[i]=ch;
  scheme2[bpos]=scheme2[epos]='|';
  return epos;
}

int ptminstart(DWORD* dwmin) {
	register int ii;
	DWORD curmin=2147483647;
	kk=0;
	for (ii=0; ii<TotalPartitions; ii++) {		/* Get partitions */
		if ((ptnfo[ii].parstart>*dwmin)&&(ptnfo[ii].parstart<=curmin)) {
			curmin=ptnfo[ii].parstart;
			kk=ii;
		}
	}
	*dwmin=curmin;
	return kk;
}

int decbig(int ofs, int cnt) {
	int flag=0;
	while (cnt) {
		if (ofs>nn) {
			if (!flag) return 0; 
			flag=ofs=0;
		}
		if (ptnfo[ofs].gsize>ptnfo[ofs].ssize+1) {
			--ptnfo[ofs].gsize;
			--cnt;
			flag=1;
		}
		++ofs;
	}
	return ofs;
}

int parlen(char *st) {
   return strlen(st)+1;
}

/*
void ShowPtnfo(void){
	int kl;
	for (kl=0; kl<9; kl++) {
		printf("\n%c %lu %lu %d %d %s",
				ptnfo[kl].partype, ptnfo[kl].parstart, ptnfo[kl].parsize,
				ptnfo[kl].gsize, ptnfo[kl].ssize, ptnfo[kl].parMK);
	
	}
	printf("\n");
	Inkey();
}
*/

void DrawPartitions(int drive) {					/* MBR partitioning scheme, all values in kilobytes */
	register int ii;
	memset(ptnfo, 0, sizeof(ptnfo));
	for (ii=0; ii<4; ii++) {						/* Get partitions */
		off=16*ii+bc_size;
        if (buffer_out[off + MBR_PART_TYPE]) {
			TotalPartitions++;
            ch=*TextPartType(buffer_out[off + MBR_PART_TYPE]);
			ptnfo[ii].partype=(buffer_out[off]!=0x80) ? tolower(ch) : ch;			/* UPPERCASE - active partition flag */
			ptnfo[ii].parstart=*((DWORD*)&buffer_out[off + 8]);
			ptnfo[ii].parsize=*((DWORD*)&buffer_out[off + 12]);
			ptnfo[ii].gsize=ptnfo[ii].parsize/(DriveSize[drive]/80);
			ptnfo[ii].ssize=parlen(CalcKM(ptnfo[ii].parMK, ptnfo[ii].parsize));
		} 
	} 
	if (!TotalPartitions) return;
	dw1=0l;
	for (ii=4; ii<4+TotalPartitions; ii++) {		/* Sort partitions */
		ptnfo[ii]=ptnfo[ptminstart(&dw1)];
	} 
	for (ii=4; ii<4+TotalPartitions; ii++) {		/* place target partitions: 1,3,5,7 */
		ptnfo[(ii-4)*2+1]=ptnfo[ii];
	} 
	dw1=0l;
	nn=TotalPartitions*2;							/* form tail empty space: 8 */
	for (ii=0; ii<nn; ii=ii+2) {					/* form 4 empty spaces: 0,2,4,6 */
			if (dw1>ptnfo[ii+1].parstart) {
				printf("\nError: partition records overlay: %d,%d\n", (ii-1)/2, ii/2);
				return;
			}
            ptnfo[ii].partype='e';
			ptnfo[ii].parstart=dw1;
			ptnfo[ii].parsize=ptnfo[ii+1].parstart-dw1;
			ptnfo[ii].gsize=ptnfo[ii].parsize/(DriveSize[drive]/80);
			ptnfo[ii].ssize=parlen(CalcKM(ptnfo[ii].parMK, ptnfo[ii].parsize));
			dw1=ptnfo[ii+1].parstart+ptnfo[ii+1].parsize;
	}
	ptnfo[nn].partype='e';
	ptnfo[nn].parstart=dw1;
	ptnfo[nn].parsize=DriveSize[drive]-dw1;
	ptnfo[nn].gsize=ptnfo[nn].parsize/(DriveSize[drive]/80);
	ptnfo[nn].ssize=parlen(CalcKM(ptnfo[nn].parMK, ptnfo[nn].parsize));
	off=xpos=0;
	for (ii=0; ii<=nn; ii++)
	  if (ptnfo[ii].gsize>0) {						/* pad all spaces/partitions */
		if ((kk=ptnfo[ii].ssize-ptnfo[ii].gsize)>0) {
			off=decbig(off,kk);
			ptnfo[ii].gsize+=kk;
		}
		kk=xpos;
		xpos+=ptnfo[ii].gsize;						/* xpos = total graph part size (sum) in symbols */
		if (!kk)  --xpos;
	  }
	if ((kk=xpos-77)>0) decbig(off,kk);				/* trunc graph part size (sum) in symbols to fit the screen width */
    off=2;
	for (ii=0; ii<=nn; ii++) {
	  if (ptnfo[ii].gsize>0) {						/* draw all spaces/partitions */
		kk=0; xpos=off;
		while (ptnfo[ii].parMK[kk]) {
		  scheme3[xpos++]=ptnfo[ii].parMK[kk++];
		}
		off=DrawPartition(ptnfo[ii].partype, off, ptnfo[ii].gsize);
	  }
	}
}

void ShowPartitionScheme(int drive)
{
  register int  ii;
  drive&=1;
  TotalPartitions=0;
  if (ParType=GetPartitionTable(drive, buffer_out))
  {
    strcpy(scheme1, "+-+---------------------------------------------------------------------------+");
    strcpy(scheme2, "|m|eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee|");
    for (ii=0; ii<79; ii++) scheme3[ii]=' ';
    if (ParType>1) {				/* single FAT/CPM partition, non-MBR scheme */
      ch='O';
      ParSize=DriveSize[drive];
      switch (ParType) {
        case 3: {
          ch='C';                           /* CPM ParSize=128*( DSM*(BLM+1)+(SPT*OFF) ) + 32*(DRM+1) */  
          ParSize=( ( (DWORD)*((WORD*)(&buffer_out[21])) * ((WORD)1+buffer_out[19]) ) + 
                    ( (DWORD)*((WORD*)(&buffer_out[16])) * (*(WORD*)(&buffer_out[29])) ) +
                    ( (*(WORD*)(&buffer_out[23])+1) /(WORD)4 )
                    + 4
                  ) / (WORD)4; 
          break; 
        }
        case 2: { ch='F'; break; }        /* TODO: ParSize=FatFilesystemSize */
      }
      scheme1[2]='-';
      scheme2[2]='e';
      scheme2[1]='e';
      DrawPartition(ch, 0, ParSize/(DriveSize[drive]/80));
      TotalPartitions=0x0FF;
    }
    else { 
      if (CheckPartitionTable(&buffer_out[bc_size])) 
        DrawPartitions(drive);
      else printf("\nWrong Partition table!");
    } 
    scheme3[79]=0;
    printf("%s\n%s\n%s\n%s\n [m]-MBR, [e]-empty; partitions: [f]-FAT, [c]-CP/M, [u]-UZIX, [o]-other",
           scheme1, scheme2, scheme1, scheme3);
  }
  else printf("\nI/O Error: read MBR\n");
}

DWORD GetDWORD(char* title, char* fmt)
{
  GotoXY(4,22); printf(title); 
  ClrEoln();
  scanf(fmt, scheme3);
  return (DWORD)atol(scheme3);
}

char* GetFName(char* ftype, char* buf)
{
  GotoXY(4,22); 
  printf("Enter name of file containig %s code: ", ftype); 
  ClrEoln();
  scanf("%s", buf);
  return &buf[0];
}

/* Displays the menus and returns the */
/* selection chosen by the user.      */

int ShowMenu(int menu)
{
  register int input;
  for(;(menu==MM) || (menu==CP) || (menu==MBR);)
  {
    switch (menu) {
      case CP: {
        maximum_number_of_options=5;
        strcpy(title,"Create Partition");
        strcpy(option[0],"Create Primary CPM Partition");
        strcpy(option[1],"Create Primary FAT Partition");
        strcpy(option[2],"Create Primary UZIX Partition");
        strcpy(option[3],"Create CPM filesystem");
        strcpy(option[4],"Sysgen CPM from the file");
        break;
      }
      case MBR: {
        maximum_number_of_options=5;
        strcpy(title,"MBR Maintenance");
        strcpy(option[0],"Create ZBootLoader MBR");
        strcpy(option[1],"Copy MBR boot code from file");
        strcpy(option[2],"Save the MBR to a file");
        strcpy(option[3],"Remove the MBR from the disk");
        strcpy(option[4],"Create empty MBR on the disk");
        break;
      }
      default: {                          /* MM */
	ClearScreen();
	PrintCentered(0, program_name);
	PrintCentered(1, copyleft);
	GotoXY(0,6); 
	ShowPartitionScheme(CurrentDrive);
        GetFreeSegment(CurrentDrive, &buffer_out[bc_size], 0l, &dw2, &dw1, &dw3);
	GotoXY(1,4); 
	printf("Current drive: %d (%s, Total %s, Free %s). UPCASE=active partition",
		CurrentDrive, (CurrentDrive) ? slave : master,
		CalcKM(title, DriveSize[CurrentDrive]+1 ),
                CalcKM(scheme3, dw3 ) );
        if (TotalDrives>1) 
          maximum_number_of_options=5;
        else 
          maximum_number_of_options=4;
        strcpy(title,"FDISK Options");
        strcpy(option[0],"Create partition");
        strcpy(option[1],"Set Active partition");
        strcpy(option[2],"Delete partition");
        strcpy(option[3],"MBR Maintenance");
        strcpy(option[4],"Select disk drive");
      }
    }
    PrintCentered(12, title); 
    GotoXY(4,14); printf("Choose one of the following:\n");
    for (input=0; input<5; input++) {
      GotoXY(4,16+input);
      if (input<maximum_number_of_options)
        printf("%d. %s\n", input+1, option[input]); 
      ClrEoln();
    }
    GotoXY(4,22); printf("Enter choice (1..%d, A,Q=Abort=Quit%s): ",
                         maximum_number_of_options, (menu==MM) ? blank : mainmenu);
    input=Inkey();
    if((input=='A') || (input=='a') || (input=='Q') || (input=='q')) {
      ClearScreen();
      exit(0);
    }
    if ((input=='M') || (input=='m')) menu=MM;
    else if ((input>'0') && (input<=maximum_number_of_options+'0')) {
      input=input-'0';
      if (menu==MM) menu=input<<4;
      else menu=menu|input;
    }
  }
  return(menu);
}

WORD op;
BYTE pt;
BOOL bl;

void xmenu()
{
  int ii;
  register WORD mode=MM;
  BOOL res=FALSE;
  CheckIDE();
  for(;;) {
    if ((mode=ShowMenu(mode)) == CD) {			/* Change Drive */
      if (TotalDrives>1) {   
        if (CurrentDrive) CurrentDrive=0; else CurrentDrive=1;
      }
    }
    else if ((ParType!=1) && (mode!=CMBR))
      PrintInkey("Error: Drive have Non-MBR partitioning scheme");
    else {
     op=mode & 7;
     if (op==0) op=mode>>4; 
     PrintCentered(12, option[op-1]); 
     for (ii=14; ii<24; ii++) {
       GotoXY(4, ii); ClrEoln();
     } 
     switch (mode) {
      case CPU:  pt=0x21; /*officially reserved*/	/* Create Primary UZIX Partition */ 
      case CPF:  					/* Create Primary FAT Partition */ 
      case CPC: { 					/* Create Primary CPM Partition */
		   if (mode==CPF) pt=0x0C;
           if (mode==CPC) pt=0x52;
           if (TotalPartitions>3) {
                PrintInkey("Four partitions allready exists! Delete one first.");
                break;
           }
           if (GetFreeSegment(CurrentDrive, &buffer_out[bc_size], 0l, &dw2, &dw1, &dw3)) {
		     dw1=dw1>>1;
		     sprintf(scheme1, "Type new partition size, kb (0..%lu): ", dw1);
                     bl = ((dw2=GetDWORD(scheme1, "%9s"))<=dw1) && (dw2>0); 
                     bl = bl && ((op=CreatePartition(CurrentDrive, dw2<<1, pt))<4);
		     if (mode==CPF)
                       bl = bl && (! f_mkfs(op*2+CurrentDrive) ) ;
		     if (bl)
			PrintInkey("Partition created");
		     else
			PrintInkey("Partition not created");
		   }
		   else
                     PrintInkey("No free space for a new partition");
           break;
	    }
      case CPCS: 					/* Sysgen CPM from a file */
      case CPCF: 					/* Create CPM filesystem on Partition */
      case SAP:  					/* Set Active Partition */
      case DP:   {					/* Delete partition */  
                   if (TotalPartitions<1) {
                     PrintInkey("No partitions exists");
                     break;
                   }
		   res=FALSE;
		   sprintf(scheme1, "Type partition ordinal number (1..%d): ",
                           TotalPartitions);
                   if (((ii=(int)GetDWORD(scheme1, "%1s"))>0) &&
                       (ii<=TotalPartitions) ) 
                     switch (mode) {
		       case CPCS: {
			 GetFName("Altair DOS", scheme3);
			 res=scheme3[0] && SysgenCPM(CurrentDrive, ii, scheme3);
			 break;
		       }
		       case CPCF: {
                         res=CreateCpmFS(CurrentDrive, ii);
                         break;
                       } 
                       case SAP:{
                         res=ActivatePartition(CurrentDrive, ii);
                         break;
                       }
                       case DP: {
                         res=DeletePartition(CurrentDrive, ii);
                         break;
                       }
                     }
		   sprintf(scheme3, "Operation `%s` with partition %d %s",
                           option[op-1], ii, res ? completed : failed);
                   PrintInkey(scheme3);
		   break;
                 }
      case RMBR: 					/* Remove MBR from disk */
      case CMBR: 					/* Create MBR on disk */
		 if (! GetCommit("Partition info will be lost"))
			break;
      case FMBR: 					/* Create MBR using the file */
      case BMBR: 					/* Write ZBootLoader MBR to drive */
      case SMBR: 					/* Save MBR to file */
		 {
		   if (mode==RMBR) {					/* Remove MBR from disk */
			sprintf(scheme2, "%d:", CurrentDrive);
			res=pt_sign(scheme2, FALSE);
		   }
		   else {
		     sprintf(scheme2, "%d:", CurrentDrive);
		     if (mode==CMBR) {		
			for (ii=0; ii<bc_size+64; ii++) buffer_in[ii]=0;
			res=bc_copy_mem(&buffer_in[0], scheme2, TRUE, sector_size);
		     } 
                     else if (mode==BMBR)
                       res=bc_copy_mem(&ZBootLoader[0], scheme2, TRUE, bc_size);
		     else {
                       GetFName("MBR", scheme3);
		       if (scheme3[0]) {
                         if (mode==SMBR) 
			   res=bc_copy(scheme2, scheme3, FALSE, sector_size);
		         else
			   res=bc_copy(scheme3, scheme2, TRUE, bc_size);
			}
		     }
		   }	
                   if (res)
                     PrintInkey("Operation completed");
                   else 
                     PrintInkey("Operation failed");
		   break;
		 }
      default:   PrintInkey("\r\rFunction Under Construction");
     } /*switch*/
    } /*if*/
    mode=MM;
  } /*for*/
}

