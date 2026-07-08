// ====================================================================
//	MSX VDP Controll
// --------------------------------------------------------------------
//	Programmed by t.hara
//  New features and porting to MSXgl by Albert Herranz
// ====================================================================

#include "msx_vdp.h"
#include "debug.h"
#include "dos.h"

// --------------------------------------------------------------------
void _ei( void ) {
__asm
	ei
__endasm;
}

// --------------------------------------------------------------------
void _di( void ) {
__asm
	di
__endasm;
}

// --------------------------------------------------------------------
int init_vdp( void ) {
	int id, s1;

	// if no hardware uses 0x89 we'll get 0xFF
	// if there is a V99x8, we'll get bit 7 set to 0
	DEBUG_LOGNUM("VDP_PORT", VDP_PORT);
	if ((VDP_PORT & 0x80) == 0x00) {
		// look only for a external VDP V9968
		_di();
		vdp_write_reg( 21, 0x3A );
		vdp_write_reg( 15, 1 );
		s1 = vdp_read_stat();
		id = ((s1 >> 1) | (s1 << 7)) & 0x1F;
		vdp_write_reg( 15, 0 );
		vdp_write_reg( 21, 0x3B );
		_ei();
		DEBUG_LOGNUM("ID", id);
		if( id >= 3 ) {
			DEBUG_LOG("Found V9968");
			return 1;
		}
	}

	DEBUG_LOG("Ouch, no V9968 found");
	return 0;
}

static unsigned char reg_data[] = {
	0, 0x06,		//	Mode0
	1, 0x00,		//	Mode1
	2, 0x1F,		//	Pattern Name Table = Page0
	5, 0xEF,		//	Sprite Attribute Table(L)
	6, 0x0F,		//	Sprite Pattern Generator
	7, 0x07,		//	Background Color
	8, 0x0A,		//	Mode2
	9, 0x80,		//	Mode3
	11, 0,			//	Sprite Attribute Table(H)
	18, 0,			//	Set Adjust
	19, 0,			//	Interrupt line
	20, 0xFF,		//	Mode5
	21, 0,			//	Mode6
	23, 0,			//	Display Offset
	25, 0,			//	Mode4
	26, 0,			//	Horizontal Offset by character
	27, 0,			//	Horizontal Offset by dot
	255
};

// --------------------------------------------------------------------
void set_screen5( void ) {

	unsigned char *p;

	_di();
	for( p = reg_data; p[0] != 255; p += 2 ) {
		vdp_write_reg( p[0], p[1] );
	}
	_ei();
}

// --------------------------------------------------------------------
// Note: bload requires FILE operations which are not available in this
// embedded environment. This is a stub implementation.
static unsigned char s_buffer[ 1024 ];

int bload( const char *p_name ) {
	// File operations not supported in this environment
  u8 file = DOS_FOpen(p_name, O_RDONLY);
  unsigned int start, end, size, block_size;
  unsigned char *p;
  u8 err;

#if (DOS_USE_VALIDATOR)
	err = DOS_GetLastError();
	if (err != DOS_ERR_NONE)
		return -1;
#endif

  DOS_FRead(file, s_buffer, 7);
  start	= (int)s_buffer[1] | ((int)s_buffer[2] << 8);
	end		= (int)s_buffer[3] | ((int)s_buffer[4] << 8);
	size	= end - start + 1;
  while(size) {
    if( size>sizeof(s_buffer) ) {
      block_size = sizeof(s_buffer);
    } else {
      block_size = size;
    }
    DOS_FRead(file, s_buffer, block_size);
    p=s_buffer;
    size -= block_size;
    while(block_size) {
      VDP_PORTl1 = *p;
      p++;
      block_size--;
    }
  }
  DOS_FClose(file);
  return 0;
}

// --------------------------------------------------------------------
int load_palette(void *buf, const char *p_name)
{
  u8 file = DOS_FOpen(p_name, O_RDONLY);
  u8 err;

#if (DOS_USE_VALIDATOR)
	err = DOS_GetLastError();
	if (err != DOS_ERR_NONE)
		return -1;
#endif

  DOS_FRead(file, buf, 48);
  DOS_FClose(file);
  return 0;
}

// --------------------------------------------------------------------
void set_display_visible( int visible ) {

	_di();
	if( visible ) {
		//	visible
		vdp_write_reg( 1, 0x40 );
	}
	else {
		//	invisible
		vdp_write_reg( 1, 0x00 );
	}
	_ei();
}

// --------------------------------------------------------------------
void wait_vscan( void ) {

	_di();
	vdp_write_reg( 15, 2 );
	while( ( VDP_PORT  & 0x40) != 0 );
    while( ( VDP_PORT  & 0x40) == 0 );
	vdp_write_reg( 15, 0 );
	_ei();
}

// --------------------------------------------------------------------
void wait_vsync( int n ) {

	_di();
	vdp_write_reg( 15, 2 );
	while( n ) {
		while( ( VDP_PORT  & 0x40) == 0 );
		while( ( VDP_PORT  & 0x40) != 0 );
		n--;
	}
	vdp_write_reg( 15, 0 );
	_ei();
}

// --------------------------------------------------------------------
void set_vram_write_address( int bank, unsigned int address ) {

	_di();
	vdp_write_reg( 14, ((bank & 3) << 2) | ((address >> 14) & 0x03) );
	VDP_PORT =  address & 0xFF ;
	VDP_PORT = ((address >> 8) & 0x3F) | 0x40 ;
	_ei();
}

// --------------------------------------------------------------------
int get_cursor_key( void ) {
__asm
	di
	in a, (#0xAA)
	and #0xF0
	or #0x08
	out (#0xAA), a
	in a, (#0xA9)
	cpl
	and #0xF1
	ld e,a
	ld d,#0
	ei
__endasm;
}

// --------------------------------------------------------------------
void wait_vdp_command( void ) {
__asm
	ld		a, #0x89
	ld		c, a
	di
	//	R#15 = 2
	ld a, #2
	out (c), a
	ld a, #(0x80 + 15)
	out (c), a
_loop:
	in		a, (c)
	and		#1
	jr		nz, _loop
	//	R#15 = 0
	xor		a
	out (c), a
	ld a, #(0x80 + 15)
	ei
	out (c), a
__endasm;
}
