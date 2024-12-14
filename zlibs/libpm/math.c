/*****************************************************************************\
|   === math.c : 2024 ===                                                     |
|                                                                             |
|    Mini libm implementation from profanOS                        .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS - under GPLv3                  q. /|\  "   |
|    Original code from sun microsystems (see below)               `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/math.h>
#include "math_private.h"

/*
 * ====================================================
 * Copyright 1993, 2004 Sun Microsystems, Inc.  All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#define DBL_EPSILON 2.2204460492503131e-16
#if FLT_EVAL_METHOD==0 || FLT_EVAL_METHOD==1
#define EPS DBL_EPSILON
#elif FLT_EVAL_METHOD==2
#define EPS LDBL_EPSILON
#endif

#define FENV_SUPPORT 1

#define ASUINT64(x) ((union {double f; uint64_t i;}){x}).i

static const double toint = 1/EPS;

/* Small multiples of pi/2 rounded to double precision. */
static const double t1pio2 = 1*M_PI_2; /* 0x3FF921FB, 0x54442D18 */
static const double t2pio2 = 2*M_PI_2; /* 0x400921FB, 0x54442D18 */
static const double t3pio2 = 3*M_PI_2; /* 0x4012D97C, 0x7F3321D2 */
static const double t4pio2 = 4*M_PI_2; /* 0x401921FB, 0x54442D18 */

/* Small multiples of pi/2 rounded to double precision. */
static const double c1pio2 = 1*M_PI_2; /* 0x3FF921FB, 0x54442D18 */
static const double c2pio2 = 2*M_PI_2; /* 0x400921FB, 0x54442D18 */
static const double c3pio2 = 3*M_PI_2; /* 0x4012D97C, 0x7F3321D2 */
static const double c4pio2 = 4*M_PI_2; /* 0x401921FB, 0x54442D18 */

static const int init_jk[] = {3,4,4,6}; /* initial value for jk */

/*
 * Table of constants for 2/pi, 396 Hex digits (476 decimal) of 2/pi
 *
 *              integer array, contains the (24*i)-th to (24*i+23)-th
 *              bit of 2/pi after binary point. The corresponding
 *              floating value is
 *
 *                      ipio2[i] * 2^(-24(i+1)).
 *
 * NB: This table must have at least (e0-3)/24 + jk terms.
 *     For quad precision (e0 <= 16360, jk = 6), this is 686.
 */

static const int32_t ipio2[] = {
0xA2F983, 0x6E4E44, 0x1529FC, 0x2757D1, 0xF534DD, 0xC0DB62,
0x95993C, 0x439041, 0xFE5163, 0xABDEBB, 0xC561B7, 0x246E3A,
0x424DD2, 0xE00649, 0x2EEA09, 0xD1921C, 0xFE1DEB, 0x1CB129,
0xA73EE8, 0x8235F5, 0x2EBB44, 0x84E99C, 0x7026B4, 0x5F7E41,
0x3991D6, 0x398353, 0x39F49C, 0x845F8B, 0xBDF928, 0x3B1FF8,
0x97FFDE, 0x05980F, 0xEF2F11, 0x8B5A0A, 0x6D1F6D, 0x367ECF,
0x27CB09, 0xB74F46, 0x3F669E, 0x5FEA2D, 0x7527BA, 0xC7EBE5,
0xF17B3D, 0x0739F7, 0x8A5292, 0xEA6BFB, 0x5FB11F, 0x8D5D08,
0x560330, 0x46FC7B, 0x6BABF0, 0xCFBC20, 0x9AF436, 0x1DA9E3,
0x91615E, 0xE61B08, 0x659985, 0x5F14A0, 0x68408D, 0xFFD880,
0x4D7327, 0x310606, 0x1556CA, 0x73A8C9, 0x60E27B, 0xC08C6B,

#if LDBL_MAX_EXP > 1024
0x47C419, 0xC367CD, 0xDCE809, 0x2A8359, 0xC4768B, 0x961CA6,
0xDDAF44, 0xD15719, 0x053EA5, 0xFF0705, 0x3F7E33, 0xE832C2,
0xDE4F98, 0x327DBB, 0xC33D26, 0xEF6B1E, 0x5EF89F, 0x3A1F35,
0xCAF27F, 0x1D87F1, 0x21907C, 0x7C246A, 0xFA6ED5, 0x772D30,
0x433B15, 0xC614B5, 0x9D19C3, 0xC2C4AD, 0x414D2C, 0x5D000C,
0x467D86, 0x2D71E3, 0x9AC69B, 0x006233, 0x7CD2B4, 0x97A7B4,
0xD55537, 0xF63ED7, 0x1810A3, 0xFC764D, 0x2A9D64, 0xABD770,
0xF87C63, 0x57B07A, 0xE71517, 0x5649C0, 0xD9D63B, 0x3884A7,
0xCB2324, 0x778AD6, 0x23545A, 0xB91F00, 0x1B0AF1, 0xDFCE19,
0xFF319F, 0x6A1E66, 0x615799, 0x47FBAC, 0xD87F7E, 0xB76522,
0x89E832, 0x60BFE6, 0xCDC4EF, 0x09366C, 0xD43F5D, 0xD7DE16,
0xDE3B58, 0x929BDE, 0x2822D2, 0xE88628, 0x4D58E2, 0x32CAC6,
0x16E308, 0xCB7DE0, 0x50C017, 0xA71DF3, 0x5BE018, 0x34132E,
0x621283, 0x014883, 0x5B8EF5, 0x7FB0AD, 0xF2E91E, 0x434A48,
0xD36710, 0xD8DDAA, 0x425FAE, 0xCE616A, 0xA4280A, 0xB499D3,
0xF2A606, 0x7F775C, 0x83C2A3, 0x883C61, 0x78738A, 0x5A8CAF,
0xBDD76F, 0x63A62D, 0xCBBFF4, 0xEF818D, 0x67C126, 0x45CA55,
0x36D9CA, 0xD2A828, 0x8D61C2, 0x77C912, 0x142604, 0x9B4612,
0xC459C4, 0x44C5C8, 0x91B24D, 0xF31700, 0xAD43D4, 0xE54929,
0x10D5FD, 0xFCBE00, 0xCC941E, 0xEECE70, 0xF53E13, 0x80F1EC,
0xC3E7B3, 0x28F8C7, 0x940593, 0x3E71C1, 0xB3092E, 0xF3450B,
0x9C1288, 0x7B20AB, 0x9FB52E, 0xC29247, 0x2F327B, 0x6D550C,
0x90A772, 0x1FE76B, 0x96CB31, 0x4A1679, 0xE27941, 0x89DFF4,
0x9794E8, 0x84E6E2, 0x973199, 0x6BED88, 0x365F5F, 0x0EFDBB,
0xB49A48, 0x6CA467, 0x427271, 0x325D8D, 0xB8159F, 0x09E5BC,
0x25318D, 0x3974F7, 0x1C0530, 0x010C0D, 0x68084B, 0x58EE2C,
0x90AA47, 0x02E774, 0x24D6BD, 0xA67DF7, 0x72486E, 0xEF169F,
0xA6948E, 0xF691B4, 0x5153D1, 0xF20ACF, 0x339820, 0x7E4BF5,
0x6863B2, 0x5F3EDD, 0x035D40, 0x7F8985, 0x295255, 0xC06437,
0x10D86D, 0x324832, 0x754C5B, 0xD4714E, 0x6E5445, 0xC1090B,
0x69F52A, 0xD56614, 0x9D0727, 0x50045D, 0xDB3BB4, 0xC576EA,
0x17F987, 0x7D6B49, 0xBA271D, 0x296996, 0xACCCC6, 0x5414AD,
0x6AE290, 0x89D988, 0x50722C, 0xBEA404, 0x940777, 0x7030F3,
0x27FC00, 0xA871EA, 0x49C266, 0x3DE064, 0x83DD97, 0x973FA3,
0xFD9443, 0x8C860D, 0xDE4131, 0x9D3992, 0x8C70DD, 0xE7B717,
0x3BDF08, 0x2B3715, 0xA0805C, 0x93805A, 0x921110, 0xD8E80F,
0xAF806C, 0x4BFFDB, 0x0F9038, 0x761859, 0x15A562, 0xBBCB61,
0xB989C7, 0xBD4010, 0x04F2D2, 0x277549, 0xF6B6EB, 0xBB22DB,
0xAA140A, 0x2F2689, 0x768364, 0x333B09, 0x1A940E, 0xAA3A51,
0xC2A31D, 0xAEEDAF, 0x12265C, 0x4DC26D, 0x9C7A2D, 0x9756C0,
0x833F03, 0xF6F009, 0x8C402B, 0x99316D, 0x07B439, 0x15200C,
0x5BC3D8, 0xC492F5, 0x4BADC6, 0xA5CA4E, 0xCD37A7, 0x36A9E6,
0x9492AB, 0x6842DD, 0xDE6319, 0xEF8C76, 0x528B68, 0x37DBFC,
0xABA1AE, 0x3115DF, 0xA1AE00, 0xDAFB0C, 0x664D64, 0xB705ED,
0x306529, 0xBF5657, 0x3AFF47, 0xB9F96A, 0xF3BE75, 0xDF9328,
0x3080AB, 0xF68C66, 0x15CB04, 0x0622FA, 0x1DE4D9, 0xA4B33D,
0x8F1B57, 0x09CD36, 0xE9424E, 0xA4BE13, 0xB52333, 0x1AAAF0,
0xA8654F, 0xA5C1D2, 0x0F3F0B, 0xCD785B, 0x76F923, 0x048B7B,
0x721789, 0x53A6C6, 0xE26E6F, 0x00EBEF, 0x584A9B, 0xB7DAC4,
0xBA66AA, 0xCFCF76, 0x1D02D1, 0x2DF1B1, 0xC1998C, 0x77ADC3,
0xDA4886, 0xA05DF7, 0xF480C6, 0x2FF0AC, 0x9AECDD, 0xBC5C3F,
0x6DDED0, 0x1FC790, 0xB6DB2A, 0x3A25A3, 0x9AAF00, 0x9353AD,
0x0457B6, 0xB42D29, 0x7E804B, 0xA707DA, 0x0EAA76, 0xA1597B,
0x2A1216, 0x2DB7DC, 0xFDE5FA, 0xFEDB89, 0xFDBE89, 0x6C76E4,
0xFCA906, 0x70803E, 0x156E85, 0xFF87FD, 0x073E28, 0x336761,
0x86182A, 0xEABD4D, 0xAFE7B3, 0x6E6D8F, 0x396795, 0x5BBF31,
0x48D784, 0x16DF30, 0x432DC7, 0x356125, 0xCE70C9, 0xB8CB30,
0xFD6CBF, 0xA200A4, 0xE46C05, 0xA0DD5A, 0x476F21, 0xD21262,
0x845CB9, 0x496170, 0xE0566B, 0x015299, 0x375550, 0xB7D51E,
0xC4F133, 0x5F6E13, 0xE4305D, 0xA92E85, 0xC3B21D, 0x3632A1,
0xA4B708, 0xD4B1EA, 0x21F716, 0xE4698F, 0x77FF27, 0x80030C,
0x2D408D, 0xA0CD4F, 0x99A520, 0xD3A2B3, 0x0A5D2F, 0x42F9B4,
0xCBDA11, 0xD0BE7D, 0xC1DB9B, 0xBD17AB, 0x81A2CA, 0x5C6A08,
0x17552E, 0x550027, 0xF0147F, 0x8607E1, 0x640B14, 0x8D4196,
0xDEBE87, 0x2AFDDA, 0xB6256B, 0x34897B, 0xFEF305, 0x9EBFB9,
0x4F6A68, 0xA82A4A, 0x5AC44F, 0xBCF82D, 0x985AD7, 0x95C7F4,
0x8D4D0D, 0xA63A20, 0x5F57A4, 0xB13F14, 0x953880, 0x0120CC,
0x86DD71, 0xB6DEC9, 0xF560BF, 0x11654D, 0x6B0701, 0xACB08C,
0xD0C0B2, 0x485551, 0x0EFB1E, 0xC37295, 0x3B06A3, 0x3540C0,
0x7BDC06, 0xCC45E0, 0xFA294E, 0xC8CAD6, 0x41F3E8, 0xDE647C,
0xD8649B, 0x31BED9, 0xC397A4, 0xD45877, 0xC5E369, 0x13DAF0,
0x3C3ABA, 0x461846, 0x5F7555, 0xF5BDD2, 0xC6926E, 0x5D2EAC,
0xED440E, 0x423E1C, 0x87C461, 0xE9FD29, 0xF3D6E7, 0xCA7C22,
0x35916F, 0xC5E008, 0x8DD7FF, 0xE26A6E, 0xC6FDB0, 0xC10893,
0x745D7C, 0xB2AD6B, 0x9D6ECD, 0x7B723E, 0x6A11C6, 0xA9CFF7,
0xDF7329, 0xBAC9B5, 0x5100B7, 0x0DB2E2, 0x24BA74, 0x607DE5,
0x8AD874, 0x2C150D, 0x0C1881, 0x94667E, 0x162901, 0x767A9F,
0xBEFDFD, 0xEF4556, 0x367ED9, 0x13D9EC, 0xB9BA8B, 0xFC97C4,
0x27A831, 0xC36EF1, 0x36C594, 0x56A8D8, 0xB5A8B4, 0x0ECCCF,
0x2D8912, 0x34576F, 0x89562C, 0xE3CE99, 0xB920D6, 0xAA5E6B,
0x9C2A3E, 0xCC5F11, 0x4A0BFD, 0xFBF4E1, 0x6D3B8E, 0x2C86E2,
0x84D4E9, 0xA9B4FC, 0xD1EEEF, 0xC9352E, 0x61392F, 0x442138,
0xC8D91B, 0x0AFC81, 0x6A4AFB, 0xD81C2F, 0x84B453, 0x8C994E,
0xCC2254, 0xDC552A, 0xD6C6C0, 0x96190B, 0xB8701A, 0x649569,
0x605A26, 0xEE523F, 0x0F117F, 0x11B5F4, 0xF5CBFC, 0x2DBC34,
0xEEBC34, 0xCC5DE8, 0x605EDD, 0x9B8E67, 0xEF3392, 0xB817C9,
0x9B5861, 0xBC57E1, 0xC68351, 0x103ED8, 0x4871DD, 0xDD1C2D,
0xA118AF, 0x462C21, 0xD7F359, 0x987AD9, 0xC0549E, 0xFA864F,
0xFC0656, 0xAE79E5, 0x362289, 0x22AD38, 0xDC9367, 0xAAE855,
0x382682, 0x9BE7CA, 0xA40D51, 0xB13399, 0x0ED7A9, 0x480569,
0xF0B265, 0xA7887F, 0x974C88, 0x36D1F9, 0xB39221, 0x4A827B,
0x21CF98, 0xDC9F40, 0x5547DC, 0x3A74E1, 0x42EB67, 0xDF9DFE,
0x5FD45E, 0xA4677B, 0x7AACBA, 0xA2F655, 0x23882B, 0x55BA41,
0x086E59, 0x862A21, 0x834739, 0xE6E389, 0xD49EE5, 0x40FB49,
0xE956FF, 0xCA0F1C, 0x8A59C5, 0x2BFA94, 0xC5C1D3, 0xCFC50F,
0xAE5ADB, 0x86C547, 0x624385, 0x3B8621, 0x94792C, 0x876110,
0x7B4C2A, 0x1A2C80, 0x12BF43, 0x902688, 0x893C78, 0xE4C4A8,
0x7BDBE5, 0xC23AC4, 0xEAF426, 0x8A67F7, 0xBF920D, 0x2BA365,
0xB1933D, 0x0B7CBD, 0xDC51A4, 0x63DD27, 0xDDE169, 0x19949A,
0x9529A8, 0x28CE68, 0xB4ED09, 0x209F44, 0xCA984E, 0x638270,
0x237C7E, 0x32B90F, 0x8EF5A7, 0xE75614, 0x08F121, 0x2A9DB5,
0x4D7E6F, 0x5119A5, 0xABF9B5, 0xD6DF82, 0x61DD96, 0x023616,
0x9F3AC4, 0xA1A283, 0x6DED72, 0x7A8D39, 0xA9B882, 0x5C326B,
0x5B2746, 0xED3400, 0x7700D2, 0x55F4FC, 0x4D5901, 0x8071E0,
#endif
};

