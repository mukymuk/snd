#include "global.h"
#include "board.h"
#include "mc.h"
#include "usbuart.h"
#include "cli.h"
#include "pmu.h"

CBUF(s_usb_write,256);
CBUF(s_usb_read,128);

CBUF(s_uart_write,256);
CBUF(s_uart_read,128);


static uart_t s_uart;


void main(void)
{
    bool a;
    board_init();
    pmu_init();

    CBUF_INIT( s_usb_read );
    CBUF_INIT( s_usb_write );
    CBUF_INIT( s_uart_read );
    CBUF_INIT( s_uart_write );

    usbuart_init( &s_usb_read.cbuf, &s_usb_write.cbuf );

    uart_common_clock( CLKMAN_SCALE_DIV_8 );
    uart_init( &s_uart, &s_uart_read.cbuf, &s_uart_write.cbuf, BOARD_UART_DBG,
               UART_CONFIG_8N1, UART_CONFIG_BAUD_SCALE8_115200 );
    cli_init( &s_usb_read.cbuf, &s_usb_write.cbuf );

    void *p;
    uint32_t size;


 //   board_motor_enable(0, true);
 //   board_snd(0,-1);

    while( 1 )
    {
        board_sleep();
  //      cli_event();

        if( size = cbuf_read_aquire( &s_uart_read.cbuf, &p ) )
        {
            cbuf_write( &s_usb_write.cbuf, p, size );
            cbuf_read_release( &s_uart_read.cbuf, size );
        }
        if( size = cbuf_read_aquire( &s_usb_read.cbuf, &p ) )
        {
            cbuf_write( &s_uart_write.cbuf, p, size );
            cbuf_read_release( &s_usb_read.cbuf, size );
        }

    }
}


