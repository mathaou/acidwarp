#include "handy.h"
#include "lut.h"

/* lut.c this code (c)Copyright 1992 by Noah Spurrier */

/* These are the new Look Up Tables for Acid Warp. Many improvements are
to be found.  These tables are totally portable. Older versions worked only on
320x200 screen sizes. The tables could not handle larger screens, and the tables
could not be enlarged because some would grow geometrically. These tables
should handle screens up to about 2048x2048 (as currently hard-coded), but
they can be easily expanded to handle larger screens as video display
technoledgy advances. (uh huh) On the other hand, who knows when I might
have access to a slide printer. The lut_dist() function is about 10% slower
than the previous non-portable version, but it's still lots faster than
trying to evaluate sqrt(x^2 + y^2). Also no math libraries need be linked
using these funcions.
*/

#define SIN_TABLE_SIZE      1024
#define FRAME_SIZE          1023

static int Sin_Table [SIN_TABLE_SIZE] =
{
    0,    3,    6,    9,   13,   16,   19,   22,
   25,   28,   31,   34,   38,   41,   44,   47,
   50,   53,   56,   59,   63,   66,   69,   72,
   75,   78,   81,   84,   87,   91,   94,   97,
  100,  103,  106,  109,  112,  115,  118,  121,
  124,  127,  130,  133,  136,  139,  142,  145,
  148,  151,  154,  157,  160,  163,  166,  169,
  172,  175,  178,  181,  184,  187,  190,  193,
  196,  199,  202,  204,  207,  210,  213,  216,
  219,  222,  224,  227,  230,  233,  236,  238,
  241,  244,  247,  249,  252,  255,  258,  260,
  263,  266,  268,  271,  274,  276,  279,  282,
  284,  287,  289,  292,  294,  297,  300,  302,
  305,  307,  310,  312,  315,  317,  320,  322,
  324,  327,  329,  332,  334,  336,  339,  341,
  343,  346,  348,  350,  353,  355,  357,  359,
  362,  364,  366,  368,  370,  373,  375,  377,
  379,  381,  383,  385,  387,  389,  391,  393,
  395,  397,  399,  401,  403,  405,  407,  409,
  411,  413,  414,  416,  418,  420,  422,  423,
  425,  427,  429,  430,  432,  434,  435,  437,
  439,  440,  442,  443,  445,  446,  448,  449,
  451,  452,  454,  455,  457,  458,  459,  461,
  462,  464,  465,  466,  467,  469,  470,  471,
  472,  474,  475,  476,  477,  478,  479,  480,
  481,  482,  483,  484,  485,  486,  487,  488,
  489,  490,  491,  492,  493,  493,  494,  495,
  496,  497,  497,  498,  499,  499,  500,  501,
  501,  502,  502,  503,  504,  504,  505,  505,
  506,  506,  506,  507,  507,  508,  508,  508,
  509,  509,  509,  509,  510,  510,  510,  510,
  510,  511,  511,  511,  511,  511,  511,  511,
  511,  511,  511,  511,  511,  511,  511,  510,
  510,  510,  510,  510,  510,  509,  509,  509,
  508,  508,  508,  507,  507,  507,  506,  506,
  505,  505,  504,  504,  503,  503,  502,  502,
  501,  500,  500,  499,  498,  498,  497,  496,
  495,  495,  494,  493,  492,  491,  491,  490,
  489,  488,  487,  486,  485,  484,  483,  482,
  481,  480,  479,  478,  476,  475,  474,  473,
  472,  471,  469,  468,  467,  465,  464,  463,
  462,  460,  459,  457,  456,  455,  453,  452,
  450,  449,  447,  446,  444,  443,  441,  439,
  438,  436,  434,  433,  431,  429,  428,  426,
  424,  423,  421,  419,  417,  415,  413,  412,
  410,  408,  406,  404,  402,  400,  398,  396,
  394,  392,  390,  388,  386,  384,  382,  380,
  378,  376,  374,  371,  369,  367,  365,  363,
  360,  358,  356,  354,  351,  349,  347,  345,
  342,  340,  338,  335,  333,  330,  328,  326,
  323,  321,  318,  316,  313,  311,  308,  306,
  303,  301,  298,  296,  293,  291,  288,  285,
  283,  280,  278,  275,  272,  270,  267,  264,
  262,  259,  256,  253,  251,  248,  245,  242,
  240,  237,  234,  231,  229,  226,  223,  220,
  217,  214,  212,  209,  206,  203,  200,  197,
  194,  191,  188,  186,  183,  180,  177,  174,
  171,  168,  165,  162,  159,  156,  153,  150,
  147,  144,  141,  138,  135,  132,  129,  126,
  123,  120,  117,  114,  111,  107,  104,  101,
   98,   95,   92,   89,   86,   83,   80,   77,
   73,   70,   67,   64,   61,   58,   55,   52,
   49,   45,   42,   39,   36,   33,   30,   27,
   24,   20,   17,   14,   11,    8,    5,    2,
   -2,   -5,   -8,  -11,  -14,  -17,  -20,  -24,
  -27,  -30,  -33,  -36,  -39,  -42,  -45,  -49,
  -52,  -55,  -58,  -61,  -64,  -67,  -70,  -73,
  -77,  -80,  -83,  -86,  -89,  -92,  -95,  -98,
 -101, -104, -107, -111, -114, -117, -120, -123,
 -126, -129, -132, -135, -138, -141, -144, -147,
 -150, -153, -156, -159, -162, -165, -168, -171,
 -174, -177, -180, -183, -186, -188, -191, -194,
 -197, -200, -203, -206, -209, -212, -214, -217,
 -220, -223, -226, -229, -231, -234, -237, -240,
 -242, -245, -248, -251, -253, -256, -259, -262,
 -264, -267, -270, -272, -275, -278, -280, -283,
 -285, -288, -291, -293, -296, -298, -301, -303,
 -306, -308, -311, -313, -316, -318, -321, -323,
 -326, -328, -330, -333, -335, -338, -340, -342,
 -345, -347, -349, -351, -354, -356, -358, -360,
 -363, -365, -367, -369, -371, -374, -376, -378,
 -380, -382, -384, -386, -388, -390, -392, -394,
 -396, -398, -400, -402, -404, -406, -408, -410,
 -412, -413, -415, -417, -419, -421, -423, -424,
 -426, -428, -429, -431, -433, -434, -436, -438,
 -439, -441, -443, -444, -446, -447, -449, -450,
 -452, -453, -455, -456, -457, -459, -460, -462,
 -463, -464, -465, -467, -468, -469, -471, -472,
 -473, -474, -475, -476, -478, -479, -480, -481,
 -482, -483, -484, -485, -486, -487, -488, -489,
 -490, -491, -491, -492, -493, -494, -495, -495,
 -496, -497, -498, -498, -499, -500, -500, -501,
 -502, -502, -503, -503, -504, -504, -505, -505,
 -506, -506, -507, -507, -507, -508, -508, -508,
 -509, -509, -509, -510, -510, -510, -510, -510,
 -510, -511, -511, -511, -511, -511, -511, -511,
 -511, -511, -511, -511, -511, -511, -511, -510,
 -510, -510, -510, -510, -509, -509, -509, -509,
 -508, -508, -508, -507, -507, -506, -506, -506,
 -505, -505, -504, -504, -503, -502, -502, -501,
 -501, -500, -499, -499, -498, -497, -497, -496,
 -495, -494, -493, -493, -492, -491, -490, -489,
 -488, -487, -486, -485, -484, -483, -482, -481,
 -480, -479, -478, -477, -476, -475, -474, -472,
 -471, -470, -469, -467, -466, -465, -464, -462,
 -461, -459, -458, -457, -455, -454, -452, -451,
 -449, -448, -446, -445, -443, -442, -440, -439,
 -437, -435, -434, -432, -430, -429, -427, -425,
 -423, -422, -420, -418, -416, -414, -413, -411,
 -409, -407, -405, -403, -401, -399, -397, -395,
 -393, -391, -389, -387, -385, -383, -381, -379,
 -377, -375, -373, -370, -368, -366, -364, -362,
 -359, -357, -355, -353, -350, -348, -346, -343,
 -341, -339, -336, -334, -332, -329, -327, -324,
 -322, -320, -317, -315, -312, -310, -307, -305,
 -302, -300, -297, -294, -292, -289, -287, -284,
 -282, -279, -276, -274, -271, -268, -266, -263,
 -260, -258, -255, -252, -249, -247, -244, -241,
 -238, -236, -233, -230, -227, -224, -222, -219,
 -216, -213, -210, -207, -204, -202, -199, -196,
 -193, -190, -187, -184, -181, -178, -175, -172,
 -169, -166, -163, -160, -157, -154, -151, -148,
 -145, -142, -139, -136, -133, -130, -127, -124,
 -121, -118, -115, -112, -109, -106, -103, -100,
  -97,  -94,  -91,  -87,  -84,  -81,  -78,  -75,
  -72,  -69,  -66,  -63,  -59,  -56,  -53,  -50,
  -47,  -44,  -41,  -38,  -34,  -31,  -28,  -25,
  -22,  -19,  -16,  -13,   -9,   -6,   -3,    0,
};

