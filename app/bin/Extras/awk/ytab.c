# define FIRSTTOKEN 257
# define FINAL 258
# define FATAL 259
# define LT 260
# define LE 261
# define GT 262
# define GE 263
# define EQ 264
# define NE 265
# define MATCH 266
# define NOTMATCH 267
# define APPEND 268
# define ADD 269
# define MINUS 270
# define MULT 271
# define DIVIDE 272
# define MOD 273
# define UMINUS 274
# define ASSIGN 275
# define ADDEQ 276
# define SUBEQ 277
# define MULTEQ 278
# define DIVEQ 279
# define MODEQ 280
# define JUMP 281
# define XBEGIN 282
# define XEND 283
# define NL 284
# define PRINT 285
# define PRINTF 286
# define SPRINTF 287
# define SPLIT 288
# define IF 289
# define ELSE 290
# define WHILE 291
# define FOR 292
# define IN 293
# define NEXT 294
# define EXIT 295
# define BREAK 296
# define CONTINUE 297
# define PROGRAM 298
# define PASTAT 299
# define PASTAT2 300
# define ASGNOP 301
# define BOR 302
# define AND 303
# define NOT 304
# define NUMBER 305
# define VAR 306
# define ARRAY 307
# define FNCN 308
# define SUBSTR 309
# define LSUBSTR 310
# define INDEX 311
# define RELOP 312
# define MATCHOP 313
# define OR 314
# define STRING 315
# define DOT 316
# define CCL 317
# define NCCL 318
# define CHAR 319
# define CAT 320
# define STAR 321
# define PLUS 322
# define QUEST 323
# define POSTINCR 324
# define PREINCR 325
# define POSTDECR 326
# define PREDECR 327
# define INCR 328
# define DECR 329
# define FIELD 330
# define INDIRECT 331
# define LASTTOKEN 332

# line 33 "awk.g.y"
#include "awk.def"
#ifndef DEBUG   
#       define  PUTS(x)
#endif
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern short yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
#ifndef YYSTYPE
#define YYSTYPE int
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256

# line 269 "awk.g.y"

short yyexca[] ={
-1, 1,
        0, -1,
        -2, 0,
        };
