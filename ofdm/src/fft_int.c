#include "fft_int.h"
#include <stdint.h>

int bitreverse(int x, int bits) {
    // This requires 10 cycles-per-bit.
	int y = 0;
	for (int i = 0; i < bits; i++) {
		y <<= 1;
        y |= x & 1;
		x >>= 1;
	}
	return y;
}

const int16_t fft1024twiddles[1024] = { \
   /* Wn^0 */ 32767, 0, \
   /* Wn^1 */ 32766, -201, \
   /* Wn^2 */ 32764, -402, \
   /* Wn^3 */ 32761, -603, \
   /* Wn^4 */ 32757, -804, \
   /* Wn^5 */ 32751, -1005, \
   /* Wn^6 */ 32744, -1206, \
   /* Wn^7 */ 32736, -1406, \
   /* Wn^8 */ 32727, -1607, \
   /* Wn^9 */ 32717, -1808, \
   /* Wn^10 */ 32705, -2009, \
   /* Wn^11 */ 32692, -2209, \
   /* Wn^12 */ 32678, -2410, \
   /* Wn^13 */ 32662, -2610, \
   /* Wn^14 */ 32646, -2811, \
   /* Wn^15 */ 32628, -3011, \
   /* Wn^16 */ 32609, -3211, \
   /* Wn^17 */ 32588, -3411, \
   /* Wn^18 */ 32567, -3611, \
   /* Wn^19 */ 32544, -3811, \
   /* Wn^20 */ 32520, -4011, \
   /* Wn^21 */ 32495, -4210, \
   /* Wn^22 */ 32468, -4409, \
   /* Wn^23 */ 32441, -4608, \
   /* Wn^24 */ 32412, -4807, \
   /* Wn^25 */ 32382, -5006, \
   /* Wn^26 */ 32350, -5205, \
   /* Wn^27 */ 32318, -5403, \
   /* Wn^28 */ 32284, -5601, \
   /* Wn^29 */ 32249, -5799, \
   /* Wn^30 */ 32213, -5997, \
   /* Wn^31 */ 32176, -6195, \
   /* Wn^32 */ 32137, -6392, \
   /* Wn^33 */ 32097, -6589, \
   /* Wn^34 */ 32056, -6786, \
   /* Wn^35 */ 32014, -6982, \
   /* Wn^36 */ 31970, -7179, \
   /* Wn^37 */ 31926, -7375, \
   /* Wn^38 */ 31880, -7571, \
   /* Wn^39 */ 31833, -7766, \
   /* Wn^40 */ 31785, -7961, \
   /* Wn^41 */ 31735, -8156, \
   /* Wn^42 */ 31684, -8351, \
   /* Wn^43 */ 31633, -8545, \
   /* Wn^44 */ 31580, -8739, \
   /* Wn^45 */ 31525, -8932, \
   /* Wn^46 */ 31470, -9126, \
   /* Wn^47 */ 31413, -9319, \
   /* Wn^48 */ 31356, -9511, \
   /* Wn^49 */ 31297, -9703, \
   /* Wn^50 */ 31236, -9895, \
   /* Wn^51 */ 31175, -10087, \
   /* Wn^52 */ 31113, -10278, \
   /* Wn^53 */ 31049, -10469, \
   /* Wn^54 */ 30984, -10659, \
   /* Wn^55 */ 30918, -10849, \
   /* Wn^56 */ 30851, -11038, \
   /* Wn^57 */ 30783, -11227, \
   /* Wn^58 */ 30713, -11416, \
   /* Wn^59 */ 30643, -11604, \
   /* Wn^60 */ 30571, -11792, \
   /* Wn^61 */ 30498, -11980, \
   /* Wn^62 */ 30424, -12166, \
   /* Wn^63 */ 30349, -12353, \
   /* Wn^64 */ 30272, -12539, \
   /* Wn^65 */ 30195, -12724, \
   /* Wn^66 */ 30116, -12909, \
   /* Wn^67 */ 30036, -13094, \
   /* Wn^68 */ 29955, -13278, \
   /* Wn^69 */ 29873, -13462, \
   /* Wn^70 */ 29790, -13645, \
   /* Wn^71 */ 29706, -13827, \
   /* Wn^72 */ 29621, -14009, \
   /* Wn^73 */ 29534, -14191, \
   /* Wn^74 */ 29446, -14372, \
   /* Wn^75 */ 29358, -14552, \
   /* Wn^76 */ 29268, -14732, \
   /* Wn^77 */ 29177, -14911, \
   /* Wn^78 */ 29085, -15090, \
   /* Wn^79 */ 28992, -15268, \
   /* Wn^80 */ 28897, -15446, \
   /* Wn^81 */ 28802, -15623, \
   /* Wn^82 */ 28706, -15799, \
   /* Wn^83 */ 28608, -15975, \
   /* Wn^84 */ 28510, -16150, \
   /* Wn^85 */ 28410, -16325, \
   /* Wn^86 */ 28309, -16499, \
   /* Wn^87 */ 28208, -16672, \
   /* Wn^88 */ 28105, -16845, \
   /* Wn^89 */ 28001, -17017, \
   /* Wn^90 */ 27896, -17189, \
   /* Wn^91 */ 27790, -17360, \
   /* Wn^92 */ 27683, -17530, \
   /* Wn^93 */ 27575, -17699, \
   /* Wn^94 */ 27466, -17868, \
   /* Wn^95 */ 27355, -18036, \
   /* Wn^96 */ 27244, -18204, \
   /* Wn^97 */ 27132, -18371, \
   /* Wn^98 */ 27019, -18537, \
   /* Wn^99 */ 26905, -18702, \
   /* Wn^100 */ 26789, -18867, \
   /* Wn^101 */ 26673, -19031, \
   /* Wn^102 */ 26556, -19194, \
   /* Wn^103 */ 26437, -19357, \
   /* Wn^104 */ 26318, -19519, \
   /* Wn^105 */ 26198, -19680, \
   /* Wn^106 */ 26077, -19840, \
   /* Wn^107 */ 25954, -20000, \
   /* Wn^108 */ 25831, -20159, \
   /* Wn^109 */ 25707, -20317, \
   /* Wn^110 */ 25582, -20474, \
   /* Wn^111 */ 25456, -20631, \
   /* Wn^112 */ 25329, -20787, \
   /* Wn^113 */ 25201, -20942, \
   /* Wn^114 */ 25072, -21096, \
   /* Wn^115 */ 24942, -21249, \
   /* Wn^116 */ 24811, -21402, \
   /* Wn^117 */ 24679, -21554, \
   /* Wn^118 */ 24546, -21705, \
   /* Wn^119 */ 24413, -21855, \
   /* Wn^120 */ 24278, -22004, \
   /* Wn^121 */ 24143, -22153, \
   /* Wn^122 */ 24006, -22301, \
   /* Wn^123 */ 23869, -22448, \
   /* Wn^124 */ 23731, -22594, \
   /* Wn^125 */ 23592, -22739, \
   /* Wn^126 */ 23452, -22883, \
   /* Wn^127 */ 23311, -23027, \
   /* Wn^128 */ 23169, -23169, \
   /* Wn^129 */ 23027, -23311, \
   /* Wn^130 */ 22883, -23452, \
   /* Wn^131 */ 22739, -23592, \
   /* Wn^132 */ 22594, -23731, \
   /* Wn^133 */ 22448, -23869, \
   /* Wn^134 */ 22301, -24006, \
   /* Wn^135 */ 22153, -24143, \
   /* Wn^136 */ 22004, -24278, \
   /* Wn^137 */ 21855, -24413, \
   /* Wn^138 */ 21705, -24546, \
   /* Wn^139 */ 21554, -24679, \
   /* Wn^140 */ 21402, -24811, \
   /* Wn^141 */ 21249, -24942, \
   /* Wn^142 */ 21096, -25072, \
   /* Wn^143 */ 20942, -25201, \
   /* Wn^144 */ 20787, -25329, \
   /* Wn^145 */ 20631, -25456, \
   /* Wn^146 */ 20474, -25582, \
   /* Wn^147 */ 20317, -25707, \
   /* Wn^148 */ 20159, -25831, \
   /* Wn^149 */ 20000, -25954, \
   /* Wn^150 */ 19840, -26077, \
   /* Wn^151 */ 19680, -26198, \
   /* Wn^152 */ 19519, -26318, \
   /* Wn^153 */ 19357, -26437, \
   /* Wn^154 */ 19194, -26556, \
   /* Wn^155 */ 19031, -26673, \
   /* Wn^156 */ 18867, -26789, \
   /* Wn^157 */ 18702, -26905, \
   /* Wn^158 */ 18537, -27019, \
   /* Wn^159 */ 18371, -27132, \
   /* Wn^160 */ 18204, -27244, \
   /* Wn^161 */ 18036, -27355, \
   /* Wn^162 */ 17868, -27466, \
   /* Wn^163 */ 17699, -27575, \
   /* Wn^164 */ 17530, -27683, \
   /* Wn^165 */ 17360, -27790, \
   /* Wn^166 */ 17189, -27896, \
   /* Wn^167 */ 17017, -28001, \
   /* Wn^168 */ 16845, -28105, \
   /* Wn^169 */ 16672, -28208, \
   /* Wn^170 */ 16499, -28309, \
   /* Wn^171 */ 16325, -28410, \
   /* Wn^172 */ 16150, -28510, \
   /* Wn^173 */ 15975, -28608, \
   /* Wn^174 */ 15799, -28706, \
   /* Wn^175 */ 15623, -28802, \
   /* Wn^176 */ 15446, -28897, \
   /* Wn^177 */ 15268, -28992, \
   /* Wn^178 */ 15090, -29085, \
   /* Wn^179 */ 14911, -29177, \
   /* Wn^180 */ 14732, -29268, \
   /* Wn^181 */ 14552, -29358, \
   /* Wn^182 */ 14372, -29446, \
   /* Wn^183 */ 14191, -29534, \
   /* Wn^184 */ 14009, -29621, \
   /* Wn^185 */ 13827, -29706, \
   /* Wn^186 */ 13645, -29790, \
   /* Wn^187 */ 13462, -29873, \
   /* Wn^188 */ 13278, -29955, \
   /* Wn^189 */ 13094, -30036, \
   /* Wn^190 */ 12909, -30116, \
   /* Wn^191 */ 12724, -30195, \
   /* Wn^192 */ 12539, -30272, \
   /* Wn^193 */ 12353, -30349, \
   /* Wn^194 */ 12166, -30424, \
   /* Wn^195 */ 11980, -30498, \
   /* Wn^196 */ 11792, -30571, \
   /* Wn^197 */ 11604, -30643, \
   /* Wn^198 */ 11416, -30713, \
   /* Wn^199 */ 11227, -30783, \
   /* Wn^200 */ 11038, -30851, \
   /* Wn^201 */ 10849, -30918, \
   /* Wn^202 */ 10659, -30984, \
   /* Wn^203 */ 10469, -31049, \
   /* Wn^204 */ 10278, -31113, \
   /* Wn^205 */ 10087, -31175, \
   /* Wn^206 */ 9895, -31236, \
   /* Wn^207 */ 9703, -31297, \
   /* Wn^208 */ 9511, -31356, \
   /* Wn^209 */ 9319, -31413, \
   /* Wn^210 */ 9126, -31470, \
   /* Wn^211 */ 8932, -31525, \
   /* Wn^212 */ 8739, -31580, \
   /* Wn^213 */ 8545, -31633, \
   /* Wn^214 */ 8351, -31684, \
   /* Wn^215 */ 8156, -31735, \
   /* Wn^216 */ 7961, -31785, \
   /* Wn^217 */ 7766, -31833, \
   /* Wn^218 */ 7571, -31880, \
   /* Wn^219 */ 7375, -31926, \
   /* Wn^220 */ 7179, -31970, \
   /* Wn^221 */ 6982, -32014, \
   /* Wn^222 */ 6786, -32056, \
   /* Wn^223 */ 6589, -32097, \
   /* Wn^224 */ 6392, -32137, \
   /* Wn^225 */ 6195, -32176, \
   /* Wn^226 */ 5997, -32213, \
   /* Wn^227 */ 5799, -32249, \
   /* Wn^228 */ 5601, -32284, \
   /* Wn^229 */ 5403, -32318, \
   /* Wn^230 */ 5205, -32350, \
   /* Wn^231 */ 5006, -32382, \
   /* Wn^232 */ 4807, -32412, \
   /* Wn^233 */ 4608, -32441, \
   /* Wn^234 */ 4409, -32468, \
   /* Wn^235 */ 4210, -32495, \
   /* Wn^236 */ 4011, -32520, \
   /* Wn^237 */ 3811, -32544, \
   /* Wn^238 */ 3611, -32567, \
   /* Wn^239 */ 3411, -32588, \
   /* Wn^240 */ 3211, -32609, \
   /* Wn^241 */ 3011, -32628, \
   /* Wn^242 */ 2811, -32646, \
   /* Wn^243 */ 2610, -32662, \
   /* Wn^244 */ 2410, -32678, \
   /* Wn^245 */ 2209, -32692, \
   /* Wn^246 */ 2009, -32705, \
   /* Wn^247 */ 1808, -32717, \
   /* Wn^248 */ 1607, -32727, \
   /* Wn^249 */ 1406, -32736, \
   /* Wn^250 */ 1206, -32744, \
   /* Wn^251 */ 1005, -32751, \
   /* Wn^252 */ 804, -32757, \
   /* Wn^253 */ 603, -32761, \
   /* Wn^254 */ 402, -32764, \
   /* Wn^255 */ 201, -32766, \
   /* Wn^256 */ 0, -32767, \
   /* Wn^257 */ -201, -32766, \
   /* Wn^258 */ -402, -32764, \
   /* Wn^259 */ -603, -32761, \
   /* Wn^260 */ -804, -32757, \
   /* Wn^261 */ -1005, -32751, \
   /* Wn^262 */ -1206, -32744, \
   /* Wn^263 */ -1406, -32736, \
   /* Wn^264 */ -1607, -32727, \
   /* Wn^265 */ -1808, -32717, \
   /* Wn^266 */ -2009, -32705, \
   /* Wn^267 */ -2209, -32692, \
   /* Wn^268 */ -2410, -32678, \
   /* Wn^269 */ -2610, -32662, \
   /* Wn^270 */ -2811, -32646, \
   /* Wn^271 */ -3011, -32628, \
   /* Wn^272 */ -3211, -32609, \
   /* Wn^273 */ -3411, -32588, \
   /* Wn^274 */ -3611, -32567, \
   /* Wn^275 */ -3811, -32544, \
   /* Wn^276 */ -4011, -32520, \
   /* Wn^277 */ -4210, -32495, \
   /* Wn^278 */ -4409, -32468, \
   /* Wn^279 */ -4608, -32441, \
   /* Wn^280 */ -4807, -32412, \
   /* Wn^281 */ -5006, -32382, \
   /* Wn^282 */ -5205, -32350, \
   /* Wn^283 */ -5403, -32318, \
   /* Wn^284 */ -5601, -32284, \
   /* Wn^285 */ -5799, -32249, \
   /* Wn^286 */ -5997, -32213, \
   /* Wn^287 */ -6195, -32176, \
   /* Wn^288 */ -6392, -32137, \
   /* Wn^289 */ -6589, -32097, \
   /* Wn^290 */ -6786, -32056, \
   /* Wn^291 */ -6982, -32014, \
   /* Wn^292 */ -7179, -31970, \
   /* Wn^293 */ -7375, -31926, \
   /* Wn^294 */ -7571, -31880, \
   /* Wn^295 */ -7766, -31833, \
   /* Wn^296 */ -7961, -31785, \
   /* Wn^297 */ -8156, -31735, \
   /* Wn^298 */ -8351, -31684, \
   /* Wn^299 */ -8545, -31633, \
   /* Wn^300 */ -8739, -31580, \
   /* Wn^301 */ -8932, -31525, \
   /* Wn^302 */ -9126, -31470, \
   /* Wn^303 */ -9319, -31413, \
   /* Wn^304 */ -9511, -31356, \
   /* Wn^305 */ -9703, -31297, \
   /* Wn^306 */ -9895, -31236, \
   /* Wn^307 */ -10087, -31175, \
   /* Wn^308 */ -10278, -31113, \
   /* Wn^309 */ -10469, -31049, \
   /* Wn^310 */ -10659, -30984, \
   /* Wn^311 */ -10849, -30918, \
   /* Wn^312 */ -11038, -30851, \
   /* Wn^313 */ -11227, -30783, \
   /* Wn^314 */ -11416, -30713, \
   /* Wn^315 */ -11604, -30643, \
   /* Wn^316 */ -11792, -30571, \
   /* Wn^317 */ -11980, -30498, \
   /* Wn^318 */ -12166, -30424, \
   /* Wn^319 */ -12353, -30349, \
   /* Wn^320 */ -12539, -30272, \
   /* Wn^321 */ -12724, -30195, \
   /* Wn^322 */ -12909, -30116, \
   /* Wn^323 */ -13094, -30036, \
   /* Wn^324 */ -13278, -29955, \
   /* Wn^325 */ -13462, -29873, \
   /* Wn^326 */ -13645, -29790, \
   /* Wn^327 */ -13827, -29706, \
   /* Wn^328 */ -14009, -29621, \
   /* Wn^329 */ -14191, -29534, \
   /* Wn^330 */ -14372, -29446, \
   /* Wn^331 */ -14552, -29358, \
   /* Wn^332 */ -14732, -29268, \
   /* Wn^333 */ -14911, -29177, \
   /* Wn^334 */ -15090, -29085, \
   /* Wn^335 */ -15268, -28992, \
   /* Wn^336 */ -15446, -28897, \
   /* Wn^337 */ -15623, -28802, \
   /* Wn^338 */ -15799, -28706, \
   /* Wn^339 */ -15975, -28608, \
   /* Wn^340 */ -16150, -28510, \
   /* Wn^341 */ -16325, -28410, \
   /* Wn^342 */ -16499, -28309, \
   /* Wn^343 */ -16672, -28208, \
   /* Wn^344 */ -16845, -28105, \
   /* Wn^345 */ -17017, -28001, \
   /* Wn^346 */ -17189, -27896, \
   /* Wn^347 */ -17360, -27790, \
   /* Wn^348 */ -17530, -27683, \
   /* Wn^349 */ -17699, -27575, \
   /* Wn^350 */ -17868, -27466, \
   /* Wn^351 */ -18036, -27355, \
   /* Wn^352 */ -18204, -27244, \
   /* Wn^353 */ -18371, -27132, \
   /* Wn^354 */ -18537, -27019, \
   /* Wn^355 */ -18702, -26905, \
   /* Wn^356 */ -18867, -26789, \
   /* Wn^357 */ -19031, -26673, \
   /* Wn^358 */ -19194, -26556, \
   /* Wn^359 */ -19357, -26437, \
   /* Wn^360 */ -19519, -26318, \
   /* Wn^361 */ -19680, -26198, \
   /* Wn^362 */ -19840, -26077, \
   /* Wn^363 */ -20000, -25954, \
   /* Wn^364 */ -20159, -25831, \
   /* Wn^365 */ -20317, -25707, \
   /* Wn^366 */ -20474, -25582, \
   /* Wn^367 */ -20631, -25456, \
   /* Wn^368 */ -20787, -25329, \
   /* Wn^369 */ -20942, -25201, \
   /* Wn^370 */ -21096, -25072, \
   /* Wn^371 */ -21249, -24942, \
   /* Wn^372 */ -21402, -24811, \
   /* Wn^373 */ -21554, -24679, \
   /* Wn^374 */ -21705, -24546, \
   /* Wn^375 */ -21855, -24413, \
   /* Wn^376 */ -22004, -24278, \
   /* Wn^377 */ -22153, -24143, \
   /* Wn^378 */ -22301, -24006, \
   /* Wn^379 */ -22448, -23869, \
   /* Wn^380 */ -22594, -23731, \
   /* Wn^381 */ -22739, -23592, \
   /* Wn^382 */ -22883, -23452, \
   /* Wn^383 */ -23027, -23311, \
   /* Wn^384 */ -23169, -23169, \
   /* Wn^385 */ -23311, -23027, \
   /* Wn^386 */ -23452, -22883, \
   /* Wn^387 */ -23592, -22739, \
   /* Wn^388 */ -23731, -22594, \
   /* Wn^389 */ -23869, -22448, \
   /* Wn^390 */ -24006, -22301, \
   /* Wn^391 */ -24143, -22153, \
   /* Wn^392 */ -24278, -22004, \
   /* Wn^393 */ -24413, -21855, \
   /* Wn^394 */ -24546, -21705, \
   /* Wn^395 */ -24679, -21554, \
   /* Wn^396 */ -24811, -21402, \
   /* Wn^397 */ -24942, -21249, \
   /* Wn^398 */ -25072, -21096, \
   /* Wn^399 */ -25201, -20942, \
   /* Wn^400 */ -25329, -20787, \
   /* Wn^401 */ -25456, -20631, \
   /* Wn^402 */ -25582, -20474, \
   /* Wn^403 */ -25707, -20317, \
   /* Wn^404 */ -25831, -20159, \
   /* Wn^405 */ -25954, -20000, \
   /* Wn^406 */ -26077, -19840, \
   /* Wn^407 */ -26198, -19680, \
   /* Wn^408 */ -26318, -19519, \
   /* Wn^409 */ -26437, -19357, \
   /* Wn^410 */ -26556, -19194, \
   /* Wn^411 */ -26673, -19031, \
   /* Wn^412 */ -26789, -18867, \
   /* Wn^413 */ -26905, -18702, \
   /* Wn^414 */ -27019, -18537, \
   /* Wn^415 */ -27132, -18371, \
   /* Wn^416 */ -27244, -18204, \
   /* Wn^417 */ -27355, -18036, \
   /* Wn^418 */ -27466, -17868, \
   /* Wn^419 */ -27575, -17699, \
   /* Wn^420 */ -27683, -17530, \
   /* Wn^421 */ -27790, -17360, \
   /* Wn^422 */ -27896, -17189, \
   /* Wn^423 */ -28001, -17017, \
   /* Wn^424 */ -28105, -16845, \
   /* Wn^425 */ -28208, -16672, \
   /* Wn^426 */ -28309, -16499, \
   /* Wn^427 */ -28410, -16325, \
   /* Wn^428 */ -28510, -16150, \
   /* Wn^429 */ -28608, -15975, \
   /* Wn^430 */ -28706, -15799, \
   /* Wn^431 */ -28802, -15623, \
   /* Wn^432 */ -28897, -15446, \
   /* Wn^433 */ -28992, -15268, \
   /* Wn^434 */ -29085, -15090, \
   /* Wn^435 */ -29177, -14911, \
   /* Wn^436 */ -29268, -14732, \
   /* Wn^437 */ -29358, -14552, \
   /* Wn^438 */ -29446, -14372, \
   /* Wn^439 */ -29534, -14191, \
   /* Wn^440 */ -29621, -14009, \
   /* Wn^441 */ -29706, -13827, \
   /* Wn^442 */ -29790, -13645, \
   /* Wn^443 */ -29873, -13462, \
   /* Wn^444 */ -29955, -13278, \
   /* Wn^445 */ -30036, -13094, \
   /* Wn^446 */ -30116, -12909, \
   /* Wn^447 */ -30195, -12724, \
   /* Wn^448 */ -30272, -12539, \
   /* Wn^449 */ -30349, -12353, \
   /* Wn^450 */ -30424, -12166, \
   /* Wn^451 */ -30498, -11980, \
   /* Wn^452 */ -30571, -11792, \
   /* Wn^453 */ -30643, -11604, \
   /* Wn^454 */ -30713, -11416, \
   /* Wn^455 */ -30783, -11227, \
   /* Wn^456 */ -30851, -11038, \
   /* Wn^457 */ -30918, -10849, \
   /* Wn^458 */ -30984, -10659, \
   /* Wn^459 */ -31049, -10469, \
   /* Wn^460 */ -31113, -10278, \
   /* Wn^461 */ -31175, -10087, \
   /* Wn^462 */ -31236, -9895, \
   /* Wn^463 */ -31297, -9703, \
   /* Wn^464 */ -31356, -9511, \
   /* Wn^465 */ -31413, -9319, \
   /* Wn^466 */ -31470, -9126, \
   /* Wn^467 */ -31525, -8932, \
   /* Wn^468 */ -31580, -8739, \
   /* Wn^469 */ -31633, -8545, \
   /* Wn^470 */ -31684, -8351, \
   /* Wn^471 */ -31735, -8156, \
   /* Wn^472 */ -31785, -7961, \
   /* Wn^473 */ -31833, -7766, \
   /* Wn^474 */ -31880, -7571, \
   /* Wn^475 */ -31926, -7375, \
   /* Wn^476 */ -31970, -7179, \
   /* Wn^477 */ -32014, -6982, \
   /* Wn^478 */ -32056, -6786, \
   /* Wn^479 */ -32097, -6589, \
   /* Wn^480 */ -32137, -6392, \
   /* Wn^481 */ -32176, -6195, \
   /* Wn^482 */ -32213, -5997, \
   /* Wn^483 */ -32249, -5799, \
   /* Wn^484 */ -32284, -5601, \
   /* Wn^485 */ -32318, -5403, \
   /* Wn^486 */ -32350, -5205, \
   /* Wn^487 */ -32382, -5006, \
   /* Wn^488 */ -32412, -4807, \
   /* Wn^489 */ -32441, -4608, \
   /* Wn^490 */ -32468, -4409, \
   /* Wn^491 */ -32495, -4210, \
   /* Wn^492 */ -32520, -4011, \
   /* Wn^493 */ -32544, -3811, \
   /* Wn^494 */ -32567, -3611, \
   /* Wn^495 */ -32588, -3411, \
   /* Wn^496 */ -32609, -3211, \
   /* Wn^497 */ -32628, -3011, \
   /* Wn^498 */ -32646, -2811, \
   /* Wn^499 */ -32662, -2610, \
   /* Wn^500 */ -32678, -2410, \
   /* Wn^501 */ -32692, -2209, \
   /* Wn^502 */ -32705, -2009, \
   /* Wn^503 */ -32717, -1808, \
   /* Wn^504 */ -32727, -1607, \
   /* Wn^505 */ -32736, -1406, \
   /* Wn^506 */ -32744, -1206, \
   /* Wn^507 */ -32751, -1005, \
   /* Wn^508 */ -32757, -804, \
   /* Wn^509 */ -32761, -603, \
   /* Wn^510 */ -32764, -402, \
   /* Wn^511 */ -32766, -201, \
};


