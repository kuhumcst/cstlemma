/*
CSTLEMMA - trainable lemmatiser

Copyright (C) 2002, 2014, 2009  Center for Sprogteknologi, University of Copenhagen

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
#include "utf8func.h"
#include <string.h>

int ENCODING = DEFAULTENCODING;

const bool space_ISO[256] =
    {
    false, false, false, false, false, false, false, false, false, true , true , true , true , true , false, false, /*   0 -  15 */ /* 00 - 0F */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  16 -  31 */ /* 10 - 1F */
    true , false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  31 -  47 */ /* 20 - 2F */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  48 -  63 */ /* 30 - 3F */

    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  64 -  79 */ /* 40 - 4F */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  80 -  95 */ /* 50 - 5F */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  96 - 111 */ /* 60 - 6F */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 112 - 127 */ /* 70 - 7F */

    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 128 - 143 */ /* 80 - 8F */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 144 - 159 */ /* 90 - 9F */
    true , false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 160 - 175 */ /* A0 - AF */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 176 - 191 */ /* B0 - BF */

    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 192 - 207 */ /* C0 - CF */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 208 - 223 */ /* D0 - DF */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 224 - 239 */ /* E0 - EF */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 240 - 255 */ /* F0 - FF */
    };

const bool space_DEFAULTENCODING[256] =
    {
    false, false, false, false, false, false, false, false, false, true , true , true , true , true , false, false, /*   0 -  15 */ /* 00 - 0F */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  16 -  31 */ /* 10 - 1F */
    true , false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  31 -  47 */ /* 20 - 2F */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  48 -  63 */ /* 30 - 3F */

    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  64 -  79 */ /* 40 - 4F */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  80 -  95 */ /* 50 - 5F */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  96 - 111 */ /* 60 - 6F */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 112 - 127 */ /* 70 - 7F */

    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 128 - 143 */ /* 80 - 8F */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 144 - 159 */ /* 90 - 9F */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 160 - 175 */ /* A0 - AF */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 176 - 191 */ /* B0 - BF */

    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 192 - 207 */ /* C0 - CF */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 208 - 223 */ /* D0 - DF */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 224 - 239 */ /* E0 - EF */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 240 - 255 */ /* F0 - FF */
    };


const bool alpha_DEFAULTENCODING[256] =
    {
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*   0 -  15 */ /* 00 - 0F */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  16 -  31 */ /* 10 - 1F */
    false, false, false, false, false, false, false, true , false, false, false, false, false, true , false, false, /*  31 -  47 */ /* 20 - 2F */ // 20070208 27 (apostrophe) 2D (hyphen) false->true
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  48 -  63 */ /* 30 - 3F */

    false, true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , /*  64 -  79 */ /* 40 - 4F */ 
    true , true , true , true , true , true , true , true , true , true , true , false, false, false, false, true , /*  80 -  95 */ /* 50 - 5F */ 
    false, true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , /*  96 - 111 */ /* 60 - 6F */ 
    true , true , true , true , true , true , true , true , true , true , true , false, false, false, false, false, /* 112 - 127 */ /* 70 - 7F */

    true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , /* 128 - 143 */ /* 80 - 8F */
    true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , /* 144 - 159 */ /* 90 - 9F */
    true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , /* 160 - 175 */ /* A0 - AF */
    true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , /* 176 - 191 */ /* B0 - BF */

    true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , /* 192 - 207 */ /* C0 - CF */ 
    true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , /* 208 - 223 */ /* D0 - DF */ 
    true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , /* 224 - 239 */ /* E0 - EF */ 
    true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , /* 240 - 255 */ /* F0 - FF */ 
    };

