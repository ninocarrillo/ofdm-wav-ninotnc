#include "dsp.h"
#include <stdint.h>
#include <stdio.h>


#ifndef INT_MATH_TABLES
#define INT_MATH_TABLES

// Scaled square root table.
// M = SquareRootTable[N];
// N {0,1,...,1023}
// M = 2 * sqrt(1024 * N)
const int16_t  SquareRootTable[1024] = {  \
      0, 64, 91, 111, 128, 143, 157, 169, \
      181, 192, 202, 212, 222, 231, 239, 248, \
      256, 264, 272, 279, 286, 293, 300, 307, \
      314, 320, 326, 333, 339, 345, 351, 356, \
      362, 368, 373, 379, 384, 389, 395, 400, \
      405, 410, 415, 420, 425, 429, 434, 439, \
      443, 448, 453, 457, 462, 466, 470, 475, \
      479, 483, 487, 492, 496, 500, 504, 508, \
      512, 516, 520, 524, 528, 532, 535, 539, \
      543, 547, 551, 554, 558, 562, 565, 569, \
      572, 576, 580, 583, 587, 590, 594, 597, \
      600, 604, 607, 611, 614, 617, 621, 624, \
      627, 630, 634, 637, 640, 643, 646, 650, \
      653, 656, 659, 662, 665, 668, 671, 674, \
      677, 680, 683, 686, 689, 692, 695, 698, \
      701, 704, 707, 710, 713, 716, 718, 721, \
      724, 727, 730, 733, 735, 738, 741, 744, \
      746, 749, 752, 755, 757, 760, 763, 765, \
      768, 771, 773, 776, 779, 781, 784, 786, \
      789, 792, 794, 797, 799, 802, 804, 807, \
      810, 812, 815, 817, 820, 822, 825, 827, \
      830, 832, 834, 837, 839, 842, 844, 847, \
      849, 851, 854, 856, 859, 861, 863, 866, \
      868, 870, 873, 875, 878, 880, 882, 884, \
      887, 889, 891, 894, 896, 898, 901, 903, \
      905, 907, 910, 912, 914, 916, 919, 921, \
      923, 925, 927, 930, 932, 934, 936, 938, \
      941, 943, 945, 947, 949, 951, 954, 956, \
      958, 960, 962, 964, 966, 968, 971, 973, \
      975, 977, 979, 981, 983, 985, 987, 989, \
      991, 994, 996, 998, 1000, 1002, 1004, 1006, \
      1008, 1010, 1012, 1014, 1016, 1018, 1020, 1022, \
      1024, 1026, 1028, 1030, 1032, 1034, 1036, 1038, \
      1040, 1042, 1044, 1046, 1048, 1050, 1052, 1054, \
      1056, 1057, 1059, 1061, 1063, 1065, 1067, 1069, \
      1071, 1073, 1075, 1077, 1079, 1080, 1082, 1084, \
      1086, 1088, 1090, 1092, 1094, 1096, 1097, 1099, \
      1101, 1103, 1105, 1107, 1109, 1110, 1112, 1114, \
      1116, 1118, 1120, 1121, 1123, 1125, 1127, 1129, \
      1130, 1132, 1134, 1136, 1138, 1139, 1141, 1143, \
      1145, 1147, 1148, 1150, 1152, 1154, 1156, 1157, \
      1159, 1161, 1163, 1164, 1166, 1168, 1170, 1171, \
      1173, 1175, 1177, 1178, 1180, 1182, 1184, 1185, \
      1187, 1189, 1190, 1192, 1194, 1196, 1197, 1199, \
      1201, 1202, 1204, 1206, 1208, 1209, 1211, 1213, \
      1214, 1216, 1218, 1219, 1221, 1223, 1224, 1226, \
      1228, 1229, 1231, 1233, 1234, 1236, 1238, 1239, \
      1241, 1243, 1244, 1246, 1248, 1249, 1251, 1253, \
      1254, 1256, 1257, 1259, 1261, 1262, 1264, 1266, \
      1267, 1269, 1270, 1272, 1274, 1275, 1277, 1278, \
      1280, 1282, 1283, 1285, 1286, 1288, 1290, 1291, \
      1293, 1294, 1296, 1297, 1299, 1301, 1302, 1304, \
      1305, 1307, 1308, 1310, 1312, 1313, 1315, 1316, \
      1318, 1319, 1321, 1322, 1324, 1326, 1327, 1329, \
      1330, 1332, 1333, 1335, 1336, 1338, 1339, 1341, \
      1342, 1344, 1346, 1347, 1349, 1350, 1352, 1353, \
      1355, 1356, 1358, 1359, 1361, 1362, 1364, 1365, \
      1367, 1368, 1370, 1371, 1373, 1374, 1376, 1377, \
      1379, 1380, 1382, 1383, 1385, 1386, 1387, 1389, \
      1390, 1392, 1393, 1395, 1396, 1398, 1399, 1401, \
      1402, 1404, 1405, 1407, 1408, 1409, 1411, 1412, \
      1414, 1415, 1417, 1418, 1420, 1421, 1422, 1424, \
      1425, 1427, 1428, 1430, 1431, 1433, 1434, 1435, \
      1437, 1438, 1440, 1441, 1442, 1444, 1445, 1447, \
      1448, 1450, 1451, 1452, 1454, 1455, 1457, 1458, \
      1459, 1461, 1462, 1464, 1465, 1466, 1468, 1469, \
      1471, 1472, 1473, 1475, 1476, 1478, 1479, 1480, \
      1482, 1483, 1484, 1486, 1487, 1489, 1490, 1491, \
      1493, 1494, 1495, 1497, 1498, 1500, 1501, 1502, \
      1504, 1505, 1506, 1508, 1509, 1510, 1512, 1513, \
      1515, 1516, 1517, 1519, 1520, 1521, 1523, 1524, \
      1525, 1527, 1528, 1529, 1531, 1532, 1533, 1535, \
      1536, 1537, 1539, 1540, 1541, 1543, 1544, 1545, \
      1547, 1548, 1549, 1551, 1552, 1553, 1555, 1556, \
      1557, 1559, 1560, 1561, 1562, 1564, 1565, 1566, \
      1568, 1569, 1570, 1572, 1573, 1574, 1575, 1577, \
      1578, 1579, 1581, 1582, 1583, 1585, 1586, 1587, \
      1588, 1590, 1591, 1592, 1594, 1595, 1596, 1597, \
      1599, 1600, 1601, 1603, 1604, 1605, 1606, 1608, \
      1609, 1610, 1611, 1613, 1614, 1615, 1617, 1618, \
      1619, 1620, 1622, 1623, 1624, 1625, 1627, 1628, \
      1629, 1630, 1632, 1633, 1634, 1635, 1637, 1638, \
      1639, 1640, 1642, 1643, 1644, 1645, 1647, 1648, \
      1649, 1650, 1652, 1653, 1654, 1655, 1657, 1658, \
      1659, 1660, 1662, 1663, 1664, 1665, 1666, 1668, \
      1669, 1670, 1671, 1673, 1674, 1675, 1676, 1677, \
      1679, 1680, 1681, 1682, 1684, 1685, 1686, 1687, \
      1688, 1690, 1691, 1692, 1693, 1694, 1696, 1697, \
      1698, 1699, 1701, 1702, 1703, 1704, 1705, 1707, \
      1708, 1709, 1710, 1711, 1713, 1714, 1715, 1716, \
      1717, 1718, 1720, 1721, 1722, 1723, 1724, 1726, \
      1727, 1728, 1729, 1730, 1732, 1733, 1734, 1735, \
      1736, 1737, 1739, 1740, 1741, 1742, 1743, 1745, \
      1746, 1747, 1748, 1749, 1750, 1752, 1753, 1754, \
      1755, 1756, 1757, 1759, 1760, 1761, 1762, 1763, \
      1764, 1766, 1767, 1768, 1769, 1770, 1771, 1772, \
      1774, 1775, 1776, 1777, 1778, 1779, 1781, 1782, \
      1783, 1784, 1785, 1786, 1787, 1789, 1790, 1791, \
      1792, 1793, 1794, 1795, 1797, 1798, 1799, 1800, \
      1801, 1802, 1803, 1805, 1806, 1807, 1808, 1809, \
      1810, 1811, 1812, 1814, 1815, 1816, 1817, 1818, \
      1819, 1820, 1821, 1823, 1824, 1825, 1826, 1827, \
      1828, 1829, 1830, 1832, 1833, 1834, 1835, 1836, \
      1837, 1838, 1839, 1840, 1842, 1843, 1844, 1845, \
      1846, 1847, 1848, 1849, 1850, 1852, 1853, 1854, \
      1855, 1856, 1857, 1858, 1859, 1860, 1862, 1863, \
      1864, 1865, 1866, 1867, 1868, 1869, 1870, 1871, \
      1872, 1874, 1875, 1876, 1877, 1878, 1879, 1880, \
      1881, 1882, 1883, 1884, 1886, 1887, 1888, 1889, \
      1890, 1891, 1892, 1893, 1894, 1895, 1896, 1897, \
      1899, 1900, 1901, 1902, 1903, 1904, 1905, 1906, \
      1907, 1908, 1909, 1910, 1911, 1913, 1914, 1915, \
      1916, 1917, 1918, 1919, 1920, 1921, 1922, 1923, \
      1924, 1925, 1926, 1927, 1929, 1930, 1931, 1932, \
      1933, 1934, 1935, 1936, 1937, 1938, 1939, 1940, \
      1941, 1942, 1943, 1944, 1945, 1946, 1948, 1949, \
      1950, 1951, 1952, 1953, 1954, 1955, 1956, 1957, \
      1958, 1959, 1960, 1961, 1962, 1963, 1964, 1965, \
      1966, 1967, 1968, 1969, 1971, 1972, 1973, 1974, \
      1975, 1976, 1977, 1978, 1979, 1980, 1981, 1982, \
      1983, 1984, 1985, 1986, 1987, 1988, 1989, 1990, \
      1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, \
      1999, 2000, 2001, 2002, 2004, 2005, 2006, 2007, \
      2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, \
      2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023, \
      2024, 2025, 2026, 2027, 2028, 2029, 2030, 2031, \
      2032, 2033, 2034, 2035, 2036, 2037, 2038, 2039, \
      2040, 2041, 2042, 2043, 2044, 2045, 2046, 2047
};

