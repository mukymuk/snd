#include <stdio.h>

#include "global.h"
#include "cli.h"
#include "lbuf.h"
#include "board.h"
#include "ps.h"
#include "config.h"
#include "mc.h"

#include <stdlib.h>
#include <ctype.h>

#define EOL_IN              '\r'
#define MAX_LINE_SIZE       64
#define ESC                 0x1B
#define BACKSPACE           0x7F
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

static bool cmd_help( const char * p_args );

static void write_string( const char * p_str )
{
    cbuf_write_string( s_p_cbuf_write, p_str );
};

static void prompt( void )
{
    cbuf_write_string( s_p_cbuf_write, "> " );
}

static void write_line( const char * p_str )
{
    write_string(p_str);
    write_string("\r\n");
}

static char * skip_space( const char * p )
{
    while( *p && isspace( *p ) )
        p++;
    return (char*)p;
}

static char * skip_letters( char * p )
{
    while( *p && isalpha( *p ) )
        p++;
    return p;
}

static char * next_arg( const char * p )
{
    while( *p && *p != ',' )
        p++;
    if( !*p )
        return NULL;
    p++;
    return skip_space(p);
}

static const char * get_axis( int32_t *p_axis, const char *p_args )
{
    int32_t axis = atoi( p_args );
    if( axis < 0 || axis >= g_config.axis_count )
        return NULL;
    *p_axis = axis;
    return next_arg( p_args );
}

static bool cmd_freq_get( void )
{
    write_line("query not supported");
    return true;
}

static bool cmd_freq_set( const char *p_args)
{
    int32_t axis;
    if( !(p_args = get_axis( &axis, p_args ) ) )
        return false;
    int32_t freq_hz = atoi( p_args );
    board_snd( axis, freq_hz );
    return true;
}

static bool set_float( float_t *p_float, const char * str )
{
    if( str )
    {
        *p_float = strtof( str, NULL );
        return true;
    }
    return false;
}

static bool get_float( float_t f )
{
    snprintf( lbuf_cmd_buffer, sizeof(lbuf_cmd_buffer), "%g", f );
    write_line( lbuf_cmd_buffer );
    return true;
}

static bool cmd_acc_set( const char *p_args )
{
    int i = 0;
    int32_t axis;
    if( !(p_args = get_axis( &axis, p_args ) ) )
        return false;
    while( p_args && i < SPEED_ACC_COUNT )
    {
        set_float( &g_config.speed_acc[axis][i].speed, p_args );
        if( p_args = next_arg(p_args) )
        {
            set_float( &g_config.speed_acc[axis][i].acceleration, p_args );
        }
        else
            return false;
        p_args = next_arg( p_args );
        i++;
    }
    return true;
}

static bool cmd_acc_get( void )
{
    uint32_t i,j;
    for(i=0;i<g_config.axis_count;i++)
    {
        if( g_config.speed_acc[i][0].speed )
        {
            snprintf( lbuf_cmd_buffer, sizeof(lbuf_cmd_buffer), "axis %d:  ", i );
            write_string( lbuf_cmd_buffer );
        }
        else
            break;
        for(j=0;j<SPEED_ACC_COUNT;j++)
        {
            if( g_config.speed_acc[i][j].speed )
            {
                if( j )
                    write_string( ", " );
                snprintf( lbuf_cmd_buffer, sizeof(lbuf_cmd_buffer), "%g,%g", g_config.speed_acc[i][j].speed, g_config.speed_acc[i][j].acceleration );
                write_string( lbuf_cmd_buffer );
            }
            else
                break;
        }
        write_line( NULL );
    }
}

static bool cmd_step_set( const char *p_args )
{
    int32_t axis;
    if( !(p_args = get_axis( &axis, p_args ) ) )
        return false;
    return set_float( &g_config.step[axis], p_args );
}

static bool cmd_step_get( void )
{
    uint8_t i;
    for(i=0;i<g_config.axis_count;i++)
    {
        get_float( g_config.step[i] );
    }
    return true;
}

