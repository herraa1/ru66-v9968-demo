// ====================================================================
//	DEVCON Demonstration
// --------------------------------------------------------------------
//	Programmed by t.hara
//  New features and porting to MSXgl by Albert Herranz
// ====================================================================

#include "msx_vdp.h"
#include "bios.h"
#include "debug.h"
#include "dos.h"
#include "string.h"

typedef struct {
	signed short	y;
	unsigned char	mgy;
	unsigned char	mode;
	signed short	x;
	unsigned char	mgx;
	unsigned char	pattern;
} ATTRIBUTE_T;

typedef void (*CALLBACK_T)( void );

static CALLBACK_T p_state;

static unsigned char rgb[48];

static unsigned char default_rgb[] = {
    0,  0,  0,  // Color 0: Transparente / Negro
    0,  0,  0,  // Color 1: Negro
    4,  26, 4,  // Color 2: Verde medio
    13, 31, 13, // Color 3: Verde claro
    8,  8,  31, // Color 4: Azul oscuro
    13, 13, 31, // Color 5: Azul claro
    31, 8,  8,  // Color 6: Rojo oscuro
    13, 26, 31, // Color 7: Celeste / Cyan
    31, 13, 13, // Color 8: Rojo claro
    31, 31, 13, // Color 9: Rojo vivo
    26, 26, 4,  // Color 10: Amarillo oscuro
    26, 26, 17, // Color 11: Amarillo claro
    4,  17, 4,  // Color 12: Verde oscuro
    26, 8,  22, // Color 13: Magenta
    22, 22, 22, // Color 14: Gris
    31, 31, 31  // Color 15: Blanco
};

static unsigned char sprite_rgb[] = {
	//	Palette set #1
	 0,  0,  0,			//	 0 透明 // transparent
	 0,  0,  0,			//	 1 黒 // black
	18,  8,  0,			//	 2 オレンジ(暗) // dark orange
	28, 12,  0,			//	 3 オレンジ(明) // bright orange
	22, 22, 22,			//	 4 灰(明) // bright gray
	18, 18, 18,			//	 5 灰(暗) // dark gray
	31, 20,  0,			//	 6 オレンジ(明明) // very bright orange
	 0,  0, 24,			//	 7 青(明) // bright blue
	26, 26, 26,			//	 8 灰(明明) // very bright gray
	10, 10, 10,			//	 9 灰(暗暗) // very dark gray
	 0,  0, 12,			//	10 青(暗) // dark blue
	 6,  6,  6,			//	11 灰(暗暗暗) // extremely dark gray
	 0,  0, 10,			//	12
	20,  0, 20,			//	13
	30, 30, 30,			//	14
	31, 31, 31,			//	15 白 // white
	//	Palette set #2
	 0,  0,  0,			//	 0 透明 // transparent
	 0,  0,  0,			//	 1 黒 // black
	 0,  4,  8,			//	 2 ダークブルー(暗) // dark dark blue
	 0,  6, 18,			//	 3 ダークブルー(明) // bright dark blue
	22, 22, 22,			//	 4 灰(明) // bright gray
	18, 18, 18,			//	 5 灰(暗) // dark gray
	 0, 10, 28,			//	 6 ダークブルー(明明) // very bright dark blue
	 0,  0, 24,			//	 7 青(明) // bright blue
	26, 26, 26,			//	 8 灰(明明) // very bright gray
	10, 10, 10,			//	 9 灰(暗暗) // very dark gray
	 0,  0, 12,			//	10 青(暗) // dark blue
	 6,  6,  6,			//	11 灰(暗暗暗) // extremely dark gray
	 0,  0, 10,			//	12
	20,  0, 20,			//	13
	30, 30, 30,			//	14
	31, 31, 31,			//	15 白 // white
	//	Palette set #3
	 0,  0,  0,			//	 0 
	 0,  0,  0,			//	 1 
	 1,  1,  3,			//	 2 
	 2,  2,  6,			//	 3 
	 3,  3,  9,			//	 4 
	 4,  4, 12,			//	 5 
	 5,  5, 15,			//	 6 
	 6,  6, 18,			//	 7 
	 7,  7, 21,			//	 8 
	 8,  8, 24,			//	 9 
	 9,  9, 27,			//	10 
	10, 10, 30,			//	11 
	11, 11, 31,			//	12 
	12, 12, 31,			//	13 
	13, 13, 31,			//	14 
	14, 14, 31,			//	15 
};

static ATTRIBUTE_T rabbit1[] = {
	{	//	左端 // left end
		(signed short)(31 | 0xC000),	//	Y (signed, 2bytes)
		128,			//	MGY
		1,				//	Palette Set#1
		16,				//	X (signed, 2bytes)
		16,				//	MGX
		0,				//	Pattern#0
	},
	{	//	中左 // center-left
		(signed short)(31 | 0xC000),	//	Y (signed, 2bytes)
		128,			//	MGY
		1,				//	Palette Set#1
		32,				//	X (signed, 2bytes)
		16,				//	MGX
		1,				//	Pattern#1
	},
	{	//	中右 // center-right
		(signed short)(31 | 0xC000),	//	Y (signed, 2bytes)
		128,			//	MGY
		1,				//	Palette Set#1
		48,				//	X (signed, 2bytes)
		16,				//	MGX
		2,				//	Pattern#2
	},
	{	//	右端 // right end
		(signed short)(31 | 0xC000),	//	Y (signed, 2bytes)
		128,			//	MGY
		1,				//	Palette Set#1
		64,				//	X (signed, 2bytes)
		16,				//	MGX
		3,				//	Pattern#3
	},
};

static ATTRIBUTE_T rabbit2[] = {
	{	//	左端 // left end
		(signed short)(31 | 0xC000),	//	Y (signed, 2bytes)
		128,			//	MGY
		2 | 16,			//	Palette Set#2
		128,			//	X (signed, 2bytes)
		16,				//	MGX
		3,				//	Pattern#0
	},
	{	//	中左 // center-left
		(signed short)(31 | 0xC000),	//	Y (signed, 2bytes)
		128,			//	MGY
		2 | 16,			//	Palette Set#2
		144,			//	X (signed, 2bytes)
		16,				//	MGX
		2,				//	Pattern#1
	},
	{	//	中右 // center-right
		(signed short)(31 | 0xC000),	//	Y (signed, 2bytes)
		128,			//	MGY
		2 | 16,			//	Palette Set#2
		160,			//	X (signed, 2bytes)
		16,				//	MGX
		1,				//	Pattern#2
	},
	{	//	右端 // right end
		(signed short)(31 | 0xC000),	//	Y (signed, 2bytes)
		128,			//	MGY
		2 | 16,			//	Palette Set#2
		176,			//	X (signed, 2bytes)
		16,				//	MGX
		0,				//	Pattern#3
	},
};

static ATTRIBUTE_T shadow[] = {
	{	//	オレンジうさぎの陰 // shadow of orange rabbit
		153,				//	Y (signed, 2bytes)
		12,				//	MGY
		(2 << 6) | 1,	//	Palette Set#1, TP=50%
		16,				//	X (signed, 2bytes)
		64,				//	MGX
		129,			//	Pattern#0
	},
	{	//	ブルーうさぎの陰 // shadow of blue rabbit
		153,			//	Y (signed, 2bytes)
		12,				//	MGY
		(2 << 6) | 1,	//	Palette Set#1, TP=50%
		32,				//	X (signed, 2bytes)
		64,				//	MGX
		129,			//	Pattern#1
	},
};