long lut_sin (long angle)
{
  if (angle < 0)
    angle = -angle + ANGLE_UNIT_HALF;

  angle %= ANGLE_UNIT;

  return (long)Sin_Table [angle * 4];
}

static int Frame_Edge_Angle [FRAME_SIZE + 1] =
{
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   1,   1,   1,   1,
   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
   1,   2,   2,   2,   2,   2,   2,   2,   2,   2,
   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,
   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
   3,   4,   4,   4,   4,   4,   4,   4,   4,   4,
   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
   4,   4,   4,   4,   4,   4,   4,   5,   5,   5,
   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
   5,   5,   6,   6,   6,   6,   6,   6,   6,   6,
   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,
   6,   6,   6,   6,   6,   6,   6,   6,   7,   7,
   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
   7,   7,   7,   7,   8,   8,   8,   8,   8,   8,
   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,
   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
   9,   9,   9,   9,   9,   9,   9,  10,  10,  10,
  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
  10,  10,  10,  10,  11,  11,  11,  11,  11,  11,
  11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
  11,  11,  11,  11,  11,  11,  11,  11,  11,  11,
  11,  12,  12,  12,  12,  12,  12,  12,  12,  12,
  12,  12,  12,  12,  12,  12,  12,  12,  12,  12,
  12,  12,  12,  12,  12,  12,  12,  12,  12,  13,
  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
  13,  13,  13,  13,  13,  13,  13,  13,  13,  13,
  13,  13,  13,  13,  13,  13,  13,  14,  14,  14,
  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
  14,  14,  14,  14,  14,  14,  15,  15,  15,  15,
  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,
  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,
  15,  15,  15,  15,  15,  16,  16,  16,  16,  16,
  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,
  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,
  16,  16,  16,  16,  17,  17,  17,  17,  17,  17,
  17,  17,  17,  17,  17,  17,  17,  17,  17,  17,
  17,  17,  17,  17,  17,  17,  17,  17,  17,  17,
  17,  17,  17,  17,  17,  18,  18,  18,  18,  18,
  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
  18,  18,  18,  18,  18,  18,  18,  18,  18,  18,
  18,  18,  18,  18,  18,  18,  19,  19,  19,  19,
  19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
  19,  19,  19,  19,  19,  19,  19,  19,  19,  19,
  19,  19,  19,  19,  19,  19,  19,  19,  20,  20,
  20,  20,  20,  20,  20,  20,  20,  20,  20,  20,
  20,  20,  20,  20,  20,  20,  20,  20,  20,  20,
  20,  20,  20,  20,  20,  20,  20,  20,  20,  20,
  20,  21,  21,  21,  21,  21,  21,  21,  21,  21,
  21,  21,  21,  21,  21,  21,  21,  21,  21,  21,
  21,  21,  21,  21,  21,  21,  21,  21,  21,  21,
  21,  21,  21,  21,  22,  22,  22,  22,  22,  22,
  22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
  22,  22,  22,  22,  22,  22,  22,  22,  22,  22,
  22,  22,  22,  22,  22,  22,  22,  22,  22,  23,
  23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
  23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
  23,  23,  23,  23,  23,  23,  23,  23,  23,  23,
  23,  23,  23,  23,  23,  24,  24,  24,  24,  24,
  24,  24,  24,  24,  24,  24,  24,  24,  24,  24,
  24,  24,  24,  24,  24,  24,  24,  24,  24,  24,
  24,  24,  24,  24,  24,  24,  24,  24,  24,  24,
  24,  24,  25,  25,  25,  25,  25,  25,  25,  25,
  25,  25,  25,  25,  25,  25,  25,  25,  25,  25,
  25,  25,  25,  25,  25,  25,  25,  25,  25,  25,
  25,  25,  25,  25,  25,  25,  25,  25,  25,  25,
  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,
  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,
  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,
  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,
  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,
  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,
  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,
  27,  27,  27,  27,  27,  27,  27,  27,  27,  27,
  27,  28,  28,  28,  28,  28,  28,  28,  28,  28,
  28,  28,  28,  28,  28,  28,  28,  28,  28,  28,
  28,  28,  28,  28,  28,  28,  28,  28,  28,  28,
  28,  28,  28,  28,  28,  28,  28,  28,  28,  28,
  28,  28,  28,  28,  29,  29,  29,  29,  29,  29,
  29,  29,  29,  29,  29,  29,  29,  29,  29,  29,
  29,  29,  29,  29,  29,  29,  29,  29,  29,  29,
  29,  29,  29,  29,  29,  29,  29,  29,  29,  29,
  29,  29,  29,  29,  29,  29,  29,  29,  29,  30,
  30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
  30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
  30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
  30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
  30,  30,  30,  30,  30,  31,  31,  31,  31,  31,
  31,  31,  31,  31,  31,  31,  31,  31,  31,  31,
  31,  31,  31,  31,  31,  31,  31,  31,  31,  31,
  31,  31,  31,  31,  31,  31,  31,  31,  31,  31,
  31,  31,  31,  31,  31,  31,  31,  31,  31,  31,
  31,  31,  31,  31
}; /* End Frame_Edge_Angle[] */

