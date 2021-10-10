typedef unsigned char bool;
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;

#define false 0
#define true 1
#define NULL 0

int printf( const char *format, ... );
int strlen( const char *s );

void os_line_edit_init( char *buf, uint8_t bufsiz );
char *os_line_edit( char *buf, char c, const char *immediates );

int cmd_handler( char c, const char *menu );
char **cmd_get_parms( int *addr_nbr, bool make_lower_case );
