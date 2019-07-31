#include "global.h"
#include "cli.h"
#include "lbuf.h"
#include "board.h"
#include "ps.h"
#include "config.h"

#include <stdlib.h>
#include <ctype.h>

#define EOL_IN		'\r'

#define MAX_LINE_SIZE   64

#define ESC         0x1B
#define BACKSPACE   0x7F

#define ACTION_ASSIGNMENT   '='
#define ACTION_QUERY        '?'
#define ACTION_COMMAND      0


typedef struct
{
    const char * cmd;
    const char * args;
    const char * help;
    bool (*set)( const char * );
    bool (*get)( void );
}
cmd_t;

static cbuf_t * s_p_cbuf_write;
static cbuf_t * s_p_cbuf_read;
static lbuf_t   lbuf_cmd;
static char lbuf_cmd_buffer[MAX_LINE_SIZE];

static bool cmd_freq_get( void )
{
}

static bool cmd_freq_set( const char *p_args)
{
    int32_t freq_hz = atoi( p_args );
    board_snd( 0, freq_hz );
    return true;
}

static bool cmd_speed_set( const char *p_args )
{
}

static bool cmd_speed_get( void )
{
}

static bool cmd_acc_set( const char *p_args )
{
}

static bool cmd_acc_get( void )
{
}

static bool cmd_step_set( const char *p_args )
{
}

static bool cmd_step_get( void )
{
}

static bool cmd_line( const char *p_args )
{
}

static bool cmd_save( const char *p_args )
{
	ps_write( &g_config, CONFIG_VERSION, sizeof(g_config));
}


static bool cmd_help( const char * p_args );

static const cmd_t s_cmd[] =
{
    { "freq", "<frequency in Hz>", "test step frequency", cmd_freq_set, cmd_freq_get },
    { "speed", "<constant speed target>", "maximum linear speed", cmd_speed_set, cmd_speed_get },
    { "acc", "<axis #>, <speed,acc[,speed2,acc2,...]>", "velocity/acceleration profile for axis #", cmd_acc_set, cmd_acc_get },
    { "step", "<axis #>, <mm/step>","indicate distance per step for axis #", cmd_step_set, cmd_step_get },

    { "line", "<x1, y1, x2, y2 [,speed]>", "constant speed move from x1,y1 to x2,y2", cmd_line, NULL },
	{ "save", NULL, "save configuration to internal flash", cmd_save, NULL },
    { "help", "you're looking at it", NULL, cmd_help, NULL }

};



static void write_string( const char * p_str )
{
	cbuf_write_string( s_p_cbuf_write, p_str );
};

static void write_line( const char * p_str )
{
	write_string(p_str);
	write_string("\r\n");
}

static bool cmd_help( const char * p_args )
{
    #define SPACES 10
    static char * space = "          ";
    uint8_t i;
    uint32_t l;
    for(i=0;i<ARRAY_COUNT(s_cmd);i++)
    {
       l = strlen( s_cmd[i].cmd );
		write_string( s_cmd[i].cmd );
        if( s_cmd[i].args || s_cmd[i].help )
        {
            cbuf_write( s_p_cbuf_write, space, SPACES - l - 2 );
            write_string( "- " );
            if( s_cmd[i].args )
            {
                write_line( s_cmd[i].args );
            }
            if( s_cmd[i].help )
            {
                cbuf_write( s_p_cbuf_write, space, SPACES );
                write_line( s_cmd[i].help );
            }
        }
        else
        {
			write_line(NULL);
        }
    }
}

static void prompt( void )
{
    cbuf_write_string( s_p_cbuf_write, "> " );
}

static char * skip_space( char * p )
{
    while( isspace( *p ) )
        p++;
    return p;
}

static char * skip_letters( char * p )
{
    while( isalpha( *p ) )
        p++;
    return p;
}

void cli_init( cbuf_t * p_cbuf_read, cbuf_t * p_cbuf_write  )
{
    s_p_cbuf_write = p_cbuf_write;
    s_p_cbuf_read = p_cbuf_read;
    lbuf_init( &lbuf_cmd, lbuf_cmd_buffer, sizeof(lbuf_cmd_buffer) );
	cbuf_write_string( s_p_cbuf_write, "\033c" );
    prompt();
}

void cli_event(void)
{
    uint8_t c;
    char * p_cmd;

    uint32_t size;
    uint32_t i = 0;

    while( cbuf_read( s_p_cbuf_read, &c, sizeof(c) ) )
    {
        if( c == BACKSPACE )
        {
            if( lbuf_cmd.write_ndx )
            {
                lbuf_cmd.write_ndx--;
            }
            continue;
        }
		if( c == EOL_IN )
		{
			write_line( NULL );
            lbuf_write_byte( &lbuf_cmd, 0 );
            char * p_cmd = skip_space( lbuf_cmd_buffer );
            char * p_action = skip_letters( p_cmd );
            char action = *p_action;
            *p_action = NULL;
            char * p_args = skip_space( ++p_action );
            uint32_t i;
            for(i=0;i<ARRAY_COUNT(s_cmd);i++)
            {
                if( !strcmp( s_cmd[i].cmd, p_cmd ) )
                {
                    if( action == ACTION_ASSIGNMENT || action == ACTION_COMMAND )
                    {
                        if( !s_cmd[i].set( p_args ) )
                        {
                            cbuf_write_string( s_p_cbuf_write, s_cmd[i].help );
                        }
                        goto cmd_dispatched;
                    }
                    else if( action == ACTION_QUERY )
                    {
                        s_cmd[i].get();
                        goto cmd_dispatched;
                    }
                    else
                    {
                        write_line( "syntax error" );
                        goto cmd_dispatched;
                    }
                }
            }
            if( i==ARRAY_COUNT(s_cmd) )
                write_line( "unknown command" );
cmd_dispatched:
            prompt();
            lbuf_reset( &lbuf_cmd );
        }
        else
        {
			c = tolower(c);
            lbuf_write_byte( &lbuf_cmd, c );
            cbuf_write_byte( s_p_cbuf_write, c );
        }
    }
}