const int16_t SineSamples[676] = {  \
      0, 76, 153, 229, 305, 381, 457, 534, \
      610, 686, 762, 839, 915, 991, 1067, 1144, \
      1220, 1296, 1372, 1448, 1524, 1601, 1677, 1753, \
      1829, 1905, 1981, 2057, 2134, 2210, 2286, 2362, \
      2438, 2514, 2590, 2666, 2742, 2818, 2894, 2970, \
      3046, 3122, 3197, 3273, 3349, 3425, 3501, 3577, \
      3653, 3728, 3804, 3880, 3955, 4031, 4107, 4182, \
      4258, 4334, 4409, 4485, 4560, 4636, 4711, 4787, \
      4862, 4938, 5013, 5088, 5164, 5239, 5314, 5389, \
      5465, 5540, 5615, 5690, 5765, 5840, 5915, 5990, \
      6065, 6140, 6215, 6290, 6364, 6439, 6514, 6589, \
      6663, 6738, 6813, 6887, 6962, 7036, 7111, 7185, \
      7259, 7334, 7408, 7482, 7557, 7631, 7705, 7779, \
      7853, 7927, 8001, 8075, 8149, 8223, 8296, 8370, \
      8444, 8518, 8591, 8665, 8738, 8812, 8885, 8958, \
      9032, 9105, 9178, 9251, 9325, 9398, 9471, 9544, \
      9617, 9689, 9762, 9835, 9908, 9980, 10053, 10126, \
      10198, 10270, 10343, 10415, 10487, 10560, 10632, 10704, \
      10776, 10848, 10920, 10992, 11064, 11135, 11207, 11279, \
      11350, 11422, 11493, 11564, 11636, 11707, 11778, 11849, \
      11920, 11991, 12062, 12133, 12204, 12275, 12345, 12416, \
      12487, 12557, 12627, 12698, 12768, 12838, 12908, 12978, \
      13048, 13118, 13188, 13258, 13328, 13397, 13467, 13536, \
      13606, 13675, 13744, 13813, 13882, 13952, 14020, 14089, \
      14158, 14227, 14296, 14364, 14433, 14501, 14569, 14638, \
      14706, 14774, 14842, 14910, 14978, 15046, 15113, 15181, \
      15248, 15316, 15383, 15450, 15518, 15585, 15652, 15719, \
      15786, 15852, 15919, 15986, 16052, 16119, 16185, 16251, \
      16317, 16383, 16449, 16515, 16581, 16647, 16713, 16778, \
      16844, 16909, 16974, 17039, 17104, 17169, 17234, 17299, \
      17364, 17428, 17493, 17557, 17622, 17686, 17750, 17814, \
      17878, 17942, 18006, 18069, 18133, 18196, 18260, 18323, \
      18386, 18449, 18512, 18575, 18638, 18701, 18763, 18826, \
      18888, 18950, 19012, 19074, 19136, 19198, 19260, 19322, \
      19383, 19445, 19506, 19567, 19628, 19689, 19750, 19811, \
      19872, 19932, 19993, 20053, 20113, 20173, 20233, 20293, \
      20353, 20413, 20472, 20532, 20591, 20651, 20710, 20769, \
      20828, 20886, 20945, 21004, 21062, 21121, 21179, 21237, \
      21295, 21353, 21411, 21468, 21526, 21583, 21641, 21698, \
      21755, 21812, 21869, 21925, 21982, 22038, 22095, 22151, \
      22207, 22263, 22319, 22375, 22431, 22486, 22541, 22597, \
      22652, 22707, 22762, 22817, 22871, 22926, 22980, 23035, \
      23089, 23143, 23197, 23251, 23304, 23358, 23411, 23464, \
      23518, 23571, 23624, 23676, 23729, 23781, 23834, 23886, \
      23938, 23990, 24042, 24094, 24145, 24197, 24248, 24300, \
      24351, 24402, 24452, 24503, 24554, 24604, 24654, 24705, \
      24755, 24804, 24854, 24904, 24953, 25003, 25052, 25101, \
      25150, 25199, 25247, 25296, 25344, 25393, 25441, 25489, \
      25537, 25584, 25632, 25679, 25727, 25774, 25821, 25868, \
      25914, 25961, 26007, 26054, 26100, 26146, 26192, 26238, \
      26283, 26329, 26374, 26419, 26464, 26509, 26554, 26598, \
      26643, 26687, 26731, 26775, 26819, 26863, 26907, 26950, \
      26993, 27036, 27080, 27122, 27165, 27208, 27250, 27292, \
      27334, 27376, 27418, 27460, 27501, 27543, 27584, 27625, \
      27666, 27707, 27748, 27788, 27828, 27869, 27909, 27948, \
      27988, 28028, 28067, 28106, 28146, 28185, 28223, 28262, \
      28300, 28339, 28377, 28415, 28453, 28491, 28528, 28566, \
      28603, 28640, 28677, 28714, 28751, 28787, 28823, 28860, \
      28896, 28932, 28967, 29003, 29038, 29073, 29109, 29144, \
      29178, 29213, 29247, 29282, 29316, 29350, 29384, 29417, \
      29451, 29484, 29517, 29550, 29583, 29616, 29648, 29681, \
      29713, 29745, 29777, 29809, 29840, 29872, 29903, 29934, \
      29965, 29996, 30026, 30057, 30087, 30117, 30147, 30177, \
      30207, 30236, 30265, 30295, 30324, 30352, 30381, 30410, \
      30438, 30466, 30494, 30522, 30549, 30577, 30604, 30631, \
      30658, 30685, 30712, 30738, 30765, 30791, 30817, 30843, \
      30868, 30894, 30919, 30944, 30969, 30994, 31019, 31043, \
      31068, 31092, 31116, 31140, 31163, 31187, 31210, 31233, \
      31256, 31279, 31302, 31324, 31346, 31368, 31390, 31412, \
      31434, 31455, 31477, 31498, 31519, 31539, 31560, 31580, \
      31601, 31621, 31641, 31660, 31680, 31699, 31719, 31738, \
      31756, 31775, 31794, 31812, 31830, 31848, 31866, 31884, \
      31901, 31919, 31936, 31953, 31970, 31986, 32003, 32019, \
      32035, 32051, 32067, 32082, 32098, 32113, 32128, 32143, \
      32158, 32172, 32187, 32201, 32215, 32229, 32242, 32256, \
      32269, 32282, 32295, 32308, 32321, 32333, 32345, 32358, \
      32370, 32381, 32393, 32404, 32415, 32427, 32437, 32448, \
      32459, 32469, 32479, 32489, 32499, 32509, 32518, 32527, \
      32537, 32545, 32554, 32563, 32571, 32579, 32587, 32595, \
      32603, 32611, 32618, 32625, 32632, 32639, 32646, 32652, \
      32658, 32664, 32670, 32676, 32682, 32687, 32692, 32697, \
      32702, 32707, 32712, 32716, 32720, 32724, 32728, 32732, \
      32735, 32738, 32741, 32744, 32747, 32750, 32752, 32754, \
      32756, 32758, 32760, 32761, 32763, 32764, 32765, 32766, \
      32766, 32767, 32767, 32767
};


