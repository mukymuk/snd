#include "global.h"
#include "board.h"
#include "mc.h"
#include "usbuart.h"
#include "cli.h"

CBUF(s_usb_write,256);
CBUF(s_usb_read,128);

void main(void)
{
    board_init();

    CBUF_INIT( s_usb_read );
    CBUF_INIT( s_usb_write );

    usbuart_init( &s_usb_read.cbuf, &s_usb_write.cbuf );
    cli_init( &s_usb_read.cbuf, &s_usb_write.cbuf );

    board_motor_enable(0, true);
    board_snd(0,-1);

    while( 1 )
    {
        board_sleep();
        cli_event();
    }
}

