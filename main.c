#include "global.h"
#include "board.h"
#include "mc.h"

int main(int argc, char *argv[])
{
    board_init();
    DBG(("snd620\n"));

    board_motor_enable(0, true);
    board_snd(0,-1);

    while(1);

    return 0;
}