const bool alpha_ISO8859_1[256] =
    {
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*   0 -  15 */ /* 00 - 0F */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  16 -  31 */ /* 10 - 1F */
    false, false, false, false, false, false, false, true , false, false, false, false, false, true , false, false, /*  31 -  47 */ /* 20 - 2F */ // 20070208 27 (apostrophe) 2D (hyphen) false->true
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  48 -  63 */ /* 30 - 3F */

    false, true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , /*  64 -  79 */ /* 40 - 4F */ 
    true , true , true , true , true , true , true , true , true , true , true , false, false, false, false, true , /*  80 -  95 */ /* 50 - 5F */ 
    false, true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , /*  96 - 111 */ /* 60 - 6F */ 
    true , true , true , true , true , true , true , true , true , true , true , false, false, false, false, false, /* 112 - 127 */ /* 70 - 7F */

    false, false, false, false, false, false, false, false, false, false, true , false, true , false, true , false, /* 128 - 143 */ /* 80 - 8F */ /* 128 - 143 */
    false, false, false, false, false, false, false, false, false, false, true , false, true , false, true , true , /* 144 - 159 */ /* 90 - 9F */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 160 - 175 */ /* A0 - AF */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 176 - 191 */ /* B0 - BF */

    true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , /* 192 - 207 */ /* C0 - CF */ 
    true , true , true , true , true , true , true , false, true , true , true , true , true , true , true , true , /* 208 - 223 */ /* D0 - DF */ 
    true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , /* 224 - 239 */ /* E0 - EF */ 
    true , true , true , true , true , true , true , false, true , true , true , true , true , true , true , true   /* 240 - 255 */ /* F0 - FF */
    };

const bool alpha_ISO8859_2[256] = /*Latin Alphabet 2 for Eastern European Latin-Alphabet languages*/
    {
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*   0 -  15 */ /* 00 - 0F */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  16 -  31 */ /* 10 - 1F */
    false, false, false, false, false, false, false, true , false, false, false, false, false, true , false, false, /*  31 -  47 */ /* 20 - 2F */ // 20070208 27 (apostrophe) 2D (hyphen) false->true
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  48 -  63 */ /* 30 - 3F */

    false, true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , /*  64 -  79 */ /* 40 - 4F */ 
    true , true , true , true , true , true , true , true , true , true , true , false, false, false, false, true , /*  80 -  95 */ /* 50 - 5F */ 
    false, true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , /*  96 - 111 */ /* 60 - 6F */ 
    true , true , true , true , true , true , true , true , true , true , true , false, false, false, false, false, /* 112 - 127 */ /* 70 - 7F */

    false, false, false, false, false, false, false, false, false, false, true , false, true , false, true , false, /* 128 - 143 */ /* 80 - 8F */ /* 128 - 143 */
    false, false, false, false, false, false, false, false, false, false, true , false, true , false, true , true , /* 144 - 159 */ /* 90 - 9F */
    false, true , false, true , false, true , true , false, false, true , true , true , true , false, true , true , /* 160 - 175 */ /* A0 - AF */
    false, true , false, true , false, true , true , false, false, true , true , true , true , false, true , true , /* 176 - 191 */ /* B0 - BF */

    true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , /* 192 - 207 */ /* C0 - CF */ 
    true , true , true , true , true , true , true , false, true , true , true , true , true , true , true , true , /* 208 - 223 */ /* D0 - DF */ 
    true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , /* 224 - 239 */ /* E0 - EF */ 
    true , true , true , true , true , true , true , false, true , true , true , true , true , true , true , false  /* 240 - 255 */ /* F0 - FF */
    };

const bool alpha_ISO8859_7[256] =
    {
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*   0 -  15 */ /* 00 - 0F */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  16 -  31 */ /* 10 - 1F */
    false, false, false, false, false, false, false, true , false, false, false, false, false, true , false, false, /*  31 -  47 */ /* 20 - 2F */ // 20070208 27 (apostrophe) 2D (hyphen) false->true
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  48 -  63 */ /* 30 - 3F */

    false, true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , /*  64 -  79 */ /* 40 - 4F */ 
    true , true , true , true , true , true , true , true , true , true , true , false, false, false, false, true , /*  80 -  95 */ /* 50 - 5F */ 
    false, true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , /*  96 - 111 */ /* 60 - 6F */ 
    true , true , true , true , true , true , true , true , true , true , true , false, false, false, false, false, /* 112 - 127 */ /* 70 - 7F */

    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 128 - 143 */ /* 80 - 8F */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 144 - 159 */ /* 90 - 9F */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 160 - 175 */ /* A0 - AF */
    false, false, false, false, false, false, true , false, true , true , true , false, true , false, true , true , /* 176 - 191 */ /* B0 - BF */

    true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , /* 192 - 207 */ /* C0 - CF */ 
    true , true , false, true , true , true , true , true , true , true , true , true , true , true , true , true , /* 208 - 223 */ /* D0 - DF */ 
    true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , /* 224 - 239 */ /* E0 - EF */ 
    true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , false  /* 240 - 255 */ /* F0 - FF */
    };

