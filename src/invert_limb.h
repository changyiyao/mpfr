/* __gmpfr_invert_limb -- implement GMP's invert_limb (which is not in GMP API)

Copyright 2016-2023 Free Software Foundation, Inc.
Contributed by the AriC and Caramba projects, INRIA.

This file is part of the GNU MPFR Library.

The GNU MPFR Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

The GNU MPFR Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the GNU MPFR Library; see the file COPYING.LESSER.  If not, see
https://www.gnu.org/licenses/ or write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA. */

#define MPFR_NEED_LONGLONG_H
#include "mpfr-impl.h"

/* for now, we only provide __gmpfr_invert_limb for 64-bit limb */
#if GMP_NUMB_BITS == 64

/* for 256 <= d9 < 512, invert_limb_table[d9-256] = floor((2^19-3*2^8)/d9) */
static const unsigned short invert_limb_table[256] =
  { 2045, 2037, 2029, 2021, 2013, 2005, 1998, 1990,
    1983, 1975, 1968, 1960, 1953, 1946, 1938, 1931,
    1924, 1917, 1910, 1903, 1896, 1889, 1883, 1876,
    1869, 1863, 1856, 1849, 1843, 1836, 1830, 1824,
    1817, 1811, 1805, 1799, 1792, 1786, 1780, 1774,
    1768, 1762, 1756, 1750, 1745, 1739, 1733, 1727,
    1722, 1716, 1710, 1705, 1699, 1694, 1688, 1683,
    1677, 1672, 1667, 1661, 1656, 1651, 1646, 1641,
    1636, 1630, 1625, 1620, 1615, 1610, 1605, 1600,
    1596, 1591, 1586, 1581, 1576, 1572, 1567, 1562,
    1558, 1553, 1548, 1544, 1539, 1535, 1530, 1526,
    1521, 1517, 1513, 1508, 1504, 1500, 1495, 1491,
    1487, 1483, 1478, 1474, 1470, 1466, 1462, 1458,
    1454, 1450, 1446, 1442, 1438, 1434, 1430, 1426,
    1422, 1418, 1414, 1411, 1407, 1403, 1399, 1396,
    1392, 1388, 1384, 1381, 1377, 1374, 1370, 1366,
    1363, 1359, 1356, 1352, 1349, 1345, 1342, 1338,
    1335, 1332, 1328, 1325, 1322, 1318, 1315, 1312,
    1308, 1305, 1302, 1299, 1295, 1292, 1289, 1286,
    1283, 1280, 1276, 1273, 1270, 1267, 1264, 1261,
    1258, 1255, 1252, 1249, 1246, 1243, 1240, 1237,
    1234, 1231, 1228, 1226, 1223, 1220, 1217, 1214,
    1211, 1209, 1206, 1203, 1200, 1197, 1195, 1192,
    1189, 1187, 1184, 1181, 1179, 1176, 1173, 1171,
    1168, 1165, 1163, 1160, 1158, 1155, 1153, 1150,
    1148, 1145, 1143, 1140, 1138, 1135, 1133, 1130,
    1128, 1125, 1123, 1121, 1118, 1116, 1113, 1111,
    1109, 1106, 1104, 1102, 1099, 1097, 1095, 1092,
    1090, 1088, 1086, 1083, 1081, 1079, 1077, 1074,
    1072, 1070, 1068, 1066, 1064, 1061, 1059, 1057,
    1055, 1053, 1051, 1049, 1047, 1044, 1042, 1040,
    1038, 1036, 1034, 1032, 1030, 1028, 1026, 1024 };

/* for 256 <= d9 < 512, invert_limb_table2[d9-256] = floor((2^19-3*2^8)/d9)^2
 *
 * Note: This table requires 4182025 to be representable in unsigned int,
 * thus disallows 16-bit int, for instance. However, this code is under
 * "#if GMP_NUMB_BITS == 64", and a system with int smaller than 32 bits
 * should better be used with GMP_NUMB_BITS == 32 (or 16 if supported).
 * This constraint is checked with MPFR_STAT_STATIC_ASSERT below.
 */