#define SineSamplesPeriod DSP_NORM_FREQ
#define SineSample90 (DSP_NORM_FREQ >> 2)
#define SineSamplesFracBits 5 // log2 (samp rate / wavetable size)

#endif // INT_MATH_TABLES

int32_t CalcPhaseAdvance(int32_t phase, int32_t freq) {
    phase += freq;
    while (phase >= SineSamplesPeriod) {
        phase -= SineSamplesPeriod;
    }
    while (phase < 0) {
        phase += SineSamplesPeriod;
    }
    return phase;
}

int16_t GetSinSample(int32_t phase) {
    // This function is normalized to an 86.4kHz sample rate. Adjust requested
    // frequency if desired sample rate differs.
    int32_t x = SineSamplesPeriod >> 2;
    int16_t result = 0;
    while (phase >= SineSamplesPeriod) {
        phase -= SineSamplesPeriod;
    }
    while (phase < 0) {
        phase += SineSamplesPeriod;
    }
    if (phase < x) { // Less than 90 degrees
        phase >>= SineSamplesFracBits;
        result = SineSamples[phase];
    } else if (phase < (x * 2)) { // 90 to less than 180
        phase = ((SineSamplesPeriod / 2) - phase) >> SineSamplesFracBits;
        result = SineSamples[phase];
    } else if (phase < (x * 3)) { // 180 to less than 270
        phase = (phase - (SineSamplesPeriod / 2)) >> SineSamplesFracBits;
        result = -SineSamples[phase];
    } else { // 270 to less than 360
        phase = phase - (SineSamplesPeriod / 2);
        phase = ((SineSamplesPeriod / 2) - phase) >> SineSamplesFracBits;
        result = -SineSamples[phase];
    }
    return result;
}