/*
 * invpio2:  53 bits of 2/pi
 * pio2_1:   first 25 bits of pi/2
 * pio2_1t:  pi/2 - pio2_1
 */
static const double pio4    = 0x1.921fb6p-1;
static const double invpio2 = 6.36619772367581382433e-01; /* 0x3FE45F30, 0x6DC9C883 */
static const double pio2_1  = 1.57079631090164184570e+00; /* 0x3FF921FB, 0x50000000 */
static const double pio2_1t = 1.58932547735281966916e-08; /* 0x3E5110b4, 0x611A6263 */

/*
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */
static const double PIo2[] = {
  1.57079625129699707031e+00, /* 0x3FF921FB, 0x40000000 */
  7.54978941586159635335e-08, /* 0x3E74442D, 0x00000000 */
  5.39030252995776476554e-15, /* 0x3CF84698, 0x80000000 */
  3.28200341580791294123e-22, /* 0x3B78CC51, 0x60000000 */
  1.27065575308067607349e-29, /* 0x39F01B83, 0x80000000 */
  1.22933308981111328932e-36, /* 0x387A2520, 0x40000000 */
  2.73370053816464559624e-44, /* 0x36E38222, 0x80000000 */
  2.16741683877804819444e-51, /* 0x3569F31D, 0x00000000 */
};

/* |tan(x)/x - t(x)| < 2**-25.5 (~[-2e-08, 2e-08]). */
static const double T[] = {
  0x15554d3418c99f.0p-54, /* 0.333331395030791399758 */
  0x1112fd38999f72.0p-55, /* 0.133392002712976742718 */
  0x1b54c91d865afe.0p-57, /* 0.0533812378445670393523 */
  0x191df3908c33ce.0p-58, /* 0.0245283181166547278873 */
  0x185dadfcecf44e.0p-61, /* 0.00297435743359967304927 */
  0x1362b9bf971bcd.0p-59, /* 0.00946564784943673166728 */
};

/* |cos(x) - c(x)| < 2**-34.1 (~[-5.37e-11, 5.295e-11]). */
static const double C0  = -0x1ffffffd0c5e81.0p-54; /* -0.499999997251031003120 */
static const double C1  =  0x155553e1053a42.0p-57; /*  0.0416666233237390631894 */
static const double C2  = -0x16c087e80f1e27.0p-62; /* -0.00138867637746099294692 */
static const double C3  =  0x199342e0ee5069.0p-68; /*  0.0000243904487962774090654 */

/* |sin(x)/x - s(x)| < 2**-37.5 (~[-4.89e-12, 4.824e-12]). */
static const double S1 = -0x15555554cbac77.0p-55; /* -0.166666666416265235595 */
static const double S2 =  0x111110896efbb2.0p-59; /*  0.0083333293858894631756 */
static const double S3 = -0x1a00f9e2cae774.0p-65; /* -0.000198393348360966317347 */
static const double S4 =  0x16cd878c3b46a7.0p-71; /*  0.0000027183114939898219064 */

static const double s1pio2 = 1*M_PI_2; /* 0x3FF921FB, 0x54442D18 */
static const double s2pio2 = 2*M_PI_2; /* 0x400921FB, 0x54442D18 */
static const double s3pio2 = 3*M_PI_2; /* 0x4012D97C, 0x7F3321D2 */
static const double s4pio2 = 4*M_PI_2; /* 0x401921FB, 0x54442D18 */


