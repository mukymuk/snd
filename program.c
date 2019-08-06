#include "global.h"
#include "program.h"
#include "config.h"

#define CMD_END     0
#define CMD_VECTOR  1

static uint32_t s_program[8192];
static uint32_t s_ndx;
static uint32_t s_size;
static bool s_error;

void program_begin( void )
{
    s_size = 0;
    s_ndx = 0;
    s_error = false;
}

void program_vector( const float_t * p_vector )
{
    if( s_ndx + g_config.axis_count + 1 <= ARRAY_COUNT(s_program) )
    {
        s_program[s_ndx++] = CMD_VECTOR;
        memcpy( &s_program[s_ndx], p_vector, g_config.axis_count * sizeof(float_t) );
        s_ndx += g_config.axis_count;
    }
    else
        s_error = true;
}

void program_end( void )
{
    s_size = s_ndx;
    s_ndx = 0;

}

void program_execute( void )
{
}

bool program_error( void )
{
    return s_error;
}