int16_t GetCosSample(int32_t phase) {
    // This function is normalized to an 86.4kHz sample rate. Adjust requested
    // frequency if desired sample rate differs.
    phase = phase + SineSample90;
    int32_t x = SineSamplesPeriod >> 2;
    int16_t result = 0;
    while (phase >= SineSamplesPeriod) {
        phase -= SineSamplesPeriod;
    }
    while (phase < 0) {
        phase += SineSamplesPeriod;
    }
    if (phase < x) { // Less than 90 degrees
        phase >>= SineSamplesFracBits;
        result = SineSamples[phase];
    } else if (phase < (x * 2)) { // 90 to less than 180
        phase = ((SineSamplesPeriod / 2) - phase) >> SineSamplesFracBits;
        result = SineSamples[phase];
    } else if (phase < (x * 3)) { // 180 to less than 270
        phase = (phase - (SineSamplesPeriod / 2)) >> SineSamplesFracBits;
        result = -SineSamples[phase];
    } else { // 270 to less than 360
        phase = phase - (SineSamplesPeriod / 2);
        phase = ((SineSamplesPeriod / 2) - phase) >> SineSamplesFracBits;
        result = -SineSamples[phase];
    }
    return result;
}




int16_t Sinc(int32_t x) {
    // Amplitude normalized to int16_t (+32767)
    int32_t result;
    if (x == 0) {
        result = 32767;
    } else {
        
        result = ((int32_t) (DSP_NORM_FREQ / (2*3.1415926535897932384626433832795)) * (int32_t) GetSinSample(x)) / x;
    }
    return (int16_t) result;
}


int16_t HannExp(int16_t index, int16_t tap_count, int16_t expand) {
    int32_t m = (DSP_NORM_FREQ / 2) / ((tap_count + (expand * 2)) - 1);
    int32_t result;
    int32_t x = ((int32_t) index + (int32_t) expand) * m;
    result = (int32_t) GetSinSample(x);
    result *= result;
    result >>= 15;
    return result;
}