static const unsigned int invert_limb_table2[256] =
  { 4182025, 4149369, 4116841, 4084441, 4052169, 4020025, 3992004, 3960100, 3932289,
    3900625, 3873024, 3841600, 3814209, 3786916, 3755844, 3728761, 3701776, 3674889,
    3648100, 3621409, 3594816, 3568321, 3545689, 3519376, 3493161, 3470769, 3444736,
    3418801, 3396649, 3370896, 3348900, 3326976, 3301489, 3279721, 3258025, 3236401,
    3211264, 3189796, 3168400, 3147076, 3125824, 3104644, 3083536, 3062500, 3045025,
    3024121, 3003289, 2982529, 2965284, 2944656, 2924100, 2907025, 2886601, 2869636,
    2849344, 2832489, 2812329, 2795584, 2778889, 2758921, 2742336, 2725801, 2709316,
    2692881, 2676496, 2656900, 2640625, 2624400, 2608225, 2592100, 2576025, 2560000,
    2547216, 2531281, 2515396, 2499561, 2483776, 2471184, 2455489, 2439844, 2427364,
    2411809, 2396304, 2383936, 2368521, 2356225, 2340900, 2328676, 2313441, 2301289,
    2289169, 2274064, 2262016, 2250000, 2235025, 2223081, 2211169, 2199289, 2184484,
    2172676, 2160900, 2149156, 2137444, 2125764, 2114116, 2102500, 2090916, 2079364,
    2067844, 2056356, 2044900, 2033476, 2022084, 2010724, 1999396, 1990921, 1979649,
    1968409, 1957201, 1948816, 1937664, 1926544, 1915456, 1907161, 1896129, 1887876,
    1876900, 1865956, 1857769, 1846881, 1838736, 1827904, 1819801, 1809025, 1800964,
    1790244, 1782225, 1774224, 1763584, 1755625, 1747684, 1737124, 1729225, 1721344,
    1710864, 1703025, 1695204, 1687401, 1677025, 1669264, 1661521, 1653796, 1646089,
    1638400, 1628176, 1620529, 1612900, 1605289, 1597696, 1590121, 1582564, 1575025,
    1567504, 1560001, 1552516, 1545049, 1537600, 1530169, 1522756, 1515361, 1507984,
    1503076, 1495729, 1488400, 1481089, 1473796, 1466521, 1461681, 1454436, 1447209,
    1440000, 1432809, 1428025, 1420864, 1413721, 1408969, 1401856, 1394761, 1390041,
    1382976, 1375929, 1371241, 1364224, 1357225, 1352569, 1345600, 1340964, 1334025,
    1329409, 1322500, 1317904, 1311025, 1306449, 1299600, 1295044, 1288225, 1283689,
    1276900, 1272384, 1265625, 1261129, 1256641, 1249924, 1245456, 1238769, 1234321,
    1229881, 1223236, 1218816, 1214404, 1207801, 1203409, 1199025, 1192464, 1188100,
    1183744, 1179396, 1172889, 1168561, 1164241, 1159929, 1153476, 1149184, 1144900,
    1140624, 1136356, 1132096, 1125721, 1121481, 1117249, 1113025, 1108809, 1104601,
    1100401, 1096209, 1089936, 1085764, 1081600, 1077444, 1073296, 1069156, 1065024,
    1060900, 1056784, 1052676, 1048576 };

/* Implements Algorithm 2 from "Improved Division by Invariant Integers",
   Niels Möller and Torbjörn Granlund, IEEE Transactions on Computers,
   volume 60, number 2, pages 165-175, 2011. */
#define __gmpfr_invert_limb(r, d)                                       \
    do {                                                                \
      mp_limb_t _d, _d0, _i, _d40, _d63, _v0, _v1, _v2, _e, _v3, _h, _l; \
      MPFR_STAT_STATIC_ASSERT (4182025 <= UINT_MAX);                    \
      _d = (d);                                                         \
      _i = (_d >> 55) - 256; /* i = d9 - 256 */                         \
      /* the shift by 11 is for free since it is hidden in the */       \
      /* invert_limb_table2[_i] * _d40 multiplication latency */        \
      _v0 = (mp_limb_t) invert_limb_table[_i] << 11;                    \
      _d40 = (_d >> 24) + 1;                                            \
      _v1 = _v0 - ((invert_limb_table2[_i] * _d40) >> 40) - 1;          \
      _v2 = (_v1 << 13) +                                               \
        ((_v1 * ((MPFR_LIMB_ONE << 60) - _v1 * _d40)) >> 47);           \
      _d0 = _d & 1;                                                     \
      _d63 = ((_d - 1) >> 1) + 1;                                       \
      _e = - _v2 * _d63 + ((_v2 & -_d0) >> 1);                          \
      umul_hi (_h, _v2, _e);                                            \
      _v3 = (_v2 << 31) + (_h >> 1);                                    \
      umul_ppmm (_h, _l, _v3, _d);                                      \
      /* v3 is too small iff (h+d)*2^64+l+d < 2^128 */                  \
      add_ssaaaa(_h, _l, _h, _l, _d, _d);                               \
      MPFR_ASSERTD(_h == MPFR_LIMB_ZERO || -_h == MPFR_LIMB_ONE);       \
      (r) = _v3 - _h;                                                   \
    } while (0)

