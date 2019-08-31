#include "global.h"
#include "program.h"
#include "config.h"

#define CMD_END     0
#define CMD_LINE    1


void program_begin( program_t * p_program )
{
    p_program->ndx = 0;
}

void program_end( program_t * p_program )
{
    uint32_t * p = p_program->program;
    p[p_program->ndx] = CMD_END;
    p_program->ndx = 0;
}

bool program_line( program_t * p_program, const float_t * p_vector )
{
    uint32_t * p = p_program->program;

    if( p_program->ndx + g_config.axis_count + 1 <= p_program->size )
    {
        p[p_program->ndx++] = CMD_LINE;
        memcpy( &p[p_program->ndx], p_vector, g_config.axis_count * sizeof(float_t) );
        p_program->ndx += g_config.axis_count;
        return true;
    }
    return false;
}

typedef void (*mc_dispatch_line_t)( const float_t * p_vector);
typedef struct
{
    mc_dispatch_line_t mc_dispatch_line;
}
mc_dispatch_t;

bool program_fetch( mc_dispatch_t * p_dispatch, program_t * p_program )
{
    uint32_t * p = p_program->program;
    switch( p[p_program->ndx++] )
    {
        case CMD_LINE:
        {
            p_dispatch->mc_dispatch_line( (const float_t*)&p[p_program->ndx] );
            p_program->ndx += g_config.axis_count;
            return true;
        }
    }
    return false;
}