int16_t Hann(int32_t x) {
    // Amplitude normalized to int16_t (+32767)
    int32_t result = (int32_t)GetSinSample(x);
    result *= result;
    result >>= 15;
    return (int16_t)result;
}

int16_t Hamming(int16_t index, int16_t tap_count) {
    // Amplitude normalized to int16_t (+32767)
    int32_t x = (int32_t)index * DSP_NORM_FREQ / (int32_t)(tap_count - 1);
    int32_t result = (int32_t)GetCosSample(x);
    result *= 15126;
    result >>= 15;
    result = 17641 - result;
    return (int16_t)result;
}

int16_t GenNonNormLowPassFIR(int16_t *taps, int16_t cutoff_freq, int16_t tap_count) {
    // Generate a Low Pass FIR, but don't normalize to unity gain.
    // Return gain normalized to int16_t.
    int16_t n = -tap_count / 2;
    int32_t x;
    int32_t gain = 0;
    for (int16_t i = 0; i < tap_count; i++) {
        x = (int32_t) cutoff_freq * (int32_t) n;
        taps[i] = Sinc(x);
        gain += taps[i];
        n++;
    }
    gain >>= 16;
    return (int16_t) gain;
}


void GenLPFIR2(int16_t *taps, int32_t cutoff_freq, int32_t sample_rate, int16_t tap_count, int16_t window) {
    // Generate a low-pass FIR and normalize gain to 32767
    // Normalize cutoff frequency
    cutoff_freq = (DSP_NORM_FREQ * cutoff_freq) / sample_rate;
    int32_t x = -cutoff_freq * (int32_t)(tap_count - 1) / 2;
    int32_t gain = 0;
    for (int16_t i = 0; i < tap_count; i++) {
        taps[i] = Sinc(x);
        if (window) {
            // Apply a filter window
            taps[i] = ((int32_t)taps[i] * (int32_t)Hamming(i, tap_count)) >> 15;
        }
        gain += taps[i];
        x += cutoff_freq;
    }
    // Normalize gain to about 1x
    for (int i = 0; i < tap_count; i++) {
        x = taps[i];
        x <<= 15;
        x = x / gain;
        taps[i] = (int16_t) x;
    }
    return;
}

void GenInterpFIR(int16_t *taps, int interp_rate, int span) {
    // Generate an interpolating filter.
    // 'interp_rate' is the integer interpolation rate.
    // 'span' is the number of baseband samples used in each
    // phase of the output.
    // tap count is interp_rate * span.
    // Cutoff frequency set to baseband Nyquist.
    // Hamming window is applied.
    // Taps are returned in phase groups.
    // Gain is normalized to 32767
    int tap_count =  interp_rate * span;
    int32_t cutoff_freq = DSP_NORM_FREQ / ((int32_t)interp_rate * 2);
    int32_t x = -cutoff_freq * (int32_t)(tap_count - 1) / 2;
    int32_t gain = 0;
    int temp_taps[tap_count];
    for (int16_t i = 0; i < tap_count; i++) {
        temp_taps[i] = Sinc(x);

            // Apply a filter window
            temp_taps[i] = ((int32_t)temp_taps[i] * (int32_t)Hamming(i, tap_count)) >> 15;

        gain += temp_taps[i];
        x += cutoff_freq;
    }
    // Normalize gain to about 1x
    for (int i = 0; i < tap_count; i++) {
        x = temp_taps[i];
        x <<= 15;
        x = x / gain;
        temp_taps[i] = (int16_t) x;
    }
    // Now arrange the taps in phases
    for (int i = 0; i < interp_rate; i++) {
        for (int j = 0; j < span; j++) {
            int k = i * span;
            int m = j * interp_rate;
            taps[k+j] = temp_taps[m+i];
        }
    }
    return;
}

void GenInterpFIR2(int16_t *taps, int32_t cutoff_freq, int32_t sample_rate, int interp_rate, int span) {
    // Generate an interpolating filter.
    // 'interp_rate' is the integer interpolation rate.
    // 'span' is the number of baseband samples used in each
    // phase of the output.
    // tap count is interp_rate * span.
    // Cutoff frequency set manually.
    // Hamming window is applied.
    // Taps are returned in phase groups.
    // Gain is normalized to 32767
    int tap_count =  interp_rate * span;
    cutoff_freq = (DSP_NORM_FREQ * cutoff_freq) / sample_rate;
    int32_t x = -cutoff_freq * (int32_t)(tap_count - 1) / 2;
    int32_t gain = 0;
    int16_t temp_taps[tap_count];
    for (int16_t i = 0; i < tap_count; i++) {
        temp_taps[i] = Sinc(x);

            // Apply a filter window
            temp_taps[i] = ((int32_t)temp_taps[i] * (int32_t)Hamming(i, tap_count)) >> 15;

        gain += temp_taps[i];
        x += cutoff_freq;
    }
    // Normalize gain to about 1x
    for (int i = 0; i < tap_count; i++) {
        x = temp_taps[i];
        x <<= 15;
        x = x / gain;
        temp_taps[i] = (int16_t) x;
    }
    // Now arrange the taps in phases
    for (int i = 0; i < interp_rate; i++) {
        for (int j = 0; j < span; j++) {
            int k = i * span;
            int m = j * interp_rate;
            taps[k+j] = temp_taps[m+i];
        }
    }
    return;
}

