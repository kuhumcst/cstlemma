/*
CSTLEMMA - trainable lemmatiser using word-end inflectional rules

Copyright (C) 2002, 2005  Center for Sprogteknologi, University of Copenhagen

This file is part of CSTLEMMA.

CSTLEMMA is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

CSTLEMMA is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with CSTLEMMA; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "caseconv.h"
#include <string.h>

#if defined  ISO8859_1
/* ISO8859-1 *//* NOT DOS compatible! */
const unsigned char lowerEquivalent[256] =
{
      0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,                 /*   0 -  15 */
     16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,                 /*  16 -  31 */
    ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',                /*  31 -  47 */
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',                 /*  48 -  63 */
                                                                                                   
    '@', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',                 /*  64 -  79 */
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '[', '\\', ']', '^', '_',                /*  80 -  95 */
    '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',                 /*  96 - 111 */
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', 127,                 /* 112 - 127 */
                                                                                                   
    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,                 /* 128 - 143 */
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,                 /* 144 - 159 */
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,                 /* 160 - 175 */
    176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,                 /* 176 - 191 */
                                                                                                   
    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,                 /* 192 - 207 */
    240, 241, 242, 243, 244, 245, 246, 215, 248, 249, 250, 251, 252, 253, 254, 223 /*ringel s*/,    /* 208 - 223 */
    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,                 /* 224 - 239 */
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255                  /* 240 - 255 */
};

const unsigned char upperEquivalent[256] =
{
      0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,           /*   0 -  15 */
     16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,           /*  16 -  31 */
    ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',          /*  31 -  47 */
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',           /*  48 -  63 */
                                                                                             
    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',           /*  64 -  79 */
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_',          /*  80 -  95 */
    '`', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',           /*  96 - 111 */
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '{', '|', '}', '~', 127,           /* 112 - 127 */
                                                                                             
    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,           /* 128 - 143 */
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,           /* 144 - 159 */
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,           /* 160 - 175 */
    176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,           /* 176 - 191 */
                                                                                             
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,           /* 192 - 207 */
    208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,           /* 208 - 223 */
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,           /* 224 - 239 */
    208, 209, 210, 211, 212, 213, 214, 247, 216, 217, 218, 219, 220, 221, 222, 255 /* ij */   /* 240 - 255 */
};

/*
/* ISO8859-5 Turkish *//* NOT DOS compatible! */

const unsigned char lowerEquivalent[256] =
{
      0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,   /*   0 -  15 */
     16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,   /*  16 -  31 */
    ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',  /*  31 -  47 */
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',   /*  48 -  63 */
                                                                                     
    '@', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 253, 'j', 'k', 'l', 'm', 'n', 'o',   /*  64 -  79 */
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '[', '\\', ']', '^', '_',  /*  80 -  95 */
    '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',   /*  96 - 111 */
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', 127,   /* 112 - 127 */
                                                                                     
    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 154, 139, 156, 141, 142, 143,   /* 128 - 143 */
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 255,   /* 144 - 159 */
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,   /* 160 - 175 */
    176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,   /* 176 - 191 */
                                                                                     
    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,   /* 192 - 207 */
    240, 241, 242, 243, 244, 245, 246, 215, 248, 249, 250, 251, 252, 'i', 254, 223,   /* 208 - 223 */
    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,   /* 224 - 239 */
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255    /* 240 - 255 */
};

const unsigned char upperEquivalent[256] =
{
      0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  /*   0 -  15 */
     16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  /*  16 -  31 */
    ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/', /*  31 -  47 */
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',  /*  48 -  63 */
                                                                                    
    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',  /*  64 -  79 */
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_', /*  80 -  95 */
    '`', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 221, 'J', 'K', 'L', 'M', 'N', 'O',  /*  96 - 111 */
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '{', '|', '}', '~', 127,  /* 112 - 127 */
                                                                                    
    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,  /* 128 - 143 */
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 138, 155, 140, 157, 158, 159,  /* 144 - 159 */
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,  /* 160 - 175 */
    176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,  /* 176 - 191 */
                                                                                    
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,  /* 192 - 207 */
    208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,  /* 208 - 223 */
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,  /* 224 - 239 */
    208, 209, 210, 211, 212, 213, 214, 247, 216, 217, 218, 219, 220, 'I', 222, 159   /* 240 - 255 */
};
*/

const bool space[256] =
{
  false, false, false, false, false, false, false, false, false, true,  true,  true,  true,  true,  false, false, /*   0 -  15 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  16 -  31 */
  true,  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  31 -  47 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  48 -  63 */
                                                                                                                 
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  64 -  79 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  80 -  95 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  96 - 111 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 112 - 127 */
                                                                                                                 
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 128 - 143 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 144 - 159 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 160 - 175 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 176 - 191 */
                                                                                                                 
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 192 - 207 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 208 - 223 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 224 - 239 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false  /* 240 - 255 */
};

