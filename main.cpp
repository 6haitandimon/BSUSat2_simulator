//#include "PCA9554.h"
#include "mcp2515.h"
//#include "ina3221.h"
//#include "INA219.h"
#include "pico/stdlib.h"


int main() {
  stdio_init_all();
  MCP2515 can0 = MCP2515(spi0, 17, 19, 16, 18);

  struct can_frame tx;

  tx.can_id = 10;
  tx.can_dlc = 5;
  tx.data[0] = 1;
  tx.data[1] = 1;
  tx.data[2] = 1;
  tx.data[3] = 1;
  tx.data[4] = 1;

  can0.reset();
  can0.setBitrate(CAN_1000KBPS, MCP_8MHZ);
  can0.setNormalMode();

  while(true){
    can0.sendMessage(&tx);
    printf("send");

    sleep_ms(2000);

  }

  return 0;
}