static const double pio2_hi = 1.57079632679489655800e+00; /* 0x3FF921FB, 0x54442D18 */
static const double pio2_lo = 6.12323399573676603587e-17; /* 0x3C91A626, 0x33145C07 */
static const double pS0 =  1.66666666666666657415e-01; /* 0x3FC55555, 0x55555555 */
static const double pS1 = -3.25565818622400915405e-01; /* 0xBFD4D612, 0x03EB6F7D */
static const double pS2 =  2.01212532134862925881e-01; /* 0x3FC9C155, 0x0E884455 */
static const double pS3 = -4.00555345006794114027e-02; /* 0xBFA48228, 0xB5688F3B */
static const double pS4 =  7.91534994289814532176e-04; /* 0x3F49EFE0, 0x7501B288 */
static const double pS5 =  3.47933107596021167570e-05; /* 0x3F023DE1, 0x0DFDF709 */
static const double qS1 = -2.40339491173441421878e+00; /* 0xC0033A27, 0x1C8A2D4B */
static const double qS2 =  2.02094576023350569471e+00; /* 0x40002AE5, 0x9C598AC8 */
static const double qS3 = -6.88283971605453293030e-01; /* 0xBFE6066C, 0x1B8D0159 */
static const double qS4 =  7.70381505559019352791e-02; /* 0x3FB3B8C5, 0xB12E9282 */

const uint16_t __rsqrt_tab[128] = {
0xb451,0xb2f0,0xb196,0xb044,0xaef9,0xadb6,0xac79,0xab43,
0xaa14,0xa8eb,0xa7c8,0xa6aa,0xa592,0xa480,0xa373,0xa26b,
0xa168,0xa06a,0x9f70,0x9e7b,0x9d8a,0x9c9d,0x9bb5,0x9ad1,
0x99f0,0x9913,0x983a,0x9765,0x9693,0x95c4,0x94f8,0x9430,
0x936b,0x92a9,0x91ea,0x912e,0x9075,0x8fbe,0x8f0a,0x8e59,
0x8daa,0x8cfe,0x8c54,0x8bac,0x8b07,0x8a64,0x89c4,0x8925,
0x8889,0x87ee,0x8756,0x86c0,0x862b,0x8599,0x8508,0x8479,
0x83ec,0x8361,0x82d8,0x8250,0x81c9,0x8145,0x80c2,0x8040,
0xff02,0xfd0e,0xfb25,0xf947,0xf773,0xf5aa,0xf3ea,0xf234,
0xf087,0xeee3,0xed47,0xebb3,0xea27,0xe8a3,0xe727,0xe5b2,
0xe443,0xe2dc,0xe17a,0xe020,0xdecb,0xdd7d,0xdc34,0xdaf1,
0xd9b3,0xd87b,0xd748,0xd61a,0xd4f1,0xd3cd,0xd2ad,0xd192,
0xd07b,0xcf69,0xce5b,0xcd51,0xcc4a,0xcb48,0xca4a,0xc94f,
0xc858,0xc764,0xc674,0xc587,0xc49d,0xc3b7,0xc2d4,0xc1f4,
0xc116,0xc03c,0xbf65,0xbe90,0xbdbe,0xbcef,0xbc23,0xbb59,
0xba91,0xb9cc,0xb90a,0xb84a,0xb78c,0xb6d0,0xb617,0xb560,
};

static const float R_pS0 =  1.6666586697e-01;
static const float R_pS1 = -4.2743422091e-02;
static const float R_pS2 = -8.6563630030e-03;
static const float R_qS1 = -7.0662963390e-01;

static const double pi = 3.141592653589793238462643383279502884;

/*
 * Domain [-0.34568, 0.34568], range ~[-6.694e-10, 6.696e-10]:
 * |6 / x * (1 + 2 * (1 / (exp(x) - 1) - 1 / x)) - q(x)| < 2**-30.04
 * Scaled coefficients: Qn_here = 2**n * Qn_for_q (see s_expm1.c):
 */
static const float Q1 = -3.3333212137e-2; /* -0x888868.0p-28 */
static const float Q2 =  1.5807170421e-3; /*  0xcf3010.0p-33 */

static const double ivln10hi  = 4.34294481878168880939e-01; /* 0x3fdbcb7b, 0x15200000 */
static const double ivln10lo  = 2.50829467116452752298e-11; /* 0x3dbb9438, 0xca9aadd5 */
static const double log10_2hi = 3.01029995663611771306e-01; /* 0x3FD34413, 0x509F6000 */
static const double log10_2lo = 3.69423907715893078616e-13; /* 0x3D59FEF3, 0x11F12B36 */
static const double Lg1 = 6.666666666666735130e-01;  /* 3FE55555 55555593 */
static const double Lg2 = 3.999999999940941908e-01;  /* 3FD99999 9997FA04 */
static const double Lg3 = 2.857142874366239149e-01;  /* 3FD24924 94229359 */
static const double Lg4 = 2.222219843214978396e-01;  /* 3FCC71C5 1D8E78AF */
static const double Lg5 = 1.818357216161805012e-01;  /* 3FC74664 96CB03DE */
static const double Lg6 = 1.531383769920937332e-01;  /* 3FC39A09 D078C69F */
static const double Lg7 = 1.479819860511658591e-01;  /* 3FC2F112 DF3E5244 */

// used for testing if a float is an integer or not
static const uint8_t gMaskShift[256] = {
    0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    //16
    0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    //32
    0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    //48
    0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    //64
    0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    //80
    0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    //96
    0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    //112
    0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7,    //128
    8, 9,10,11, 12,13,14,15,16,17,18,19,20,21,22,23,    //144
    24,25,26,27, 28,29,30,31,31,31,31,31,31,31,31,31,    //160
    31,31,31,31, 31,31,31,31,31,31,31,31,31,31,31,31,    //176
    31,31,31,31, 31,31,31,31,31,31,31,31,31,31,31,31,    //192
    31,31,31,31, 31,31,31,31,31,31,31,31,31,31,31,31,    //208
    31,31,31,31, 31,31,31,31,31,31,31,31,31,31,31,31,    //224
    31,31,31,31, 31,31,31,31,31,31,31,31,31,31,31,31,    //240
    31,31,31,31, 31,31,31,31,31,31,31,31,31,31,31,31,    //256
};

