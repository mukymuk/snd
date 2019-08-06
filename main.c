#include "global.h"
#include "board.h"
#include "mc.h"
#include "usbuart.h"
#include "cli.h"
#include "ps.h"
#include "config.h"

//CBUF(s_usb_write,256);
//CBUF(s_usb_read,128);

CBUF(s_uart_write,256);
CBUF(s_uart_read,1);

static uart_t s_uart;

config_t g_config;

static const ps_t s_ps_config =
{
    .address = 0x1FE000,
    .size = 8192/4,
    .version = 1000
};

void main(void)
{
    bool a;
    board_init();
    
    //CBUF_INIT( s_usb_read );
    //CBUF_INIT( s_usb_write );
    CBUF_INIT( s_uart_read );
    CBUF_INIT( s_uart_write );

    //usbuart_init( &s_usb_read.cbuf, &s_usb_write.cbuf );

    const config_t * p_config;
    g_config.ps = &s_ps_config;
    if( sizeof(config_t) == ps_locate( &s_ps_config, (void**)&p_config ) )
        memcpy( &g_config, (void*)p_config, sizeof(config_t) );

    uart_common_clock( CLKMAN_SCALE_DIV_1 );
    uart_init( &s_uart, &s_uart_read.cbuf, &s_uart_write.cbuf, BOARD_UART_DBG,
               UART_CONFIG_8N1, UART_CONFIG_BAUD_DIV16 | 52 );
    cli_init( &s_uart_read.cbuf, &s_uart_write.cbuf );

    void *p;


    board_motor_enable(0, true);
 
    while( 1 )
    {
        board_sleep();
        cli_event();
/*
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
*/
    }
}