# define YYNPROD 120
# define YYLAST 1706
short yyact[]={

  54,  32,  34,  29,  68,  28,  66,  67, 157, 158,
 159,  33, 174, 222, 190, 225, 153, 197, 198,  44,
  45, 198, 206,  47,  45,  81,  36,  37, 170, 194,
  39,  66,  67,   6,   3,  42, 176, 156, 180,  54,
 172,  43,  29, 179,  28, 199,   7,  54,  38,  15,
  29, 113,  28,  81, 207, 114,  13,  40,  70,  72,
   4,  48,  65,  56,  15,  17,  65,  63,  61, 233,
  62,  63,  64, 231, 213, 164,  64, 212, 131, 131,
 178, 227, 121,  93, 119, 209,  14, 107,  15,  15,
  11,  15, 101, 218,  54, 104, 118,  29, 150,  28,
  58,  14,  46, 149, 148,  76,  75, 115,  74, 112,
  59,  60,  69,  88,  16,  87,  49,   9,  86,  35,
 169,   8,   5,   2,   1,  14,  14,   0,  14,   0,
  93,   0, 171, 133,  55, 102, 103,   0, 105,   0,
   0,  54, 145, 138,  29,   0,  28, 141, 142, 143,
 144,   0, 160, 146, 147,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,  93,   0, 152,
   0,   0,   0,   0,   0,   0, 183,   0, 175,  54,
   0,   0,  29, 187,  28,  44,  45,   0,   0,   0,
 173,  85, 186,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,  97,   0, 204, 205,
 200,   0,   0,   0,  93, 201, 151, 208,   0,   0,
   0,   0,   0,   0, 215, 216,  54,   0,   0,  29,
   0,  28,   0,   0, 220,  94,  95,  24,  26,  98,
   0,  99, 100,   0,  89,  90,  91,  92, 197, 198,
  41,   0, 203,  97,   0,  32,  34,   0,  23,  25,
   0,  27,  93,  19, 106,  33,  29,   0,  28,   0,
  17,  44,  45, 197, 198,   0,  24,  26,  30,  31,
  36,  37,  94,  95,  24,  26,  98,   0,  99, 100,
   0,  89,  90,  91,  92,   0, 197, 198,   0,   0,
  97,   0,  32,  34,  33,  23,  25,   0,  27,  93,
  54,  83,  33,  29,   0,  28,   0,  30,  31,  36,
  37, 109, 110, 111, 108,  30,  31,  36,  37,  94,
  95,  24,  26,  98,   0,  99, 100,   0,  89,  90,
  91,  92, 184,   0,   0,   0,  12,  97,   0,  32,
  34,   0,  23,  25,   0,  27,   0,  54,   0,  33,
  29,   0,  28,   0,   0,  54, 120,   0,  29,   0,
  28,   0,  30,  31,  36,  37,  94,  95,  24,  26,
  98,   0,  99, 100,   0,  89,  90,  91,  92,   0,
   0,   0,   0,  93,   0,  97,  32,  34,   0,  23,
  25,   0,  27,   0,  54,   0,  33,  29,   0,  28,
   0,   0,   0, 226,   0,   0,   0, 230,   0,  30,
  31,  36,  37,   0,  94,  95,  24,  26,  98,   0,
  99, 100,   0,  89,  90,  91,  92,   0, 181,   0,
   0,  29,  97,  28,  32,  34,   0,  23,  25,   0,
  27,   0,   0,   0,  33,   0,   0, 221,   0,   0,
   0, 113,   0,   0,   0, 114, 188,  30,  31,  36,
  37,  94,  95,  24,  26,  98,   0,  99, 100,   0,
  89,  90,  91,  92,  54, 211,   0,  29, 210,  28,
   0,  32,  34,   0,  23,  25,   0,  27,   0,   0,
   0,  33,   0,   0,   0,   0,  10,   0,   0,   0,
  24,  26,   0,   0,  30,  31,  36,  37,  54, 112,
   0,  29,   0,  28,   0,   0,  97,  20,  32,  34,
  19,  23,  25,  29,  27,  28,   0,  17,  33,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,  30,  31,  36,  37,  94,  95,  24,  26,  98,
   0,  99, 100,   0,  89,  90,  91,  92,   0,   0,
   0,   0,   0,  97,   0,  32,  34,   0,  23,  25,
   0,  27,   0,  54, 120,  33,  29, 130,  28,   0,
   0,  54, 229,   0,  29,   0,  28,   0,  30,  31,
  36,  37,  94,  95,  24,  26,   0,   0,   0,   0,
  84,   0,  24,  26,   0,   0,   0,   0,   0,   0,
  97,   0,  32,  34,   0,  23,  25,   0,  27,   0,
  32,  34,  33,  23,  25, 193,  27,  52,  50,   0,
  33,   0,   0,   0,   0,  30,  31,  36,  37,  94,
  95,  24,  26,  30,  31,  36,  37,  54, 228,   0,
  29,   0,  28,   0,   0,   0,   0,   0,   0,  32,
 185,   0,  23,  25,   0,  27,   0,   0,   0,  33,
   0,   0,   0,   0,   0,  24,  26,   0,   0,   0,
   0,   0,  30,  31,  36,  37, 181, 139, 140,  29,
   0,  28, 182,  32,  34,   0,  23,  25,   0,  27,
 113,   0,   0,  33, 114,   0,   0,   0,   0,   0,
   0, 154,   0,   0,   0,   0,  30,  31,  36,  37,
   0,  24,  26,  54, 214,   0,  29,   0,  28, 155,
   0, 109, 110, 111, 108,   0, 157, 158, 159,  32,
  34,   0,  23,  25,   0,  27,   0,   0,   0,  33,
   0,   0,   0,   0,  54,  24,  26,  29, 112,  28,
   0,   0,  30,  31,  36,  37,   0,  24,  26,   0,
 192,   0,   0,  32,  34,   0,  23,  25,   0,  27,
  52,  50,   0,  33,  20,  32,  34,   0,  23,  25,
   0,  27,   0,   0,   0,  33,  30,  31,  36,  37,
  54,   0,   0,  29, 167,  28,   0, 168,  30,  31,
  36,  37,  54,   0,   0,  29, 166,  28,   0,   0,
  24,  26,   0,   0,   0, 217,   0, 113,  24,  26,
   0, 114, 219, 235,   0, 237,   0, 238,  32,  34,
   0,  23,  25,   0,  27,   0,  32,  34,  33,  23,
  25,   0,  27, 232,   0,   0,  33, 234,   0, 236,
   0,  30,  31,  36,  37,   0,   0,   0,   0,  30,
  31,  36,  37,  54,   0,   0,  29, 165,  28,   0,
   0,  54, 161,   0,  29, 112,  28,   0,   0,   0,
   0,   0,   0,   0,  24,  26,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,  32,  34,   0,  23,  25,   0,  27,   0,
  54, 120,  33,  29,   0,  28,   0,   0,   0,   0,
   0,   0,   0,  24,  26,  30,  31,  36,  37,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
 182,  32,  34,   0,  23,  25,   0,  27,   0,  54,
   0,  33,  29, 130,  28,   0,   0,   0,   0,   0,
  24,  26,   0,   0,  30,  31,  36,  37, 155,   0,
 109, 110, 111, 108,   0, 157, 158, 159,  32,  34,
   0,  23,  25,   0,  27,   0,   0,   0,  33,   0,
   0,  24,  26,   0,  54, 128,   0,  29,   0,  28,
   0,  30,  31,  36,  37,   0,   0,   0,   0,  32,
  34,   0,  23,  25,   0,  27,   0,   0,   0,  33,
   0,   0,   0,  54,   0,   0,  29,   0,  28,   0,
   0,   0,  30,  31,  36,  37,   0,  24,  26,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,  24,
  26,   0,   0,   0,   0,  32,  34,  73,  23,  25,
  29,  27,  28,   0,   0,  33,   0,  32,  34,   0,
  23,  25,   0,  27,   0,   0,   0,  33,  30,  31,
  36,  37,   0,   0,   0,   0,   0,   0,   0,   0,
  30,  31,  36,  37,   0,   0,   0, 109, 110, 111,
 108,   0, 157, 158, 159,   0,   0,   0,   0,   0,
  24,  26,   0,   0,   0,   0,   0,   0,  24,  26,
   0,   0,   0,   0,   0,   0,   0,   0,  32,  34,
   0,  23,  25,   0,  27,   0,  32,  34,  33,  23,
  25,   0,  27,   0,   0,   0,  33,   0,   0,   0,
   0,  30,  31,  36,  37,   0,   0,  24,  26,  30,
  31,  36,  37,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,  32,  34,   0,  23,  25,
   0,  27,   0,   0,   0,  33,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,  24,  26,  30,  31,
  36,  37,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,  32,  34,   0,  23,  25,   0,
  27,   0,   0,   0,  33,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,  30,  31,  36,
  37,  24,  26,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,  32,
  34,   0,  23,  25,   0,  27,   0,   0,   0,  33,
  24,  26,   0,   0,   0,   0,   0,  22,   0,   0,
   0,   0,  30,  31,  36,  37,   0,   0,  32,  34,
   0,  23,  25,   0,  27,   0,  53,   0,  33,   0,
  21,   0,   0,   0,  24,  26,  53,  53,  79,  80,
   0,  30,  31,  36,  37,  53,   0,   0,   0,  51,
   0,   0,  32,  34,   0,  23,  25,   0,  27,  77,
  78,   0,  33,   0,   0,  53,   0,   0,  82,  53,
  53,  53,  53,  53,   0,  30,  31,  36,  37,  53,
   0,   0,   0,   0,   0,   0,   0,   0,  51,   0,
   0,   0, 122, 123, 124, 125, 126,   0,   0,   0,
   0,   0,  51,   0,  53,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,  53,  53,   0,  51,   0,   0,
   0,   0,   0,   0,   0,  53,   0,  53,   0,   0,
  53,   0,  53,  53,  53,  53,   0,  51,  51,   0,
   0,   0,   0,   0,   0,   0,   0,   0,  51,   0,
  51,   0,   0,  51,   0,  51,  51,  51,  51,   0,
  53,  53,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,  53,   0,   0,   0,   0,
   0,   0,  96,  51,  51,   0,   0,  53,  18,  53,
   0,   0,   0,  53,  53,   0,   0,   0,  51,   0,
  53,   0,  57,  18,   0,   0,   0,  71,   0,   0,
  51,   0,  51,   0,   0,   0,  51,  51,   0,   0,
   0,  53,  53,  51,   0,   0,   0,  18,  18,   0,
  18,   0,   0,   0,   0, 116,   0, 117,   0,   0,
   0,   0,   0,   0,  51,  51,   0,   0,   0,   0,
   0, 127, 129,   0,   0,   0, 132, 134, 135, 136,
   0,   0,   0,   0, 137,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,  71,  71,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0, 162, 163,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0, 177, 177,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0, 189,   0,
 191,   0,   0,   0,   0, 195,   0,   0, 196,   0,
   0,   0,   0,   0, 202, 177,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
 177, 177,   0,   0,   0,   0,   0,   0,   0,   0,
 177,   0,   0, 223,   0, 224 };