static const double powf_log_table[256] = {
    0x1p+0,                  -0x0p+0,               // 1, -log2l(1)
    0x1.fcp-1,               0x1.72c7ba20f7327p-7,  // 0.992188, -log2l(0.992188)
    0x1.f80fe03f80fep-1,     0x1.715662c7f3dbcp-6,  // 0.984496, -log2l(0.984496)
    0x1.f42f42f42f42fp-1,    0x1.13eea2b6545dfp-5,  // 0.976923, -log2l(0.976923)
    0x1.f05dcd30dadecp-1,    0x1.6e7f0bd9710ddp-5,  // 0.969466, -log2l(0.969466)
    0x1.ec9b26c9b26cap-1,    0x1.c85f25e12da51p-5,  // 0.962121, -log2l(0.962121)
    0x1.e8e6fa39be8e7p-1,    0x1.10c8cd0c74414p-4,  // 0.954887, -log2l(0.954887)
    0x1.e540f4898d5f8p-1,    0x1.3d0c813e48ep-4,    // 0.947761, -log2l(0.947761)
    0x1.e1a8c536fe1a9p-1,    0x1.68fbf5169e028p-4,  // 0.940741, -log2l(0.940741)
    0x1.de1e1e1e1e1e2p-1,    0x1.949866f0b017bp-4,  // 0.933824, -log2l(0.933824)
    0x1.daa0b3630957dp-1,    0x1.bfe30e28821cp-4,   // 0.927007, -log2l(0.927007)
    0x1.d7303b5cc0ed7p-1,    0x1.eadd1b4ef9a1fp-4,  // 0.92029, -log2l(0.92029)
    0x1.d3cc6e80ebbdbp-1,    0x1.0ac3dc2e0ca0cp-3,  // 0.913669, -log2l(0.913669)
    0x1.d075075075075p-1,    0x1.1ff2046fb7116p-3,  // 0.907143, -log2l(0.907143)
    0x1.cd29c244fe2f3p-1,    0x1.34f99517622aep-3,  // 0.900709, -log2l(0.900709)
    0x1.c9ea5dbf193d5p-1,    0x1.49db19c99a54dp-3,  // 0.894366, -log2l(0.894366)
    0x1.c6b699f5423cep-1,    0x1.5e971b3a4ee8p-3,   // 0.888112, -log2l(0.888112)
    0x1.c38e38e38e38ep-1,    0x1.732e1f41ccdbap-3,  // 0.881944, -log2l(0.881944)
    0x1.c070fe3c070fep-1,    0x1.87a0a8f0ff9b2p-3,  // 0.875862, -log2l(0.875862)
    0x1.bd5eaf57abd5fp-1,    0x1.9bef38a4ffae5p-3,  // 0.869863, -log2l(0.869863)
    0x1.ba5713280dee9p-1,    0x1.b01a4c19f6811p-3,  // 0.863946, -log2l(0.863946)
    0x1.b759f2298375ap-1,    0x1.c4225e7d5e3c6p-3,  // 0.858108, -log2l(0.858108)
    0x1.b4671655e7f24p-1,    0x1.d807e87fa4521p-3,  // 0.852349, -log2l(0.852349)
    0x1.b17e4b17e4b18p-1,    0x1.ebcb6065350a2p-3,  // 0.846667, -log2l(0.846667)
    0x1.ae9f5d3eba7d7p-1,    0x1.ff6d3a16f617fp-3,  // 0.84106, -log2l(0.84106)
    0x1.abca1af286bcap-1,    0x1.0976f3991af9ep-2,  // 0.835526, -log2l(0.835526)
    0x1.a8fe53a8fe53bp-1,    0x1.1326eb8c0aba3p-2,  // 0.830065, -log2l(0.830065)
    0x1.a63bd81a98ef6p-1,    0x1.1cc6bb7e3870fp-2,  // 0.824675, -log2l(0.824675)
    0x1.a3827a3827a38p-1,    0x1.265698fa26c0ap-2,  // 0.819355, -log2l(0.819355)
    0x1.a0d20d20d20d2p-1,    0x1.2fd6b881e82d3p-2,  // 0.814103, -log2l(0.814103)
    0x1.9e2a65187566cp-1,    0x1.39474d95e1649p-2,  // 0.808917, -log2l(0.808917)
    0x1.9b8b577e61371p-1,    0x1.42a88abb54986p-2,  // 0.803797, -log2l(0.803797)
    0x1.98f4bac46d7cp-1,     0x1.4bfaa182b7fe3p-2,  // 0.798742, -log2l(0.798742)
    0x1.9666666666666p-1,    0x1.553dc28dd9724p-2,  // 0.79375, -log2l(0.79375)
    0x1.93e032e1c9f02p-1,    0x1.5e721d95d124dp-2,  // 0.78882, -log2l(0.78882)
    0x1.9161f9add3c0dp-1,    0x1.6797e170c5221p-2,  // 0.783951, -log2l(0.783951)
    0x1.8eeb9533d4065p-1,    0x1.70af3c177f74p-2,   // 0.779141, -log2l(0.779141)
    0x1.8c7ce0c7ce0c8p-1,    0x1.79b85aaad8878p-2,  // 0.77439, -log2l(0.77439)
    0x1.8a15b8a15b8a1p-1,    0x1.82b36978f76d5p-2,  // 0.769697, -log2l(0.769697)
    0x1.87b5f9d4d1bc2p-1,    0x1.8ba09402697edp-2,  // 0.76506, -log2l(0.76506)
    0x1.855d824ca58e9p-1,    0x1.948004ff12dbfp-2,  // 0.760479, -log2l(0.760479)
    0x1.830c30c30c30cp-1,    0x1.9d51e662f92a2p-2,  // 0.755952, -log2l(0.755952)
    0x1.80c1e4bbd595fp-1,    0x1.a6166162e9ec8p-2,  // 0.751479, -log2l(0.751479)
    0x1.7e7e7e7e7e7e8p-1,    0x1.aecd9e78fdbeap-2,  // 0.747059, -log2l(0.747059)
    0x1.7c41df1077c42p-1,    0x1.b777c568f9ae2p-2,  // 0.74269, -log2l(0.74269)
    0x1.7a0be82fa0be8p-1,    0x1.c014fd448fe3ap-2,  // 0.738372, -log2l(0.738372)
    0x1.77dc7c4cf2aeap-1,    0x1.c8a56c6f80bcap-2,  // 0.734104, -log2l(0.734104)
    0x1.75b37e875b37fp-1,    0x1.d12938a39d6fp-2,   // 0.729885, -log2l(0.729885)
    0x1.7390d2a6c405ep-1,    0x1.d9a086f4ad416p-2,  // 0.725714, -log2l(0.725714)
    0x1.71745d1745d17p-1,    0x1.e20b7bd4365a8p-2,  // 0.721591, -log2l(0.721591)
    0x1.6f5e02e4850ffp-1,    0x1.ea6a3b152b1e6p-2,  // 0.717514, -log2l(0.717514)
    0x1.6d4da9b536a6dp-1,    0x1.f2bce7ef7d06bp-2,  // 0.713483, -log2l(0.713483)
    0x1.6b4337c6cb157p-1,    0x1.fb03a50395dbap-2,  // 0.709497, -log2l(0.709497)
    0x1.693e93e93e93fp-1,    0x1.019f4a2edc134p-1,  // 0.705556, -log2l(0.705556)
    0x1.673fa57b0cbabp-1,    0x1.05b6ebbca3d9ap-1,  // 0.701657, -log2l(0.701657)
    0x1.6546546546546p-1,    0x1.09c8c7a1fd74cp-1,  // 0.697802, -log2l(0.697802)
    0x1.63528917c80b3p-1,    0x1.0dd4ee107ae0ap-1,  // 0.693989, -log2l(0.693989)
    0x1.61642c8590b21p-1,    0x1.11db6ef5e7873p-1,  // 0.690217, -log2l(0.690217)
    0x1.5f7b282135f7bp-1,    0x1.15dc59fdc06b7p-1,  // 0.686486, -log2l(0.686486)
    0x1.5d9765d9765d9p-1,    0x1.19d7be92a231p-1,   // 0.682796, -log2l(0.682796)
    0x1.5bb8d015e75bcp-1,    0x1.1dcdabdfad537p-1,  // 0.679144, -log2l(0.679144)
    0x1.59df51b3bea36p-1,    0x1.21be30d1e0ddbp-1,  // 0.675532, -log2l(0.675532)
    0x1.580ad602b580bp-1,    0x1.25a95c196bef3p-1,  // 0.671958, -log2l(0.671958)
    0x1.563b48c20563bp-1,    0x1.298f3c2af6595p-1,  // 0.668421, -log2l(0.668421)
    0x1.5470961d7ca63p-1,    0x1.2d6fdf40e09c5p-1,  // 0.664921, -log2l(0.664921)
    0x1.52aaaaaaaaaabp-1,    0x1.314b535c7b89ep-1,  // 0.661458, -log2l(0.661458)
    0x1.50e97366227cbp-1,    0x1.3521a64737cf3p-1,  // 0.658031, -log2l(0.658031)
    0x1.4f2cddb0d3225p-1,    0x1.38f2e593cda73p-1,  // 0.654639, -log2l(0.654639)
    0x1.4d74d74d74d75p-1,    0x1.3cbf1e9f5cf2fp-1,  // 0.651282, -log2l(0.651282)
    0x1.4bc14e5e0a72fp-1,    0x1.40865e9285f33p-1,  // 0.647959, -log2l(0.647959)
    0x1.4a1231617641p-1,     0x1.4448b2627ade3p-1,  // 0.64467, -log2l(0.64467)
    0x1.48676f31219dcp-1,    0x1.480626d20a876p-1,  // 0.641414, -log2l(0.641414)
    0x1.46c0f6feb6ac6p-1,    0x1.4bbec872a4505p-1,  // 0.638191, -log2l(0.638191)
    0x1.451eb851eb852p-1,    0x1.4f72a3a555958p-1,  // 0.635, -log2l(0.635)
    0x1.4380a3065e3fbp-1,    0x1.5321c49bc0c91p-1,  // 0.631841, -log2l(0.631841)
    0x1.41e6a74981447p-1,    0x1.56cc37590e6c5p-1,  // 0.628713, -log2l(0.628713)
    0x1.4050b59897548p-1,    0x1.5a7207b2d815ap-1,  // 0.625616, -log2l(0.625616)
    0x1.3ebebebebebecp-1,    0x1.5e1341520dbp-1,    // 0.622549, -log2l(0.622549)
    0x1.3d30b3d30b3d3p-1,    0x1.61afefb3d5201p-1,  // 0.619512, -log2l(0.619512)
    0x1.3ba68636adfbp-1,     0x1.65481e2a6477bp-1,  // 0.616505, -log2l(0.616505)
    0x1.3a2027932b48fp-1,    0x1.68dbd7ddd6e15p-1,  // 0.613527, -log2l(0.613527)
    0x1.389d89d89d89ep-1,    0x1.6c6b27ccfc698p-1,  // 0.610577, -log2l(0.610577)
    0x1.371e9f3c04e64p-1,    0x1.6ff618ce24cd7p-1,  // 0.607656, -log2l(0.607656)
    0x1.35a35a35a35a3p-1,    0x1.737cb58fe5716p-1,  // 0.604762, -log2l(0.604762)
    0x1.342bad7f64b39p-1,    0x1.76ff0899daa49p-1,  // 0.601896, -log2l(0.601896)
    0x1.32b78c13521dp-1,     0x1.7a7d1c4d6452p-1,   // 0.599057, -log2l(0.599057)
    0x1.3146e92a10d38p-1,    0x1.7df6fae65e424p-1,  // 0.596244, -log2l(0.596244)
    0x1.2fd9b8396ba9ep-1,    0x1.816cae7bd40b1p-1,  // 0.593458, -log2l(0.593458)
    0x1.2e6fecf2e6fedp-1,    0x1.84de4100b0ce2p-1,  // 0.590698, -log2l(0.590698)
    0x1.2d097b425ed09p-1,    0x1.884bbc446ae3fp-1,  // 0.587963, -log2l(0.587963)
    0x1.2ba6574cae996p-1,    0x1.8bb529f3ab8f3p-1,  // 0.585253, -log2l(0.585253)
    0x1.2a46756e62a46p-1,    0x1.8f1a9398f2d58p-1,  // 0.582569, -log2l(0.582569)
    0x1.28e9ca3a728eap-1,    0x1.927c029d3798ap-1,  // 0.579909, -log2l(0.579909)
    0x1.27904a7904a79p-1,    0x1.95d980488409ap-1,  // 0.577273, -log2l(0.577273)
    0x1.2639eb2639eb2p-1,    0x1.993315c28e8fbp-1,  // 0.574661, -log2l(0.574661)
    0x1.24e6a171024e7p-1,    0x1.9c88cc134f3c3p-1,  // 0.572072, -log2l(0.572072)
    0x1.239662b9f91cbp-1,    0x1.9fdaac2391e1cp-1,  // 0.569507, -log2l(0.569507)
    0x1.2249249249249p-1,    0x1.a328bebd84e8p-1,   // 0.566964, -log2l(0.566964)
    0x1.20fedcba98765p-1,    0x1.a6730c8d44efap-1,  // 0.564444, -log2l(0.564444)
    0x1.1fb78121fb781p-1,    0x1.a9b99e21655ebp-1,  // 0.561947, -log2l(0.561947)
    0x1.1e7307e4ef157p-1,    0x1.acfc7beb75e94p-1,  // 0.559471, -log2l(0.559471)
    0x1.1d31674c59d31p-1,    0x1.b03bae40852ap-1,   // 0.557018, -log2l(0.557018)
    0x1.1bf295cc93903p-1,    0x1.b3773d59a05ffp-1,  // 0.554585, -log2l(0.554585)
    0x1.1ab68a0473c1bp-1,    0x1.b6af315450638p-1,  // 0.552174, -log2l(0.552174)
    0x1.197d3abc65f4fp-1,    0x1.b9e3923313e58p-1,  // 0.549784, -log2l(0.549784)
    0x1.18469ee58469fp-1,    0x1.bd1467ddd70a7p-1,  // 0.547414, -log2l(0.547414)
    0x1.1712ad98b8957p-1,    0x1.c041ba2268731p-1,  // 0.545064, -log2l(0.545064)
    0x1.15e15e15e15e1p-1,    0x1.c36b90b4ebc3ap-1,  // 0.542735, -log2l(0.542735)
    0x1.14b2a7c2fee92p-1,    0x1.c691f33049bap-1,   // 0.540426, -log2l(0.540426)
    0x1.1386822b63cbfp-1,    0x1.c9b4e9169de22p-1,  // 0.538136, -log2l(0.538136)
    0x1.125ce4feeb7a1p-1,    0x1.ccd479d1a1f94p-1,  // 0.535865, -log2l(0.535865)
    0x1.1135c81135c81p-1,    0x1.cff0acb3170e3p-1,  // 0.533613, -log2l(0.533613)
    0x1.10112358e75d3p-1,    0x1.d30988f52c6d3p-1,  // 0.531381, -log2l(0.531381)
    0x1.0eeeeeeeeeeefp-1,    0x1.d61f15bae4663p-1,  // 0.529167, -log2l(0.529167)
    0x1.0dcf230dcf231p-1,    0x1.d9315a1076fa2p-1,  // 0.526971, -log2l(0.526971)
    0x1.0cb1b810ecf57p-1,    0x1.dc405cebb27dcp-1,  // 0.524793, -log2l(0.524793)
    0x1.0b96a673e2808p-1,    0x1.df4c252c5a3e1p-1,  // 0.522634, -log2l(0.522634)
    0x1.0a7de6d1d6086p-1,    0x1.e254b99c83339p-1,  // 0.520492, -log2l(0.520492)
    0x1.096771e4d528cp-1,    0x1.e55a20f0eecf9p-1,  // 0.518367, -log2l(0.518367)
    0x1.0853408534085p-1,    0x1.e85c61c963f0dp-1,  // 0.51626, -log2l(0.51626)
    0x1.07414ba8f0741p-1,    0x1.eb5b82b10609bp-1,  // 0.51417, -log2l(0.51417)
    0x1.06318c6318c63p-1,    0x1.ee578a1eaa83fp-1,  // 0.512097, -log2l(0.512097)
    0x1.0523fbe3367d7p-1,    0x1.f1507e752c6c8p-1,  // 0.51004, -log2l(0.51004)
    0x1.04189374bc6a8p-1,    0x1.f4466603be71dp-1,  // 0.508, -log2l(0.508)
    0x1.030f4c7e7859cp-1,    0x1.f73947063b3fdp-1,  // 0.505976, -log2l(0.505976)
    0x1.0208208208208p-1,    0x1.fa2927a574422p-1,  // 0.503968, -log2l(0.503968)
    0x1.0103091b51f5ep-1,    0x1.fd160df77ed7ap-1,  // 0.501976, -log2l(0.501976)
    0x1p-1,                  0x1p+0,                // 0.5, -log2l(0.5)
};