/* same algorithm, but return the value v3, which is such that
   v3 <= invert_limb (d) <= v3 + 1 */
#define __gmpfr_invert_limb_approx(r, d)                                \
    do {                                                                \
      mp_limb_t _d, _d0, _i, _d40, _d63, _v0, _v1, _v2, _e, _h;         \
      MPFR_STAT_STATIC_ASSERT (4182025 <= UINT_MAX);                    \
      _d = (d);                                                         \
      _i = (_d >> 55) - 256; /* i = d9 - 256 */                         \
      _v0 = (mp_limb_t) invert_limb_table[_i] << 11;                    \
      _d40 = (_d >> 24) + 1;                                            \
      _v1 = _v0 - ((invert_limb_table2[_i] * _d40) >> 40) - 1;          \
      _v2 = (_v1 << 13) +                                               \
        ((_v1 * ((MPFR_LIMB_ONE << 60) - _v1 * _d40)) >> 47);           \
      _d0 = _d & 1;                                                     \
      _d63 = ((_d - 1) >> 1) + 1;                                       \
      _e = - _v2 * _d63 + ((_v2 & -_d0) >> 1);                          \
      umul_hi (_h, _v2, _e);                                            \
      (r) = (_v2 << 31) + (_h >> 1);                                    \
    } while (0)

#elif GMP_NUMB_BITS == 32