static ATTRIBUTE_T shadow_mag[] = {
	{	//	オレンジうさぎの陰 // shadow of orange rabbit
		147,				//	Y (signed, 2bytes)
		20,				//	MGY
		(2 << 6) | 1,	//	Palette Set#1, TP=50%
		16,				//	X (signed, 2bytes)
		128,			//	MGX
		129,			//	Pattern#0
	},
	{	//	ブルーうさぎの陰 // shadow of blue rabbit
		147,			//	Y (signed, 2bytes)
		20,				//	MGY
		(2 << 6) | 1,	//	Palette Set#1, TP=50%
		32,				//	X (signed, 2bytes)
		128,			//	MGX
		129,			//	Pattern#1
	},
};

static signed short rotate[] = {
	0, 1024, 256, 0,
	2, 1020, 255, 6,
	5, 1017, 255, 12,
	8, 1014, 255, 18,
	10, 1012, 254, 25,
	13, 1009, 254, 31,
	16, 1006, 253, 37,
	19, 1003, 252, 43,
	23, 1001, 251, 49,
	26, 998, 249, 56,
	29, 996, 248, 62,
	32, 993, 246, 68,
	36, 991, 244, 74,
	39, 989, 243, 80,
	42, 987, 241, 86,
	46, 985, 238, 92,
	50, 983, 236, 97,
	53, 981, 234, 103,
	57, 979, 231, 109,
	61, 977, 228, 115,
	64, 976, 225, 120,
	68, 974, 222, 126,
	72, 973, 219, 131,
	76, 972, 216, 136,
	80, 970, 212, 142,
	84, 969, 209, 147,
	87, 968, 205, 152,
	91, 967, 201, 157,
	95, 967, 197, 162,
	99, 966, 193, 167,
	103, 965, 189, 171,
	107, 965, 185, 176,
	111, 964, 181, 181,
	115, 964, 176, 185,
	120, 964, 171, 189,
	124, 964, 167, 193,
	128, 964, 162, 197,
	132, 964, 157, 201,
	136, 964, 152, 205,
	140, 964, 147, 209,
	144, 964, 142, 212,
	148, 965, 136, 216,
	152, 965, 131, 219,
	156, 966, 126, 222,
	160, 967, 120, 225,
	164, 968, 115, 228,
	168, 969, 109, 231,
	172, 970, 103, 234,
	176, 971, 97, 236,
	180, 972, 92, 238,
	183, 973, 86, 241,
	187, 975, 80, 243,
	191, 976, 74, 244,
	195, 978, 68, 246,
	198, 980, 62, 248,
	202, 981, 56, 249,
	206, 983, 49, 251,
	209, 985, 43, 252,
	213, 987, 37, 253,
	216, 990, 31, 254,
	219, 992, 25, 254,
	223, 994, 18, 255,
	226, 996, 12, 255,
	229, 999, 6, 255,
	233, 1002, 0, 256,
	236, 1004, -7, 255,
	239, 1007, -13, 255,
	242, 1010, -19, 255,
	244, 1012, -26, 254,
	247, 1015, -32, 254,
	250, 1018, -38, 253,
	253, 1021, -44, 252,
	255, 1025, -50, 251,
	258, 1028, -57, 249,
	260, 1031, -63, 248,
	263, 1034, -69, 246,
	265, 1038, -75, 244,
	267, 1041, -81, 243,
	269, 1044, -87, 241,
	271, 1048, -93, 238,
	273, 1052, -98, 236,
	275, 1055, -104, 234,
	277, 1059, -110, 231,
	279, 1063, -116, 228,
	280, 1066, -121, 225,
	282, 1070, -127, 222,
	283, 1074, -132, 219,
	284, 1078, -137, 216,
	286, 1082, -143, 212,
	287, 1086, -148, 209,
	288, 1089, -153, 205,
	289, 1093, -158, 201,
	289, 1097, -163, 197,
	290, 1101, -168, 193,
	291, 1105, -172, 189,
	291, 1109, -177, 185,
	292, 1113, -182, 181,
	292, 1117, -186, 176,
	292, 1122, -190, 171,
	292, 1126, -194, 167,
	292, 1130, -198, 162,
	292, 1134, -202, 157,
	292, 1138, -206, 152,
	292, 1142, -210, 147,
	292, 1146, -213, 142,
	291, 1150, -217, 136,
	291, 1154, -220, 131,
	290, 1158, -223, 126,
	289, 1162, -226, 120,
	288, 1166, -229, 115,
	287, 1170, -232, 109,
	286, 1174, -235, 103,
	285, 1178, -237, 97,
	284, 1182, -239, 92,
	283, 1185, -242, 86,
	281, 1189, -244, 80,
	280, 1193, -245, 74,
	278, 1197, -247, 68,
	276, 1200, -249, 62,
	275, 1204, -250, 56,
	273, 1208, -252, 49,
	271, 1211, -253, 43,
	269, 1215, -254, 37,
	266, 1218, -255, 31,
	264, 1221, -255, 25,
	262, 1225, -256, 18,
	260, 1228, -256, 12,
	257, 1231, -256, 6,
	255, 1235, -256, 0,
	252, 1238, -256, -7,
	249, 1241, -256, -13,
	246, 1244, -256, -19,
	244, 1246, -255, -26,
	241, 1249, -255, -32,
	238, 1252, -254, -38,
	235, 1255, -253, -44,
	231, 1257, -252, -50,
	228, 1260, -250, -57,
	225, 1262, -249, -63,
	222, 1265, -247, -69,
	218, 1267, -245, -75,
	215, 1269, -244, -81,
	212, 1271, -242, -87,
	208, 1273, -239, -93,
	204, 1275, -237, -98,
	201, 1277, -235, -104,
	197, 1279, -232, -110,
	193, 1281, -229, -116,
	190, 1282, -226, -121,
	186, 1284, -223, -127,
	182, 1285, -220, -132,
	178, 1286, -217, -137,
	174, 1288, -213, -143,
	170, 1289, -210, -148,
	167, 1290, -206, -153,
	163, 1291, -202, -158,
	159, 1291, -198, -163,
	155, 1292, -194, -168,
	151, 1293, -190, -172,
	147, 1293, -186, -177,
	143, 1294, -182, -182,
	139, 1294, -177, -186,
	134, 1294, -172, -190,
	130, 1294, -168, -194,
	126, 1294, -163, -198,
	122, 1294, -158, -202,
	118, 1294, -153, -206,
	114, 1294, -148, -210,
	110, 1294, -143, -213,
	106, 1293, -137, -217,
	102, 1293, -132, -220,
	98, 1292, -127, -223,
	94, 1291, -121, -226,
	90, 1290, -116, -229,
	86, 1289, -110, -232,
	82, 1288, -104, -235,
	78, 1287, -98, -237,
	74, 1286, -93, -239,
	71, 1285, -87, -242,
	67, 1283, -81, -244,
	63, 1282, -75, -245,
	59, 1280, -69, -247,
	56, 1278, -63, -249,
	52, 1277, -57, -250,
	48, 1275, -50, -252,
	45, 1273, -44, -253,
	41, 1271, -38, -254,
	38, 1268, -32, -255,
	35, 1266, -26, -255,
	31, 1264, -19, -256,
	28, 1262, -13, -256,
	25, 1259, -7, -256,
	22, 1257, -1, -256,
	18, 1254, 6, -256,
	15, 1251, 12, -256,
	12, 1248, 18, -256,
	10, 1246, 25, -255,
	7, 1243, 31, -255,
	4, 1240, 37, -254,
	1, 1237, 43, -253,
	-1, 1233, 49, -252,
	-4, 1230, 56, -250,
	-6, 1227, 62, -249,
	-9, 1224, 68, -247,
	-11, 1220, 74, -245,
	-13, 1217, 80, -244,
	-15, 1214, 86, -242,
	-17, 1210, 92, -239,
	-19, 1206, 97, -237,
	-21, 1203, 103, -235,
	-23, 1199, 109, -232,
	-25, 1195, 115, -229,
	-26, 1192, 120, -226,
	-28, 1188, 126, -223,
	-29, 1184, 131, -220,
	-30, 1180, 136, -217,
	-32, 1176, 142, -213,
	-33, 1172, 147, -210,
	-34, 1169, 152, -206,
	-35, 1165, 157, -202,
	-35, 1161, 162, -198,
	-36, 1157, 167, -194,
	-37, 1153, 171, -190,
	-37, 1149, 176, -186,
	-38, 1145, 181, -182,
	-38, 1141, 185, -177,
	-38, 1136, 189, -172,
	-38, 1132, 193, -168,
	-38, 1128, 197, -163,
	-38, 1124, 201, -158,
	-38, 1120, 205, -153,
	-38, 1116, 209, -148,
	-38, 1112, 212, -143,
	-37, 1108, 216, -137,
	-37, 1104, 219, -132,
	-36, 1100, 222, -127,
	-35, 1096, 225, -121,
	-34, 1092, 228, -116,
	-33, 1088, 231, -110,
	-32, 1084, 234, -104,
	-31, 1080, 236, -98,
	-30, 1076, 238, -93,
	-29, 1073, 241, -87,
	-27, 1069, 243, -81,
	-26, 1065, 244, -75,
	-24, 1061, 246, -69,
	-22, 1058, 248, -63,
	-21, 1054, 249, -57,
	-19, 1050, 251, -50,
	-17, 1047, 252, -44,
	-15, 1043, 253, -38,
	-12, 1040, 254, -32,
	-10, 1037, 254, -26,
	-8, 1033, 255, -19,
	-6, 1030, 255, -13,
	-3, 1027, 255, -7,
};