// origin: FreeBSD /usr/src/lib/msun/src/e_rem_pio2f.c

/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 * Debugged and optimized by Bruce D. Evans.
 */

/* __rem_pio2f(x,y)
 *
 * return the remainder of x rem pi/2 in *y
 * use double precision for everything except passing x
 * use __rem_pio2_large() for large x
 */

static double scalbn(double x, int n) {
    union {double f; uint64_t i;} u;
    double y = x;

    if (n > 1023) {
        y *= 0x1p1023;
        n -= 1023;
        if (n > 1023) {
            y *= 0x1p1023;
            n -= 1023;
            if (n > 1023)
                n = 1023;
        }
    } else if (n < -1022) {
        /* make sure final n < -53 to avoid double
           rounding in the subnormal range */
        y *= 0x1p-1022 * 0x1p53;
        n += 1022 - 53;
        if (n < -1022) {
            y *= 0x1p-1022 * 0x1p53;
            n += 1022 - 53;
            if (n < -1022)
                n = -1022;
        }
    }
    u.i = (uint64_t)(0x3ff+n)<<52;
    x = y * u.f;
    return x;
}

static int __rem_pio2_large(double *x, double *y, int e0, int nx, int prec) {
    int32_t jz,jx,jv,jp,jk,carry,n,iq[20],i,j,k,m,q0,ih;
    double z,fw,f[20],fq[20],q[20];

    /* initialize jk*/
    jk = init_jk[prec];
    jp = jk;

    /* determine jx,jv,q0, note that 3>q0 */
    jx = nx-1;
    jv = (e0-3)/24;  if(jv<0) jv=0;
    q0 = e0-24*(jv+1);

    /* set up f[0] to f[jx+jk] where f[jx+jk] = ipio2[jv+jk] */
    j = jv-jx; m = jx+jk;
    for (i=0; i<=m; i++,j++)
        f[i] = j<0 ? 0.0 : (double)ipio2[j];

    /* compute q[0],q[1],...q[jk] */
    for (i=0; i<=jk; i++) {
        for (j=0,fw=0.0; j<=jx; j++)
            fw += x[j]*f[jx+i-j];
        q[i] = fw;
    }

    jz = jk;
recompute:
    /* distill q[] into iq[] reversingly */
    for (i=0,j=jz,z=q[jz]; j>0; i++,j--) {
        fw    = (double)(int32_t)(0x1p-24*z);
        iq[i] = (int32_t)(z - 0x1p24*fw);
        z     = q[j-1]+fw;
    }

    /* compute n */
    z  = scalbn(z,q0);       /* actual value of z */
    z -= 8.0*floor(z*0.125); /* trim off integer >= 8 */
    n  = (int32_t)z;
    z -= (double)n;
    ih = 0;
    if (q0 > 0) {  /* need iq[jz-1] to determine n */
        i  = iq[jz-1]>>(24-q0); n += i;
        iq[jz-1] -= i<<(24-q0);
        ih = iq[jz-1]>>(23-q0);
    }
    else if (q0 == 0) ih = iq[jz-1]>>23;
    else if (z >= 0.5) ih = 2;

    if (ih > 0) {  /* q > 0.5 */
        n += 1; carry = 0;
        for (i=0; i<jz; i++) {  /* compute 1-q */
            j = iq[i];
            if (carry == 0) {
                if (j != 0) {
                    carry = 1;
                    iq[i] = 0x1000000 - j;
                }
            } else
                iq[i] = 0xffffff - j;
        }
        if (q0 > 0) {  /* rare case: chance is 1 in 12 */
            switch(q0) {
            case 1:
                iq[jz-1] &= 0x7fffff; break;
            case 2:
                iq[jz-1] &= 0x3fffff; break;
            }
        }
        if (ih == 2) {
            z = 1.0 - z;
            if (carry != 0)
                z -= scalbn(1.0,q0);
        }
    }

    /* check if recomputation is needed */
    if (z == 0.0) {
        j = 0;
        for (i=jz-1; i>=jk; i--) j |= iq[i];
        if (j == 0) {  /* need recomputation */
            for (k=1; iq[jk-k]==0; k++);  /* k = no. of terms needed */

            for (i=jz+1; i<=jz+k; i++) {  /* add q[jz+1] to q[jz+k] */
                f[jx+i] = (double)ipio2[jv+i];
                for (j=0,fw=0.0; j<=jx; j++)
                    fw += x[j]*f[jx+i-j];
                q[i] = fw;
            }
            jz += k;
            goto recompute;
        }
    }

    /* chop off zero terms */
    if (z == 0.0) {
        jz -= 1;
        q0 -= 24;
        while (iq[jz] == 0) {
            jz--;
            q0 -= 24;
        }
    } else { /* break z into 24-bit if necessary */
        z = scalbn(z,-q0);
        if (z >= 0x1p24) {
            fw = (double)(int32_t)(0x1p-24*z);
            iq[jz] = (int32_t)(z - 0x1p24*fw);
            jz += 1;
            q0 += 24;
            iq[jz] = (int32_t)fw;
        } else
            iq[jz] = (int32_t)z;
    }

    /* convert integer "bit" chunk to floating-point value */
    fw = scalbn(1.0,q0);
    for (i=jz; i>=0; i--) {
        q[i] = fw*(double)iq[i];
        fw *= 0x1p-24;
    }

    /* compute PIo2[0,...,jp]*q[jz,...,0] */
    for(i=jz; i>=0; i--) {
        for (fw=0.0,k=0; k<=jp && k<=jz-i; k++)
            fw += PIo2[k]*q[i+k];
        fq[jz-i] = fw;
    }

    /* compress fq[] into y[] */
    switch(prec) {
    case 0:
        fw = 0.0;
        for (i=jz; i>=0; i--)
            fw += fq[i];
        y[0] = ih==0 ? fw : -fw;
        break;
    case 1:
    case 2:
        fw = 0.0;
        for (i=jz; i>=0; i--)
            fw += fq[i];
        // TODO: drop excess precision here once double is used
        fw = (double)fw;
        y[0] = ih==0 ? fw : -fw;
        fw = fq[0]-fw;
        for (i=1; i<=jz; i++)
            fw += fq[i];
        y[1] = ih==0 ? fw : -fw;
        break;
    case 3:  /* painful */
        for (i=jz; i>0; i--) {
            fw      = fq[i-1]+fq[i];
            fq[i]  += fq[i-1]-fw;
            fq[i-1] = fw;
        }
        for (i=jz; i>1; i--) {
            fw      = fq[i-1]+fq[i];
            fq[i]  += fq[i-1]-fw;
            fq[i-1] = fw;
        }
        for (fw=0.0,i=jz; i>=2; i--)
            fw += fq[i];
        if (ih==0) {
            y[0] =  fq[0]; y[1] =  fq[1]; y[2] =  fw;
        } else {
            y[0] = -fq[0]; y[1] = -fq[1]; y[2] = -fw;
        }
    }
    return n&7;
}