const bool alpha_ISO8859_9[256] =
    {
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*   0 -  15 */ /* 00 - 0F */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  16 -  31 */ /* 10 - 1F */
    false, false, false, false, false, false, false, true , false, false, false, false, false, true , false, false, /*  31 -  47 */ /* 20 - 2F */ // 20070208 27 (apostrophe) 2D (hyphen) false->true
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /*  48 -  63 */ /* 30 - 3F */

    false, true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , /*  64 -  79 */ /* 40 - 4F */ 
    true , true , true , true , true , true , true , true , true , true , true , false, false, false, false, true , /*  80 -  95 */ /* 50 - 5F */ 
    false, true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , /*  96 - 111 */ /* 60 - 6F */ 
    true , true , true , true , true , true , true , true , true , true , true , false, false, false, false, false, /* 112 - 127 */ /* 70 - 7F */

    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,  /* 128 - 143 */ /* 80 - 8F */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,  /* 144 - 159 */ /* 90 - 9F */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,  /* 160 - 175 */ /* A0 - AF */
    false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,  /* 176 - 191 */ /* B0 - BF */

    true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , true ,  /* 192 - 207 */ /* C0 - CF */
    true , true , true , true , true , true , true , false, true , true , true , true , true , true , true , true ,  /* 208 - 223 */ /* D0 - DF */
    true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , true ,  /* 224 - 239 */ /* E0 - EF */
    true , true , true , true , true , true , true , false, true , true , true , true , true , true , true , true    /* 240 - 255 */ /* F0 - FF */
    };


const unsigned char lowerEquivalent_ISO8859_1[256] =
    {
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  /*   0 -  15 */ /* 00 - 0F */
    16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  /*  16 -  31 */ /* 10 - 1F */
    ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/', /*  31 -  47 */ /* 20 - 2F */
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',  /*  48 -  63 */ /* 30 - 3F */

    '@', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',  /*  64 -  79 */ /* 40 - 4F */
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '[', '\\', ']', '^', '_', /*  80 -  95 */ /* 50 - 5F */
    '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',  /*  96 - 111 */ /* 60 - 6F */
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', 127,  /* 112 - 127 */ /* 70 - 7F */

    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 154, 139, 156, 141, 158, 143,  /* 128 - 143 */ /* 80 - 8F */
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 255,  /* 144 - 159 */ /* 90 - 9F */
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,  /* 160 - 175 */ /* A0 - AF */
    176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,  /* 176 - 191 */ /* B0 - BF */

    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,  /* 192 - 207 */ /* C0 - CF */
    240, 241, 242, 243, 244, 245, 246, 215, 248, 249, 250, 251, 252, 253, 254, 223,  /* 208 - 223 */ /* D0 - DF */
    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,  /* 224 - 239 */ /* E0 - EF */
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255   /* 240 - 255 */ /* F0 - FF */
    };

const unsigned char lowerEquivalent_ISO8859_2[256] =
    {
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  /*   0 -  15 */ /* 00 - 0F */
    16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  /*  16 -  31 */ /* 10 - 1F */
    ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/', /*  31 -  47 */ /* 20 - 2F */
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',  /*  48 -  63 */ /* 30 - 3F */

    '@', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',  /*  64 -  79 */ /* 40 - 4F */
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '[', '\\', ']', '^', '_', /*  80 -  95 */ /* 50 - 5F */
    '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',  /*  96 - 111 */ /* 60 - 6F */
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', 127,  /* 112 - 127 */ /* 70 - 7F */

    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 154, 139, 156, 141, 158, 143,  /* 128 - 143 */ /* 80 - 8F */
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 255,  /* 144 - 159 */ /* 90 - 9F */
    160, 177, 162, 179, 164, 181, 182, 167, 168, 185, 186, 187, 188, 173, 190, 191,  /* 160 - 175 */ /* A0 - AF */
    176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,  /* 176 - 191 */ /* B0 - BF */

    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,  /* 192 - 207 */ /* C0 - CF */
    240, 241, 242, 243, 244, 245, 246, 215, 248, 249, 250, 251, 252, 253, 254, 223,  /* 208 - 223 */ /* D0 - DF */
    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,  /* 224 - 239 */ /* E0 - EF */
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255   /* 240 - 255 */ /* F0 - FF */
    };