static int Frame_Edge_Distance [FRAME_SIZE + 1] =
{
  1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
  1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
  1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
  1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
  1024,1024,1024,1024,1024,1024,1025,1025,1025,1025,
  1025,1025,1025,1025,1025,1025,1025,1025,1025,1025,
  1025,1025,1025,1025,1025,1026,1026,1026,1026,1026,
  1026,1026,1026,1026,1026,1026,1026,1026,1026,1027,
  1027,1027,1027,1027,1027,1027,1027,1027,1027,1027,
  1027,1028,1028,1028,1028,1028,1028,1028,1028,1028,
  1028,1028,1029,1029,1029,1029,1029,1029,1029,1029,
  1029,1029,1030,1030,1030,1030,1030,1030,1030,1030,
  1031,1031,1031,1031,1031,1031,1031,1031,1031,1032,
  1032,1032,1032,1032,1032,1032,1032,1033,1033,1033,
  1033,1033,1033,1033,1034,1034,1034,1034,1034,1034,
  1034,1035,1035,1035,1035,1035,1035,1035,1036,1036,
  1036,1036,1036,1036,1037,1037,1037,1037,1037,1037,
  1038,1038,1038,1038,1038,1038,1039,1039,1039,1039,
  1039,1039,1040,1040,1040,1040,1040,1040,1041,1041,
  1041,1041,1041,1042,1042,1042,1042,1042,1042,1043,
  1043,1043,1043,1043,1044,1044,1044,1044,1044,1045,
  1045,1045,1045,1045,1046,1046,1046,1046,1046,1047,
  1047,1047,1047,1048,1048,1048,1048,1048,1049,1049,
  1049,1049,1049,1050,1050,1050,1050,1051,1051,1051,
  1051,1051,1052,1052,1052,1052,1053,1053,1053,1053,
  1054,1054,1054,1054,1055,1055,1055,1055,1056,1056,
  1056,1056,1056,1057,1057,1057,1057,1058,1058,1058,
  1058,1059,1059,1059,1060,1060,1060,1060,1061,1061,
  1061,1061,1062,1062,1062,1062,1063,1063,1063,1064,
  1064,1064,1064,1065,1065,1065,1065,1066,1066,1066,
  1067,1067,1067,1067,1068,1068,1068,1069,1069,1069,
  1069,1070,1070,1070,1071,1071,1071,1071,1072,1072,
  1072,1073,1073,1073,1074,1074,1074,1074,1075,1075,
  1075,1076,1076,1076,1077,1077,1077,1078,1078,1078,
  1078,1079,1079,1079,1080,1080,1080,1081,1081,1081,
  1082,1082,1082,1083,1083,1083,1084,1084,1084,1085,
  1085,1085,1086,1086,1086,1087,1087,1087,1088,1088,
  1088,1089,1089,1089,1090,1090,1090,1091,1091,1091,
  1092,1092,1092,1093,1093,1093,1094,1094,1095,1095,
  1095,1096,1096,1096,1097,1097,1097,1098,1098,1098,
  1099,1099,1100,1100,1100,1101,1101,1101,1102,1102,
  1103,1103,1103,1104,1104,1104,1105,1105,1106,1106,
  1106,1107,1107,1107,1108,1108,1109,1109,1109,1110,
  1110,1111,1111,1111,1112,1112,1112,1113,1113,1114,
  1114,1114,1115,1115,1116,1116,1116,1117,1117,1118,
  1118,1118,1119,1119,1120,1120,1120,1121,1121,1122,
  1122,1122,1123,1123,1124,1124,1125,1125,1125,1126,
  1126,1127,1127,1127,1128,1128,1129,1129,1130,1130,
  1130,1131,1131,1132,1132,1133,1133,1133,1134,1134,
  1135,1135,1136,1136,1136,1137,1137,1138,1138,1139,
  1139,1139,1140,1140,1141,1141,1142,1142,1143,1143,
  1143,1144,1144,1145,1145,1146,1146,1147,1147,1148,
  1148,1148,1149,1149,1150,1150,1151,1151,1152,1152,
  1153,1153,1153,1154,1154,1155,1155,1156,1156,1157,
  1157,1158,1158,1159,1159,1160,1160,1160,1161,1161,
  1162,1162,1163,1163,1164,1164,1165,1165,1166,1166,
  1167,1167,1168,1168,1169,1169,1170,1170,1170,1171,
  1171,1172,1172,1173,1173,1174,1174,1175,1175,1176,
  1176,1177,1177,1178,1178,1179,1179,1180,1180,1181,
  1181,1182,1182,1183,1183,1184,1184,1185,1185,1186,
  1186,1187,1187,1188,1188,1189,1189,1190,1190,1191,
  1191,1192,1192,1193,1193,1194,1195,1195,1196,1196,
  1197,1197,1198,1198,1199,1199,1200,1200,1201,1201,
  1202,1202,1203,1203,1204,1204,1205,1205,1206,1207,
  1207,1208,1208,1209,1209,1210,1210,1211,1211,1212,
  1212,1213,1213,1214,1215,1215,1216,1216,1217,1217,
  1218,1218,1219,1219,1220,1220,1221,1222,1222,1223,
  1223,1224,1224,1225,1225,1226,1227,1227,1228,1228,
  1229,1229,1230,1230,1231,1231,1232,1233,1233,1234,
  1234,1235,1235,1236,1237,1237,1238,1238,1239,1239,
  1240,1240,1241,1242,1242,1243,1243,1244,1244,1245,
  1246,1246,1247,1247,1248,1248,1249,1250,1250,1251,
  1251,1252,1252,1253,1254,1254,1255,1255,1256,1256,
  1257,1258,1258,1259,1259,1260,1261,1261,1262,1262,
  1263,1263,1264,1265,1265,1266,1266,1267,1268,1268,
  1269,1269,1270,1271,1271,1272,1272,1273,1274,1274,
  1275,1275,1276,1277,1277,1278,1278,1279,1280,1280,
  1281,1281,1282,1283,1283,1284,1284,1285,1286,1286,
  1287,1287,1288,1289,1289,1290,1290,1291,1292,1292,
  1293,1293,1294,1295,1295,1296,1296,1297,1298,1298,
  1299,1300,1300,1301,1301,1302,1303,1303,1304,1305,
  1305,1306,1306,1307,1308,1308,1309,1309,1310,1311,
  1311,1312,1313,1313,1314,1314,1315,1316,1316,1317,
  1318,1318,1319,1320,1320,1321,1321,1322,1323,1323,
  1324,1325,1325,1326,1326,1327,1328,1328,1329,1330,
  1330,1331,1332,1332,1333,1334,1334,1335,1335,1336,
  1337,1337,1338,1339,1339,1340,1341,1341,1342,1343,
  1343,1344,1344,1345,1346,1346,1347,1348,1348,1349,
  1350,1350,1351,1352,1352,1353,1354,1354,1355,1356,
  1356,1357,1358,1358,1359,1360,1360,1361,1361,1362,
  1363,1363,1364,1365,1365,1366,1367,1367,1368,1369,
  1369,1370,1371,1371,1372,1373,1373,1374,1375,1375,
  1376,1377,1377,1378,1379,1379,1380,1381,1381,1382,
  1383,1383,1384,1385,1385,1386,1387,1388,1388,1389,
  1390,1390,1391,1392,1392,1393,1394,1394,1395,1396,
  1396,1397,1398,1398,1399,1400,1400,1401,1402,1402,
  1403,1404,1404,1405,1406,1407,1407,1408,1409,1409,
  1410,1411,1411,1412,1413,1413,1414,1415,1416,1416,
  1417,1418,1418,1419,1420,1420,1421,1422,1422,1423,
  1424,1425,1425,1426,1427,1427,1428,1429,1429,1430,
  1431,1431,1432,1433,1434,1434,1435,1436,1436,1437,
  1438,1438,1439,1440,1441,1441,1442,1443,1443,1444,
  1445,1446,1446,1447
}; /* End Frame_Edge_Distance[] */