const bool alpha[256] =
{
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*   0 -  15 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  16 -  31 */
  false, false, false, false, false, false, false, true , false, false, false, false, false, false, false, false, /*  31 -  47 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  48 -  63 */
  
  false, true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  /*  64 -  79 */
  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, false, false, false, true,  /*  80 -  95 */
  false, true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  /*  96 - 111 */
  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, false, false, false, false, /* 112 - 127 */
  
  false, false, false, false, false, false, false, false, false, false, true , false, true , false, false, false, /* 128 - 143 */
  false, false, false, false, false, false, false, false, false, false, true , false, true , false, false, true , /* 144 - 159 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 160 - 175 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 176 - 191 */
  
  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  /* 192 - 207 */
  true,  true,  true,  true,  true,  true,  true,  false, true,  true,  true,  true,  true,  true,  true,  true,  /* 208 - 223 */
  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  /* 224 - 239 */
  true,  true,  true,  true,  true,  true,  true,  false, true,  true,  true,  true,  true,  true,  true,  true   /* 240 - 255 */
};


void AllToLower(char * s)
    {
    while(*s)
        {
        *s = (char)lowerEquivalent[*s & 0xFF];
        ++s;
        }
    }

const char * allToLower(const char * s)
    {
    static char buf[256];
    static char * ret = buf;
    if(ret != buf)
        delete [] ret;
    int l = strlen(s) + 1;
    if(l > 256)
        ret = new char[l];
    else
        ret = buf;
    char * d = ret;
    while(*s)
        {
        *d = (char)lowerEquivalent[*s & 0xFF];
        ++d;
        ++s;
        }
    *d = '\0';
    return ret;
    }


void AllToUpper(char * s)
    {
    while(*s)
        {
        *s = (char)upperEquivalent[*s & 0xFF];
        ++s;
        }
    }

void toLower(char * s)
    {
    *s = (char)lowerEquivalent[*s & 0xFF];
    }

void toUpper(char * s)
    {
    *s = (char)upperEquivalent[*s & 0xFF];
    }
/*
bool isUpper(const char * s)
    {
    int S = *s & 0xFF;
    return upperEquivalent[S] == S;
    }
*/
bool isAllUpper(const char * s)
    {
    while(*s)
        {
        int S = *s & 0xFF;
        if(upperEquivalent[S] != S)
            return false;
        ++s;
        }
    return true;
    }
/*
bool isSpace(int s)
    {
    return space[s & 0xFF];
    }

bool isAlpha(int s)
    {
    return alpha[s & 0xFF];
    }
*/
#elif defined  ISO8859_7
/* ISO8859-7 Greek *//* NOT DOS compatible! */
const unsigned char lowerEquivalent[256] =
{
      0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,    /*   0 -  15 */
     16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,    /*  16 -  31 */
    ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',   /*  31 -  47 */
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',    /*  48 -  63 */
                                                                                      
    '@', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',    /*  64 -  79 */
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '[', '\\', ']', '^', '_',   /*  80 -  95 */
    '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',    /*  96 - 111 */
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', 127,    /* 112 - 127 */
                                                                                      
    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,    /* 128 - 143 */
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,    /* 144 - 159 */
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,    /* 160 - 175 */
    176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,    /* 176 - 191 */
                                                                                      
    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,    /* 192 - 207 */
    240, 241, 210, 243, 244, 245, 246, 215, 248, 249, 250, 251, 220, 221, 222, 223,    /* 208 - 223 */
    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,    /* 224 - 239 */
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255     /* 240 - 255 */
};

const unsigned char upperEquivalent[256] =
{
      0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,    /*   0 -  15 */
     16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,    /*  16 -  31 */
    ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',   /*  31 -  47 */
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',    /*  48 -  63 */
                                                                                      
    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',    /*  64 -  79 */
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_',   /*  80 -  95 */
    '`', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',    /*  96 - 111 */
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '{', '|', '}', '~', 127,    /* 112 - 127 */
                                                                                      
    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,    /* 128 - 143 */
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,    /* 144 - 159 */
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,    /* 160 - 175 */
    176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,    /* 176 - 191 */
                                                                                      
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,    /* 192 - 207 */
    208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,    /* 208 - 223 */
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,    /* 224 - 239 */
    208, 209, 211, 211, 212, 213, 214, 247, 216, 217, 218, 219, 252, 253, 254, 255     /* 240 - 255 */
};

