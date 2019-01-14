#include "global.h"
#include "mc.h"

typedef struct _point_t
{
    float_t x, y;
}
point_t;

typedef enum _motion_id_t
{
    motion_rapid, motion_line, motion_arc
}
motion_id_t;

typedef struct _link_t
{
    motion_id_t     motion_id;
    struct _geometry_t * p_next;
}
link_t;

typedef struct _line_t
{
    link_t      link;
    point_t     p;
}
line_t;

typedef line_t rapid_t;

typedef struct _arc_t
{
    link_t      link;
    point_t     p;
    point_t     center;

}
arc_t;



typedef struct _mc_move_t
{
    int32_t     delta;
    uint32_t    count;
}
mc_move_t;

uint32_t mc_line( mc_move_t *p_mc_move, float_t distance, float_t speed, float_t max_accleration )
{

    return 0;
}