long lut_angle(long x, long y)
{
  int quadrant;
  long angle;
  long swap;

  /* Get and preserve quadrant info before we convert everything into
     quadrant one for processing.
   */
  quadrant = 1;
  if ((x * y) > 0)
  {
    if (x < 0)
    {
      quadrant = 3;
      x = -x;
      y = -y;
    }
  }
  else
  {
    if (x < 0)
    {
      quadrant = 2;
      swap = -x;
      x = y;
      y = swap;
    }
    if (y < 0)
    {
      quadrant = 4;
      swap = -y;
      y = x;
      x = swap;
    }
  }

  if (x < y)
  {
    if (y)
      angle = ANGLE_UNIT_QUART - Frame_Edge_Angle[FRAME_SIZE * x / y] - 1;
    else
      angle = ANGLE_UNIT_QUART;
  }
  else
  {
    if (x)
      angle = Frame_Edge_Angle[FRAME_SIZE * y / x];
    else
      angle = 0;
  }

  /* Now put the angle back in the proper quadrant */
  switch (quadrant)
  {
  case 1:
    return angle;

  case 2:
    return angle + ANGLE_UNIT_QUART;

  case 3:
    return angle + ANGLE_UNIT_HALF;

  case 4:
    return angle + ANGLE_UNIT_THREE_QUARTERS;
  }

  return -1;
} /* End lut_angle() */