void GenInterpFIR4(int16_t *taps, int32_t cutoff_freq, int32_t sample_rate, int interp_rate, int span, int32_t set_gain) {
    // Generate an interpolating filter.
    // 'interp_rate' is the integer interpolation rate.
    // 'span' is the number of baseband samples used in each
    // phase of the output.
    // tap count is interp_rate * span.
    // Cutoff frequency set manually.
    // Hamming window is applied.
    // Taps are returned in phase groups.
    // Gain is normalized to 32767
    int tap_count =  interp_rate * span;
    cutoff_freq = (DSP_NORM_FREQ * cutoff_freq) / sample_rate;
    int32_t x = -cutoff_freq * (int32_t)(tap_count - 1) / 2;
    int32_t gain = 0;
    int16_t temp_taps[tap_count];
    for (int16_t i = 0; i < tap_count; i++) {
        temp_taps[i] = Sinc(x);

            // Apply a filter window
            temp_taps[i] = ((int32_t)temp_taps[i] * (int32_t)Hamming(i, tap_count)) >> 15;

        gain += temp_taps[i];
        x += cutoff_freq;
    }
    // Normalize gain to about 1x
    for (int i = 0; i < tap_count; i++) {
        x = temp_taps[i];
        x *= set_gain;
        x = x / gain;
        temp_taps[i] = (int16_t) x;
    }
    // Now arrange the taps in phases
    for (int i = 0; i < interp_rate; i++) {
        for (int j = 0; j < span; j++) {
            int k = i * span;
            int m = j * interp_rate;
            taps[k+j] = temp_taps[m+i];
        }
    }
    return;
}


void GenInterpFIR3(int16_t *taps, int32_t hpf_band, int32_t lpf_band, int32_t sample_rate, int interp_rate, int span) {
    // Generate an interpolating filter.
    // 'interp_rate' is the integer interpolation rate.
    // 'span' is the number of baseband samples used in each
    // phase of the output.
    // tap count is interp_rate * span.
    // Cutoff frequency set manually.
    // Hamming window is applied.
    // Taps are returned in phase groups.
    // Gain is normalized to 32767
    int tap_count =  interp_rate * span;
    int16_t temp_taps[tap_count];
    GenBandFIR3(temp_taps, hpf_band, lpf_band, sample_rate, tap_count, 1);
    // Now arrange the taps in phases
    for (int i = 0; i < interp_rate; i++) {
        for (int j = 0; j < span; j++) {
            int k = i * span;
            int m = j * interp_rate;
            taps[k+j] = temp_taps[m+i];
        }
    }
    return;
}

int16_t GetHPFTap(int32_t cutoff_freq, int16_t tap_count, int16_t index) {
    int16_t tap;
    cutoff_freq = (DSP_NORM_FREQ /2) - cutoff_freq;
    tap = GetLPFTap(cutoff_freq, tap_count, index);
    if (index & 1) {
        tap = -tap;
    }
    return tap;
}

int16_t GetLPFTap(int32_t cutoff_freq, int16_t tap_count, int16_t index) {
    int32_t x = -cutoff_freq * (int32_t)((tap_count - 1) / 2);
    x += (int32_t)index * cutoff_freq;
    return Sinc(x);
}


void GenHPFIR3(int16_t *taps, int32_t cutoff_freq, int32_t sample_rate, int16_t tap_count, int16_t window) {

    // Generate a high-pass FIR and normalize gain to 32767
    // Uses spectral reversal.
    // Start by generating a low-pass FIR
    // Normalize the cutoff frequency
    cutoff_freq = (DSP_NORM_FREQ * cutoff_freq) / sample_rate;
    cutoff_freq = (DSP_NORM_FREQ/2) - cutoff_freq;
    GenLPFIR2(taps, cutoff_freq, DSP_NORM_FREQ, tap_count, 0);
    // Now invert every other tap
    for (int16_t i = 0; i < tap_count; i++) {
        if (i & 1) {
            taps[i] = -taps[i];
        }
    }
    if (window) {
        for (int16_t i = 0; i < tap_count; i++) {
            // Apply a filter window
            //taps[i] = MulQ0_15(taps[i], HannExp(i, tap_count, tap_count / 16));
            taps[i] = ((int32_t)taps[i] * (int32_t)Hamming(i, tap_count)) >> 15;;
        }
    }
    return;
}