const unsigned char lowerEquivalent_ISO8859_7[256] =
    {
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  /*   0 -  15 */ /* 00 - 0F */
    16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  /*  16 -  31 */ /* 10 - 1F */
    ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/', /*  31 -  47 */ /* 20 - 2F */
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',  /*  48 -  63 */ /* 30 - 3F */

    '@', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',  /*  64 -  79 */ /* 40 - 4F */
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '[', '\\', ']', '^', '_', /*  80 -  95 */ /* 50 - 5F */
    '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',  /*  96 - 111 */ /* 60 - 6F */
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', 127,  /* 112 - 127 */ /* 70 - 7F */

    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,  /* 128 - 143 */ /* 80 - 8F */
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,  /* 144 - 159 */ /* 90 - 9F */
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,  /* 160 - 175 */ /* A0 - AF */
    176, 177, 178, 179, 180, 181, 220, 183, 221, 222, 223, 187, 252, 189, 253, 254,  /* 176 - 191 */ /* B0 - BF */

    192, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,  /* 192 - 207 */ /* C0 - CF */
    240, 241, 210, 243, 244, 245, 246, 247, 248, 249, 250, 251, 220, 221, 222, 223,  /* 208 - 223 */ /* D0 - DF */
    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,  /* 224 - 239 */ /* E0 - EF */
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255   /* 240 - 255 */ /* F0 - FF */
    };

const unsigned char lowerEquivalent_ISO8859_9[256] =
    {
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  /*   0 -  15 */ /* 00 - 0F */
    16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  /*  16 -  31 */ /* 10 - 1F */
    ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/', /*  31 -  47 */ /* 20 - 2F */
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',  /*  48 -  63 */ /* 30 - 3F */

    '@', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 253, 'j', 'k', 'l', 'm', 'n', 'o',  /*  64 -  79 */ /* 40 - 4F */
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '[', '\\', ']', '^', '_', /*  80 -  95 */ /* 50 - 5F */
    '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',  /*  96 - 111 */ /* 60 - 6F */
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', 127,  /* 112 - 127 */ /* 70 - 7F */

    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 154, 139, 156, 141, 142, 143,  /* 128 - 143 */ /* 80 - 8F */
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,  /* 144 - 159 */ /* 90 - 9F */
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,  /* 160 - 175 */ /* A0 - AF */
    176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,  /* 176 - 191 */ /* B0 - BF */

    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,  /* 192 - 207 */ /* C0 - CF */
    240, 241, 242, 243, 244, 245, 246, 215, 248, 249, 250, 251, 252, 'i', 254, 223,  /* 208 - 223 */ /* D0 - DF */
    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,  /* 224 - 239 */ /* E0 - EF */
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255   /* 240 - 255 */ /* F0 - FF */
    };


const unsigned char upperEquivalent_ISO8859_1[256] =
    {
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  /*   0 -  15 */ /* 00 - 0F */
    16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  /*  16 -  31 */ /* 10 - 1F */
    ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/', /*  31 -  47 */ /* 20 - 2F */
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',  /*  48 -  63 */ /* 30 - 3F */

    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',  /*  64 -  79 */ /* 40 - 4F */
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_', /*  80 -  95 */ /* 50 - 5F */
    '`', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',  /*  96 - 111 */ /* 60 - 6F */
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '{', '|', '}', '~', 127,  /* 112 - 127 */ /* 70 - 7F */

    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,  /* 128 - 143 */ /* 80 - 8F */
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 138, 155, 140, 157, 142, 159,  /* 144 - 159 */ /* 90 - 9F */
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,  /* 160 - 175 */ /* A0 - AF */
    176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,  /* 176 - 191 */ /* B0 - BF */

    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,  /* 192 - 207 */ /* C0 - CF */
    208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,  /* 208 - 223 */ /* D0 - DF */
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,  /* 224 - 239 */ /* E0 - EF */
    208, 209, 210, 211, 212, 213, 214, 247, 216, 217, 218, 219, 220, 221, 222, 159   /* 240 - 255 */ /* F0 - FF */
    };