static int x1 = -64;
static int pattern1 = 0;

static int x2 = 256;
static int pattern2 = 0;

static int run_demo = 0;
static int message_state = 32;
static int mag = 0;
static int scroll_x = 0, scroll_y = 0;
static int theta = 0;


static u16 lap_started;

void state_window_animation( void );
void state_dot_by_dot( void );
void state_size_select( void );
void state_transparent( void );
void state_magnify( void );
void state_maximum_puts( void );
void state_all_screen( void );
void state_color( void );
void state_reverse( void );
void state_patterns( void );
void state_easy( void );
void state_highspeed_command( void );
void state_font_command( void );
void state_end( void );
void state_fighter( void );
void block_copy( int sx, int sy, int nx, int ny, int dx, int dy );

#define DEFAULT_LAP_TICKS 7 * 60

// --------------------------------------------------------------------
u8 lap_elapsed_ticks(u16 ticks)
{
	if ((g_JIFFY - lap_started) > ticks)
		return 1;
	return 0;
}

// --------------------------------------------------------------------
u8 need_transition(void)
{
	int key = get_cursor_key();
	if ( (key & KEY_DOWN_DEMO) || lap_elapsed_ticks(DEFAULT_LAP_TICKS) ) {
		lap_started = g_JIFFY;
		return 1;
	}
	if( key & KEY_SPACE_DEMO ) {
		//	終わる
		run_demo = 0;
	}
	return 0;
}

// --------------------------------------------------------------------
void *my_memcpy(void *dest, const void *src, unsigned char n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;

    while (n--) {
        *d = *s;
        d++;
        s++;
    }

    return dest;
}

char bg_filename[8+3+1];
char pal_filename[8+3+1];
unsigned char bg_idx = 0;
const char *bg_prefix = "bg";
const char *bg_ext = ".SC5";
const char *pal_ext = ".PAL";

// --------------------------------------------------------------------
void build_filename(char *dest, const char *prefix, const char *ext, unsigned char index) {
    while (*prefix) {
        *dest++ = *prefix++;
    }
    *dest++ = (char)('0' + (index / 100));
    *dest++ = (char)('0' + ((index / 10) % 10));
    *dest++ = (char)('0' + (index % 10));
    while (*ext) {
        *dest++ = *ext++;
    }
    *dest = '\0';
}

// --------------------------------------------------------------------
void load_background(void (*cb)(void *), void *data)
{
	build_filename(pal_filename, bg_prefix, pal_ext, bg_idx);
	build_filename(bg_filename, bg_prefix, bg_ext, bg_idx++);
	set_vram_write_address(0, 0x0000);
	if (bload(bg_filename) != 0) {
		bg_idx = 0;
		build_filename(pal_filename, bg_prefix, pal_ext, bg_idx);
		build_filename(bg_filename, bg_prefix, bg_ext, bg_idx++);
		bload(bg_filename);
	}
	if (cb)
		cb(data);
	if (load_palette(rgb, pal_filename) != 0) {
		my_memcpy(rgb, default_rgb, 48);
	}
	//	page0背景を page4へコピー // copy page0 background to page4
	block_copy(0, 0, 256, 256, 0, 1024);
}

// --------------------------------------------------------------------
void fill_rectangle( int sx, int sy, int nx, int ny, unsigned char color ) {
__asm
	push iy
	push hl
	push de
	call _wait_vdp_command
	pop de
	pop hl

	ld		iy, #0 // skip return address
	add		iy, sp

	ld		a, #0x89
	ld		c, a
	ld a, #36
	di
	out		(c), a
	ld a, #(0x80 + 17)
	out		(c), a
	inc		c
	inc		c

	out		(c), l  // first param goes into HL = sx
	out		(c), h

	out		(c), e  // second param goes into DE = sy
	out		(c), d

	                // third+ params go into stack, right to left
	ld l, 4(iy)		//	nx
	ld h, 5(iy)		//	nx
	out		(c), l
	out		(c), h

	ld l, 6(iy)		//	ny
	ld h, 7(iy)		//	ny
	out		(c), l
	out		(c), h

	ld a, 8(iy)		//	color
	out		(c), a
	xor		a
	out		(c), a
	ld a, #0x80
	out		(c), a
	ei
	pop iy
__endasm;
}