void complex_mul_q15(int16_t *a, int16_t *b, int16_t *result) {
    result[0] = ((int32_t)a[0] * (int32_t)b[0]) >> 15;
    result[0] -= ((int32_t)a[1] * (int32_t)b[1]) >> 15;
    result[1] = ((int32_t)a[1] * (int32_t)b[0]) >> 15;
    result[1] += ((int32_t)a[0] * (int32_t)b[1]) >> 15;
}

void ComplexRotate_q15(int16_t *cplx_pair, int angle) {
   // Apply rotation to a complex sample pair.
   // Used twiddle factors.
   // cplx_pair points to real value, +1 points to imag value.
   // angle specified in units of pi/512

   // Bound angle in acceptable range

   while (angle < -512) {
      angle += 1024;
   }
   while (angle >= 512) {
      angle -= 1024;
   }

   int16_t twiddle[2];
   int16_t sample[2];
   sample[0] = cplx_pair[0];
   sample[1] = cplx_pair[1];

   // Double angle due to i+1 sequential memory
   if (angle == -512) {
      twiddle[0] = -fft1024twiddles[0];
      twiddle[1] = 0;
   } else if (angle < 0) {
      angle = -angle;
      angle <<= 1;
      twiddle[0] = fft1024twiddles[angle];
      twiddle[1] = -fft1024twiddles[angle+1];
   } else {
      angle <<= 1;
      twiddle[0] = fft1024twiddles[angle];
      twiddle[1] = fft1024twiddles[angle+1];
   }
   complex_mul_q15(twiddle, sample, cplx_pair);
}