const unsigned char upperEquivalent_ISO8859_2[256] =
    {
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  /*   0 -  15 */ /* 00 - 0F */
    16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  /*  16 -  31 */ /* 10 - 1F */
    ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/', /*  31 -  47 */ /* 20 - 2F */
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',  /*  48 -  63 */ /* 30 - 3F */

    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',  /*  64 -  79 */ /* 40 - 4F */
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_', /*  80 -  95 */ /* 50 - 5F */
    '`', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',  /*  96 - 111 */ /* 60 - 6F */
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '{', '|', '}', '~', 127,  /* 112 - 127 */ /* 70 - 7F */

    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,  /* 128 - 143 */ /* 80 - 8F */
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 138, 155, 140, 157, 142, 159,  /* 144 - 159 */ /* 90 - 9F */
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,  /* 160 - 175 */ /* A0 - AF */
    176, 161, 178, 163, 180, 165, 166, 183, 184, 169, 170, 171, 172, 189, 174, 175,  /* 176 - 191 */ /* B0 - BF */

    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,  /* 192 - 207 */ /* C0 - CF */
    208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,  /* 208 - 223 */ /* D0 - DF */
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,  /* 224 - 239 */ /* E0 - EF */
    208, 209, 210, 211, 212, 213, 214, 247, 216, 217, 218, 219, 220, 221, 222, 159   /* 240 - 255 */ /* F0 - FF */
    };

const unsigned char upperEquivalent_ISO8859_7[256] =
    {
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  /*   0 -  15 */ /* 00 - 0F */
    16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  /*  16 -  31 */ /* 10 - 1F */
    ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/', /*  31 -  47 */ /* 20 - 2F */
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',  /*  48 -  63 */ /* 30 - 3F */

    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',  /*  64 -  79 */ /* 40 - 4F */
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_', /*  80 -  95 */ /* 50 - 5F */
    '`', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',  /*  96 - 111 */ /* 60 - 6F */
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '{', '|', '}', '~', 127,  /* 112 - 127 */ /* 70 - 7F */

    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,  /* 128 - 143 */ /* 80 - 8F */
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,  /* 144 - 159 */ /* 90 - 9F */
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,  /* 160 - 175 */ /* A0 - AF */
    176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,  /* 176 - 191 */ /* B0 - BF */

    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,  /* 192 - 207 */ /* C0 - CF */
    208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 182, 184, 185, 186,  /* 208 - 223 */ /* D0 - DF */
    224, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,  /* 224 - 239 */ /* E0 - EF */
    208, 209, 211, 211, 212, 213, 214, 215, 216, 217, 218, 219, 188, 190, 191, 255   /* 240 - 255 */ /* F0 - FF */
    };

const unsigned char upperEquivalent_ISO8859_9[256] =
    {
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  /*   0 -  15 */ /* 00 - 0F */
    16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  /*  16 -  31 */ /* 10 - 1F */
    ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/', /*  31 -  47 */ /* 20 - 2F */
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',  /*  48 -  63 */ /* 30 - 3F */

    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',  /*  64 -  79 */ /* 40 - 4F */
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_', /*  80 -  95 */ /* 50 - 5F */
    '`', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 221, 'J', 'K', 'L', 'M', 'N', 'O',  /*  96 - 111 */ /* 60 - 6F */
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '{', '|', '}', '~', 127,  /* 112 - 127 */ /* 70 - 7F */

    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,  /* 128 - 143 */ /* 80 - 8F */
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 138, 155, 140, 157, 158, 159,  /* 144 - 159 */ /* 90 - 9F */
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,  /* 160 - 175 */ /* A0 - AF */
    176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,  /* 176 - 191 */ /* B0 - BF */

    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,  /* 192 - 207 */ /* C0 - CF */
    208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,  /* 208 - 223 */ /* D0 - DF */
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,  /* 224 - 239 */ /* E0 - EF */
    208, 209, 210, 211, 212, 213, 214, 247, 216, 217, 218, 219, 220, 'I', 222, 255   /* 240 - 255 */ /* F0 - FF */
    };