void GenBandFIR3(int16_t *taps, int32_t hpf_band, int32_t lpf_band, int32_t sample_rate, int16_t tap_count, int16_t window) {

    hpf_band = (DSP_NORM_FREQ * hpf_band) / sample_rate;
    lpf_band = (DSP_NORM_FREQ * lpf_band) / sample_rate;
    
    int sub_tap_count = (tap_count * 2) - 1;
    
    int32_t tap_sum = 0;
    for (int i = 0; i < sub_tap_count; i++) {
        tap_sum += (int32_t)GetLPFTap(lpf_band, sub_tap_count, i) * (int32_t)GetHPFTap(hpf_band, sub_tap_count, i);
    }
    int32_t scale_factor = tap_sum / 32767;
    while ((tap_sum / scale_factor) > 32767) {
        scale_factor++;
    }
    
    // Combine filters through convolution
    for (int n = 0; n < tap_count; n++) {
        tap_sum = 0;
        for (int m = 0; m < sub_tap_count; m++) {
            int fi = (n - m) + (3 * (tap_count - 1) / 2);
            if ((fi >= 0) && (fi < sub_tap_count)) {
                int32_t x = (int32_t)GetLPFTap(lpf_band, sub_tap_count, m);
                int32_t y = (int32_t)GetHPFTap(hpf_band, sub_tap_count, fi);
                tap_sum += x * y;
            }
        }
        tap_sum /= scale_factor;
        taps[n] = (int16_t)tap_sum;
        if (window) {
            taps[n] = ((int32_t)taps[n] * (int32_t)Hamming(n, tap_count)) >> 15;;
        }
    }
}

void GenLPMAFIR(int16_t *taps, int32_t lpf_band, int32_t sample_rate, int16_t ma_tap_count, int16_t tap_count, int16_t window) {

    lpf_band = (DSP_NORM_FREQ * lpf_band) / sample_rate;
    
    //int sub_tap_count = (tap_count * 2) - 1;
    int lpf_tap_count = (tap_count + ma_tap_count) - 1;
    
    // Combine filters through convolution
    int32_t gain = 0;
    for (int n = 0; n < tap_count; n++) {
        int32_t tap_sum = 0;
        for (int m = 0; m < lpf_tap_count; m++) {
            int fi = (n - m) + (ma_tap_count-1);
            if ((fi >= 0) && (fi < ma_tap_count)) {
                int32_t x = (int32_t)GetLPFTap(lpf_band, lpf_tap_count, m);
                //int32_t y = (int32_t)GetHPFTap(hpf_band, sub_tap_count, fi);
                int32_t y = 1;
                tap_sum += x * y;
            }
        }
        tap_sum /= ma_tap_count;
        taps[n] = (int16_t)tap_sum;
        if (window) {
            taps[n] = ((int32_t)taps[n] * (int32_t)Hamming(n, tap_count)) >> 15;;
        }
        gain += taps[n];
    }
    // Normalize gain to about 1x
    for (int i = 0; i < tap_count; i++) {
        int32_t x = taps[i];
        x <<= 15;
        x = x / gain;
        taps[i] = (int16_t) x;
    }
}

int32_t Sqrt(int32_t x) {
    int32_t error = 100000;
    int32_t estimate = 1;
    int32_t maxiter = 2000;
    while ((error > estimate) && (maxiter > 0)) {
        estimate = (estimate + (x / estimate)) / 2;
        error = (estimate * estimate) - x;
        if (error < 0) {
            error = -error;
        }
        maxiter--;
    }
    return estimate;
}

int16_t GenHilbertFIR(int16_t *taps, int16_t tap_count) {
    int16_t n = -tap_count / 2;
    for (int i = 0; i < tap_count; i++) {
        if (n % 2) {
            // n is odd
            taps[i] = (int32_t) 683565276 / ((int32_t) n << 15);
            // Apply a Hamming window
            taps[i] = ((int32_t)taps[i] * (int32_t)Hamming(i, tap_count)) >> 15;
        } else {
            // n is even
            taps[i] = 0;
        }
        n++;
    }
    return (tap_count - 1) / 2;
}


int16_t GetFastFilterOutput(int16_t *samples, int sample_i, int sample_n, int16_t *filter, int filter_n, int shift) {
	// Generate a single FIR sample output via circular convolution.
	register int32_t sum = 0;
	register int norm_shift = 15 + shift;
    register int16_t *inputSamples = &samples[sample_i];
    register int16_t *coeffs = filter;
	for (register int i = 0; i < filter_n; i++) {
        sum += *inputSamples-- * *coeffs++ >> norm_shift;
        if (inputSamples < samples) {
            inputSamples = &samples[sample_n-1];
        }
	}
	if ((sum > 32767) || (sum < -32768)) {
		printf("**************************************************GFFO OVERFLOW******************************************\r\n");
	}
	return (int16_t)sum;
}

int Interpolate(Interpolator_struct *interpolator, int16_t *input_data, int input_count, int16_t *output_data, int shift) {
	int output_index = 0;
    int delay = (interpolator->TapN >> 1) - 1;
	for (int i = 0; i < input_count+delay; i++) {
		// Put input data into circular buffer
        if (i < input_count) {
            interpolator->CircBuf[interpolator->CircIndex++] = input_data[i];
        } else {
            interpolator->CircBuf[interpolator->CircIndex++] = 0;
        }
		if (interpolator->CircIndex >= interpolator->CircN) {
			interpolator->CircIndex = 0;
		}
        if (i >= delay) {
			for (int j = 0; j < interpolator->Rate; j++) {
				output_data[output_index++] = GetFastFilterOutput(interpolator->CircBuf, interpolator->CircIndex, interpolator->CircN, &interpolator->Taps[j*interpolator->TapN], interpolator->TapN, shift);
			}
		}
	}
	return output_index;
}