/* for 512 <= d10 < 1024, l[d10-512] = floor((2^24-2^14+2^9)/d10) */
static const unsigned short invert_limb_table[512] =
  { 32737, 32673, 32609, 32546, 32483, 32420, 32357, 32295, 32233, 32171,
    32109, 32048, 31987, 31926, 31865, 31805, 31744, 31684, 31625, 31565,
    31506, 31447, 31388, 31329, 31271, 31212, 31154, 31097, 31039, 30982,
    30924, 30868, 30811, 30754, 30698, 30642, 30586, 30530, 30475, 30419,
    30364, 30309, 30255, 30200, 30146, 30092, 30038, 29984, 29930, 29877,
    29824, 29771, 29718, 29666, 29613, 29561, 29509, 29457, 29405, 29354,
    29303, 29251, 29200, 29150, 29099, 29049, 28998, 28948, 28898, 28849,
    28799, 28750, 28700, 28651, 28602, 28554, 28505, 28457, 28409, 28360,
    28313, 28265, 28217, 28170, 28123, 28075, 28029, 27982, 27935, 27889,
    27842, 27796, 27750, 27704, 27658, 27613, 27568, 27522, 27477, 27432,
    27387, 27343, 27298, 27254, 27209, 27165, 27121, 27078, 27034, 26990,
    26947, 26904, 26861, 26818, 26775, 26732, 26690, 26647, 26605, 26563,
    26521, 26479, 26437, 26395, 26354, 26312, 26271, 26230, 26189, 26148,
    26108, 26067, 26026, 25986, 25946, 25906, 25866, 25826, 25786, 25747,
    25707, 25668, 25628, 25589, 25550, 25511, 25473, 25434, 25395, 25357,
    25319, 25281, 25242, 25205, 25167, 25129, 25091, 25054, 25016, 24979,
    24942, 24905, 24868, 24831, 24794, 24758, 24721, 24685, 24649, 24612,
    24576, 24540, 24504, 24469, 24433, 24397, 24362, 24327, 24291, 24256,
    24221, 24186, 24151, 24117, 24082, 24047, 24013, 23979, 23944, 23910,
    23876, 23842, 23808, 23774, 23741, 23707, 23674, 23640, 23607, 23574,
    23541, 23508, 23475, 23442, 23409, 23377, 23344, 23312, 23279, 23247,
    23215, 23183, 23151, 23119, 23087, 23055, 23023, 22992, 22960, 22929,
    22898, 22866, 22835, 22804, 22773, 22742, 22711, 22681, 22650, 22619,
    22589, 22559, 22528, 22498, 22468, 22438, 22408, 22378, 22348, 22318,
    22289, 22259, 22229, 22200, 22171, 22141, 22112, 22083, 22054, 22025,
    21996, 21967, 21938, 21910, 21881, 21853, 21824, 21796, 21767, 21739,
    21711, 21683, 21655, 21627, 21599, 21571, 21544, 21516, 21488, 21461,
    21433, 21406, 21379, 21352, 21324, 21297, 21270, 21243, 21216, 21190,
    21163, 21136, 21110, 21083, 21056, 21030, 21004, 20977, 20951, 20925,
    20899, 20873, 20847, 20821, 20795, 20769, 20744, 20718, 20693, 20667,
    20642, 20616, 20591, 20566, 20540, 20515, 20490, 20465, 20440, 20415,
    20390, 20366, 20341, 20316, 20292, 20267, 20243, 20218, 20194, 20170,
    20145, 20121, 20097, 20073, 20049, 20025, 20001, 19977, 19953, 19930,
    19906, 19882, 19859, 19835, 19812, 19789, 19765, 19742, 19719, 19696,
    19672, 19649, 19626, 19603, 19581, 19558, 19535, 19512, 19489, 19467,
    19444, 19422, 19399, 19377, 19354, 19332, 19310, 19288, 19265, 19243,
    19221, 19199, 19177, 19155, 19133, 19112, 19090, 19068, 19046, 19025,
    19003, 18982, 18960, 18939, 18917, 18896, 18875, 18854, 18832, 18811,
    18790, 18769, 18748, 18727, 18706, 18686, 18665, 18644, 18623, 18603,
    18582, 18561, 18541, 18520, 18500, 18479, 18459, 18439, 18419, 18398,
    18378, 18358, 18338, 18318, 18298, 18278, 18258, 18238, 18218, 18199,
    18179, 18159, 18139, 18120, 18100, 18081, 18061, 18042, 18022, 18003,
    17984, 17964, 17945, 17926, 17907, 17888, 17869, 17850, 17831, 17812,
    17793, 17774, 17755, 17736, 17718, 17699, 17680, 17662, 17643, 17624,
    17606, 17587, 17569, 17551, 17532, 17514, 17496, 17477, 17459, 17441,
    17423, 17405, 17387, 17369, 17351, 17333, 17315, 17297, 17279, 17261,
    17244, 17226, 17208, 17191, 17173, 17155, 17138, 17120, 17103, 17085,
    17068, 17051, 17033, 17016, 16999, 16982, 16964, 16947, 16930, 16913,
    16896, 16879, 16862, 16845, 16828, 16811, 16794, 16778, 16761, 16744,
    16727, 16711, 16694, 16677, 16661, 16644, 16628, 16611, 16595, 16578,
    16562, 16546, 16529, 16513, 16497, 16481, 16464, 16448, 16432, 16416,
    16400, 16384 };

/* Implements Algorithm 3 from "Improved Division by Invariant Integers",
   Niels Möller and Torbjörn Granlund, IEEE Transactions on Computers,
   volume 60, number 2, pages 165-175, 2011. */
#define __gmpfr_invert_limb(r, d)                                       \
    do {                                                                \
      mp_limb_t _d, _d0, _d10, _d21, _d31, _v0, _v1, _v2, _e, _h, _l;   \
      _d = (d);                                                         \
      _d0 = _d & 1;                                                     \
      _d10 = _d >> 22;                                                  \
      _d21 = (_d >> 11) + 1;                                            \
      _d31 = ((_d - 1) >> 1) + 1;                                       \
      _v0 = invert_limb_table[_d10 - 512];                              \
      umul_ppmm (_h, _l, _v0 * _v0, _d21);                              \
      _v1 = (_v0 << 4) - _h - 1;                                        \
      _e = - _v1 * _d31 + ((_v1 & - _d0) >> 1);                         \
      umul_ppmm (_h, _l, _v1, _e);                                      \
      _v2 = (_v1 << 15) + (_h >> 1);                                    \
      umul_ppmm (_h, _l, _v2, _d);                                      \
      /* v2 is too small iff (h+d)*2^32+l+d < 2^64 */                   \
      add_ssaaaa(_h, _l, _h, _l, _d, _d);                               \
      MPFR_ASSERTD(_h == MPFR_LIMB_ZERO || -_h == MPFR_LIMB_ONE);       \
      (r) = _v2 - _h;                                                   \
    } while (0)

#endif /* GMP_NUMB_BITS == 64 or 32 */
