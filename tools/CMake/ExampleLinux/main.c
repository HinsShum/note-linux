
/**************************************************
 *
 * file: demo_rx.c
 * purpose: simple demo that receives characters from
 * the serial port and print them on the screen,
 * exit the program by pressing Ctrl-C
 *
 * compile with the command: gcc demo_rx.c rs232.c -Wall -Wextra -o2 -o test_rx
 *
 * *********************************************/

#include <stdlib.h>
#include <stdio.h>
#include "includes.h"
#include "ryat_if.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "lib/rs232/rs232.h"

static void _delay_ms(uint32_t delay)
{
#ifdef _WIN32
    Sleep(delay);
#else
    usleep(1000 * delay);
#endif    
}

int test_uart(void) {
  int i, n,
      cport_nr = 6,        /* /dev/ttyS0 (COM1 on windows) */
      bdrate = 115200;       /* 9600 baud */

  unsigned char buf[4096];

  char mode[] = { '8', 'N', '1', 0 };


  if (RS232_OpenComport(cport_nr, bdrate, mode)) {
    printf("Can not open comport\n");

    return (0);
  }

  while (1) {
    n = RS232_PollComport(cport_nr, (unsigned char *)buf, 4095);

    if (n > 0) {
      buf[n] = 0;   /* always put a "null" at the end of a string! */

      for (i = 0; i < n; i++) {
        if (buf[i] < 32) {  /* replace unreadable control-codes by dots */
          buf[i] = '.';
        }
      }
      printf("received %i bytes: %s\n", n, (char*)buf);
    }

    _delay_ms(100);
  }
}

static void port_delay(int16_t tick) {
    _delay_ms(10 * tick);
}

int port_handle = 0;
static void port_drop_input_data(void) {
  RS232_flushRX(port_handle);
}


int16_t port_write_string(const char* pdata, int16_t len) {
  int res;
  res = RS232_SendBuf(port_handle, (unsigned char *)pdata, len);
  return res;
}


bool_t port_read_char(char* pchar) {
  char buf[2];
  if (1 == RS232_PollComport(port_handle, (unsigned char *)buf, 1)) {
    *pchar = buf[0];
    return true;
  } else {
    return false;
  }
}

struct st_ryat_describe at_engine;

void test(void) {

}

void port_init(void) {
  int bdrate = 115200;       /* 9600 baud */

  port_handle = 11;        /* /dev/ttyS0 (COM1 on windows) */

  char mode[] = { '8', 'N', '1', 0 };


  if (RS232_OpenComport(port_handle, bdrate, mode)) {
    printf("Can not open comport\n");

    exit(0);
  }
}