const bool * spaces[10] = {
    space_DEFAULTENCODING,
    space_ISO,
    space_ISO,
    space_ISO,
    space_ISO,
    space_ISO,
    space_ISO,
    space_ISO,
    space_ISO,
    space_ISO,
    };

const bool * alphas[10] = {
    alpha_DEFAULTENCODING,
    alpha_ISO8859_1,
    alpha_ISO8859_2,
    alpha_DEFAULTENCODING,//alpha_ISO8859_3,
    alpha_DEFAULTENCODING,//alpha_ISO8859_4,
    alpha_DEFAULTENCODING,//alpha_ISO8859_5,
    alpha_DEFAULTENCODING,//alpha_ISO8859_6,
    alpha_ISO8859_7,
    alpha_DEFAULTENCODING,//alpha_ISO8859_8,
    alpha_ISO8859_9
    };

const unsigned char * lowerEquivalents[10] = {
    0,
    lowerEquivalent_ISO8859_1,
    lowerEquivalent_ISO8859_2,
    0,//lowerEquivalent_ISO8859_3,
    0,//lowerEquivalent_ISO8859_4,
    0,//lowerEquivalent_ISO8859_5,
    0,//lowerEquivalent_ISO8859_6,
    lowerEquivalent_ISO8859_7,
    0,//lowerEquivalent_ISO8859_8,
    lowerEquivalent_ISO8859_9
    };

const unsigned char * upperEquivalents[10] = {
    0,
    upperEquivalent_ISO8859_1,
    upperEquivalent_ISO8859_2,
    0,//upperEquivalent_ISO8859_3,
    0,//upperEquivalent_ISO8859_4,
    0,//upperEquivalent_ISO8859_5,
    0,//upperEquivalent_ISO8859_6,
    upperEquivalent_ISO8859_7,
    0,//upperEquivalent_ISO8859_8,
    upperEquivalent_ISO8859_9
    };

const bool * space = space_DEFAULTENCODING;
const bool * alpha = alphas[DEFAULTENCODING];
const unsigned char * LowerEquivalent = lowerEquivalents[DEFAULTENCODING];
const unsigned char * UpperEquivalent = upperEquivalents[DEFAULTENCODING];

bool isAllUpperUTF8(const char * s)
    {
    return isAllUpper(s,0);
    }

bool (*IsAllUpper)(const char * s) = NULL;


void AllToLowerISO(char * s)
    {
    if(LowerEquivalent)
        {
        while(*s)
            {
            *s = (char)LowerEquivalent[*s & 0xFF];
            ++s;
            }
        }
    }

void NToLower0(char * s,const char * stop)
    {
    while(s < stop && *s)
        {
        ++s;
        }
    *s = '\0';
    }

void NToLowerISO(char * s,const char * stop)
    {
    if(LowerEquivalent)
        {
        while(s < stop && *s)
            {
            *s = (char)LowerEquivalent[*s & 0xFF];
            ++s;
            }
        *s = '\0';
        }
    else
        NToLower0(s,stop);
    }

int strCaseCmpN0(const char *s, const char *p,ptrdiff_t & is,ptrdiff_t & ip)
    {
    int i;
    int ret;
    for(i = 0;;++i,++s,++p)
        {
        if(*s)
            {
            if(*p)
                {
                if(*s != *p)
                    {
                    ret = *s - *p;
                    break;
                    }
                }
            else
                {
                ret = 1;
                break;
                }
            }
        else
            {
            if(*p)
                {
                ret = -1;
                break;
                }
            else
                {
                ret = 0;
                break;
                }
            }
        }
    is = i;
    ip = i;
    return ret;
    }

const char * allToLowerISO(const char * s)
    {
    static char buf[256];
    static char * ret = buf;
    if(ret != buf)
        delete [] ret;
    size_t l = strlen(s) + 1;
    if(l > 256)
        ret = new char[l];
    else
        ret = buf;
    if(!LowerEquivalent)
        {
        strcpy(ret,s);
        }
    else
        {
        char * d = ret;
        while(*s)
            {
            *d = (char)LowerEquivalent[*s & 0xFF];
            ++d;
            ++s;
            }
        *d = '\0';
        }
    return ret;
    }


void AllToUpperISO(char * s)
    {
    if(UpperEquivalent)
        {
        while(*s)
            {
            *s = (char)UpperEquivalent[*s & 0xFF];
            ++s;
            }
        }
    }

