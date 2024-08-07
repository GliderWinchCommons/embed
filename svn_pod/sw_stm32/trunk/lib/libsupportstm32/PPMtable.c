/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : PPMtable.c
* Hackerees          : deh
* Date First Issued  : 07/21/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : calibration table for adjusting 32 KHz osc
*******************************************************************************/
/* 
The index into this table is the thermistor adc reading minus the table
offset.  The table only covers a range of adc readings that correspond to the
max/min temperatures expected.

The table is in 1/100th of a ppm.  Each entry is the ppm*100 that the xtal osc
frequency will be lower than nominal.  

The turnpoint is 25 degree C and at this temp the table assumes the osc frequency is 
correct.  There may an offset for the particular xtal/board that is in addition to
the frequency deviation versus temp from this table.

The table was made with these definitions--
#define B	3380	// Thermistor constant
#define P	0.034	// Xtal freq v temp^2 constant
#define RT	10	// Ro thermistor resistance (K @ 25 deg C)
#define RF	7.15	// Fixed resistor resistance
#define ADCCOUNT	4095	// Max ADC reading
#define ADCHILIMIT	3025	// Highest ADC reading we deal with
#define ADCLOLIMIT	924	// Lowest ADC reading we deal with
#define SIZEOFTABLE	(ADCHILIMIT-ADCLOLIMIT)	// Number of ADC steps
#define LOTEMP		0.0	// Temp (deg C) for highest R
#define HITEMP		60.0	// Temp (deg C) for lowest R
#define TURNPOINTTEMP	25	// Temp (deg C) for xtal turnpoint
#define KTURNPOINT	273+TURNPOINTTEMP	// Kelvin temp of turnpoint
#define FREQVTEMP	-.034	// Crystal freq deviation v temp^2
#define SCALE		100	// Scale ppm to integer

==> See table generator 'svn_pod/hw/trunk/eagle/ADC-ppm.c' <==
Thermistor equation:
R = Ro * exp(B*(1/T - 1/To));

ADC equations:
ADC count = ADCCOUNT * (R/(R+Rf));

R = (ADCreading * Rf)/(ADCCOUNT - ADCreading);

ADCreading = R * ADCCOUNT / (R + Rf);

Crystal temp compensation--
ppm = -0.034 T^2 
where: T is difference in deg C from "turnpoint" (25 deg C is standard)

*/