static int __rem_pio2f(float x, double *y) {
    union {float f; uint32_t i;} u = {x};
    double tx[1],ty[1];
    double fn;
    uint32_t ix;
    int n, sign, e0;

    ix = u.i & 0x7fffffff;
    /* 25+53 bit pi is good enough for medium size */
    if (ix < 0x4dc90fdb) {  /* |x| ~< 2^28*(pi/2), medium size */
        /* Use a specialized rint() to get fn. */
        fn = (double)x*invpio2 + toint - toint;
        n  = (int32_t)fn;
        *y = x - fn*pio2_1 - fn*pio2_1t;
        /* Matters with directed rounding. */
        if (*y < -pio4) {
            n--;
            fn--;
            *y = x - fn*pio2_1 - fn*pio2_1t;
        } else if (*y > pio4) {
            n++;
            fn++;
            *y = x - fn*pio2_1 - fn*pio2_1t;
        }
        return n;
    }
    if(ix>=0x7f800000) {  /* x is inf or NaN */
        *y = x-x;
        return 0;
    }
    /* scale x into [2^23, 2^24-1] */
    sign = u.i>>31;
    e0 = (ix>>23) - (0x7f+23);  /* e0 = ilogb(|x|)-23, positive */
    u.i = ix - (e0<<23);
    tx[0] = u.f;
    n  =  __rem_pio2_large(tx,ty,e0,1,0);
    if (sign) {
        *y = -ty[0];
        return -n;
    }
    *y = ty[0];
    return n;
}

// origin: FreeBSD /usr/src/lib/msun/src/k_tanf.c

/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 * Optimized by Bruce D. Evans.
 */

static float __tandf(double x, int odd) {
    double z,r,w,s,t,u;

    z = x*x;
    /*
     * Split up the polynomial into small independent terms to give
     * opportunities for parallel evaluation.  The chosen splitting is
     * micro-optimized for Athlons (XP, X64).  It costs 2 multiplications
     * relative to Horner's method on sequential machines.
     *
     * We add the small terms from lowest degree up for efficiency on
     * non-sequential machines (the lowest degree terms tend to be ready
     * earlier).  Apart from this, we don't care about order of
     * operations, and don't need to to care since we have precision to
     * spare.  However, the chosen splitting is good for accuracy too,
     * and would give results as accurate as Horner's method if the
     * small terms were added from highest degree down.
     */
    r = T[4] + z*T[5];
    t = T[2] + z*T[3];
    w = z*z;
    s = z*x;
    u = T[0] + z*T[1];
    r = (x + s*u) + (s*w)*(t + w*r);
    return odd ? -1.0/r : r;
}

static float __cosdf(double x) {
    double r, w, z;

    /* Try to optimize for parallel evaluation as in __tandf.c. */
    z = x*x;
    w = z*z;
    r = C2+z*C3;
    return ((1.0+z*C0) + w*C1) + (w*z)*r;
}

static float __sindf(double x) {
    double r, s, w, z;

    /* Try to optimize for parallel evaluation as in __tandf.c. */
    z = x*x;
    w = z*z;
    r = S3 + z*S4;
    s = z*x;
    return (x + s*(S1 + z*S2)) + s*w*r;
}

static double R_acos(double z) {
    double p, q;
    p = z*(pS0+z*(pS1+z*(pS2+z*(pS3+z*(pS4+z*pS5)))));
    q = 1.0+z*(qS1+z*(qS2+z*(qS3+z*qS4)));
    return p/q;
}

static double __math_invalid(double x) {
    return (x - x) / (x - x);
}

/* returns a*b*2^-32 - e, with error 0 <= e < 1.  */
static inline uint32_t mul32(uint32_t a, uint32_t b) {
    return (uint64_t)a*b >> 32;
}

/* returns a*b*2^-64 - e, with error 0 <= e < 3.  */
static inline uint64_t mul64(uint64_t a, uint64_t b) {
    uint64_t ahi = a>>32;
    uint64_t alo = a&0xffffffff;
    uint64_t bhi = b>>32;
    uint64_t blo = b&0xffffffff;
    return ahi*bhi + (ahi*blo >> 32) + (alo*bhi >> 32);
}

static inline double eval_as_double(double x) {
    double y = x;
    return y;
}

static inline float eval_as_float(float x) {
    float y = x;
    return y;
}

static float R_acosf(float z) {
    float p, q;
    p = z*(R_pS0+z*(R_pS1+z*R_pS2));
    q = 1.0f+z*R_qS1;
    return p/q;
}

static float __math_invalidf(float x) {
    return (x - x) / (x - x);
}

// origin: FreeBSD /usr/src/lib/msun/src/e_acosf.c

/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */

float acosf(float x) {
    float z,w,s,c,df;
    uint32_t hx,ix;

    GET_FLOAT_WORD(hx, x);
    ix = hx & 0x7fffffff;
    /* |x| >= 1 or nan */
    if (ix >= 0x3f800000) {
        if (ix == 0x3f800000) {
            if (hx >> 31)
                return 2*pio2_hi + 0x1p-120f;
            return 0;
        }
        return 0/(x-x);
    }
    /* |x| < 0.5 */
    if (ix < 0x3f000000) {
        if (ix <= 0x32800000) /* |x| < 2**-26 */
            return pio2_hi + 0x1p-120f;
        return pio2_hi - (x - (pio2_lo-x*R_acosf(x*x)));
    }
    /* x < -0.5 */
    if (hx >> 31) {
        z = (1+x)*0.5f;
        s = sqrtf(z);
        w = R_acosf(z)*s-pio2_lo;
        return 2*(pio2_hi - (s+w));
    }
    /* x > 0.5 */
    z = (1-x)*0.5f;
    s = sqrtf(z);
    GET_FLOAT_WORD(hx,s);
    SET_FLOAT_WORD(df,hx&0xfffff000);
    c = (z-df*df)/(s+df);
    w = R_acosf(z)*s+c;
    return 2*(df+w);
}

// origin: FreeBSD /usr/src/lib/msun/src/s_cosf.c

/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 * Optimized by Bruce D. Evans.
 */

float cosf(float x) {
    double y;
    uint32_t ix;
    unsigned n, sign;

    GET_FLOAT_WORD(ix, x);
    sign = ix >> 31;
    ix &= 0x7fffffff;

    if (ix <= 0x3f490fda) {  /* |x| ~<= pi/4 */
        if (ix < 0x39800000) {  /* |x| < 2**-12 */
            /* raise inexact if x != 0 */
            FORCE_EVAL(x + 0x1p120f);
            return 1.0f;
        }
        return __cosdf(x);
    }
    if (ix <= 0x407b53d1) {  /* |x| ~<= 5*pi/4 */
        if (ix > 0x4016cbe3)  /* |x|  ~> 3*pi/4 */
            return -__cosdf(sign ? x+c2pio2 : x-c2pio2);
        else {
            if (sign)
                return __sindf(x + c1pio2);
            else
                return __sindf(c1pio2 - x);
        }
    }
    if (ix <= 0x40e231d5) {  /* |x| ~<= 9*pi/4 */
        if (ix > 0x40afeddf)  /* |x| ~> 7*pi/4 */
            return __cosdf(sign ? x+c4pio2 : x-c4pio2);
        else {
            if (sign)
                return __sindf(-x - c3pio2);
            else
                return __sindf(x - c3pio2);
        }
    }

    /* cos(Inf or NaN) is NaN */
    if (ix >= 0x7f800000)
        return x-x;

    /* general argument reduction needed */
    n = __rem_pio2f(x,&y);
    switch (n&3) {
    case 0: return  __cosdf(y);
    case 1: return  __sindf(-y);
    case 2: return -__cosdf(y);
    default:
        return  __sindf(y);
    }
}

float fabsf(float x) {
    union {float f; uint32_t i;} u = {x};
    u.i &= 0x7fffffff;
    return u.f;
}

double floor(double x) {
    int interger = (int) x;
    if (x < 0 && x != interger) {
        interger--;
    }
    return (double) interger;
}

// origin: FreeBSD /usr/src/lib/msun/src/e_log10.c

/*
 * Return the base 10 logarithm of x.  See log.c for most comments.
 *
 * Reduce x to 2^k (1+f) and calculate r = log(1+f) - f + f*f/2
 * as in log.c, then combine and scale in extra precision:
 *    log10(x) = (f - f*f/2 + r)/log(10) + k*log10(2)
 */

