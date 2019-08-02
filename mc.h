#ifndef __MC__H__
#define __MC__H__

typedef struct
{
    float_t x;
    float_t y;
}
mc_point_t;

void mc_timer_isr(  uint32_t timer_ndx  );
void mc_set_position( const mc_point_t * p_point );
void mc_get_position( mc_point_t * p_point );

#endif