short yypact[]={

-222,-1000,-251,-1000, -77, 233,-1000,-1000,-254, -24,
 -82, -21,-1000,-1000,-1000,-1000,-1000,-1000, 488, 500,
 500,  25,-297,  72,1047,  68,  66,  65,1013,1013,
-304,-304,-1000,-1000, -38,-1000,-1000,1013, 196,-1000,
-1000,-1000,-1000,-1000, 500, 500,-1000, 500, 149,  15,
  18,  25,1013,-322,1013,  55,  43, 335,  41,-283,
-1000,1013,1013,1013,1013,1013,-1000,-1000,1013, 984,
-1000, 939,  35,1047,1013,1013,1013,-1000,-1000,-1000,
-1000,1013,-1000,-1000,-1000, -24, 280, 280,-1000, -24,
 -24, -24, -24,-1000,1047,1047,1013,-1000,  64,  63,
  58, 101,-279,-1000,  54,-107,-1000, 684,-1000,-1000,
-1000,-1000,-1000,-1000,  15,-1000,  -1, 900,-1000,-1000,
-1000,-1000,  29,  29,-1000,-1000,-1000,1013,-1000, 861,
1013,1013, 553,  34, 853, 792, 780, 734,-1000,-262,
-1000,-1000,-1000,-1000,-1000,   7,-112,-112, 666, 666,
 374,-1000,-1000,-1000,-1000,  15,-313,-1000,-1000,-1000,
 435,-1000,1013,1013,-1000,1013,-292,1013,-1000, 280,
-255,-1000,1013,-1000,-1000,1013,   4, 488,-1000,-1000,
-1000, 666, 666, -19,  -5, -66, -40, 811,-1000, 454,
  33, 703,-1000,-1000,-1000,1013,1013, 666, 666,-255,
  52,  43, 335,  41,-285,-1000,-255, 408,-293,-1000,
1013,-1000,1013,-1000,-1000,-282,-1000,-1000,-1000,-1000,
 -44, 327,  40, 627, 561, 327,  32,-255,-1000,-1000,
  28,-255, 280,-255, 280,-1000, 280,-1000,-1000 };