int Decimate(Decimator_struct *deci, int16_t sample) {
    int data_flag = 0;
    // Advance the circular buffer index;
    deci->CircIndex++;
    // Check for rollover
    if (deci->CircIndex >= deci->CircN) {
        deci->CircIndex = 0;
    }
    // Put new sample in circular buffer
    deci->CircBuf[deci->CircIndex] = sample;
    // Advance the decimation index
    deci->DeciIndex++;
    // Check for rollover
    if (deci->DeciIndex >= deci->Rate) {
        deci->DeciIndex = 0;
        // There is rollover, indicate new data will be available
        data_flag = 1;
        // Perform filter
        deci->Output = GetFastFilterOutput(deci->CircBuf, deci->CircIndex, deci->CircN, deci->Taps, deci->TapN, 1);
    }
    return data_flag;
}

void ComplexMul_q15(int16_t *a, int16_t *b, int16_t *result) {
    result[0] = ((int32_t)a[0] * (int32_t)b[0]) >> 15;
    result[0] -= ((int32_t)a[1] * (int32_t)b[1]) >> 15;
    result[1] = ((int32_t)a[1] * (int32_t)b[0]) >> 15;
    result[1] += ((int32_t)a[0] * (int32_t)b[1]) >> 15;
}


void ComplexMul_var(int16_t *a, int16_t *b, int16_t *result, int norm) {
	// Reduce the post-multiply shift to norm bits, to provide 15-norm
	// bits of mantissa (besides sign). 
    result[0] = ((int32_t)a[0] * (int32_t)b[0]) >> norm;
    result[0] -= ((int32_t)a[1] * (int32_t)b[1]) >> norm;
    result[1] = ((int32_t)a[1] * (int32_t)b[0]) >> norm;
    result[1] += ((int32_t)a[0] * (int32_t)b[1]) >> norm;
}

void ComplexMul_q13(int16_t *a, int16_t *b, int16_t *result) {
	// Reduce the post-multiply shift to 13 bits, to provide 2
	// bits of mantissa (besides sign). This allows up to 4x gain.
    result[0] = ((int32_t)a[0] * (int32_t)b[0]) >> 13;
    result[0] -= ((int32_t)a[1] * (int32_t)b[1]) >> 13;
    result[1] = ((int32_t)a[1] * (int32_t)b[0]) >> 13;
    result[1] += ((int32_t)a[0] * (int32_t)b[1]) >> 13;
}

int16_t ApproxATan_q15(int16_t real, int16_t imag) {
    // 0.97239411x - 0.19194795x^3
    // atan (z) ~ 0.9724х — 0.1919x^3
    // z = y / x 
    // https://computingandrecording.wordpress.com/2017/04/24/how-to-find-a-fast-floating-point-atan2-approximation/
    int32_t result;
    if (real != 0) {
        int32_t z = ((int32_t)imag<<15) / (int32_t)real;
        // Calculate z^3
        int32_t z3 = (((z * z)>>15) * z)>>15;
        // calculate result
        result = ((z * 10142)>>15) - ((z3 * 2002)>>15);
    } else {
        // avoid divide by zero
        // result is zero if real part is zero.
        result = 0;
    }
    // result scaled to q15: 32768 = pi radians
    // Valid results from -pi/4 to pi/4
    return (int16_t)result;
}

int16_t ApproxATan2_q15(int16_t real, int16_t imag) {
    // Returns result in radians scaled to pi rad = 32768;
    int16_t result;
    // Select quadrant
    if (real >= 0) {
        if (imag >= 0) {
            // Quadrant I, top right
            // Check octant
            if (real >= imag) {
                // Closer to real axis (0-45 degrees)
                result = ApproxATan_q15(real, imag);
            } else {
                // Closer to imag axis (45-90 degrees)
                result = 16384 - ApproxATan_q15(imag, real);
            }
        } else {
            // Quadrant IV, bottom right
            // Check octant
            if (real >= -imag) {
                // Closer to real axis (335-360 degrees)
                result = ApproxATan_q15(real, imag);
            } else {
                // Closer to imag axis (270-335 degrees)
                result = (-16384) - ApproxATan_q15(imag, real);
            }
        }
    } else {
        if (imag >= 0) {
            // Quadrant II, top left
            if (-real >= imag) {
                // Closer to real axis (135-180 degrees)
                result = 32767 - ApproxATan_q15(-real, imag);
            } else {
                // Closer to imag axis (90-135 degrees)
                result = 16384 - ApproxATan_q15(imag, real);
            }
        } else {
            // Quadrant III, bottom left
            if (-real >= -imag) {
                // Closer to real axis (180-225 degrees)
                result = -32768 + ApproxATan_q15(-real, -imag);
            } else {
                // Closer to imag axis (225-270 degrees)
                result = (-16384) - ApproxATan_q15(imag, real);
            }
        }
    }
    return result;
}

int16_t EuclidianDistance_q15(int16_t *a, int16_t *b) {
    int32_t real = ((int32_t)a[0] - (int32_t)b[0])>>1;
    int32_t imag = ((int32_t)a[1] - (int32_t)b[1])>>1;
    real = (real * real) >> 16;
    imag = (imag * imag) >> 16;
    return (real + imag);
}

int16_t CalcDecibelEnergy(int32_t a, int32_t b) {
    int32_t x = 1;
    int16_t snr = 0;
    //a = a / b;
    if (a > 0) {
        while (x < a) {
            snr++;
            x<<=1;
        }
    }
    x = 1;
    if (b > 0) {
        while (x < b) {
            snr--;
            x<<=1;
        }
    }
    return snr*3;
}