/* Finds the distance between two points; the (0,0) and (x,y)
   using a look up table. This is the new and improved version. Notice
   how is gleams.

 Find intersection with the edge of the virtual "frame" or virtual screen.
 Use look-up-table to determine the the distance of that point from the
 origin. Use similar triangles to scale that distance down to the actual
 distance of x,y from the origin. I.e. the ratio of the frame intersection
 distance to the size of the frame EQUALS the ratio of the x,y distance to
 y. Multiply by y to get x,y distance.
                                                   _________. intersection
                                                  |        /|
     FrameIntrsctDist        Dist(x,y)            |  x,y./  |
    ------------------  ==  -----------           |    /|   |
       FRAME_SIZE               y                 |  /  |   |
                                                  |/____|___|

 The usual formula from high school is (sqrt(x^2 + y^2)). This is slower.
 Requires doubles. And requires linking math library.

 frame_intersect = FRAME_SIZE * x / y;         find intersection
 frame_dist = Edge_Distance [frame_intersect]; find intersect distance in table
 return frame_dist * y / FRAME_SIZE;  scale using ratios to get x,y distance
*/

long lut_dist(long x, long y)
{
  x = ABS(x);
  y = ABS(y); /* Keep it in the first quadrant. */

  if (y == 0)
    return x; /* Simple cases. Also avoid division by zero. */
  if (x == 0)
    return y;

  /* Is the Intersection with top or with the side of the Frame? */
  if (x < y)
    return Frame_Edge_Distance[FRAME_SIZE * x / y] * y / FRAME_SIZE;
  else
    return Frame_Edge_Distance[FRAME_SIZE * y / x] * x / FRAME_SIZE;
} /* End lut_dist() */

/* Many tables are hashed. Some tables are split into two parts to
 * save space. The works because some functions vary greatly at one
 * part of the curve, but they remain roughly linear at another part.
 * rather than have one large highly accurate table I used a small
 * highly accurate table for areas where the slope of a graph is steep
 * and I used a small low accuracy table for areas where the slope of
 * a graph is small.
 */
/* These coorespond to half of 320 and 200 plus one for fencepost
 * correction. The MCGA screen is 320x200 with 256 colors, so
 * many of these functions are optimised strictly for that screen size.
 * lut_angle() and lut_dist() are two that would require extensive
 * rewriting to make work on a non 320x200 screen
 */
#define HMAX 161
#define VMAX 101