void fft_dit_bfly_inplace_q15(int16_t *a_real, int16_t *a_imag, int16_t *b_real, int16_t *b_imag, int16_t *twid) {
	// a' = a + (twid*b)
	// b' = a - (twid*b)
	// twid_real = twid[0]
	// twid_imag = twid[1]
	int real_prod = (((int32_t)b_real[0] * (int32_t)twid[0]) - ((int32_t)b_imag[0] * (int32_t)twid[1])) >> 15;
	int imag_prod = (((int32_t)b_imag[0] * (int32_t)twid[0]) + ((int32_t)b_real[0] * (int32_t)twid[1])) >> 15;

	b_real[0] = (a_real[0] - real_prod);
	b_imag[0] = (a_imag[0] - imag_prod);
	a_real[0] = (a_real[0] + real_prod);
	a_imag[0] = (a_imag[0] + imag_prod);
}

void fft_dit_bfly_inplace_q15_first(int16_t *a_real, int16_t *a_imag, int16_t *b_real, int16_t *b_imag) {
	// a' = a + (twid*b)
	// b' = a - (twid*b)
	// twid_real = 1
	// twid_imag = 0
	int b_real_save = b_real[0];
	int b_imag_save = b_imag[0];
	b_real[0] = (a_real[0] - b_real_save);
	b_imag[0] = (a_imag[0] - b_imag_save);
	a_real[0] = (a_real[0] + b_real_save);
	a_imag[0] = (a_imag[0] + b_imag_save);
}