static bool cmd_save( const char *p_args )
{
    ps_write( g_config.ps, &g_config, sizeof(g_config));
    return true;
}

static bool cmd_reset( const char *p_args )
{
    board_reset();
    return true;  // not really
}

static bool cmd_speed_set( const char *p_args )
{
    return set_float( &g_config.speed, p_args );
}

static bool cmd_speed_get( void )
{
    return get_float( g_config.speed );
}

static bool cmd_pos_set( const char *p_args )
{
    bool boo = false;
    float_t point[AXIS_COUNT];
    uint8_t i;
    for(i=0;i<AXIS_COUNT;i++)
    {
        set_float( &point[i], p_args );
        if( p_args = next_arg(p_args) )
        {
            set_float( &point[i], p_args );
            if( !(p_args = next_arg(p_args) ) )
            {
                break;
            }
        }
        else
        {
            boo = true;
            break;
        }
    }
    if( i < g_config.axis_count )
        boo = true;

    if( !boo )
    {
        mc_set_position( &point[0] );
    }
    return boo;
}

static bool cmd_pos_get( void )
{
}

static bool cmd_axis_set( const char *p_args )
{
    int32_t axis_count = atoi( p_args );
    if( axis_count < 0 || axis_count >= AXIS_COUNT )
        return false;
    g_config.axis_count = axis_count;
    return true;
}

static bool cmd_axis_get( void )
{
    snprintf( lbuf_cmd_buffer, sizeof(lbuf_cmd_buffer), "%d", g_config.axis_count );
    write_line( lbuf_cmd_buffer );
    return true;
}

static bool cmd_begin( const char * p_args )
{
    mc_begin();
    return true;
}

static bool cmd_end( void )
{
    mc_end();
    return true;
}
static const cmd_t s_cmd[] =
{
    { "axis", "<axis count>", "number of axis supported", cmd_axis_set, cmd_axis_get },
    { "begin", NULL, "begin accumulating positions", cmd_begin, NULL },
    { "end", NULL, "discontinue accumulating positions", cmd_begin, NULL },
    { "speed", NULL, "default speed in mm/s", cmd_speed_set, cmd_speed_get },
    { "freq", "<frequency in Hz>", "test step frequency", cmd_freq_set, cmd_freq_get },
    { "acc", "<axis #>, <speed,acc[,speed2,acc2,...]>", "velocity/acceleration profile for axis # in mm/sec^2", cmd_acc_set, cmd_acc_get },
    { "step", "<axis #>, <mm/step>","indicate distance per step for axis # in millimeters", cmd_step_set, cmd_step_get },
    { "pos", "<x1, y1, x2, y2 [,speed]>", "constant speed move from x1,y1 to xN,yN in millimeters", cmd_pos_set, cmd_pos_get },
    { "reset",  NULL, "resets the processor", cmd_reset, NULL },
    { "save", NULL, "save configuration to internal flash", cmd_save, NULL },
    { "help", "you're looking at it", NULL, cmd_help, NULL }
};

static bool cmd_help( const char * p_args )
{
    #define SPACES 10
    static const char * space = "          ";
    uint8_t i;
    uint32_t l;
    static char * general = "Format is <attribute>=<value> for assignment, <attribute>? for query, and <command>.";
    write_line( general );
    write_line( NULL );
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
                if( s_cmd[i].args )
                    cbuf_write( s_p_cbuf_write, space, SPACES );
                write_line( s_cmd[i].help );
            }
        }
        else
        {
            write_line(NULL);
        }
    }
    return true;
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
                cbuf_write_byte( s_p_cbuf_write, c );
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
                            write_line( "argument error" );
                            goto cmd_dispatched;
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
            if( isprint(c) )
            {
                c = tolower(c);
                lbuf_write_byte( &lbuf_cmd, c );
                cbuf_write_byte( s_p_cbuf_write, c );
            }
        }
    }
}


