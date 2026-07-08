// ====================================================================
//	MSX VDP Controll
// --------------------------------------------------------------------
//	Programmed by t.hara
//  New features and porting to MSXgl by Albert Herranz
// ====================================================================

#ifndef __MSX_VDP_H__
#define __MSX_VDP_H__

// this is just for V9968 in a external cartridge using 8xh ports

__sfr __at(0x88) VDP_PORTl1;
__sfr __at(0x89) VDP_PORT;
__sfr __at(0x8A) VDP_PORTp1;
__sfr __at(0x8B) VDP_PORTp2;

void _di( void );
void _ei( void );

#define vdp_write_reg( reg, dat )		do { VDP_PORT = (dat); VDP_PORT = ((reg) | 0x80); } while(0)
#define vdp_read_stat()	 (VDP_PORT)

#define MSX_FALSE	0
#define MSX_TRUE	1

int init_vdp( void );
void set_screen5( void );
int bload( const char *p_name );
int load_palette(void *buf, const char *p_name);
void set_display_visible( int visible );
void wait_vscan( void );
void wait_vsync( int n );
void set_vram_write_address( int bank, unsigned int address );
int get_cursor_key( void );
void wait_vdp_command( void );

#define KEY_SPACE_DEMO	0x01
#define KEY_UP_DEMO		0x20
#define KEY_DOWN_DEMO	0x40
#define KEY_LEFT_DEMO	0x10
#define KEY_RIGHT_DEMO	0x80

#endif