void fft_1024_engine(int16_t *data, int16_t *imem, int stage_reduction) {
	// Perform Decimation-In-Time FFT using in-place memory management.
	// Operates on real-valued input data.
	// imem points to N-place buffer to store imaginary parts of internal operations.
	
	int bcount = 1;
	int twiddle_step = 1024;
    int line_count = 512 >> stage_reduction;
    { // In the first stage the twiddle factor is 1+0j
		int bbase = 0;
		int bindex = 0;
		for (int zi = 0; zi < line_count; zi ++) {
			int ai = bbase + bindex;
			int bi = ai + bcount;
            
            fft_dit_bfly_inplace_q15_first(&data[ai], &imem[ai], &data[bi], &imem[bi]);

			bindex++;
			if (bindex == bcount) {
				bindex = 0;
				bbase += (2 * bcount);
			}
		}
		bcount <<= 1;
		twiddle_step >>= 1;
	}
    // After first stage, do full butterfly
    int stage_count = 10 - stage_reduction;
	for (int stage_index = 1; stage_index < stage_count; stage_index++) {

		int bbase = 0;
		int bindex = 0;
		for (int zi = 0; zi < line_count; zi ++) {
			int ai = bbase + bindex;
			int bi = ai + bcount;
            
			int ti = twiddle_step * bindex;
			
            int16_t twiddle[2];
            twiddle[0] = fft1024twiddles[ti++];
            twiddle[1] = fft1024twiddles[ti];
            
			fft_dit_bfly_inplace_q15(&data[ai], &imem[ai], &data[bi], &imem[bi], twiddle);

			bindex++;
			if (bindex == bcount) {
				bindex = 0;
				bbase += (2 * bcount);
			}
		}
		bcount <<= 1;
		twiddle_step >>= 1;
	}
}