unsigned short usPPMtbloffset 	= 924;
short usPPMtablesize		= 2101;
short sPPMtable[2101] = { 
  7776,
  7760,
  7744,
  7728,
  7712,
  7696,
  7680,
  7664,
  7649,
  7633,
  7617,
  7601,
  7586,
  7570,
  7554,
  7539,
  7523,
  7508,
  7492,
  7477,
  7461,
  7446,
  7431,
  7415,
  7400,
  7384,
  7369,
  7354,
  7339,
  7324,
  7309,
  7293,
  7278,
  7263,
  7248,
  7233,
  7218,
  7203,
  7189,
  7174,
  7159,
  7144,
  7129,
  7115,
  7100,
  7085,
  7071,
  7056,
  7041,
  7027,
  7012,
  6998,
  6983,
  6969,
  6955,
  6940,
  6926,
  6911,
  6897,
  6883,
  6869,
  6854,
  6840,
  6826,
  6812,
  6798,
  6784,
  6770,
  6756,
  6742,
  6728,
  6714,
  6700,
  6686,
  6672,
  6658,
  6645,
  6631,
  6617,
  6603,
  6590,
  6576,
  6562,
  6549,
  6535,
  6522,
  6508,
  6495,
  6481,
  6468,
  6454,
  6441,
  6428,
  6414,
  6401,
  6388,
  6374,
  6361,
  6348,
  6335,
  6322,
  6308,
  6295,
  6282,
  6269,
  6256,
  6243,
  6230,
  6217,
  6204,
  6191,
  6178,
  6166,
  6153,
  6140,
  6127,
  6114,
  6102,
  6089,
  6076,
  6064,
  6051,
  6038,
  6026,
  6013,
  6001,
  5988,
  5976,
  5963,
  5951,
  5938,
  5926,
  5914,
  5901,
  5889,
  5877,
  5864,
  5852,
  5840,
  5828,
  5816,
  5803,
  5791,
  5779,
  5767,
  5755,
  5743,
  5731,
  5719,
  5707,
  5695,
  5683,
  5671,
  5659,
  5647,
  5636,
  5624,
  5612,
  5600,
  5588,
  5577,
  5565,
  5553,
  5542,
  5530,
  5518,
  5507,
  5495,
  5484,
  5472,
  5461,
  5449,
  5438,
  5426,
  5415,
  5403,
  5392,
  5381,
  5369,
  5358,
  5347,
  5335,
  5324,
  5313,
  5302,
  5291,
  5280,
  5268,
  5257,
  5246,
  5235,
  5224,
  5213,
  5202,
  5191,
  5180,
  5169,
  5158,
  5147,
  5136,
  5125,
  5114,
  5104,
  5093,
  5082,
  5071,
  5060,
  5050,
  5039,
  5028,
  5018,
  5007,
  4996,
  4986,
  4975,
  4965,
  4954,
  4944,
  4933,
  4922,
  4912,
  4902,
  4891,
  4881,
  4870,
  4860,
  4850,
  4839,
  4829,
  4819,
  4808,
  4798,
  4788,
  4777,
  4767,
  4757,
  4747,
  4737,
  4727,
  4716,
  4706,
  4696,
  4686,
  4676,
  4666,
  4656,
  4646,
  4636,
  4626,
  4616,
  4606,
  4596,
  4587,
  4577,
  4567,
  4557,
  4547,
  4537,
  4528,
  4518,
  4508,
  4498,
  4489,
  4479,
  4469,
  4460,
  4450,
  4440,
  4431,
  4421,
  4412,
  4402,
  4393,
  4383,
  4374,
  4364,
  4355,
  4345,
  4336,
  4326,
  4317,
  4308,
  4298,
  4289,
  4280,
  4270,
  4261,
  4252,
  4242,
  4233,
  4224,
  4215,
  4205,
  4196,
  4187,
  4178,
  4169,
  4160,
  4151,
  4142,
  4132,
  4123,
  4114,
  4105,
  4096,
  4087,
  4078,
  4070,
  4060,
  4052,
  4043,
  4034,
  4025,
  4016,
  4007,
  3998,
  3990,
  3981,
  3972,
  3963,
  3955,
  3946,
  3937,
  3928,
  3920,
  3911,
  3902,
  3894,
  3885,
  3876,
  3868,
  3859,
  3851,
  3842,
  3834,
  3825,
  3817,
  3808,
  3800,
  3791,
  3783,
  3774,
  3766,
  3758,
  3749,
  3741,
  3733,
  3724,
  3716,
  3708,
  3699,
  3691,
  3683,
  3674,
  3666,
  3658,
  3650,
  3642,
  3633,
  3625,
  3617,
  3609,
  3601,
  3593,
  3585,
  3577,
  3569,
  3561,
  3552,
  3544,
  3537,
  3529,
  3521,
  3513,
  3505,
  3497,
  3489,
  3481,
  3473,
  3465,
  3457,
  3449,
  3442,
  3434,
  3426,
  3418,
  3410,
  3403,
  3395,
  3387,
  3379,
  3372,
  3364,
  3356,
  3349,
  3341,
  3333,
  3326,
  3318,
  3310,
  3303,
  3295,
  3288,
  3280,
  3273,
  3265,
  3258,
  3250,
  3243,
  3235,
  3228,
  3220,
  3213,
  3205,
  3198,
  3191,
  3183,
  3176,
  3168,
  3161,
  3154,
  3146,
  3139,
  3132,
  3125,
  3117,
  3110,
  3103,
  3096,
  3088,
  3081,
  3074,
  3067,
  3060,
  3053,
  3045,
  3038,
  3031,
  3024,
  3017,
  3010,
  3003,
  2996,
  2989,
  2982,
  2975,
  2968,
  2961,
  2954,
  2947,
  2940,
  2933,
  2926,
  2919,
  2912,
  2905,
  2898,
  2892,
  2885,
  2878,
  2871,
  2864,
  2857,
  2851,
  2844,
  2837,
  2830,
  2824,
  2817,
  2810,
  2803,
  2797,
  2790,
  2783,
  2777,
  2770,
  2763,
  2757,
  2750,
  2743,
  2737,
  2730,
  2724,
  2717,
  2710,
  2704,
  2697,
  2691,
  2684,
  2678,
  2671,
  2665,
  2659,
  2652,
  2646,
  2639,
  2633,
  2626,
  2620,
  2614,
  2607,
  2601,
  2595,
  2588,
  2582,
  2576,
  2569,
  2563,
  2557,
  2550,
  2544,
  2538,
  2532,
  2526,
  2519,
  2513,
  2507,
  2501,
  2495,
  2488,
  2482,
  2476,
  2470,
  2464,
  2458,
  2452,
  2446,
  2440,
  2433,
  2427,
  2421,
  2415,
  2409,
  2403,
  2397,
  2391,
  2385,
  2379,
  2373,
  2367,
  2361,
  2356,
  2350,
  2344,
  2338,
  2332,
  2326,
  2320,
  2314,
  2309,
  2303,
  2297,
  2291,
  2285,
  2279,
  2274,
  2268,
  2262,
  2256,
  2251,
  2245,
  2239,
  2234,
  2228,
  2222,
  2216,
  2211,
  2205,
  2199,
  2194,
  2188,
  2183,
  2177,
  2171,
  2166,
  2160,
  2155,
  2149,
  2143,
  2138,
  2132,
  2127,
  2121,
  2116,
  2110,
  2105,
  2099,
  2094,
  2089,
  2083,
  2078,
  2072,
  2067,
  2061,
  2056,
  2051,
  2045,
  2040,
  2035,
  2029,
  2024,
  2019,
  2013,
  2008,
  2003,
  1997,
  1992,
  1987,
  1982,
  1976,
  1971,
  1966,
  1961,
  1955,
  1950,
  1945,
  1940,
  1935,
  1929,
  1924,
  1919,
  1914,
  1909,
  1904,
  1899,
  1893,
  1888,
  1883,
  1878,
  1873,
  1868,
  1863,
  1858,
  1853,
  1848,
  1843,
  1838,
  1833,
  1828,
  1823,
  1818,
  1813,
  1808,
  1803,
  1798,
  1793,
  1788,
  1783,
  1779,
  1774,
  1769,
  1764,
  1759,
  1754,
  1749,
  1745,
  1740,
  1735,
  1730,
  1725,
  1721,
  1716,
  1711,
  1706,
  1701,
  1697,
  1692,
  1687,
  1683,
  1678,
  1673,
  1668,
  1664,
  1659,
  1654,
  1650,
  1645,
  1640,
  1636,
  1631,
  1627,
  1622,
  1617,
  1613,
  1608,
  1604,
  1599,
  1594,
  1590,
  1585,
  1581,
  1576,
  1572,
  1567,
  1563,
  1558,
  1554,
  1549,
  1545,
  1540,
  1536,
  1531,
  1527,
  1523,
  1518,
  1514,
  1509,
  1505,
  1501,
  1496,
  1492,
  1487,
  1483,
  1479,
  1474,
  1470,
  1466,
  1461,
  1457,
  1453,
  1449,
  1444,
  1440,
  1436,
  1432,
  1427,
  1423,
  1419,
  1415,
  1410,
  1406,
  1402,
  1398,
  1393,
  1389,
  1385,
  1381,
  1377,
  1373,
  1369,
  1364,
  1360,
  1356,
  1352,
  1348,
  1344,
  1340,
  1336,
  1332,
  1328,
  1324,
  1319,
  1315,
  1311,
  1307,
  1303,
  1299,
  1295,
  1291,
  1287,
  1283,
  1279,
  1275,
  1272,
  1268,
  1264,
  1260,
  1256,
  1252,
  1248,
  1244,
  1240,
  1236,
  1232,
  1229,
  1225,
  1221,
  1217,
  1213,
  1209,
  1205,
  1202,
  1198,
  1194,
  1190,
  1186,
  1183,
  1179,
  1175,
  1171,
  1168,
  1164,
  1160,
  1156,
  1153,
  1149,
  1145,
  1142,
  1138,
  1134,
  1130,
  1127,
  1123,
  1119,
  1116,
  1112,
  1109,
  1105,
  1101,
  1098,
  1094,
  1091,
  1087,
  1083,
  1080,
  1076,
  1073,
  1069,
  1066,
  1062,
  1058,
  1055,
  1051,
  1048,
  1044,
  1041,
  1037,
  1034,
  1030,
  1027,
  1023,
  1020,
  1017,
  1013,
  1010,
  1006,
  1003,
   999,
   996,
   993,
   989,
   986,
   982,
   979,
   976,
   972,
   969,
   966,
   962,
   959,
   956,
   952,
   949,
   946,
   942,
   939,
   936,
   932,
   929,
   926,
   923,
   919,
   916,
   913,
   910,
   906,
   903,
   900,
   897,
   894,
   890,
   887,
   884,
   881,
   878,
   875,
   871,
   868,
   865,
   862,
   859,
   856,
   853,
   850,
   846,
   843,
   840,
   837,
   834,
   831,
   828,
   825,
   822,
   819,
   816,
   813,
   810,
   807,
   804,
   801,
   798,
   795,
   792,
   789,
   786,
   783,
   780,
   777,
   774,
   771,
   768,
   765,
   762,
   759,
   756,
   754,
   751,
   748,
   745,
   742,
   739,
   736,
   734,
   731,
   728,
   725,
   722,
   719,
   716,
   714,
   711,
   708,
   705,
   702,
   700,
   697,
   694,
   691,
   689,
   686,
   683,
   680,
   678,
   675,
   672,
   670,
   667,
   664,
   661,
   659,
   656,
   653,
   651,
   648,
   645,
   643,
   640,
   638,
   635,
   632,
   630,
   627,
   624,
   622,
   619,
   617,
   614,
   612,
   609,
   606,
   604,
   601,
   599,
   596,
   594,
   591,
   589,
   586,
   584,
   581,
   579,
   576,
   574,
   571,
   569,
   566,
   564,
   561,
   559,
   556,
   554,
   552,
   549,
   547,
   544,
   542,
   539,
   537,
   535,
   532,
   530,
   528,
   525,
   523,
   520,
   518,
   516,
   513,
   511,
   509,
   506,
   504,
   502,
   500,
   497,
   495,
   493,
   490,
   488,
   486,
   484,
   481,
   479,
   477,
   475,
   472,
   470,
   468,
   466,
   464,
   461,
   459,
   457,
   455,
   453,
   450,
   448,
   446,
   444,
   442,
   440,
   437,
   435,
   433,
   431,
   429,
   427,
   425,
   423,
   421,
   418,
   416,
   414,
   412,
   410,
   408,
   406,
   404,
   402,
   400,
   398,
   396,
   394,
   392,
   390,
   388,
   386,
   384,
   382,
   380,
   378,
   376,
   374,
   372,
   370,
   368,
   366,
   364,
   362,
   360,
   358,
   356,
   354,
   353,
   351,
   349,
   347,
   345,
   343,
   341,
   339,
   338,
   336,
   334,
   332,
   330,
   328,
   326,
   325,
   323,
   321,
   319,
   317,
   316,
   314,
   312,
   310,
   308,
   307,
   305,
   303,
   301,
   300,
   298,
   296,
   294,
   293,
   291,
   289,
   288,
   286,
   284,
   283,
   281,
   279,
   277,
   276,
   274,
   272,
   271,
   269,
   268,
   266,
   264,
   263,
   261,
   259,
   258,
   256,
   255,
   253,
   251,
   250,
   248,
   247,
   245,
   243,
   242,
   240,
   239,
   237,
   236,
   234,
   233,
   231,
   230,
   228,
   227,
   225,
   224,
   222,
   221,
   219,
   218,
   216,
   215,
   213,
   212,
   210,
   209,
   207,
   206,
   205,
   203,
   202,
   200,
   199,
   198,
   196,
   195,
   193,
   192,
   191,
   189,
   188,
   186,
   185,
   184,
   182,
   181,
   180,
   178,
   177,
   176,
   174,
   173,
   172,
   171,
   169,
   168,
   167,
   165,
   164,
   163,
   162,
   160,
   159,
   158,
   157,
   155,
   154,
   153,
   152,
   150,
   149,
   148,
   147,
   146,
   144,
   143,
   142,
   141,
   140,
   138,
   137,
   136,
   135,
   134,
   133,
   132,
   130,
   129,
   128,
   127,
   126,
   125,
   124,
   123,
   122,
   120,
   119,
   118,
   117,
   116,
   115,
   114,
   113,
   112,
   111,
   110,
   109,
   108,
   107,
   106,
   105,
   104,
   103,
   102,
   101,
   100,
    99,
    98,
    97,
    96,
    95,
    94,
    93,
    92,
    91,
    90,
    89,
    88,
    87,
    86,
    85,
    85,
    84,
    83,
    82,
    81,
    80,
    79,
    78,
    77,
    77,
    76,
    75,
    74,
    73,
    72,
    71,
    71,
    70,
    69,
    68,
    67,
    67,
    66,
    65,
    64,
    63,
    63,
    62,
    61,
    60,
    60,
    59,
    58,
    57,
    57,
    56,
    55,
    54,
    54,
    53,
    52,
    51,
    51,
    50,
    49,
    49,
    48,
    47,
    47,
    46,
    45,
    45,
    44,
    43,
    43,
    42,
    41,
    41,
    40,
    40,
    39,
    38,
    38,
    37,
    37,
    36,
    35,
    35,
    34,
    34,
    33,
    32,
    32,
    31,
    31,
    30,
    30,
    29,
    29,
    28,
    28,
    27,
    27,
    26,
    26,
    25,
    25,
    24,
    24,
    23,
    23,
    22,
    22,
    21,
    21,
    20,
    20,
    19,
    19,
    19,
    18,
    18,
    17,
    17,
    17,
    16,
    16,
    15,
    15,
    15,
    14,
    14,
    13,
    13,
    13,
    12,
    12,
    12,
    11,
    11,
    11,
    10,
    10,
    10,
     9,
     9,
     9,
     9,
     8,
     8,
     8,
     7,
     7,
     7,
     7,
     6,
     6,
     6,
     6,
     5,
     5,
     5,
     5,
     4,
     4,
     4,
     4,
     4,
     3,
     3,
     3,
     3,
     3,
     3,
     2,
     2,
     2,
     2,
     2,
     2,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     0,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     1,
     2,
     2,
     2,
     2,
     2,
     2,
     2,
     3,
     3,
     3,
     3,
     3,
     4,
     4,
     4,
     4,
     4,
     5,
     5,
     5,
     5,
     5,
     6,
     6,
     6,
     6,
     7,
     7,
     7,
     8,
     8,
     8,
     8,
     9,
     9,
     9,
    10,
    10,
    10,
    11,
    11,
    11,
    12,
    12,
    12,
    13,
    13,
    13,
    14,
    14,
    14,
    15,
    15,
    16,
    16,
    16,
    17,
    17,
    18,
    18,
    18,
    19,
    19,
    20,
    20,
    21,
    21,
    21,
    22,
    22,
    23,
    23,
    24,
    24,
    25,
    25,
    26,
    26,
    27,
    27,
    28,
    28,
    29,
    29,
    30,
    30,
    31,
    32,
    32,
    33,
    33,
    34,
    34,
    35,
    36,
    36,
    37,
    37,
    38,
    38,
    39,
    40,
    40,
    41,
    42,
    42,
    43,
    44,
    44,
    45,
    46,
    46,
    47,
    48,
    48,
    49,
    50,
    50,
    51,
    52,
    52,
    53,
    54,
    55,
    55,
    56,
    57,
    57,
    58,
    59,
    60,
    60,
    61,
    62,
    63,
    64,
    64,
    65,
    66,
    67,
    68,
    68,
    69,
    70,
    71,
    72,
    72,
    73,
    74,
    75,
    76,
    77,
    78,
    78,
    79,
    80,
    81,
    82,
    83,
    84,
    85,
    86,
    86,
    87,
    88,
    89,
    90,
    91,
    92,
    93,
    94,
    95,
    96,
    97,
    98,
    99,
   100,
   101,
   102,
   103,
   104,
   105,
   106,
   107,
   108,
   109,
   110,
   111,
   112,
   113,
   114,
   115,
   116,
   117,
   118,
   119,
   120,
   122,
   123,
   124,
   125,
   126,
   127,
   128,
   129,
   130,
   131,
   133,
   134,
   135,
   136,
   137,
   138,
   140,
   141,
   142,
   143,
   144,
   145,
   147,
   148,
   149,
   150,
   151,
   153,
   154,
   155,
   156,
   158,
   159,
   160,
   161,
   163,
   164,
   165,
   166,
   168,
   169,
   170,
   172,
   173,
   174,
   175,
   177,
   178,
   179,
   181,
   182,
   183,
   185,
   186,
   188,
   189,
   190,
   192,
   193,
   194,
   196,
   197,
   199,
   200,
   201,
   203,
   204,
   206,
   207,
   208,
   210,
   211,
   213,
   214,
   216,
   217,
   219,
   220,
   222,
   223,
   225,
   226,
   228,
   229,
   231,
   232,
   234,
   235,
   237,
   238,
   240,
   241,
   243,
   244,
   246,
   248,
   249,
   251,
   252,
   254,
   255,
   257,
   259,
   260,
   262,
   263,
   265,
   267,
   268,
   270,
   272,
   273,
   275,
   277,
   278,
   280,
   282,
   283,
   285,
   287,
   288,
   290,
   292,
   294,
   295,
   297,
   299,
   300,
   302,
   304,
   306,
   307,
   309,
   311,
   313,
   315,
   316,
   318,
   320,
   322,
   324,
   325,
   327,
   329,
   331,
   333,
   335,
   336,
   338,
   340,
   342,
   344,
   346,
   348,
   349,
   351,
   353,
   355,
   357,
   359,
   361,
   363,
   365,
   367,
   369,
   370,
   372,
   374,
   376,
   378,
   380,
   382,
   384,
   386,
   388,
   390,
   392,
   394,
   396,
   398,
   400,
   402,
   404,
   406,
   408,
   410,
   413,
   415,
   417,
   419,
   421,
   423,
   425,
   427,
   429,
   431,
   433,
   436,
   438,
   440,
   442,
   444,
   446,
   448,
   451,
   453,
   455,
   457,
   459,
   461,
   464,
   466,
   468,
   470,
   473,
   475,
   477,
   479,
   481,
   484,
   486,
   488,
   490,
   493,
   495,
   497,
   500,
   502,
   504,
   506,
   509,
   511,
   513,
   516,
   518,
   520,
   523,
   525,
   528,
   530,
   532,
   535,
   537,
   539,
   542,
   544,
   547,
   549,
   551,
   554,
   556,
   559,
   561,
   564,
   566,
   568,
   571,
   573,
   576,
   578,
   581,
   583,
   586,
   588,
   591,
   593,
   596,
   598,
   601,
   604,
   606,
   609,
   611,
   614,
   616,
   619,
   622,
   624,
   627,
   629,
   632,
   635,
   637,
   640,
   642,
   645,
   648,
   650,
   653,
   656,
   658,
   661,
   664,
   666,
   669,
   672,
   675,
   677,
   680,
   683,
   685,
   688,
   691,
   694,
   696,
   699,
   702,
   705,
   708,
   710,
   713,
   716,
   719,
   722,
   724,
   727,
   730,
   733,
   736,
   739,
   742,
   744,
   747,
   750,
   753,
   756,
   759,
   762,
   765,
   768,
   770,
   773,
   776,
   779,
   782,
   785,
   788,
   791,
   794,
   797,
   800,
   803,
   806,
   809,
   812,
   815,
   818,
   821,
   824,
   827,
   830,
   833,
   837,
   840,
   843,
   846,
   849,
   852,
   855,
   858,
   861,
   864,
   868,
   871,
   874,
   877,
   880,
   883,
   887,
   890,
   893,
   896,
   899,
   903,
   906,
   909,
   912,
   916,
   919,
   922,
   925,
   929,
   932,
   935,
   939,
   942,
   945,
   948,
   952,
   955,
   958,
   962,
   965,
   968,
   972,
   975,
   979,
   982,
   985,
   989,
   992,
   996,
   999,
  1002,
  1006,
  1009,
  1013,
  1016,
  1020,
  1023,
  1027,
};