// --------------------------------------------------------------------
void rotate_copy( void ) {
__asm
	call _wait_vdp_command

	ld		a, #0x89
	ld		c, a

	ld hl, (_theta)
	add		hl, hl			//	sizeof(signed short) = 2 の 2倍 // 2 times the size of signed short = 2
	add		hl, hl			//	4要素あるので 4倍 // 4 elements, so multiply by 4
	add		hl, hl

	ld de, #_rotate + 7

	add		hl, de
	//	VX, VY
	//	R#17 = 47
	ld a, #47
	di
	out		(c), a
	ld a, #(0x80 + 17)
	out		(c), a
	inc		c
	inc		c
	ld b, (hl)			//	VY High
	dec		hl
	ld a, (hl)			//	VY Low
	dec		hl
	ld d, (hl)			//	VX High
	dec		hl
	ld e, (hl)			//	VX Low
	dec		hl
	out		(c), e
	out		(c), d
	out		(c), a
	out		(c), b
	//	SX, SY
	//	R#17 = 32
	dec		c
	dec		c
	ld a, #32
	out		(c), a
	ld a, #(0x80 + 17)
	out		(c), a
	inc		c
	inc		c
	ld b, (hl)			//	SY High
	dec		hl
	ld a, (hl)			//	SY Low
	dec		hl
	ld d, (hl)			//	SX High
	dec		hl
	ld e, (hl)			//	SX Low
	out		(c), e
	out		(c), d
	out		(c), a
	out		(c), b
	//	DX, DY
	xor		a
	out		(c), a
	out		(c), a
	out		(c), a
	out		(c), a
	//	NX, NY
	ld de, #256			//	NX
	out		(c), e
	out		(c), d
	ld de, #212			//	NY
	out		(c), e
	out		(c), d
	//	COLOR, ARG, CMD
	out		(c), a			//	COLOR
	out		(c), a			//	ARG
	ld a, #0x30			//	CMD: LRMM, IMP
	ei
	out		(c), a
__endasm;
}

// --------------------------------------------------------------------
void draw_line( int x, int y, int nx, int ny, unsigned char color ) {
	wait_vdp_command();
	vdp_write_reg(17, 36);
	VDP_PORTp2 = x & 255 ;
	VDP_PORTp2 = x >> 8 ;
	VDP_PORTp2 = y & 255 ;
	VDP_PORTp2 = y >> 8 ;
	VDP_PORTp2 = nx & 255 ;
	VDP_PORTp2 = nx >> 8 ;
	VDP_PORTp2 = ny & 255 ;
	VDP_PORTp2 = ny >> 8 ;
	VDP_PORTp2 = color ;
	VDP_PORTp2 = 0 ;
	VDP_PORTp2 = 0x70 ;
}

// --------------------------------------------------------------------
void block_copy( int sx, int sy, int nx, int ny, int dx, int dy ) {
	wait_vdp_command();
	vdp_write_reg(17, 32);
	VDP_PORTp2 = sx & 255 ;
	VDP_PORTp2 = sx >> 8 ;
	VDP_PORTp2 = sy & 255 ;
	VDP_PORTp2 = sy >> 8 ;
	VDP_PORTp2 = dx & 255 ;
	VDP_PORTp2 = dx >> 8 ;
	VDP_PORTp2 = dy & 255 ;
	VDP_PORTp2 = dy >> 8 ;
	VDP_PORTp2 = nx & 255 ;
	VDP_PORTp2 = nx >> 8 ;
	VDP_PORTp2 = ny & 255 ;
	VDP_PORTp2 = ny >> 8 ;
	VDP_PORTp2 = 0 ;
	VDP_PORTp2 = 0 ;
	VDP_PORTp2 = 0x90 ;
}

// --------------------------------------------------------------------
void set_initial_palette( void ) {
	unsigned char *p_rgb;
	int i, j, d;

	_di();
	vdp_write_reg( 16, (0 << 4) | 0 );		//	palette set#0, palette#0
	_ei();
	p_rgb = rgb;
	for( i = 0; i < 16 * 3; i++ ) {
		VDP_PORTp1=  0 ;
	}
	set_display_visible( MSX_TRUE );

	for( j = 32; j >= 0; j-- ) {
		_di();
		vdp_write_reg( 16, (0 << 4) | 0 );		//	palette set#0, palette#0
		_ei();
		p_rgb = rgb;
		for( i = 0; i < sizeof(rgb); i++ ) {
			d = (int)*p_rgb - j;
			if( d < 0 ) {
				d = 0;
			}
			VDP_PORTp1 = d ;
			p_rgb++;
		}
		wait_vsync(2);
	}
}

// --------------------------------------------------------------------
void fadeout_palette( void ) {
	unsigned char *p_rgb;
	int i, j, d;

	p_rgb = rgb;
	for( j = 0; j <= 32; j++ ) {
		_di();
		vdp_write_reg( 16, (0 << 4) | 0 );		//	palette set#0, palette#0
		_ei();
		p_rgb = rgb;
		for( i = 0; i < sizeof(rgb); i++ ) {
			d = (int)*p_rgb - j;
			if( d < 0 ) {
				d = 0;
			}
			VDP_PORTp1 = d;
			p_rgb++;
		}
		wait_vsync(2);
	}
	fill_rectangle(0, 0, 256, 256, 0);
}

static const unsigned char _init_palette1[] = {
	0x00, 0x0,
	0x00, 0x0,
	0x11, 0x6,
	0x33, 0x7,
	0x17, 0x1,
	0x27, 0x3,
	0x51, 0x1,
	0x27, 0x6,
	0x71, 0x1,
	0x73, 0x3,
	0x61, 0x6,
	0x64, 0x6,
	0x11, 0x4,
	0x65, 0x2,
	0x55, 0x5,
	0x77, 0x7,
};

// --------------------------------------------------------------------
void set_default_palette( void ) {
	int i;
	vdp_write_reg(16, 0);
	for( i = 0; i < sizeof(_init_palette1); i++ ) {
		VDP_PORTp1 = _init_palette1[i];
	}
}

// --------------------------------------------------------------------
void set_sprite_palette( void ) {
	unsigned char *p_rgb;
	int i;

	_di();
	vdp_write_reg( 16, (1 << 4) | 0 );		//	palette set#1, palette#0
	_ei();
	p_rgb = sprite_rgb;
	for( i = 0; i < sizeof(sprite_rgb); i++ ) {
		VDP_PORTp1 = *p_rgb;
		p_rgb++;
	}
}

// --------------------------------------------------------------------
static unsigned char sprite_fade_count1 = 0;
static unsigned char sprite_fade_count2 = 0;

void set_sprite_fade_palette( void ) {
__asm
	ld a, (_sprite_fade_count1)
	ld		e, a
	ld a, (_sprite_fade_count2)
	ld		d, a

	ld hl, #_sprite_rgb
	//	R#16 = (1 << 4) | 0
	ld		a, #0x89
	ld		c, a
	ld a, #((1 << 4) | 0)
	di
	out		(c), a
	ld a, #(0x80 | 16)
	ei
	out		(c), a

	inc		c
	ld b, #16 * 3
loop1:
	ld a, (hl)
	sub		a, e
	jr		nc, skip1
	xor		a
skip1:
	out		(c), a
	inc		hl
	djnz	loop1

	ld b, #16 * 3
loop2:
	ld a, (hl)
	sub		a, d
	jr		nc, skip2
	xor		a
skip2:
	out		(c), a
	inc		hl
	djnz	loop2

	ld		a, e
	inc		a
	and		#31
	ld		(_sprite_fade_count1), a
	and		#1
	jr		z, skip3

	ld		a, d
	inc		a
	and		#31
	ld		(_sprite_fade_count2), a
skip3:
__endasm;
}

