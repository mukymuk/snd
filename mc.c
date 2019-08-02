#include "global.h"
#include "mc.h"
#include "cbuf.h"


typedef struct
{
    float_t     a;
    float_t     d;
      
}
mc_segment_t;


typedef struct
{
    float_t     v_max;
    float_t     a_max;
    float_t     coef;
    float_t     v;
    uint32_t    timer_ndx;
}
mc_axis_t;

/*
static uint32_t next_step( mc_axis_t * p_axis )
{
    float_t vi = p_axis->v;
    float_t vf;

    vf = 0.5f * ( vi + sqrtf( 4.0f*a + vi*vi ) );

    float_t va = board_snd( vf );
    p_axis->v = va;

}
*/
float_t mc_profile( mc_segment_t *p_segment, uint32_t count, float_t vi, float_t vt, float_t * p_vf, float_t a, float_t d )
{
    const float_t sr2 = 1.41421356f;

    float_t vf = *p_vf;
    float_t ia = 0.5f / a;
    float_t vti = vt - vi;
    float_t vtf = vt - vf;
    float_t dvt = vti * ia;
    float_t dvf = vtf * ia;

    if( d < dvf )
    {
        if( count )
        {
            vf = vi + sr2 * sqrt( a ) * sqrt( d );
            p_segment[0].a = a;
            p_segment[0].d = d;
            count--;
        }
    }
    else if( d <= (dvt+dvf) )
    {
        if( count >= 2 )
        {
            vt = 0.5f * (vi + vf + sqrt( 4.0f * a * d - vf * vf + 2.0f * vi * vf - vi * vi ));
            vti = vt - vi;
            vtf = vt - vf;
            p_segment[0].a = a;
            p_segment[0].d = vti * vti * ia;
            p_segment[1].a = -a;
            p_segment[1].d = vtf * vtf * ia;
            count-=2;
        }
    }
    else
    {
        if( count >= 3 )
        {
            p_segment[0].a = a;
            p_segment[0].d = dvt;
            p_segment[1].a = 0;
            p_segment[1].d = d - dvt - dvf;
            p_segment[2].a = -a;
            p_segment[2].d = dvf;
            count-=3;
        }
    }
    *p_vf = vf;
    return count;
}

static const float_t s_tick = 2.0; ///RO_FREQ;

float_t mc_a( float_t vi, float_t vf, float_t time )
{
    return (vf - vi) / time;
}

float_t mc_v2d( float_t v )
{
    return roundf( 1.0f / (v * s_tick) );
}

static inline float_t acc( float_t a, float_t vi )
{
    // returns final speed given initial speed and accleration factor
    return ( sqrtf( 4.0f * a + vi * vi ) + vi ) / 2.0f;
}

static inline float_t p2v( int32_t p )
{
    // returns velocity given step period
    return 1.0f / ( (float_t)p * s_tick );
}

static inline int32_t v2p( float_t v )
{
    // returns step period (rounded) given velocity
    return roundf( s_tick / v );
}

float mc_calc_acc( uint32_t start_period, uint32_t end_period, uint32_t time )
{
    uint32_t a = time * end_period;
    uint32_t b = time * start_period;
    float_t accel = 1.0f/(float_t)a - 1.0f/(float_t)b;
    return accel;
}

void mc_set_position( const mc_point_t * p_point )
{
}

void mc_get_position( mc_point_t * p_point )
{
}
void mc_timer_isr( uint32_t timer_ndx )
{
}