double log10(double x) {
    union {double f; uint64_t i;} u = {x};
    double hfsq,f,s,z,R,w,t1,t2,dk,y,hi,lo,val_hi,val_lo;
    uint32_t hx;
    int k;

    hx = u.i>>32;
    k = 0;
    if (hx < 0x00100000 || hx>>31) {
        if (u.i<<1 == 0)
            return -1/(x*x);  /* log(+-0)=-inf */
        if (hx>>31)
            return (x-x)/0.0; /* log(-#) = NaN */
        /* subnormal number, scale x up */
        k -= 54;
        x *= 0x1p54;
        u.f = x;
        hx = u.i>>32;
    } else if (hx >= 0x7ff00000) {
        return x;
    } else if (hx == 0x3ff00000 && u.i<<32 == 0)
        return 0;

    /* reduce x into [sqrt(2)/2, sqrt(2)] */
    hx += 0x3ff00000 - 0x3fe6a09e;
    k += (int)(hx>>20) - 0x3ff;
    hx = (hx&0x000fffff) + 0x3fe6a09e;
    u.i = (uint64_t)hx<<32 | (u.i&0xffffffff);
    x = u.f;

    f = x - 1.0;
    hfsq = 0.5*f*f;
    s = f/(2.0+f);
    z = s*s;
    w = z*z;
    t1 = w*(Lg2+w*(Lg4+w*Lg6));
    t2 = z*(Lg1+w*(Lg3+w*(Lg5+w*Lg7)));
    R = t2 + t1;

    /* See log2.c for details. */
    /* hi+lo = f - hfsq + s*(hfsq+R) ~ log(1+f) */
    hi = f - hfsq;
    u.f = hi;
    u.i &= (uint64_t)-1<<32;
    hi = u.f;
    lo = f - hi - hfsq + s*(hfsq+R);

    /* val_hi+val_lo ~ log10(1+f) + k*log10(2) */
    val_hi = hi*ivln10hi;
    dk = k;
    y = dk*log10_2hi;
    val_lo = dk*log10_2lo + (lo+hi)*ivln10lo + lo*ivln10hi;

    /*
     * Extra precision in for adding y is not strictly needed
     * since there is no very large cancellation near x = sqrt(2) or
     * x = 1/sqrt(2), but we do it anyway since it costs little on CPUs
     * with some parallelism and it reduces the error for many args.
     */
    w = y + val_hi;
    val_lo += (y - w) + val_hi;
    val_hi = w;

    return val_lo + val_hi;
}

float powf(float x, float y) {
    static const double recip_ln2 = 0x1.71547652b82fep0;

    if (x == 1.0f || y == 1.0f)
        return x;

    //Move the arguments to the integer registers for bitwise inspection
    union { float f; uint32_t u; } ux, uy;
    ux.f = x;
    uy.f = y;
    uint32_t absux = ux.u & 0x7fffffff;
    uint32_t absuy = uy.u & 0x7fffffff;

    // Handle most edge cases
    //If |x| or |y| is in { +-0, +-Inf, +-NaN }
    if ((ux.u - 1U) >= 0x7f7fffff || (absuy - 1) >= 0x4affffff) {
        // any**0 = 1.0f for all values, including NaN
        if (0 == absuy)
            return 1.0f;

        // handle NaNs
        if (x != x || y != y)
            return x + y;

        // figure out if y is an odd integer
        // Find out if y is an integer or not without raising inexact
        // Note -- independently tested over entire range. Fails for Inf/NaN. We don't care about that here.

        // mask bits cover fractional part of value
        uint32_t fractMask = 0x3fffffffU >> gMaskShift[absuy >> 23];

        // we get away with this because leading exponent bit is never set for |y| < 2.0
        uint32_t onesMask = 0x40000000U >> gMaskShift[absuy >> 23];

        uint32_t fractionalBits = absuy & fractMask;
        uint32_t onesBit = absuy & onesMask;

        if (0 == absux) {
            //if y is an odd integer
            if (0 == fractionalBits && 0 != onesBit) {
                if (y < 0.0f)
                    return 1.0f / x;
                return x;
            }

            // y is not an odd integer
            if (0.0f < y)
                return 0.0f;

            return 1.0f / fabsf(x);
        }

        // deal with infinite y
        if (0x7f800000 == absuy) {
            if (-1.0f == x)
                return 1.0f;

            if (absux > 0x3f800000) { // |x| > 1.0f
                if (0.0f < y)
                    return y;
                else
                    return 0.0f;
            } else { // |x| < 1.0f
                if (0.0f < y)
                    return 0.0f;
                else
                    return 0x7f800000; // +inf
            }
        }

        // we can also deal with x == +inf at this point.
        if (x == 0x7f800000) {
            if (y < 0.0f)
                return 0.0f;
            else
                return x;
        }

        if (x > -0x7f800000) {
            if (fractionalBits)
                goto nan_sqrt;

            goto ipowf;
        }

        // At this point, we know that x is in { +-0, -Inf } and y is finite non-zero.
        // Deal with y is odd integer cases
        if (0 == fractionalBits && 0 != onesBit)    // if (|y| >= 1.0f || |y| < 0x1.0p24f)
            return 0.0f < y ? x : -0.0f;

        // x == -inf
        return 0.0f < y ? -x : 0.0f;
    }

    //special case for sqrts
    if (0x3f000000U == absuy)
        goto nan_sqrt;

    // break |x| into exponent and fractional parts: |x| = 2**i * m     1.0 <= m < 2.0
    int32_t i = ((absux >> 23) & 0xff) - 127;
    union {
        uint32_t u;
        float f;
    } m = { (absux & 0x007fffffU) | 0x3f800000U };

    // normalize denormals
    if (-127 == i) { // denormal
        m.f -= 1.0f;                                // exact
        i = ((m.u >> 23) & 0xff) - (127+126);
        m.u = (m.u & 0x807fffffU) | 0x3f800000U;
    }

    //    We further break down m as :
    //
    //          m = (1+a/256.0)(1+r)              a = high 8 explicit bits of mantissa(m), b = next 7 bits
    //          log2f(m) = log2(1+a/256.0) + log2(1+r)
    //
    //      We use the high 7 bits of the mantissa to look up log2(1+a/256.0) in log2f_table above
    //      We calculate 1+r as:
    //
    //          1+r = m * (1 /(1+a/256.0))
    //
    //      We can lookup (from the same table) the value of 1/(1+a/256.0) based on a too.

    double log2x = i;

    if (m.f != 1.0f) {
        int index = (m.u >> (23-7-4)) & 0x7f0;        //top 7 bits of mantissa
        const double *tablep = (void*) powf_log_table + index;
        double r = (double) m.f;

        // reduce r to  1-2**-7 < r < 1+2**-7
        r *= tablep[0];

        // do this early to force -1.0 + 1.0 to cancel so that we don't end up with (1.0 + tiny) - 1.0 later on.
        log2x += tablep[1];

        // -2**-7 < r < 1+2**-7
        r -= 1.0;

        // ln(1+r) = r - rr/2 + rrr/3 - rrrr/4 + rrrrr/5
        // should provide log(1+r) to at least 35 bits of accuracy for the worst case
        double rr = r*r;
        double small = -0.5 + 0.3333333333333333333333*r;
        double large = -0.25 + 0.2*r;
        double rrrr = rr * rr;
        small *= rr;
        small += r;
        large *= rrrr;
        r = small + large;
        log2x += r * recip_ln2;
    }

    // multiply by Y
    double ylog2x = y * log2x;

    // now we need to calculate 2**ylog2x

    // deal with overflow
    if (ylog2x >= 128.0)
        return (float) (0x1.0p128 * ylog2x);    //set overflow and return inf

    // minimum y * maximum log2(x) is ~-1.0p128 * ~128 = -1.0p135, so we can be sure that we'll drive this to underflow
    if (ylog2x <= -150.0)
        return (float) (ylog2x * 0x1.0p-1022);

    // separate ylog2x into integer and fractional parts
    int exp = (int) ylog2x;
    double f = ylog2x - exp;    // may be negative

    // Calculate 2**fract
    // 8th order minimax fit of exp2 on [-1.0,1.0].  |error| < 0.402865722354948566583852e-9:
    static const double c0 =  1.0 + 0.278626872016317130037181614004e-10;
    static const double c1 = .693147176943623740308984004029708;
    static const double c2 = .240226505817268621584559118975830;
    static const double c3 = 0.555041568519883074165425891257052e-1;
    static const double c4overc8 = 0.961813690023115610862381719985771e-2 / 0.134107709538786543922336536865157e-5;
    static const double c5overc8 = 0.133318252930790403741964203236548e-2 / 0.134107709538786543922336536865157e-5;
    static const double c6overc8 = 0.154016177542147239746127455226575e-3 / 0.134107709538786543922336536865157e-5;
    static const double c7overc8 = 0.154832722143258821052933667742417e-4 / 0.134107709538786543922336536865157e-5;
    static const double c8 = 0.134107709538786543922336536865157e-5;

    double z = 1.0;

    // don't set inexact if we don't need to
    if (0.0 != f) {
        double ff = f * f;
        double s7 = c7overc8 * f;           double s3 = c3 * f;
        double s5 = c5overc8 * f;           double s1 = c1 * f;
        double ffff = ff * ff;
        s7 += c6overc8;                     s3 += c2;
        s5 += c4overc8;                     s1 += c0;
        s7 *= ff;                           s3 *= ff;
        s5 += ffff;
        double c8ffff = ffff * c8;
        s7 += s5;                           s3 += s1;
        s7 *= c8ffff;
        z = s3 + s7;
    }

    // prepare 2**i
    union { uint64_t u; double d; } two_exp = { ((uint64_t) exp + 1023) << 52 };

    return (float) (z * two_exp.d);

    // one last edge case -- pow(x, y) returns NaN and raises invalid for x < 0 and finite non-integer y
    // and one special case --    call sqrt for |y| == 0.5
nan_sqrt:
    if (x < 0.0f || y > 0.0f)
        return sqrtf(x);

    return (float) sqrt(1.0 / (double) x);

ipowf:
    // clamp  -0x1.0p31 < y < 0x1.0p31
    y = y > -0x1.fffffep30f ? y : -0x1.fffffep30f;
    y = y <  0x1.fffffep30f ? y :  0x1.fffffep30f;
    i = (int) y;
    double dx = (double) x;
    double r = 1.0;

    if (i < 0) {
        i = -i;
        dx = 1.0 / dx;
    }

    if (i & 1)
        r = dx;

    do {
        i >>= 1;
        if (0 == i)
            break;
        dx *= dx;
        if (i & 1)
            r *= dx;
    } while(1);

    return (float) r;
}

