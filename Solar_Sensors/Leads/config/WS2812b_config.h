#ifndef WS2812b_config_H
#define WS2812b_config_H

#define TOTAL_TIME   0x3C                     /** Время передачи(TH+TL=1.25µs ± 600ns). Считается ( 1,25µs / (1 / f_шины ) ) */
#define TIME_0_H     (TOTAL_TIME / 3)         /** HIGH VOLTAGE Передача лог (0) 0.4us ± 150ns */
#define TIME_1_H     ((TOTAL_TIME / 3) * 2 )  /** HIGH VOLTAGE Передача лог (1) 0.8us ± 150ns */
#define TIME_0_L     ((TOTAL_TIME / 3) * 2 )  /** LOW VOLTAGE Передача лог  (0) 0.85us ± 150ns */
#define TIME_1_L     (TOTAL_TIME / 3)         /** LOW VOLTAGE Передача лог  (1) 0.45us ± 150ns */
#define RESET_TIME   (TOTAL_TIME * 50)        /** LOW VOLTAGE Передача (RES) > 50µs */

#endif //WS2812b_config_H