// --------------------------------------------------------------------
void initializer( void ) {
	int i, x, pattern;
	unsigned char c[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 14 };

	DOS_StringOutput("Looking for a V9968 cartridge... $");

	if( !init_vdp() ) {
		DEBUG_LOG("Aborting...");
		DOS_StringOutput("NOT found!\r\n$");
		DOS_Exit0();
	}

	DOS_StringOutput("OK\r\n$");
	DOS_StringOutput("Please, switch to the V9968 display now!\r\n$");

	set_screen5();
	//	周辺色黒 // border color black
	vdp_write_reg(7, 0x00);
	//	左端マスク // left end mask
	vdp_write_reg(25, 0x02);
	//	横スクロール初期値 // initial horizontal scroll value
	vdp_write_reg(27, 7);
	//	背景読み込み // background loading
	load_background(NULL, NULL);
	set_initial_palette();
	//	スプライト画像読み込み // sprite image loading
	set_sprite_palette();
	set_vram_write_address(0, 0x8000);
	bload( "usa.SC5" );
	set_vram_write_address(1, 0x8000);
	bload( "font.bin" );
	_di();
	//	スプライトアトリビュートテーブル 0x10000 // sprite attribute table 0x10000
	vdp_write_reg(5, 0x03);
	vdp_write_reg(11, 0x02);
	//	スプライトパターンジェネレーターテーブル 0x08000 // sprite pattern generator table 0x08000
	vdp_write_reg(6, 0x10);
	//	スプライトアトリビュートテーブルをクリアする // clear sprite attribute table
	set_vram_write_address(1, 0x0000);
	for( i = 0; i < 10 * 8; i+=2 ) {	//	うさぎ2体の分 Plane#0～#9 // for 2 rabbits, Plane#0-#9
		VDP_PORTl1 =  0 ;
		VDP_PORTl1 =  1 ;
	}
	x = 16;
	pattern = 128 + 16;
	for( i = 0; i < 14 * 8; i+=8 ) {	//	メッセージ表示エリア #10～#23 // message display area #10-#23
		VDP_PORTl1 =  170 ;			//	Y
		VDP_PORTl1 =  1 << 6 ;		//	SZ = 16x32
		VDP_PORTl1 =  32 ;			//	MGY
		VDP_PORTl1 =  0 ;			//	PaletteSet#0, TP = 0% (不透明) // PaletteSet#0, TP = 0% (opaque)
		VDP_PORTl1 =  x ;			//	X
		VDP_PORTl1 =  0 ;
		VDP_PORTl1 =  16 ;			//	MGX
		VDP_PORTl1 =  pattern ;
		x += 16;
		pattern++;
	}
	for( i = 0; i < 1 * 8; i+=2 ) {		//	ウィンドウ枠の分 Plane#24 // for window frame, Plane#24
		VDP_PORTl1 =  0 ;
		VDP_PORTl1 =  1 ;
	}
	VDP_PORTl1 =  216 ;						//	Plane#25 以降非表示 // Plane#25 and beyond, not displayed
	VDP_PORTl1 =  0 ;
	//	スプライトパターン更新 // sprite pattern update
	for( i = 0; i < 16; i++ ) {
		fill_rectangle(0, 256 + 128 + i, 16, 1, c[i]);
	}
	fill_rectangle(0, 256 + 128 + 16, 224, 32, 0);
	//	page0背景を page4へコピー // copy page0 background to page4
	// XXX block_copy(0, 0, 256, 256, 0, 1024);
	//	LRMM時のソースウィンドウを設定 // set source window for LRMM
	vdp_write_reg(17, 51);
	VDP_PORTp2 =  0 ;			//	Wsx = 0
	VDP_PORTp2 =  0 ;
	VDP_PORTp2 =  0 ;			//	Wsy = 0, page4
	VDP_PORTp2 =  4 ;
	VDP_PORTp2 =  255 ;			//	Wex = 255
	VDP_PORTp2 =  0 ;
	VDP_PORTp2 =  255 ;			//	Wey = 255, page4
	VDP_PORTp2 =  4 ;
	//	スプライト表示 // sprite display
	vdp_write_reg(8, 0x08);
	_ei();
}

// --------------------------------------------------------------------
void terminator( void ) {
	//	パレットをフェードアウトして画面を消す // fade out palette and clear screen
	fadeout_palette();

	//	V9958互換モードに戻す // return to V9958 compatible mode
	_di();
	vdp_write_reg(9, 0x00);
	vdp_write_reg(10, 0x00);
	vdp_write_reg(11, 0x00);
	vdp_write_reg(20, 0x00);
	vdp_write_reg(21, 0x00);
	vdp_write_reg(23, 0x00);
	vdp_write_reg(25, 0x00);
	vdp_write_reg(26, 0x00);
	vdp_write_reg(27, 0x00);

	//	初期状態のパレットに戻す // return to initial palette state
	set_default_palette();

#if 0
	int i;

	//	レジスタを初期状態に戻す // return registers to initial state
	vdp_write_reg(14, 0);
	for( i = 0; i < 8; i++ ) {
		vdp_write_reg( i, Peek(0xF3DF + i) );
	}
#endif
	_ei();

#if 0
	//	いったん SCREEN 3 に変える // temporarily change to SCREEN 3
	__asm
	ld a, #3
	ld		ix, #0x005f		//	CHGMOD
	ld iy, #(0xFCC1 - 1)
	call	#0x001C			//	CALSLT
    __endasm;
#endif
}

// --------------------------------------------------------------------
void do_otir(void *dst)
{
__asm
	ld		a, #0x89
	dec		a
	ld		c, a
	ld		b,#8*4
	otir
__endasm;
}

// --------------------------------------------------------------------
void put_usagi( ATTRIBUTE_T *p_attribute, ATTRIBUTE_T *p_shadow, int x, int dir, int pattern ) {
	unsigned char *p = (unsigned char*) p_attribute;

	x = x & 0x3FF;
	p_shadow->x = x;
	if( dir ) {
		//	左向き // facing left
		pattern					+= 3;
		p_attribute->mode		= p_attribute->mode | 0x10;
		p_attribute->x			= x;
		p_attribute->pattern	= pattern--;
		p_attribute++;
		p_attribute->mode		= p_attribute->mode | 0x10;
		p_attribute->x			= x + 16;
		p_attribute->pattern	= pattern--;
		p_attribute++;
		p_attribute->mode		= p_attribute->mode | 0x10;
		p_attribute->x			= x + 32;
		p_attribute->pattern	= pattern--;
		p_attribute++;
		p_attribute->mode		= p_attribute->mode | 0x10;
		p_attribute->x			= x + 48;
		p_attribute->pattern	= pattern;
	}
	else {
		//	右向き // facing right
		p_attribute->mode		= p_attribute->mode & 0xEF;
		p_attribute->x			= x;
		p_attribute->pattern	= pattern++;
		p_attribute++;
		p_attribute->mode		= p_attribute->mode & 0xEF;
		p_attribute->x			= x + 16;
		p_attribute->pattern	= pattern++;
		p_attribute++;
		p_attribute->mode		= p_attribute->mode & 0xEF;
		p_attribute->x			= x + 32;
		p_attribute->pattern	= pattern++;
		p_attribute++;
		p_attribute->mode		= p_attribute->mode & 0xEF;
		p_attribute->x			= x + 48;
		p_attribute->pattern	= pattern;
	}

	do_otir(p);
}