void RFFT(int16_t *data, int stage_count){
    // Maximum stage count 10, maximum FFT N 1024
    int fft_n = 1 << stage_count;
    int16_t imem[fft_n];
    
	// Reorder input data in bit-reversed fashion:
	for (int i = 1; i < fft_n - 1; i++) {
		int bri = bitreverse(i, stage_count);
		if ((bri != i) && (bri > i)) {
			// The bit reversed index is not equal to the index, and the bit
            // reversed index is greater than the index. Do the swap.
			// Save the data to be replaced at this index.
			int ztemp_r = data[i];
			data[i] = data[bri];
			data[bri] = ztemp_r;
		}
	}

	// Zero imaginary memory
	for (int i = 0; i < fft_n; i++) {
		imem[i] = 0;
	}
    
    fft_1024_engine(data, imem, 10-stage_count);
	// Make output one-sided
	for (int i = fft_n - 2; i > 0; i-=2) {
		data[i] = data[i>>1];
		data[i+1] = imem[i>>1];
	}
	data[1] = imem[0];
}

void ifft_bfly(int16_t *a_real, int16_t *a_imag, int16_t *b_real, int16_t *b_imag, int16_t *twid) {
	// a' = a + (twid* x b)
	// b' = a - (twid* x b)
	// twid_real = twid[0]
	// twid_imag = -twid[1]
	int real_prod = (((int32_t)b_real[0] * (int32_t)twid[0]) + ((int32_t)b_imag[0] * (int32_t)twid[1])) >> 16;
	int imag_prod = (((int32_t)b_imag[0] * (int32_t)twid[0]) - ((int32_t)b_real[0] * (int32_t)twid[1])) >> 16;
   b_real[0] = (a_real[0]>>1) - real_prod;
   b_imag[0] = (a_imag[0]>>1) - imag_prod;
   a_real[0] = (a_real[0]>>1) + real_prod;
   a_imag[0] = (a_imag[0]>>1) + imag_prod;
}