float sinf(float x) {
    double y;
    uint32_t ix;
    int n, sign;

    GET_FLOAT_WORD(ix, x);
    sign = ix >> 31;
    ix &= 0x7fffffff;

    if (ix <= 0x3f490fda) {  /* |x| ~<= pi/4 */
        if (ix < 0x39800000) {  /* |x| < 2**-12 */
            /* raise inexact if x!=0 and underflow if subnormal */
            FORCE_EVAL(ix < 0x00800000 ? x/0x1p120f : x+0x1p120f);
            return x;
        }
        return __sindf(x);
    }
    if (ix <= 0x407b53d1) {  /* |x| ~<= 5*pi/4 */
        if (ix <= 0x4016cbe3) {  /* |x| ~<= 3pi/4 */
            if (sign)
                return -__cosdf(x + s1pio2);
            else
                return __cosdf(x - s1pio2);
        }
        return __sindf(sign ? -(x + s2pio2) : -(x - s2pio2));
    }
    if (ix <= 0x40e231d5) {  /* |x| ~<= 9*pi/4 */
        if (ix <= 0x40afeddf) {  /* |x| ~<= 7*pi/4 */
            if (sign)
                return __cosdf(x + s3pio2);
            else
                return -__cosdf(x - s3pio2);
        }
        return __sindf(sign ? x + s4pio2 : x - s4pio2);
    }

    /* sin(Inf or NaN) is NaN */
    if (ix >= 0x7f800000)
        return x - x;

    /* general argument reduction needed */
    n = __rem_pio2f(x, &y);
    switch (n&3) {
    case 0: return  __sindf(y);
    case 1: return  __cosdf(y);
    case 2: return  __sindf(-y);
    default:
        return -__cosdf(y);
    }
}

double sqrt(double x) {
    uint64_t ix, top, m;

    /* special case handling.  */
    ix = ASUINT64(x);
    top = ix >> 52;
    if (top - 0x001 >= 0x7ff - 0x001) {
        /* x < 0x1p-1022 or inf or nan.  */
        if (ix * 2 == 0)
            return x;
        if (ix == 0x7ff0000000000000)
            return x;
        if (ix > 0x7ff0000000000000)
            return __math_invalid(x);
        /* x is subnormal, normalize it.  */
        ix = ASUINT64(x * 0x1p52);
        top = ix >> 52;
        top -= 52;
    }

    /* argument reduction:
       x = 4^e m; with integer e, and m in [1, 4)
       m: fixed point representation [2.62]
       2^e is the exponent part of the result.  */
    int even = top & 1;
    m = (ix << 11) | 0x8000000000000000;
    if (even) m >>= 1;
    top = (top + 0x3ff) >> 1;

    static const uint64_t three = 0xc0000000;
    uint64_t r, s, d, u, i;

    i = (ix >> 46) % 128;
    r = (uint32_t)__rsqrt_tab[i] << 16;
    /* |r sqrt(m) - 1| < 0x1.fdp-9 */
    s = mul32(m>>32, r);
    /* |s/sqrt(m) - 1| < 0x1.fdp-9 */
    d = mul32(s, r);
    u = three - d;
    r = mul32(r, u) << 1;
    /* |r sqrt(m) - 1| < 0x1.7bp-16 */
    s = mul32(s, u) << 1;
    /* |s/sqrt(m) - 1| < 0x1.7bp-16 */
    d = mul32(s, r);
    u = three - d;
    r = mul32(r, u) << 1;
    /* |r sqrt(m) - 1| < 0x1.3704p-29 (measured worst-case) */
    r = r << 32;
    s = mul64(m, r);
    d = mul64(s, r);
    u = (three<<32) - d;
    s = mul64(s, u);  /* repr: 3.61 */
    /* -0x1p-57 < s - sqrt(m) < 0x1.8001p-61 */
    s = (s - 2) >> 9; /* repr: 12.52 */
    /* -0x1.09p-52 < s - sqrt(m) < -0x1.fffcp-63 */

    /* s < sqrt(m) < s + 0x1.09p-52,
       compute nearest rounded result:
       the nearest result to 52 bits is either s or s+0x1p-52,
       we can decide by comparing (2^52 s + 0.5)^2 to 2^104 m.  */
    uint64_t d0, d1, d2;
    double y, t;
    d0 = (m << 42) - s*s;
    d1 = s - d0;
    d2 = d1 + s + 1;
    s += d1 >> 63;
    s &= 0x000fffffffffffff;
    s |= top << 52;
    y = asdouble(s);
    if (FENV_SUPPORT) {
        /* handle rounding modes and inexact exception:
           only (s+1)^2 == 2^42 m case is exact otherwise
           add a tiny value to cause the fenv effects.  */
        uint64_t tiny = d2==0 ? 0 : 0x0010000000000000;
        tiny |= (d1^d2) & 0x8000000000000000;
        t = asdouble(tiny);
        y = eval_as_double(y + t);
    }
    return y;
}

float sqrtf(float x) {
    uint32_t ix, m, m1, m0, even, ey;

    ix = asuint(x);
    if (ix - 0x00800000 >= 0x7f800000 - 0x00800000) {
        /* x < 0x1p-126 or inf or nan.  */
        if (ix * 2 == 0)
            return x;
        if (ix == 0x7f800000)
            return x;
        if (ix > 0x7f800000)
            return __math_invalidf(x);
        /* x is subnormal, normalize it.  */
        ix = asuint(x * 0x1p23f);
        ix -= 23 << 23;
    }

    /* x = 4^e m; with int e and m in [1, 4).  */
    even = ix & 0x00800000;
    m1 = (ix << 8) | 0x80000000;
    m0 = (ix << 7) & 0x7fffffff;
    m = even ? m0 : m1;

    /* 2^e is the exponent part of the return value.  */
    ey = ix >> 1;
    ey += 0x3f800000 >> 1;
    ey &= 0x7f800000;

    /* compute r ~ 1/sqrt(m), s ~ sqrt(m) with 2 goldschmidt iterations.  */
    static const uint32_t three = 0xc0000000;
    uint32_t r, s, d, u, i;
    i = (ix >> 17) % 128;
    r = (uint32_t)__rsqrt_tab[i] << 16;
    /* |r*sqrt(m) - 1| < 0x1p-8 */
    s = mul32(m, r);
    /* |s/sqrt(m) - 1| < 0x1p-8 */
    d = mul32(s, r);
    u = three - d;
    r = mul32(r, u) << 1;
    /* |r*sqrt(m) - 1| < 0x1.7bp-16 */
    s = mul32(s, u) << 1;
    /* |s/sqrt(m) - 1| < 0x1.7bp-16 */
    d = mul32(s, r);
    u = three - d;
    s = mul32(s, u);
    /* -0x1.03p-28 < s/sqrt(m) - 1 < 0x1.fp-31 */
    s = (s - 1)>>6;
    /* s < sqrt(m) < s + 0x1.08p-23 */

    /* compute nearest rounded result.  */
    uint32_t d0, d1, d2;
    float y, t;
    d0 = (m << 16) - s*s;
    d1 = s - d0;
    d2 = d1 + s + 1;
    s += d1 >> 31;
    s &= 0x007fffff;
    s |= ey;
    y = asfloat(s);
    if (FENV_SUPPORT) {
        /* handle rounding and inexact exception. */
        uint32_t tiny = d2==0 ? 0 : 0x01000000;
        tiny |= (d1^d2) & 0x80000000;
        t = asfloat(tiny);
        y = eval_as_float(y + t);
    }
    return y;
}

// origin: FreeBSD /usr/src/lib/msun/src/s_tanf.c

/*
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 * Optimized by Bruce D. Evans.
 */

float tanf(float x) {
    double y;
    uint32_t ix;
    unsigned n, sign;

    GET_FLOAT_WORD(ix, x);
    sign = ix >> 31;
    ix &= 0x7fffffff;

    if (ix <= 0x3f490fda) {  /* |x| ~<= pi/4 */
        if (ix < 0x39800000) {  /* |x| < 2**-12 */
            /* raise inexact if x!=0 and underflow if subnormal */
            FORCE_EVAL(ix < 0x00800000 ? x/0x1p120f : x+0x1p120f);
            return x;
        }
        return __tandf(x, 0);
    }
    if (ix <= 0x407b53d1) {  /* |x| ~<= 5*pi/4 */
        if (ix <= 0x4016cbe3)  /* |x| ~<= 3pi/4 */
            return __tandf((sign ? x+t1pio2 : x-t1pio2), 1);
        else
            return __tandf((sign ? x+t2pio2 : x-t2pio2), 0);
    }
    if (ix <= 0x40e231d5) {  /* |x| ~<= 9*pi/4 */
        if (ix <= 0x40afeddf)  /* |x| ~<= 7*pi/4 */
            return __tandf((sign ? x+t3pio2 : x-t3pio2), 1);
        else
            return __tandf((sign ? x+t4pio2 : x-t4pio2), 0);
    }

    /* tan(Inf or NaN) is NaN */
    if (ix >= 0x7f800000)
        return x - x;

    /* argument reduction */
    n = __rem_pio2f(x, &y);
    return __tandf(y, n&1);
}

double trunc(double x) {
    union {double f; uint64_t i;} u = {x};
    int e = (int)(u.i >> 52 & 0x7ff) - 0x3ff + 12;
    uint64_t m;

    if (e >= 52 + 12)
        return x;
    if (e < 12)
        e = 1;
    m = -1ULL >> e;
    if ((u.i & m) == 0)
        return x;
    FORCE_EVAL(x + 0x1p120f);
    u.i &= ~m;
    return u.f;
}

float truncf(float x) {
    union {float f; uint32_t i;} u = {x};
    int e = (int)(u.i >> 23 & 0xff) - 0x7f + 9;
    uint32_t m;

    if (e >= 23 + 9)
        return x;
    if (e < 9)
        e = 1;
    m = -1U >> e;
    if ((u.i & m) == 0)
        return x;
    FORCE_EVAL(x + 0x1p120f);
    u.i &= ~m;
    return u.f;
}