short yypgo[]={

   0, 124, 123, 122, 121,  48,  38,  36, 114,  90,
1492,  80,  43, 120, 645, 119,1330, 118,  56,1307,
  58, 117,  57,  59,  40, 116,  37, 201, 620, 115,
 113 };
short yyr1[]={

   0,   1,   1,   2,   2,   2,   4,   4,   4,   6,
   6,   6,   6,   8,   8,   8,   8,   7,   7,   7,
   7,  13,  15,  15,  17,  12,  12,  19,  19,  19,
  19,  19,  16,  16,  16,  16,  16,  16,  16,  16,
  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,
  16,  16,  16,  16,  10,  10,  10,  14,  14,  21,
  21,  21,  21,  21,   3,   3,   3,   9,   9,   9,
   9,  20,  20,  20,  23,  23,  23,  24,  24,  25,
  18,  26,  26,  26,  26,  26,  26,  26,  26,  26,
  26,  26,  26,  11,  11,  22,  22,  27,  27,  27,
  27,  27,  27,  27,  28,  28,  28,  28,  28,  28,
  28,  28,  28,  28,   5,   5,  29,  30,  30,  30 };
short yyr2[]={

   0,   3,   1,   4,   2,   0,   4,   2,   0,   3,
   3,   2,   3,   3,   3,   2,   3,   1,   1,   1,
   1,   2,   1,   2,   5,   3,   3,   1,   1,   1,
   4,   1,   1,   1,   3,   4,   2,   8,   6,   8,
   6,   6,   3,   3,   3,   3,   3,   3,   2,   2,
   2,   2,   2,   2,   1,   2,   3,   1,   0,   1,
   4,   3,   6,   3,   3,   0,   2,   1,   1,   1,
   1,   1,   1,   0,   3,   3,   3,   1,   1,   0,
   4,   1,   1,   1,   1,   1,   1,   3,   2,   2,
   2,   2,   3,   3,   3,   1,   1,   4,   2,   4,
   2,   1,   0,   1,   2,   2,   4,   2,   1,   2,
   2,   2,   2,   3,   2,   0,   5,  10,   9,   8 };
short yychk[]={

-1000,  -1,  -2, 256, 282,  -3, 284, 123,  -4, -21,
 283,  -9, 123, -18, -11, -12,  -8,  47, -10,  40,
 304, -16, -19, 308, 287, 309, 288, 311,  45,  43,
 328, 329, 305, 315, 306, -15, 330, 331,  -5, 284,
 -22, 284,  59, 123, 302, 303, 123,  44,  -5, -25,
 313, -16, 312, -19,  40,  -8, -12, -10, -11,  -9,
  -9,  43,  45,  42,  47,  37, 328, 329, 301,  40,
 -20, -10, -23,  40,  40,  40,  40, -16, -16, -19,
 -19,  91, -16, 125, -28, -27, -17, -29, -30, 294,
 295, 296, 297, 123, 285, 286, -10, 256, 289, 291,
 292,  -5,  -9,  -9,  -5,  -9, 125, -26, 319, 316,
 317, 318,  94,  36,  40, -18, -10, -10,  41,  41,
  41,  41, -16, -16, -16, -16, -16, -10,  41, -10,
  44,  44, -10, -23, -10, -10, -10, -10, -22, -28,
 -28, -22, -22, -22, -22,  -5, -20, -20,  40,  40,
  40, 125, 125, 123,  47, 314, -26, 321, 322, 323,
 -26,  41, -10, -10,  41,  44,  44,  44,  93, -13,
 290, 125, -24, 312, 124, -24,  -7, -10, -11, -12,
  -6,  40, 304,  -7, -27, 306,  -5, -26,  41, -10,
 306, -10, -28, -14, 284, -10, -10, 302, 303,  41,
  -6, -12, -10, -11,  -7,  -7,  41,  59, 293, 125,
  44,  41,  44,  41,  41,  -7,  -7, -14,  41, -14,
  -7,  59, 306, -10, -10,  59, -27,  41,  41,  41,
 -27,  41, -14,  41, -14, -28, -14, -28, -28 };
short yydef[]={

   5,  -2,  65,   2,   0,   8,   4, 115,   1,  66,
   0,  59, 115,  67,  68,  69,  70,  79,   0,   0,
   0,  54,  32,  33,  73,   0,   0,   0,   0,   0,
   0,   0,  27,  28,  29,  31,  22,   0, 102,   7,
  64,  95,  96, 115,   0,   0, 115,   0, 102,   0,
   0,  55,   0,  32,   0,  70,  69,   0,  68,   0,
  15,   0,   0,   0,   0,   0,  52,  53,   0,   0,
  36,  71,  72,   0,   0,   0,   0,  48,  49,  50,
  51,   0,  23,   3, 114,   0, 102, 102, 108,   0,
   0,   0,   0, 115,  73,  73, 101, 103,   0,   0,
   0, 102,  13,  14, 102,  61,  63,   0,  81,  82,
  83,  84,  85,  86,   0,  25,  93,   0,  16,  26,
  42,  94,  43,  44,  45,  46,  47,  56,  34,   0,
   0,   0,   0,   0,   0,   0,   0,   0, 104, 105,
 107, 109, 110, 111, 112, 102,  98, 100,   0,   0,
 102,   6,  60, 115,  80,   0,  88,  89,  90,  91,
   0,  35,  74,  75,  76,   0,   0,   0,  30, 102,
  58, 113,   0,  77,  78,   0,   0,  17,  18,  19,
  20,   0,   0,   0,   0,  29, 102,  87,  92,   0,
   0,   0, 106,  21,  57,  97,  99,   0,   0,  58,
  20,  19,  17,  18,   0,  11,  58,   0,   0,  62,
   0,  38,   0,  40,  41,   9,  10,  24,  12, 116,
   0, 102,   0,   0,   0, 102,   0,  58,  37,  39,
   0,  58, 102,  58, 102, 119, 102, 118, 117 };
