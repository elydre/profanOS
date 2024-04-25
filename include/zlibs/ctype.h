/****** This file is part of profanOS **************************\
|   == ctype.h ==                                    .pi0iq.    |
|                                                   d"  . `'b   |
|   Implementation ctype.h header file              q. /|\  u   |
|   for the C standard library                       `// \\     |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

#ifndef CTYPE_H
#define CTYPE_H

int isalnum(int c);
int isalpha(int c);
int iscntrl(int c);
int isdigit(int c);
int isgraph(int c);
int islower(int c);
int isprint(int c);
int ispunct(int c);
int isspace(int c);
int isupper(int c);
int isxdigit(int c);

int tolower(int c);
int toupper(int c);

#endif