// --------------------------------------------------------------------
void put_usagi_mag( ATTRIBUTE_T *p_attribute, ATTRIBUTE_T *p_shadow, int x, int dir, int pattern ) {
	unsigned char *p = (unsigned char*) p_attribute;

	x = x & 0x3FF;
	p_shadow->x = x;
	if( dir ) {
		//	左向き // facing left
		pattern					+= 3;
		p_attribute->y			= (signed short)0xFF9F;
		p_attribute->mgy		= 0;
		p_attribute->mode		= p_attribute->mode | 0x10;
		p_attribute->x			= x;
		p_attribute->mgx		= 32;
		p_attribute->pattern	= pattern--;
		p_attribute++;
		p_attribute->y			= (signed short)0xFF9F;
		p_attribute->mgy		= 0;
		p_attribute->mode		= p_attribute->mode | 0x10;
		p_attribute->x			= x + 32;
		p_attribute->mgx		= 32;
		p_attribute->pattern	= pattern--;
		p_attribute++;
		p_attribute->y			= (signed short)0xFF9F;
		p_attribute->mgy		= 0;
		p_attribute->mode		= p_attribute->mode | 0x10;
		p_attribute->x			= x + 64;
		p_attribute->mgx		= 32;
		p_attribute->pattern	= pattern--;
		p_attribute++;
		p_attribute->y			= (signed short)0xFF9F;
		p_attribute->mgy		= 0;
		p_attribute->mode		= p_attribute->mode | 0x10;
		p_attribute->x			= x + 96;
		p_attribute->mgx		= 32;
		p_attribute->pattern	= pattern;
	}
	else {
		//	右向き // facing right
		p_attribute->y			= (signed short)0xFF9F;
		p_attribute->mgy		= 0;
		p_attribute->mode		= p_attribute->mode & 0xEF;
		p_attribute->x			= x;
		p_attribute->mgx		= 32;
		p_attribute->pattern	= pattern++;
		p_attribute++;
		p_attribute->y			= (signed short)0xFF9F;
		p_attribute->mgy		= 0;
		p_attribute->mode		= p_attribute->mode & 0xEF;
		p_attribute->x			= x + 32;
		p_attribute->mgx		= 32;
		p_attribute->pattern	= pattern++;
		p_attribute++;
		p_attribute->y			= (signed short)0xFF9F;
		p_attribute->mgy		= 0;
		p_attribute->mode		= p_attribute->mode & 0xEF;
		p_attribute->x			= x + 64;
		p_attribute->mgx		= 32;
		p_attribute->pattern	= pattern++;
		p_attribute++;
		p_attribute->y			= (signed short)0xFF9F;
		p_attribute->mgy		= 0;
		p_attribute->mode		= p_attribute->mode & 0xEF;
		p_attribute->x			= x + 96;
		p_attribute->mgx		= 32;
		p_attribute->pattern	= pattern;
	}

	do_otir(p);
}

// --------------------------------------------------------------------
void put_shadow( void ) {
__asm
	ld hl, #_shadow
	ld		a, #0x89
	dec		a
	ld		c, a
	ld b, #8 * 2
	otir
__endasm;
}

// --------------------------------------------------------------------
void put_shadow_mag( void ) {
__asm
	ld hl, #_shadow_mag
	ld		a, #0x89
	dec		a
	ld		c, a
	ld b, #8 * 2
	otir
__endasm;
}

// --------------------------------------------------------------------
void put_message( int n ) {
__asm
	push hl
	xor a
	ld (_message_state),a
	call _wait_vdp_command
	pop hl

	//	HL = n * 896 + 0x8000 (フォントデータは 0x18000 + n * 896 にある) // HL = n * 896 + 0x8000 (font data is at 0x18000 + n * 896)
	//	896 = 0x380 → n * 896 = ((n * 256) >> 1) + (n * 256) + ((n * 256) << 1) // calculation: n * 896 = shift_right(n*256) + (n*256) + shift_left(n*256)
	ld		h, l			//	HL = n * 256
	ld		l, #0
	ld		e, l			//	DE = n * 256
	ld		d, h
	rrc		h				//	HL >>= 1 // right shift HL
	rr		l
	add		hl, de			//	HL = HL + DE = ((n * 256) >> 1) + (n * 256)
	ex		de, hl			//	HL = (n * 256); DE = ((n * 256) >> 1) + (n * 256)
	add		hl, hl			//	HL = ((n * 256) << 1) // left shift HL
	add		hl, de			//	HL = n * 896
	ld a, #0x80
	or		a, h
	ld		h, a			//	HL = n * 896 + 0x8000
	//	R#12 = 0
	ld		a, #0x89
	ld		c, a
	ld de, #0 | ((0x80 + 12) << 8)
	di
	out		(c), e
	out		(c), d
	//	R#17 = 32
	ld de, #32 | ((0x80 + 17) << 8)
	out		(c), e
	out		(c), d
	inc		c
	inc		c
	//	ADDRESS (R#32, R#33, R#34, R#35) = n * 896 + 0x18000
	out		(c), l
	out		(c), h
	ld a, #1
	out		(c), a
	dec		a
	out		(c), a
	//	DX (R#36, R#37) = 0
	out		(c), a
	out		(c), a
	//	DY (R#38, R#39) = 256 + 128 + 16 + 32
	ld de, #256 + 128 + 16 + 32
	out		(c), e
	out		(c), d
	//	NX (R#40, R#41) = 224 >> 3
	ld e, #224 >> 3
	out		(c), e								//	ここに書き込むと暗転する、なぜ！？ // writing here causes blackout, why!?
	out		(c), a
	//	NY (R#42, R#43) = 32
	ld e, #32
	out		(c), e
	out		(c), a
	//	CLR (R#44) = 15
	ld e, #15
	out		(c), e
	//	ARG (R#45) = 0
	out		(c), a
	//	CMD (R#46) = 0x10
	ld e, #0x10				//	run LFMM
	ei
	out		(c), e
__endasm;
}

// --------------------------------------------------------------------
void scroll_message( void ) {
__asm
	call _wait_vdp_command

	//	R#17 = 32
	ld		a, #0x89
	ld		c, a
	ld de, #32 | ((0x80 + 17) << 8)
	di
	out		(c), e
	out		(c), d
	inc		c
	inc		c
	//	SX (R#32, R#33) = 0
	xor		a
	out		(c), a
	out		(c), a
	//	SY (R#34, R#35) = 256 + 128 + 16 + 1
	ld de, #256 + 128 + 16 + 1
	out		(c), e
	out		(c), d
	//	DX (R#36, R#37) = 0
	out		(c), a
	out		(c), a
	//	DY (R#38, R#39) = 256 + 16
	dec		de
	out		(c), e
	out		(c), d
	//	NX (R#40, R#41) = 224
	ld e, #224
	out		(c), e
	out		(c), a
	//	NY (R#42, R#43) = 63
	ld e, #63
	out		(c), e
	out		(c), a
	//	CLR (R#44) = 0
	out		(c), a
	//	ARG (R#45) = 0
	out		(c), a
	//	CMD (R#46) = 0xD0
	ld e, #0xD0				//	run HMMM
	ei
	out		(c), e
__endasm;
}

