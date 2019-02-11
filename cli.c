#include "global.h"
#include "cli.h"
#include "lbuf.h"
#include "board.h"

#include <ctype.h>

#define MAX_LINE_SIZE   64

#define EOL         '\r'
#define LF          '\n'
#define ESC         0x1B
#define BACKSPACE   0x7F

#define ACTION_ASSIGNMENT   '='
#define ACTION_QUERY        '?'
#define ACTION_COMMAND      ' '


typedef struct
{
    const char * cmd;
    const char * help;
    bool (*set)( char* );
    void (*get)( void );
}
cmd_t;

static void cmd_freq_get( void )
{
}

static bool cmd_freq_set( char *p_args)
{
    int32_t freq_hz = atoi( p_args );
    board_snd( 0, freq_hz );
    return true;
}
static const cmd_t s_cmd[] =
{
    { "freq", " = <frequency in Hz>", cmd_freq_set, cmd_freq_get }
};

static cbuf_t * s_p_cbuf_write;
static cbuf_t * s_p_cbuf_read;
static lbuf_t   lbuf_cmd;
static char lbuf_cmd_buffer[MAX_LINE_SIZE];

static void write_line( const char * p_str )
{
    static const uint8_t line[2] = { LF, EOL };
    cbuf_write_string( s_p_cbuf_write, p_str );
    cbuf_write( s_p_cbuf_write, line, sizeof(line) );
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
    prompt();
}

void cli_event(void)
{
    uint8_t c;
    while( cbuf_read( s_p_cbuf_read, &c, sizeof(c) ) )
    {
        if( c == BACKSPACE )
        {
            if( lbuf_cmd.write_ndx )
            {
                cbuf_write( s_p_cbuf_write, &c, sizeof(c) );
                lbuf_cmd.write_ndx--;
            }
            continue;
        }
        cbuf_write( s_p_cbuf_write, &c, sizeof(c) );
        if( c == EOL )
        {
            cbuf_write_byte( s_p_cbuf_write, LF );
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
                        cbuf_write_string( s_p_cbuf_write, s_cmd[i].cmd );
                        write_line( s_cmd[i].help );
                        goto cmd_dispatched;
                    }
                }
            }
cmd_dispatched:
            if( i==ARRAY_COUNT(s_cmd) )
                write_line( "unknown command" );
            else
                prompt();
            lbuf_reset( &lbuf_cmd );
        }
        else
        {
            lbuf_write_byte( &lbuf_cmd, c );
        }
    }
}