void toUpper(char * s)
    {
    if(UpperEquivalent)
        *s = (char)UpperEquivalent[*s & 0xFF];
    }

bool isAllUpper(const char * s)
    {
    if(!UpperEquivalent)
        return false;
    else
        {
        while(*s)
            {
            int S = *s & 0xFF;
            if(UpperEquivalent[S] != S)
                return false;
            ++s;
            }
        return true;
        }
    }

bool isAlphaISO(int s)
    {
    return alpha[s & 0xFF];
    }

bool isUpper0(const char * s){return 'A' <= *s && *s <= 'Z';}

bool isUpper19(const char * s){return (UpperEquivalent[(int)(*s & 0xFF)] == (int)(*s & 0xFF));}

bool (*is_Upper)(const char * s) = isUpper0;
bool (*is_Alpha)(int k) = isAlphaISO;
const char * (*allToLower)(const char * s);
int (*strcasecmpN)(const char *s, const char *p,ptrdiff_t & is,ptrdiff_t & ip) = strCaseCmpN0; // partly replaces Lower
int (*strcmpN)(const char *s, const char *p,ptrdiff_t & is,ptrdiff_t & ip) = strCaseCmpN0; // partly replaces Lower

void setEncoding(int encoding)
    {
    if(encoding == 'u')
        encoding = ENUNICODE;
    else if(  encoding != ENUNICODE 
           && (  encoding < 0 
              || encoding >= (int)(sizeof(alphas)/sizeof(alphas[0]))
              )
           )
        encoding = DEFAULTENCODING;

    if(encoding == ENUNICODE)
        {
        space = spaces[DEFAULTENCODING];
        
        is_Upper = isUpperUTF8;
        is_Alpha = isAlpha;
        allToLower = allToLowerUTF8;
        strcasecmpN = strCaseCmpN;
        strcmpN = strCmpN;
        IsAllUpper = isAllUpperUTF8;
        }
    else
        {
        space = spaces[encoding];
        alpha = alphas[encoding];
        LowerEquivalent = lowerEquivalents[encoding];
        UpperEquivalent = upperEquivalents[encoding];
        if(encoding == DEFAULTENCODING)
            {
            is_Upper = isUpper0;
            strcasecmpN = strCaseCmpN0;
            strcmpN = strCaseCmpN0;
            }
        else
            {
            is_Upper = isUpper19;
            strcmpN = strCaseCmpN0;
            }
        is_Alpha = isAlphaISO;
        allToLower = allToLowerISO;
        IsAllUpper = isAllUpper;
        }
    }

#if CHARTEST
#include <stdio.h>
int main()
    {
    for(ENCODING = 0;ENCODING < 10;++ENCODING)
        {
        switch(ENCODING)
            {
            case DEFAULTENCODING:
            case ISO8859_1:
            case ISO8859_2:
            case ISO8859_7:
            case ISO8859_9:
                {
                setEncoding(ENCODING);
                char name[256];
                sprintf(name,"ISO8859_%d.html",ENCODING);
                FILE * fo = fopen(name,"w");
                if(fo)
                    {
                    fprintf(fo,"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">%s<head><meta http-equiv=\"content-type\" content=\"text/html;charset=ISO-8859-%d\"></head><body><pre><big>%s",
                        (ENCODING == 6 ? "<html dir=\"RTL\">" : "<html>"),ENCODING,(ENCODING == 6 ? "<bdo dir=\"RTL\">" : "")
                        );

                    fprintf(fo,"dec\thex\tchar\tlow\tupp\tspace\talpha\n",ENCODING);
                    int i;
                    for(i = 32;i < 256;++i)
                        {
                        fprintf(fo,"%3d\t%2x\t%c\t%c\t%c\t%c\t%c\n",i,i,i,LowerEquivalent?LowerEquivalent[i]:i,UpperEquivalent?UpperEquivalent[i]:i,space[i]?'s':' ',alpha[i]?'a':' ');
                        }
                    fprintf(fo,"%s%s",(ENCODING == 6 ? "</bdo>":""),"</pre></big></body></html>\n");
                    fclose(fo);
                    }
                }
            }
        }
    return 0;
    }
#endif