void ifft_bfly_final(int16_t *a_real, int16_t *a_imag, int16_t *b_real, int16_t *b_imag, int16_t *twid) {
	// a' = a + (twid* x b)
	// b' = a - (twid* x b)
	// twid_real = twid[0]
	// twid_imag = -twid[1]
	int real_prod = (((int32_t)b_real[0] * (int32_t)twid[0]) + ((int32_t)b_imag[0] * (int32_t)twid[1])) >> 16;
	b_real[0] = (a_real[0]>>1) - real_prod;
	a_real[0] = (a_real[0]>>1) + real_prod;
}

void ifft_bfly_first(int16_t *a_real, int16_t *a_imag, int16_t *b_real, int16_t *b_imag) {
	int b_real_save = b_real[0]>>1;
	int b_imag_save = b_imag[0]>>1;
	b_real[0] = (a_real[0]>>1) - b_real_save;
	b_imag[0] = (a_imag[0]>>1) - b_imag_save;
	a_real[0] = (a_real[0]>>1) + b_real_save;
	a_imag[0] = (a_imag[0]>>1) + b_imag_save;
}

void ifft_1024_engine(int16_t *data, int16_t *imem, int stage_reduction) {
	// Perform Decimation-In-Time IFFT using in-place memory management.
	// Operates on one-sided input spectrum.
	// imem points to N-place buffer to store imaginary parts of internal operations.

	int bcount = 1;
	int twiddle_step = 1024;
    int line_count = 512 >> stage_reduction;
    { // In the first stage the twiddle factor is 1+0j
		int bbase = 0;
		int bindex = 0;
		for (int zi = 0; zi < line_count; zi ++) {
			int ai = bbase + bindex;
			int bi = ai + bcount;
            
            ifft_bfly_first(&data[ai], &imem[ai], &data[bi], &imem[bi]);

			bindex++;
			if (bindex == bcount) {
				bindex = 0;
				bbase += (2 * bcount);
			}
		}
		bcount <<= 1;
		twiddle_step >>= 1;
	}
    // Middle stages, do full butterfly
    int stage_count = 9 - stage_reduction;
	for (int stage_index = 1; stage_index < stage_count; stage_index++) {

		int bbase = 0;
		int bindex = 0;
		for (int zi = 0; zi < line_count; zi ++) {
			int ai = bbase + bindex;
			int bi = ai + bcount;

			int ti = twiddle_step * bindex;
			
            int16_t twiddle[2];
            twiddle[0] = fft1024twiddles[ti++];
            twiddle[1] = fft1024twiddles[ti];
            
			ifft_bfly(&data[ai], &imem[ai], &data[bi], &imem[bi], twiddle);

			bindex++;
			if (bindex == bcount) {
				bindex = 0;
				bbase += (2 * bcount);
			}
		}
		bcount <<= 1;
		twiddle_step >>= 1;
	}
    { // In the last stage only need the real parts

		int bbase = 0;
		int bindex = 0;
		for (int zi = 0; zi < line_count; zi ++) {
			int ai = bbase + bindex;
			int bi = ai + bcount;

			int ti = twiddle_step * bindex;
			
            int16_t twiddle[2];
            twiddle[0] = fft1024twiddles[ti++];
            twiddle[1] = fft1024twiddles[ti];
            
			ifft_bfly_final(&data[ai], &imem[ai], &data[bi], &imem[bi], twiddle);

			bindex++;
			if (bindex == bcount) {
				bindex = 0;
				bbase += (2 * bcount);
			}
		}
	}
}