#
# define YYFLAG -1000
# define YYERROR goto yyerrlab
# define YYACCEPT return(0)
# define YYABORT return(1)

/*      parser for yacc output  */

int yydebug = 0; /* 1 for debugging */
YYSTYPE yyv[YYMAXDEPTH]; /* where the values are stored */
int yychar = -1; /* current input token number */
int yynerrs = 0;  /* number of errors */
short yyerrflag = 0;  /* error recovery flag */

yyparse() {

        short yys[YYMAXDEPTH];
        short yyj, yym;
        register YYSTYPE *yypvt;
        register short yystate, *yyps, yyn;
        register YYSTYPE *yypv;
        register short *yyxi;

        yystate = 0;
        yychar = -1;
        yynerrs = 0;
        yyerrflag = 0;
        yyps= &yys[-1];
        yypv= &yyv[-1];

 yystack:    /* put a state and value onto the stack */

        if( yydebug  ) printf( "state %d, char 0%o\n", yystate, yychar );
                if( ++yyps> &yys[YYMAXDEPTH] ) { yyerror( "yacc stack overflow" ); return(1); }
                *yyps = yystate;
                ++yypv;
                *yypv = yyval;

 yynewstate:

        yyn = yypact[yystate];

        if( yyn<= YYFLAG ) goto yydefault; /* simple state */

        if( yychar<0 ) if( (yychar=yylex())<0 ) yychar=0;
        if( (yyn += yychar)<0 || yyn >= YYLAST ) goto yydefault;

        if( yychk[ yyn=yyact[ yyn ] ] == yychar ){ /* valid shift */
                yychar = -1;
                yyval = yylval;
                yystate = yyn;
                if( yyerrflag > 0 ) --yyerrflag;
                goto yystack;
                }

 yydefault:
        /* default state action */

        if( (yyn=yydef[yystate]) == -2 ) {
                if( yychar<0 ) if( (yychar=yylex())<0 ) yychar = 0;
                /* look through exception table */

                for( yyxi=yyexca; (*yyxi!= (-1)) || (yyxi[1]!=yystate) ; yyxi += 2 ) ; /* VOID */

                while( *(yyxi+=2) >= 0 ){
                        if( *yyxi == yychar ) break;
                        }
                if( (yyn = yyxi[1]) < 0 ) return(0);   /* accept */
                }

        if( yyn == 0 ){ /* error */
                /* error ... attempt to resume parsing */

                switch( yyerrflag ){

                case 0:   /* brand new error */

                        yyerror( "syntax error" );
                yyerrlab:
                        ++yynerrs;

                case 1:
                case 2: /* incompletely recovered error ... try again */

                        yyerrflag = 3;

                        /* find a state where "error" is a legal shift action */

                        while ( yyps >= yys ) {
                           yyn = yypact[*yyps] + YYERRCODE;
                           if( yyn>= 0 && yyn < YYLAST && yychk[yyact[yyn]] == YYERRCODE ){
                              yystate = yyact[yyn];  /* simulate a shift of "error" */
                              goto yystack;
                              }
                           yyn = yypact[*yyps];

                           /* the current yyps has no shift onn "error", pop stack */

                           if( yydebug ) printf( "error recovery pops state %d, uncovers %d\n", *yyps, yyps[-1] );
                           --yyps;
                           --yypv;
                           }

                        /* there is no state on the stack with an error shift ... abort */

        yyabort:
                        return(1);


                case 3:  /* no shift yet; clobber input char */

                        if( yydebug ) printf( "error recovery discards char %d\n", yychar );

                        if( yychar == 0 ) goto yyabort; /* don't discard EOF, quit */
                        yychar = -1;
                        goto yynewstate;   /* try again in the same state */

                        }

                }

        /* reduction by production yyn */

                if( yydebug ) printf("reduce %d\n",yyn);
                yyps -= yyr2[yyn];
                yypvt = yypv;
                yypv -= yyr2[yyn];
                yyval = yypv[1];
                yym=yyn;
                        /* consult goto table to find next state */
                yyn = yyr1[yyn];
                yyj = yypgo[yyn] + *yyps + 1;
                if( yyj>=YYLAST || yychk[ yystate = yyact[yyj] ] != -yyn ) yystate = yyact[yypgo[yyn]];
                switch(yym){
                        
case 1:
# line 41 "awk.g.y"
{ if (errorflag==0) winner = stat3(PROGRAM, yypvt[-2], yypvt[-1], yypvt[-0]); } break;
case 2:
# line 42 "awk.g.y"
{ yyclearin; yyerror("bailing out"); } break;
case 3:
# line 46 "awk.g.y"
{ PUTS("XBEGIN list"); yyval = yypvt[-1]; } break;
case 5:
# line 48 "awk.g.y"
{ PUTS("empty XBEGIN"); yyval = nullstat; } break;
case 6:
# line 52 "awk.g.y"
{ PUTS("XEND list"); yyval = yypvt[-1]; } break;
case 8:
# line 54 "awk.g.y"
{ PUTS("empty END"); yyval = nullstat; } break;
case 9:
# line 58 "awk.g.y"
{ PUTS("cond||cond"); yyval = op2(BOR, yypvt[-2], yypvt[-0]); } break;
case 10:
# line 59 "awk.g.y"
{ PUTS("cond&&cond"); yyval = op2(AND, yypvt[-2], yypvt[-0]); } break;
case 11:
# line 60 "awk.g.y"
{ PUTS("!cond"); yyval = op1(NOT, yypvt[-0]); } break;
case 12:
# line 61 "awk.g.y"
{ yyval = yypvt[-1]; } break;
case 13:
# line 65 "awk.g.y"
{ PUTS("pat||pat"); yyval = op2(BOR, yypvt[-2], yypvt[-0]); } break;
case 14:
# line 66 "awk.g.y"
{ PUTS("pat&&pat"); yyval = op2(AND, yypvt[-2], yypvt[-0]); } break;
case 15:
# line 67 "awk.g.y"
{ PUTS("!pat"); yyval = op1(NOT, yypvt[-0]); } break;
case 16:
# line 68 "awk.g.y"
{ yyval = yypvt[-1]; } break;
case 17:
# line 72 "awk.g.y"
{ PUTS("expr"); yyval = op2(NE, yypvt[-0], valtonode(lookup("0", symtab), CCON)); } break;
case 18:
# line 73 "awk.g.y"
{ PUTS("relexpr"); } break;
case 19:
# line 74 "awk.g.y"
{ PUTS("lexexpr"); } break;
case 20:
# line 75 "awk.g.y"
{ PUTS("compcond"); } break;
case 21:
# line 79 "awk.g.y"
{ PUTS("else"); } break;
case 22:
# line 83 "awk.g.y"
{ PUTS("field"); yyval = valtonode(yypvt[-0], CFLD); } break;
case 23:
# line 84 "awk.g.y"
{ PUTS("ind field"); yyval = op1(INDIRECT, yypvt[-0]); } break;
case 24:
# line 88 "awk.g.y"
{ PUTS("if(cond)"); yyval = yypvt[-2]; } break;
case 25:
# line 92 "awk.g.y"
{ PUTS("expr~re"); yyval = op2(yypvt[-1], yypvt[-2], makedfa(yypvt[-0])); } break;
case 26:
# line 93 "awk.g.y"
{ PUTS("(lex_expr)"); yyval = yypvt[-1]; } break;
case 27:
# line 97 "awk.g.y"
{PUTS("number"); yyval = valtonode(yypvt[-0], CCON); } break;
case 28:
# line 98 "awk.g.y"
{ PUTS("string"); yyval = valtonode(yypvt[-0], CCON); } break;
case 29:
# line 99 "awk.g.y"
{ PUTS("var"); yyval = valtonode(yypvt[-0], CVAR); } break;
case 30:
# line 100 "awk.g.y"
{ PUTS("array[]"); yyval = op2(ARRAY, yypvt[-3], yypvt[-1]); } break;
case 33:
# line 105 "awk.g.y"
{ PUTS("func");
                        yyval = op2(FNCN, yypvt[-0], valtonode(lookup("$record", symtab), CFLD));
                        } break;
case 34:
# line 108 "awk.g.y"
{ PUTS("func()"); 
                        yyval = op2(FNCN, yypvt[-2], valtonode(lookup("$record", symtab), CFLD));
                        } break;
case 35:
# line 111 "awk.g.y"
{ PUTS("func(expr)"); yyval = op2(FNCN, yypvt[-3], yypvt[-1]); } break;
case 36:
# line 112 "awk.g.y"
{ PUTS("sprintf"); yyval = op1(yypvt[-1], yypvt[-0]); } break;
case 37:
# line 114 "awk.g.y"
{ PUTS("substr(e,e,e)"); yyval = op3(SUBSTR, yypvt[-5], yypvt[-3], yypvt[-1]); } break;
case 38:
# line 116 "awk.g.y"
{ PUTS("substr(e,e,e)"); yyval = op3(SUBSTR, yypvt[-3], yypvt[-1], nullstat); } break;
case 39:
# line 118 "awk.g.y"
{ PUTS("split(e,e,e)"); yyval = op3(SPLIT, yypvt[-5], yypvt[-3], yypvt[-1]); } break;
case 40:
# line 120 "awk.g.y"
{ PUTS("split(e,e,e)"); yyval = op3(SPLIT, yypvt[-3], yypvt[-1], nullstat); } break;
case 41:
# line 122 "awk.g.y"
{ PUTS("index(e,e)"); yyval = op2(INDEX, yypvt[-3], yypvt[-1]); } break;
case 42:
# line 123 "awk.g.y"
{PUTS("(expr)");  yyval = yypvt[-1]; } break;
case 43:
# line 124 "awk.g.y"
{ PUTS("t+t"); yyval = op2(ADD, yypvt[-2], yypvt[-0]); } break;
case 44:
# line 125 "awk.g.y"
{ PUTS("t-t"); yyval = op2(MINUS, yypvt[-2], yypvt[-0]); } break;
case 45:
# line 126 "awk.g.y"
{ PUTS("t*t"); yyval = op2(MULT, yypvt[-2], yypvt[-0]); } break;
case 46:
# line 127 "awk.g.y"
{ PUTS("t/t"); yyval = op2(DIVIDE, yypvt[-2], yypvt[-0]); } break;
case 47:
# line 128 "awk.g.y"
{ PUTS("t%t"); yyval = op2(MOD, yypvt[-2], yypvt[-0]); } break;
case 48:
# line 129 "awk.g.y"
{ PUTS("-term"); yyval = op1(UMINUS, yypvt[-0]); } break;
case 49:
# line 130 "awk.g.y"
{ PUTS("+term"); yyval = yypvt[-0]; } break;
case 50:
# line 131 "awk.g.y"
{ PUTS("++var"); yyval = op1(PREINCR, yypvt[-0]); } break;
case 51:
# line 132 "awk.g.y"
{ PUTS("--var"); yyval = op1(PREDECR, yypvt[-0]); } break;
case 52:
# line 133 "awk.g.y"
{ PUTS("var++"); yyval= op1(POSTINCR, yypvt[-1]); } break;
case 53:
# line 134 "awk.g.y"
{ PUTS("var--"); yyval= op1(POSTDECR, yypvt[-1]); } break;
case 54:
# line 138 "awk.g.y"
{ PUTS("term"); } break;
case 55:
# line 139 "awk.g.y"
{ PUTS("expr term"); yyval = op2(CAT, yypvt[-1], yypvt[-0]); } break;
case 56:
# line 140 "awk.g.y"
{ PUTS("var=expr"); yyval = stat2(yypvt[-1], yypvt[-2], yypvt[-0]); } break;
case 59:
# line 149 "awk.g.y"
{ PUTS("pattern"); yyval = stat2(PASTAT, yypvt[-0], genprint()); } break;
case 60:
# line 150 "awk.g.y"
{ PUTS("pattern {...}"); yyval = stat2(PASTAT, yypvt[-3], yypvt[-1]); } break;
case 61:
# line 151 "awk.g.y"
{ PUTS("srch,srch"); yyval = pa2stat(yypvt[-2], yypvt[-0], genprint()); } break;
case 62:
# line 153 "awk.g.y"
{ PUTS("srch, srch {...}"); yyval = pa2stat(yypvt[-5], yypvt[-3], yypvt[-1]); } break;
case 63:
# line 154 "awk.g.y"
{ PUTS("null pattern {...}"); yyval = stat2(PASTAT, nullstat, yypvt[-1]); } break;
case 64:
# line 158 "awk.g.y"
{ PUTS("pa_stats pa_stat"); yyval = linkum(yypvt[-2], yypvt[-1]); } break;
case 65:
# line 159 "awk.g.y"
{ PUTS("null pa_stat"); yyval = nullstat; } break;
case 66:
# line 160 "awk.g.y"
{PUTS("pa_stats pa_stat"); yyval = linkum(yypvt[-1], yypvt[-0]); } break;
case 67:
# line 164 "awk.g.y"
{ PUTS("regex");
                yyval = op2(MATCH, valtonode(lookup("$record", symtab), CFLD), makedfa(yypvt[-0]));
                } break;
case 68:
# line 167 "awk.g.y"
{ PUTS("relexpr"); } break;
case 69:
# line 168 "awk.g.y"
{ PUTS("lexexpr"); } break;
case 70:
# line 169 "awk.g.y"
{ PUTS("comp pat"); } break;
case 71:
# line 173 "awk.g.y"
{ PUTS("expr"); } break;
case 72:
# line 174 "awk.g.y"
{ PUTS("pe_list"); } break;
case 73:
# line 175 "awk.g.y"
{ PUTS("null print_list"); yyval = valtonode(lookup("$record", symtab), CFLD); } break;
case 74:
# line 179 "awk.g.y"
{yyval = linkum(yypvt[-2], yypvt[-0]); } break;
case 75:
# line 180 "awk.g.y"
{yyval = linkum(yypvt[-2], yypvt[-0]); } break;
case 76:
# line 181 "awk.g.y"
{yyval = yypvt[-1]; } break;
case 79:
# line 190 "awk.g.y"
{ startreg(); } break;
case 80:
# line 192 "awk.g.y"
{ PUTS("/r/"); yyval = yypvt[-1]; } break;
case 81:
# line 196 "awk.g.y"
{ PUTS("regex CHAR"); yyval = op2(CHAR, (node *) 0, yypvt[-0]); } break;
case 82:
# line 197 "awk.g.y"
{ PUTS("regex DOT"); yyval = op2(DOT, (node *) 0, (node *) 0); } break;
case 83:
# line 198 "awk.g.y"
{ PUTS("regex CCL"); yyval = op2(CCL, (node *) 0, cclenter(yypvt[-0])); } break;
case 84:
# line 199 "awk.g.y"
{ PUTS("regex NCCL"); yyval = op2(NCCL, (node *) 0, cclenter(yypvt[-0])); } break;
case 85:
# line 200 "awk.g.y"
{ PUTS("regex ^"); yyval = op2(CHAR, (node *) 0, HAT); } break;
case 86:
# line 201 "awk.g.y"
{ PUTS("regex $"); yyval = op2(CHAR, (node *) 0 ,(node *) 0); } break;
case 87:
# line 202 "awk.g.y"
{ PUTS("regex OR"); yyval = op2(OR, yypvt[-2], yypvt[-0]); } break;
case 88:
# line 204 "awk.g.y"
{ PUTS("regex CAT"); yyval = op2(CAT, yypvt[-1], yypvt[-0]); } break;
case 89:
# line 205 "awk.g.y"
{ PUTS("regex STAR"); yyval = op2(STAR, yypvt[-1], (node *) 0); } break;
case 90:
# line 206 "awk.g.y"
{ PUTS("regex PLUS"); yyval = op2(PLUS, yypvt[-1], (node *) 0); } break;
case 91:
# line 207 "awk.g.y"
{ PUTS("regex QUEST"); yyval = op2(QUEST, yypvt[-1], (node *) 0); } break;
case 92:
# line 208 "awk.g.y"
{ PUTS("(regex)"); yyval = yypvt[-1]; } break;
case 93:
# line 213 "awk.g.y"
{ PUTS("expr relop expr"); yyval = op2(yypvt[-1], yypvt[-2], yypvt[-0]); } break;
case 94:
# line 215 "awk.g.y"
{ PUTS("(relexpr)"); yyval = yypvt[-1]; } break;
case 97:
# line 225 "awk.g.y"
{ PUTS("print>stat"); yyval = stat3(yypvt[-3], yypvt[-2], yypvt[-1], yypvt[-0]); } break;
case 98:
# line 227 "awk.g.y"
{ PUTS("print list"); yyval = stat3(yypvt[-1], yypvt[-0], nullstat, nullstat); } break;
case 99:
# line 229 "awk.g.y"
{ PUTS("printf>stat"); yyval = stat3(yypvt[-3], yypvt[-2], yypvt[-1], yypvt[-0]); } break;
case 100:
# line 231 "awk.g.y"
{ PUTS("printf list"); yyval = stat3(yypvt[-1], yypvt[-0], nullstat, nullstat); } break;
case 101:
# line 232 "awk.g.y"
{ PUTS("expr"); yyval = exptostat(yypvt[-0]); } break;
case 102:
# line 233 "awk.g.y"
{ PUTS("null simple statement"); yyval = nullstat; } break;
case 103:
# line 234 "awk.g.y"
{ yyclearin; yyerror("illegal statement"); } break;
case 104:
# line 238 "awk.g.y"
{ PUTS("simple stat"); } break;
case 105:
# line 239 "awk.g.y"
{ PUTS("if stat"); yyval = stat3(IF, yypvt[-1], yypvt[-0], nullstat); } break;
case 106:
# line 241 "awk.g.y"
{ PUTS("if-else stat"); yyval = stat3(IF, yypvt[-3], yypvt[-2], yypvt[-0]); } break;
case 107:
# line 242 "awk.g.y"
{ PUTS("while stat"); yyval = stat2(WHILE, yypvt[-1], yypvt[-0]); } break;
case 108:
# line 243 "awk.g.y"
{ PUTS("for stat"); } break;
case 109:
# line 244 "awk.g.y"
{ PUTS("next"); yyval = genjump(NEXT); } break;
case 110:
# line 245 "awk.g.y"
{ PUTS("exit"); yyval = genjump(EXIT); } break;
case 111:
# line 246 "awk.g.y"
{ PUTS("break"); yyval = genjump(BREAK); } break;
case 112:
# line 247 "awk.g.y"
{ PUTS("continue"); yyval = genjump(CONTINUE); } break;
case 113:
# line 248 "awk.g.y"
{ PUTS("{statlist}"); yyval = yypvt[-1]; } break;
case 114:
# line 252 "awk.g.y"
{ PUTS("stat_list stat"); yyval = linkum(yypvt[-1], yypvt[-0]); } break;
case 115:
# line 253 "awk.g.y"
{ PUTS("null stat list"); yyval = nullstat; } break;
case 116:
# line 257 "awk.g.y"
{ PUTS("while(cond)"); yyval = yypvt[-2]; } break;
case 117:
# line 262 "awk.g.y"
{ PUTS("for(e;e;e)"); yyval = stat4(FOR, yypvt[-7], yypvt[-5], yypvt[-3], yypvt[-0]); } break;
case 118:
# line 264 "awk.g.y"
{ PUTS("for(e;e;e)"); yyval = stat4(FOR, yypvt[-6], nullstat, yypvt[-3], yypvt[-0]); } break;
case 119:
# line 266 "awk.g.y"
{ PUTS("for(v in v)"); yyval = stat3(IN, yypvt[-5], yypvt[-3], yypvt[-0]); } break;
                }
                goto yystack;  /* stack new state and value */

        }