// --------------------------------------------------------------------
void background_scroll( void ) {
	scroll_x = (scroll_x + 2) & 255;
	scroll_y = (scroll_y + 1) & 255;

	//	背景スクロール // background scroll
	vdp_write_reg(23, scroll_y);
	vdp_write_reg(26, scroll_x >> 3);
	vdp_write_reg( 27, (scroll_x & 7) ^ 7 );
}

// --------------------------------------------------------------------
void background_scroll_finish( void ) {
	if( scroll_x ) scroll_x--;
	if( scroll_y ) scroll_y--;

	//	背景スクロール // background scroll
	vdp_write_reg(23, scroll_y);
	vdp_write_reg(26, scroll_x >> 3);
	vdp_write_reg( 27, (scroll_x & 7) ^ 7 );
}

static unsigned int _seed1 = 19739;
// --------------------------------------------------------------------
int random( void ) {

	_seed1 = (_seed1 ^ 0x8412) + 1917;
	return (int) _seed1;
}

// --------------------------------------------------------------------
void fill_rectangle_sub() {
	int sx, sy, nx, ny;

	sx = random() & 255;
	sy = random() & 255;
	nx = (random() & 255) | 1;
	ny = (random() & 255) | 1;
	if( (sy + ny) >= 256 ) {
		ny = 256 - sy;
	}
	fill_rectangle( sx, sy, nx, ny, random() & 15 );
}

// --------------------------------------------------------------------
void background_fill( void ) {
	fill_rectangle_sub();
	fill_rectangle_sub();
	fill_rectangle_sub();
}

static int _result1 = 0;

// --------------------------------------------------------------------
int message_animation( void ) {

	if( message_state < 32 ) {
		scroll_message();
		message_state++;
		_result1 = 0;
	}
	else if( message_state == 32 ) {
		if( get_cursor_key() == 0 ) {
			_result1 = 1;
		}
	}
	return _result1;
}

static int _countf1 = 8;
static int _countf2 = 12;

// --------------------------------------------------------------------
void puts_fighter( void ) {
	//	スプライト（うさぎファイター） // sprite (rabbit fighter)
	if( mag ) {
		set_vram_write_address(1, 0x0000 + 0 * 8);
		put_usagi_mag( rabbit1, &shadow_mag[0], x1, 0, pattern1 );
		set_vram_write_address(1, 0x0000 + 4 * 8);
		put_usagi_mag( rabbit2, &shadow_mag[1], x2, 1, pattern2 );
		set_vram_write_address(1, 0x0000 + 8 * 8);
		put_shadow_mag();
	}
	else {
		set_vram_write_address(1, 0x0000 + 0 * 8);
		put_usagi( rabbit1, &shadow[0], x1, 0, pattern1 );
		set_vram_write_address(1, 0x0000 + 4 * 8);
		put_usagi( rabbit2, &shadow[1], x2, 1, pattern2 );
		set_vram_write_address(1, 0x0000 + 8 * 8);
		put_shadow();
	}

	x1++;
	if( x1 == 256 ) {
		x1 = -64;
	}
	x2--;
	if( x2 == -64 ) {
		x2 = 255;
	}
	if( (--_countf1) == 0 ) {
		pattern1 = 4 - pattern1;
		_countf1 = 8;
	}
	if( (--_countf2) == 0 ) {
		pattern2 = 4 - pattern2;
		_countf2 = 12;
	}
}

static ATTRIBUTE_T _window1 = {
	-40 & 0x03FF,	//	Y (signed, 2bytes)
	40,				//	MGY
	3 | (1 << 6),	//	Palette Set#3
	16,				//	X (signed, 2bytes)
	16,				//	MGX
	128,			//	Pattern#0
};

static int _y1 = -40;

// --------------------------------------------------------------------
void state_window_animation( void ) {
	int i;
	unsigned char *p = (unsigned char *) _window1;

	wait_vscan();
	//	ウィンドウ // window
	if( _y1 != 166 ) {
		_y1++;
		_window1.y = _y1 & 0x03FF;
	}
	else if( _window1.mgx != 224 ) {
		_window1.mgx += 2;
	}
	else {
		//	次のステートへ // to next state
		p_state = state_dot_by_dot;
		put_message(0);
		lap_started = g_JIFFY;
		theta = 0;
	}
	set_vram_write_address(1, 0x0000 + 24 * 8);
	for( i = 0; i < 8; i++ ) {
		VDP_PORTl1 = *p ;
		p++;
	}
	background_scroll();
	message_animation();
}

// --------------------------------------------------------------------
void state_dot_by_dot( void ) {
	wait_vscan();
	background_scroll();
	puts_fighter();

	if( !message_animation() ) {
		return;
	}

	if( need_transition() ) {
		//	次のステートへ // to next state
		p_state = state_size_select;
		put_message(1);
	}
}

static int _count1 = 10;
static int _size1 = 0;
static int _mgy1 = 16;

// --------------------------------------------------------------------
void state_size_select( void ) {
	wait_vscan();
	background_scroll();
	puts_fighter();

	_count1--;
	if( _count1 == 0 ) {
		_count1 = 10;
		rabbit1[0].y = 31 | _size1;
		rabbit1[0].mgy = _mgy1;
		_size1 = _size1 + 0x4000;
		_mgy1 = (_mgy1 == 128) ? 16: (_mgy1 << 1);
		rabbit1[1].y = 31 | _size1;
		rabbit1[1].mgy = _mgy1;
		_size1 = _size1 + 0x4000;
		_mgy1 = (_mgy1 == 128) ? 16: (_mgy1 << 1);
		rabbit1[2].y = 31 | _size1;
		rabbit1[2].mgy = _mgy1;
		_size1 = _size1 + 0x4000;
		_mgy1 = (_mgy1 == 128) ? 16: (_mgy1 << 1);
		rabbit1[3].y = 31 | _size1;
		rabbit1[3].mgy = _mgy1;
	}

	if( !message_animation() ) {
		return;
	}
	
	if( need_transition() ) {
		rabbit1[0].y = (signed short)(31 | 0xC000);
		rabbit1[0].mgy = 128;
		rabbit1[1].y = (signed short)(31 | 0xC000);
		rabbit1[1].mgy = 128;
		rabbit1[2].y = (signed short)(31 | 0xC000);
		rabbit1[2].mgy = 128;
		rabbit1[3].y = (signed short)(31 | 0xC000);
		rabbit1[3].mgy = 128;
		//	次のステートへ
		p_state = state_transparent;
		put_message(2);
	}
}

static int _count2 = 10;
static int _mode2 = 1;

// --------------------------------------------------------------------
void state_transparent( void ) {
	wait_vscan();
	background_scroll();
	puts_fighter();

	_count2--;
	if( _count2 == 0 ) {
		_count2 = 10;
		rabbit1[0].mode = _mode2;
		rabbit1[1].mode = _mode2;
		rabbit1[2].mode = _mode2;
		rabbit1[3].mode = _mode2;
		_mode2 = _mode2 + 0x40;
	}

	if( !message_animation() ) {
		return;
	}
	
	if( need_transition() ) {
		rabbit1[0].mode = 1;
		rabbit1[1].mode = 1;
		rabbit1[2].mode = 1;
		rabbit1[3].mode = 1;
		//	次のステートへ
		p_state = state_magnify;
		put_message(3);
	}
}