void RIFFT(int16_t *data, int stage_count) {
    // Maximum stage count 10, maximum FFT N 1024
    int fft_n = 1 << stage_count;
    int16_t imem[fft_n];
	// Populate imaginary memory
    // Input imaginary memory is in odd buffer addresses
    // First fill positive spectrum imaginary mem
	for (int i = 0; i < fft_n>>1; i++) {
		imem[i] = data[(i<<1)+1];
	}
    // Now fill negative spectrum imaginary mem.
    // Negate imaginary value on this side of spectrum.
    for (int i = 1; i < fft_n>>1; i++) {
        imem[fft_n-i] = -imem[i];
    }
    imem[fft_n>>1] = 0; // nyquist
    // Now rearrange real data
    for (int i = 0; i < fft_n>>1; i++) {
        data[i] = data[i<<1];
    }
    // Now fill negative spectrum real mem.
    for (int i = 1; i < fft_n>>1; i++) {
        data[fft_n-i] = data[i];
    }
    data[fft_n>>1] = 0; // nyquist
	// Reorder input data in bit-reversed fashion:
	for (int i = 1; i < fft_n-1; i++) {
		//int bri = bri1024[i];
        int bri = bitreverse(i,stage_count);
		if (bri > i) {
			// The bit reversed index is not equal to the index, and the bit
            // reversed index is greater than the index. Do the swap.
			// Save the data to be replaced at this index.
			int ztemp_r = data[i];
            int ztemp_i = imem[i];
			data[i] = data[bri];
            imem[i] = imem[bri];
			data[bri] = ztemp_r;
            imem[bri] = ztemp_i;
		}
	}
    ifft_1024_engine(data, imem, 10-stage_count);
}