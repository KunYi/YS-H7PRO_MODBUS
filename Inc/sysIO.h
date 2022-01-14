#ifndef _SYS_IO_H_
#define _SYS_IO_H_

#include <stdint.h>

struct bitIn {
  uint8_t  X0: 1;
  uint8_t  X1: 1;
  uint8_t  X2: 1;
  uint8_t  X3: 1;
};

union INPUT {
    struct bitIn in;
    uint8_t _in;
};

struct bitOut {
  uint16_t Y0: 1;
  uint16_t Y1: 1;
  uint16_t Y2: 1;
  uint16_t Y3: 1;
  uint16_t Y4: 1;
  uint16_t Y5: 1;
  uint16_t Y6: 1;
  uint16_t Y7: 1;
  uint16_t Y8: 1;
  uint16_t Y9: 1;
  uint16_t Y10: 1;
  uint16_t Y11: 1;
  /*
  uint16_t Y12: 1;
  uint16_t Y13: 1;
  uint16_t Y14: 1;
  uint16_t Y15: 1;
  */
};

union OUTPUT {
  struct bitOut out;
  uint16_t _out;
};

enum _DIGITAL_OUTPUT_ {
  NUM_DOO = 0,
  NUM_DO1 = 1,
  NUM_DO2 = 2,
  NUM_DO3 = 3,
  NUM_DO4 = 4,
  NUM_DO5 = 5,
  NUM_DO6 = 6,
  NUM_DO7 = 7,
  NUM_DO8 = 8,
  NUM_DO9 = 9,
  NUM_DO10 = 10,
  NUM_DO11 = 11,
  /*
  NUM_DO12 = 12,
  NUM_DO13 = 13,
  NUM_DO14 = 14,
  NUM_DO15 = 15,
  */
  MAX_NUM_DO
};

extern volatile union INPUT sysIn;
extern volatile union OUTPUT sysOut;

#define DO0  sysOut.out.Y0
#define DO1  sysOut.out.Y1
#define DO2  sysOut.out.Y2
#define DO3  sysOut.out.Y3
#define DO4  sysOut.out.Y4
#define DO5  sysOut.out.Y5
#define DO6  sysOut.out.Y6
#define DO7  sysOut.out.Y7
#define DO8  sysOut.out.Y8
#define DO9  sysOut.out.Y9
#define DO10 sysOut.out.Y10
#define DO11 sysOut.out.Y11
/*
#define DO12 sysOut.out.Y12
#define DO13 sysOut.out.Y13
#define DO14 sysOut.out.Y14
#define DO15 sysOut.out.Y15
*/

#define DI0  sysIn.in.X0
#define DI1  sysIn.in.X1
#define DI2  sysIn.in.X2
#define DI3  sysIn.in.X3

#define VALVE_ON_CTRL         DO0
#define TURN_ON_VALVE         (1)
#define TURN_OFF_VALVE        (0)

#define NBIOT_ON_CTRL         DO1
#define TURN_ON_NBIOT         (1)
#define TURN_OFF_NBIOT        (0)

#define PUMP_ON_CTRL          DO2
#define TURN_ON_PUMP          (1)
#define TURN_OFF_PUMP         (0)

#define ELECTRODE_ON_CTRL     (DO3)
#define TURN_ON_ELECTRODE     (1)
#define TURN_OFF_ELECTRODE    (0)

#define FLOW_STATUS           DI0   /* water flow status */
#define FLOW_ACTIVE_STATUS    (1)
#define FLOW_INACTIVE_STATUS  (0)

#define PUMP_STATUS           DI1  /* pump active status */
#define PUMP_ACTIVE_STATUS    (1)
#define PUMP_INACTIVE_STATUS  (0)


void initSysIO(void);
void sysSetOutput(uint8_t num);
void sysTurnOnTower(uint8_t num);
void sysTurnOffTower(uint8_t num);
#endif /* end of file, _SYS_IO_H_ */