// --------------------------------------------------------------------
void state_magnify( void ) {
	int i;

	wait_vscan();
	background_scroll();
	mag = 1;
	puts_fighter();

	if( !message_animation() ) {
		return;
	}
	
	if( need_transition() ) {
		mag = 0;
		for( i = 0; i < 4; i++ ) {
			rabbit1[i].y		= (signed short)(31 | 0xC000);
			rabbit1[i].mgy		= 128;
			rabbit1[i].mgx		= 16;
			rabbit2[i].y		= (signed short)(31 | 0xC000);
			rabbit2[i].mgy		= 128;
			rabbit2[i].mgx		= 16;
		}
		//	次のステートへ
		p_state = state_maximum_puts;
		put_message(4);
	}
}

// --------------------------------------------------------------------
void state_maximum_puts( void ) {
	wait_vscan();
	background_scroll();
	puts_fighter();

	if( !message_animation() ) {
		return;
	}
	
	if( need_transition() ) {
		//	次のステートへ
		p_state = state_all_screen;
		put_message(5);
	}
}

// --------------------------------------------------------------------
void state_all_screen( void ) {
	wait_vscan();
	background_scroll();
	puts_fighter();

	if( !message_animation() ) {
		return;
	}
	
	if( need_transition() ) {
		//	次のステートへ
		p_state = state_color;
		put_message(6);
	}
}

// --------------------------------------------------------------------
void state_color( void ) {
	wait_vscan();
	background_scroll();
	puts_fighter();
	set_sprite_fade_palette();

	if( !message_animation() ) {
		return;
	}
	
	if( need_transition() ) {
		set_sprite_palette();
		//	次のステートへ
		p_state = state_reverse;
		put_message(7);
	}
}

	static int _count3 = 10;
	static int _reverse3 = 1;

// --------------------------------------------------------------------
void state_reverse( void ) {
	wait_vscan();
	background_scroll();
	puts_fighter();

	_count3--;
	if( _count3 == 0 ) {
		_count3 = 10;
		if( _reverse3 ) {
			rabbit1[0].mode |= 0x20;
			rabbit1[1].mode |= 0x20;
			rabbit1[2].mode |= 0x20;
			rabbit1[3].mode |= 0x20;
		}
		else {
			rabbit1[0].mode &= 0xDF;
			rabbit1[1].mode &= 0xDF;
			rabbit1[2].mode &= 0xDF;
			rabbit1[3].mode &= 0xDF;
		}
		_reverse3 ^= 1;
	}

	if( !message_animation() ) {
		return;
	}
	
	if( need_transition() ) {
		rabbit1[0].mode &= 0xDF;
		rabbit1[1].mode &= 0xDF;
		rabbit1[2].mode &= 0xDF;
		rabbit1[3].mode &= 0xDF;
		//	次のステートへ
		p_state = state_patterns;
		put_message(8);
	}
}

// --------------------------------------------------------------------
void state_patterns( void ) {
	wait_vscan();
	background_scroll_finish();
	puts_fighter();

	if( !message_animation() ) {
		return;
	}
	if( scroll_x || scroll_y ) {
		return;
	}
	
	if( need_transition() ) {
		//	次のステートへ
		p_state = state_easy;
		put_message(9);
	}
}

// --------------------------------------------------------------------
void state_easy( void ) {
	wait_vscan();

	rotate_copy();
	theta = (theta + 1) & 255;

	puts_fighter();

	if( !message_animation() ) {
		return;
	}
	
	if( need_transition() ) {
		//	次のステートへ
		p_state = state_highspeed_command;
		put_message(10);
	}
}

// --------------------------------------------------------------------
void state_highspeed_command( void ) {
	wait_vscan();

	background_fill();
	puts_fighter();

	if( !message_animation() ) {
		return;
	}
	
	if( need_transition() ) {
		//	次のステートへ
		p_state = state_font_command;
		put_message(11);
	}
}

// --------------------------------------------------------------------
void state_font_command( void ) {
	wait_vscan();

	rotate_copy();
	theta = (theta + 2) & 255;

	puts_fighter();

	if( !message_animation() ) {
		return;
	}
	
	if( need_transition() ) {
		//	次のステートへ
		p_state = state_end;
		fill_rectangle(0, 256 + 128 + 16 + 32, 224, 32, 0);
		message_state = 0;
	}
}

// --------------------------------------------------------------------
void state_end( void ) {
	wait_vscan();

	rotate_copy();
	if (theta < 3 && theta > -3)
		theta = 0;
	else
		theta = (theta - 3) & 255;

	if( !message_animation() || theta != 0 ) {
		return;
	}
	
	rotate_copy();
	fadeout_palette();

	load_background(NULL, NULL);
	set_initial_palette();

	p_state = state_window_animation;
	message_state = 1;
}

// --------------------------------------------------------------------
int main() {
	DEBUG_INIT();

	DEBUG_LOG("Let's begin!");

	DOS_StringOutput("V9968 demo for RU66\r\n$");

	initializer();

	p_state = state_window_animation;
	run_demo = 1;

	DEBUG_LOG("About to enter demo loop...");

	while( run_demo ) {
	 	p_state();
	}

	DEBUG_LOG("Done!");

	terminator();

	// clean keyboard buffer
	Bios_InterSlotCall(0, 0x0156); // KILBUF

	DOS_Exit0();
	return 0;
}

/*
  Comments from MoltSXalats

  The text is stored as bitmap/font data in three asset files:
  
  1. font.bin (11 KB) - Contains the actual Japanese character bitmap data
  2. bg.SC5 (33 KB) - Background graphics with UI text
  3. usa.SC5 (33 KB) - Sprite graphics (rabbits)

  How It Works
  
  In the put_message(int n) function (line 1123), the Japanese text display is handled as follows:

  void put_message( int n ) {
      // HL = n * 896 + 0x8000
      // Address = 0x18000 + n * 896  (font data at this offset)
      // Uses LFMM command to copy font bitmap data from font.bin to screen
  }

  Key points:
  - There are 12 different messages (n = 0 to 11)
  - Each message is 896 bytes of bitmap data
  - Total: 12 × 896 = 10,752 bytes (fits in the 11 KB font.bin)
  - The text is displayed using the LFMM (Logical Font Memory to Memory) command
  - Text content is pre-rendered as bitmaps in the font.bin file 
  
  How Messages Are Loaded

  In initializer() (line 836):
  set_vram_write_address(1, 0x8000);
  bload( "usa.SC5" );        // Load rabbit sprites
  
  set_vram_write_address(1, 0x8000);
  bload( "font.bin" );       // Load Japanese text bitmaps
  
  The bload() function loads binary files from disk and copies them into VRAM at the specified address.

  To Modify the Japanese Text

  You would need to:
  1. Edit the bitmap data in font.bin to change the displayed Japanese characters
  2. Use graphics tools to create new message bitmaps (each 224×32 pixels at 256 colors)
  3. Replace the corresponding 896-byte chunks in font.bin

 */