int main(void) {
  int32_t result;
  uint8_t errcnt = 0;
  uint8_t module_type = 0;

  port_init();

  at_engine.delay_tick = port_delay;
  at_engine.port_discard_inpurt = port_drop_input_data;
  at_engine.port_read_char = port_read_char;
  at_engine.port_write_string = port_write_string;

  ryat_if_set_at(&at_engine);
  module_type = ryat_if_get_module_model_type();
  printf("Module type: %d\r\n", module_type);
  _delay_ms(10000);

  /*
   * test at
   */
#if 1
  if (RYAT_E_OK == at_engine.method->test()) {
    printf("-----Test: test ok\r\n");
  } else {
    printf("-----Test: test failed\r\n");
  }

  /*
   * test echo
   */
  if (RYAT_E_OK == at_engine.method->close_echo()) {
    printf("-----Test: echo ok\r\n");
  } else {
    printf("-----Test: echo failed\r\n");
  }
#endif

  /*
   * test imei
   */
  uint8_t imei[20] = { 0 };
  if (RYAT_E_OK == at_engine.method->get_imei(imei)) {
    printf("-----Test: imei :%s\r\n", imei);
  } else {
    printf("-----Test: imei failed\r\n");
  }

  /*
   * test iccid
   */
  uint8_t iccid[22] = { 0 };
  if (RYAT_E_OK == at_engine.method->get_iccid(iccid)) {
    printf("-----Test: iccid :%s\r\n", iccid);
  } else {
    printf("-----Test: iccid failed\r\n");
  }


  /*
   * test imsi
   */
  uint8_t imsi[22] = { 0 };
  if (RYAT_E_OK == at_engine.method->get_imsi(imsi)) {
    printf("-----Test: imsi :%s\r\n", imsi);
  } else {
    printf("-----Test: imsi failed\r\n");
  }
rep_attach:
  /*
   * test attach the gprs
   */
  if (RYAT_E_OK == at_engine.method->attach()) {
    printf("-----Test: attach ok\r\n");
  } else {
    printf("-----Test: attach failed\r\n");
    errcnt++;
    if(errcnt < 5) {
      goto rep_attach;
    }
    errcnt = 0;
  }

  /*
   * get attach the gprs status
   */
  result = at_engine.method->is_attached();

  if (result >= 0) {
    printf("-----Test: get attach :%d\r\n", result);
  } else {
    printf("-----Test: get attach failed\r\n");
  }

  /*
   * test csq
   */
  uint8_t csq = 0;
  if (RYAT_E_OK == at_engine.method->get_csq(&csq)) {
    printf("-----Test: csq :%d\r\n", csq);
  } else {
    printf("-----Test: csq failed\r\n");
  }

  /*
   * test rxlevel
   */
  uint8_t rxlev = 0;
  if (at_engine.method->get_rxlev) {
    if (RYAT_E_OK == at_engine.method->get_rxlev(&rxlev)) {
      printf("-----Test: rxlev :%d\r\n", rxlev);
    } else {
      printf("-----Test: rxlev failed\r\n");
    }
  }

  /*
   * get the register status
   */
  struct st_ryat_cell_describe cell;
  if (RYAT_E_OK == at_engine.method->get_creg_cell(&cell)) {
    printf("-----Test: get cell ok:%d-%d-%04X-%04X\r\n", cell.mcc, cell.mnc, cell.lac, cell.cellid);
  } else {
    printf("-----Test: get reg cell failed\r\n");
  }

  if(module_type != RYAT_MODULE_ID_NBXX_01 &&
     module_type != RYAT_MODULE_ID_BC26 &&
     module_type != RYAT_MODULE_ID_SLM750 &&
     module_type != RYAT_MODULE_ID_SLM790 &&
     module_type != RYAT_MODULE_ID_EC20 &&
     module_type != RYAT_MODULE_ID_L620) {
      /*
      * disactive the gprs
      */
      if(module_type == RYAT_MODULE_ID_G500) {
        _delay_ms(4000);
      }
      if (RYAT_E_OK == at_engine.method->deactivate()) {
        printf("-----Test: disactive ok\r\n");
      } else {
        printf("-----test: disactive failed\r\n");
      }
  }

  if(module_type != RYAT_MODULE_ID_NBXX_01 &&
     module_type != RYAT_MODULE_ID_U9X07) {
      /*
      * initial the socket
      */
      if (RYAT_E_OK == at_engine.method->socket_init(true)) {
        printf("-----Test: gprs socket initial ok\r\n");
      } else {
        printf("-----Test: gprs socket initial failed\r\n");
      }
  }

  /*
   * set the apn
   */
  if(module_type == RYAT_MODULE_ID_G500) {
    _delay_ms(2000);
  }
  if (RYAT_E_OK == at_engine.method->set_apn("CMIOT", NULL, NULL)) {
    printf("-----Test: set apn ok\r\n");
  } else {
    printf("-----Test: set apn failed\r\n");
  }
  _delay_ms(10000);
  /*
   * active the gprs
   */
  char ipbuf[64] = { 0 };
  if (RYAT_E_OK == at_engine.method->active((uint8_t *)ipbuf, 64)) {
    printf("-----Test: gprs active:%s\r\n", ipbuf);
  } else {
    printf("-----Test: gprs active failed\r\n");
  }

  if(module_type == RYAT_MODULE_ID_L620) {
    /*
    * connect the socket
    */
    if (RYAT_E_OK == at_engine.method->socket_udp("221.229.214.202", 5683, 0)) {
    //if (RYAT_E_OK == ryat_socket_udp("115.196.131.13", 20047, 1)) {
      printf("-----Test: socket udp connect ok\r\n");
    } else {
      printf("-----Test: socket udp connect failed\r\n");
    }
  } else {
    /*
    * connect the socket
    */
    if (RYAT_E_OK == at_engine.method->socket_tcp("sv.openihcs.win", 30000, 1)) {
      printf("-----Test: socket tcp connect ok\r\n");
    } else {
      printf("-----Test: socket tcp connect failed\r\n");
    }
  }
  _delay_ms(1000);
  /*
   * send the data
   */
  if (RYAT_E_OK <= at_engine.method->write_data((uint8_t *)"RUOK", 4, 1)) {
    printf("-----Test: write data ok\r\n");
  } else {
    printf("-----Test: write data failed\r\n");
  }

  _delay_ms(5000);
#if 1
  /*
   * check the read
   */
  char data[120] = { 0 };
  uint16_t remain = 0;
  result = at_engine.method->read_cache(1, (uint8_t *)data, sizeof(data), &remain, 10000);
  if(result >=0 ) {
    printf("-----Test: read data %d %d:%s\r\n", result,remain,data);
  } else {
    printf("-----Test: read data failed:%s\r\n", data);
  }
#endif
  if (RYAT_E_OK == at_engine.method->socket_shut(1)) {
    printf("-----Test: close socket ok\r\n");
  } else {
    printf("-----Test: close socket failed\r\n");
  }
  
  return (0);
}

