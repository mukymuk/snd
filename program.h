#ifndef __PROGRAM_H__
#define __PROGRAM_H__

typedef struct
{
    uint32_t *  program;
    uint32_t    size;
    uint32_t    ndx;
}
program_t;


void program_begin( program_t * p_program );
void program_end( program_t * p_program );
bool program_line( program_t * p_program, const float_t * p_vector );

#endif