const bool space[256] =
{
  false, false, false, false, false, false, false, false, false, true,  true,  true,  true,  true,  false, false, /*   0 -  15 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  16 -  31 */
  true,  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  31 -  47 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  48 -  63 */
                                                                                                                 
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  64 -  79 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  80 -  95 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  96 - 111 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 112 - 127 */
                                                                                                                 
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 128 - 143 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 144 - 159 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 160 - 175 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 176 - 191 */
                                                                                                                 
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 192 - 207 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 208 - 223 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 224 - 239 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false  /* 240 - 255 */
};

const bool alpha[256] =
{
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*   0 -  15 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  16 -  31 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  31 -  47 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  48 -  63 */
                                                                                                                 
  false, true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  /*  64 -  79 */
  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, false, false, false, true,  /*  80 -  95 */
  false, true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  /*  96 - 111 */
  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, false, false, false, false, /* 112 - 127 */
                                                                                                                 
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 128 - 143 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 144 - 159 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 160 - 175 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 176 - 191 */
                                                                                                                 
  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  /* 192 - 207 */
  true,  true,  false, true,  true,  true,  true,  false, true,  true,  true,  true,  true,  true,  true,  true,  /* 208 - 223 */
  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  /* 224 - 239 */
  true,  true,  true,  true,  true,  true,  true,  false, true,  true,  true,  true,  true,  true,  true,  false  /* 240 - 255 */
};


void AllToLower(char * s)
    {
    while(*s)
        {
        *s = (char)lowerEquivalent[*s & 0xFF];
        ++s;
        }
    }

const char * allToLower(const char * s)
    {
    static char buf[256];
    static char * ret = buf;
    if(ret != buf)
        delete [] ret;
    int l = strlen(s) + 1;
    if(l > 256)
        ret = new char[l];
    else
        ret = buf;
    char * d = ret;
    while(*s)
        {
        *d = (char)lowerEquivalent[*s & 0xFF];
        ++d;
        ++s;
        }
    *d = '\0';
    return ret;
    }


void AllToUpper(char * s)
    {
    while(*s)
        {
        *s = (char)upperEquivalent[*s & 0xFF];
        ++s;
        }
    }

void toLower(char * s)
    {
    *s = (char)lowerEquivalent[*s & 0xFF];
    }

void toUpper(char * s)
    {
    *s = (char)upperEquivalent[*s & 0xFF];
    }
/*
bool isUpper(const char * s)
    {
    int S = *s & 0xFF;
    return upperEquivalent[S] == S;
    }
*/
bool isAllUpper(const char * s)
    {
    while(*s)
        {
        int S = *s & 0xFF;
        if(upperEquivalent[S] != S)
            return false;
        ++s;
        }
    return true;
    }
/*
bool isSpace(int s)
    {
    return space[s & 0xFF];
    }

bool isAlpha(int s)
    {
    return alpha[s & 0xFF];
    }
*/
#else

const bool alpha[256] =
{
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*   0 -  15 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  16 -  31 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  31 -  47 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  48 -  63 */
                                                                                                                 
  false, true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  /*  64 -  79 */
  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, false, false, false, true,  /*  80 -  95 */
  false, true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  /*  96 - 111 */
  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, false, false, false, false, /* 112 - 127 */
                                                                                                                 
  true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true,                 /* 128 - 143 */
  true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true,                 /* 144 - 159 */
  true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true,                 /* 160 - 175 */
  true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true,                 /* 176 - 191 */
                                                                                                                 
  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  /* 192 - 207 */
  true,  true,  true,  true,  true,  true,  true,  true, true,  true,  true,  true,  true,  true,  true,  true,   /* 208 - 223 */
  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  /* 224 - 239 */
  true,  true,  true,  true,  true,  true,  true,  true, true,  true,  true,  true,  true,  true,  true,  true,   /* 240 - 255 */
};

const bool space[256] =
{
  false, false, false, false, false, false, false, false, false, true,  true,  true,  true,  true,  false, false, /*   0 -  15 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  16 -  31 */
  true,  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  31 -  47 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  48 -  63 */
                                                                                                                 
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  64 -  79 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  80 -  95 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  96 - 111 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 112 - 127 */
                                                                                                                 
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 128 - 143 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 144 - 159 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 160 - 175 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 176 - 191 */
                                                                                                                 
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 192 - 207 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 208 - 223 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 224 - 239 */
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 240 - 255 */
};



void AllToLower(char * s)
    {
    }

const char * allToLower(const char * s)
    {
    static char buf[256];
    static char * ret = buf;
    if(ret != buf)
        delete [] ret;
    int l = strlen(s) + 1;
    if(l > 256)
        ret = new char[l];
    else
        ret = buf;
    strcpy(ret,s);
    return ret;
    }


void AllToUpper(char * s)
    {
    }

void toLower(char * s)
    {
    }

void toUpper(char * s)
    {
    }

bool isAllUpper(const char * s)
    {
    return false;
    }
#endif

