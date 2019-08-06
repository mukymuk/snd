#ifndef __MC__H__
#define __MC__H__

void mc_timer_isr(  uint32_t timer_ndx  );
void mc_set_position( const float_t * p_point );
void mc_get_position( float_t * p_point );
void mc_begin( void );
void mc_end( void );

#endif

