#if defined(ARDUINO_BLACK_F407VE)
#include "board_stm32F407VE.h"
#include "globals.h"
#include "auxiliaries.h"
#include "idle.h"
#include "scheduler.h"
#if defined(STM32F4)
    #define NR_OFF_TIMERS 9
    //stimer_t HardwareTimers[NR_OFF_TIMERS + 1];
    stimer_t HardwareTimers_1;
    stimer_t HardwareTimers_2;
    stimer_t HardwareTimers_3;
    stimer_t HardwareTimers_4;
    stimer_t HardwareTimers_5;
    stimer_t HardwareTimers_8;
    #define LED_BUILTIN PA7
    //These should really be in the stm32GENERIC libs, but for somereason they only have timers 1-4
//    #include <stm32_TIM_variant_11.h>
//      #include "src/HardwareTimers/HardwareTimer.h"
//    HardwareTimer Timer5(TIM5, chip_tim5, sizeof(chip_tim5) / sizeof(chip_tim5[0]));
//    HardwareTimer Timer8(TIM8, chip_tim8, sizeof(chip_tim8) / sizeof(chip_tim8[0]));
#else
  #include "HardwareTimer.h"
#endif
extern void oneMSIntervalIRQ(stimer_t *Timer){oneMSInterval();}

extern void EmptyIRQCallback(stimer_t *Timer, uint32_t channel){}




void initBoard()
{
    /*
     * Initialize timers
     */

    HardwareTimers_1.timer = TIM1;
    HardwareTimers_2.timer = TIM2;
    HardwareTimers_3.timer = TIM3;
    HardwareTimers_4.timer = TIM4;

    HardwareTimers_5.timer = TIM5;
    HardwareTimers_8.timer = TIM8;
    
 
    /*
    ***********************************************************************************************************
    * General
    */
    #define FLASH_LENGTH 8192

    /*
    ***********************************************************************************************************
    * Idle
    */
    if( (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_OL) || (configPage6.iacAlgorithm == IAC_ALGORITHM_PWM_CL) )
    {
        idle_pwm_max_count = 1000000L / (configPage6.idleFreq * 2); //Converts the frequency in Hz to the number of ticks (at 2uS) it takes to complete 1 cycle. Note that the frequency is divided by 2 coming from TS to allow for up to 5KHz
    } 

    //This must happen at the end of the idle init
    TimerPulseInit(&HardwareTimers_1, 0xFFFF, 0, EmptyIRQCallback);
    //setTimerPrescalerRegister(&HardwareTimers_1, (uint32_t)(getTimerClkFreq(HardwareTimers_1.timer) / (500000)) - 1);
    if(idle_pwm_max_count > 0) { attachIntHandleOC(&HardwareTimers_1, idleInterrupt, 4, 0);} //on first flash the configPage4.iacAlgorithm is invalid
    //Timer1.setMode(4, TIMER_OUTPUT_COMPARE);
    //timer_set_mode(TIMER1, 4, TIMER_OUTPUT_COMPARE;
    //if(idle_pwm_max_count > 0) { Timer1.attachInterrupt(4, idleInterrupt);} //on first flash the configPage4.iacAlgorithm is invalid
    //Timer1.resume();


    /*
    ***********************************************************************************************************
    * Timers
    */
    #if defined(ARDUINO_BLACK_F407VE) || defined(STM32F4) || defined(_STM32F4_)
        TimerHandleInit(&HardwareTimers_8, 1000, 168);
        attachIntHandle(&HardwareTimers_8, oneMSIntervalIRQ);
    #else
        Timer4.setPeriod(1000);  // Set up period
        Timer4.setMode(1, TIMER_OUTPUT_COMPARE);
        Timer4.attachInterrupt(1, oneMSInterval);
        Timer4.resume(); //Start Timer
    #endif
    pinMode(LED_BUILTIN, OUTPUT); //Visual WDT

    /*
    ***********************************************************************************************************
    * Auxilliaries
    */
    //2uS resolution Min 8Hz, Max 5KHz
    boost_pwm_max_count = 1000000L / (2 * configPage6.boostFreq * 2); //Converts the frequency in Hz to the number of ticks (at 2uS) it takes to complete 1 cycle. The x2 is there because the frequency is stored at half value (in a byte) to allow freqneucies up to 511Hz
    vvt_pwm_max_count = 1000000L / (2 * configPage6.vvtFreq * 2); //Converts the frequency in Hz to the number of ticks (at 2uS) it takes to complete 1 cycle

    //Need to be initialised last due to instant interrupt
//    Timer1.setMode(2, TIMER_OUTPUT_COMPARE);
//    Timer1.setMode(3, TIMER_OUTPUT_COMPARE);
//    if(boost_pwm_max_count > 0) { Timer1.attachInterrupt(2, boostInterrupt);}
//    if(vvt_pwm_max_count > 0) { Timer1.attachInterrupt(3, vvtInterrupt);}
      if(idle_pwm_max_count > 0) { attachIntHandleOC(&HardwareTimers_1, boostInterrupt, 2, 0);}
      if(vvt_pwm_max_count > 0) { attachIntHandleOC(&HardwareTimers_1, vvtInterrupt, 3, 0);}
//    Timer1.resume();
      
    TimerPulseInit(&HardwareTimers_3, 0xFFFF, 0, EmptyIRQCallback);
    attachIntHandleOC(&HardwareTimers_3, fuelSchedule1Interrupt, 1, 0);
    attachIntHandleOC(&HardwareTimers_3, fuelSchedule2Interrupt, 2, 0);
    attachIntHandleOC(&HardwareTimers_3, fuelSchedule3Interrupt, 3, 0);
    attachIntHandleOC(&HardwareTimers_3, fuelSchedule4Interrupt, 4, 0);
    

    TimerPulseInit(&HardwareTimers_2, 0xFFFF, 0, EmptyIRQCallback);
    attachIntHandleOC(&HardwareTimers_2, ignitionSchedule1Interrupt, 1, 0);
    attachIntHandleOC(&HardwareTimers_2, ignitionSchedule2Interrupt, 2, 0);
    attachIntHandleOC(&HardwareTimers_2, ignitionSchedule3Interrupt, 3, 0);
    attachIntHandleOC(&HardwareTimers_2, ignitionSchedule4Interrupt, 4, 0);
    
    //Attach interupt functions
    //Injection

    TimerPulseInit(&HardwareTimers_5, 0xFFFF, 0, EmptyIRQCallback);
    //setTimerPrescalerRegister(&HardwareTimers_5, (uint32_t)(getTimerClkFreq(HardwareTimers_5.timer) / (1000000)) - 1);
    #if (INJ_CHANNELS >= 5)
    attachIntHandleOC(&HardwareTimers_5, fuelSchedule5Interrupt, 1, 0);
    //Timer5.attachInterrupt(1, fuelSchedule5Interrupt);
    #endif
    #if (INJ_CHANNELS >= 6)
    attachIntHandleOC(&HardwareTimers_5, fuelSchedule6Interrupt, 2, 0);
    //Timer5.attachInterrupt(2, fuelSchedule6Interrupt);
    #endif
    #if (INJ_CHANNELS >= 7)
    attachIntHandleOC(&HardwareTimers_5, fuelSchedule7Interrupt, 3, 0);
    //Timer5.attachInterrupt(3, fuelSchedule7Interrupt);
    #endif
    #if (INJ_CHANNELS >= 8)
    attachIntHandleOC(&HardwareTimers_5, fuelSchedule8Interrupt, 4, 0);
    //Timer5.attachInterrupt(4, fuelSchedule8Interrupt);
    #endif

    TimerPulseInit(&HardwareTimers_4, 0xFFFF, 0, EmptyIRQCallback);
    //setTimerPrescalerRegister(&HardwareTimers_4, (uint32_t)(getTimerClkFreq(HardwareTimers_4.timer) / (1000000)) - 1);
    #if (IGN_CHANNELS >= 5)
    attachIntHandleOC(&HardwareTimers_4, ignitionSchedule5Interrupt, 1, 0);
    //Timer4.attachInterrupt(1, ignitionSchedule5Interrupt);
    #endif
    #if (IGN_CHANNELS >= 6)
    attachIntHandleOC(&HardwareTimers_4, ignitionSchedule6Interrupt, 2, 0);
    //Timer4.attachInterrupt(2, ignitionSchedule6Interrupt);
    #endif
    #if (IGN_CHANNELS >= 7)
    attachIntHandleOC(&HardwareTimers_4, ignitionSchedule7Interrupt, 3, 0);
    //Timer4.attachInterrupt(3, ignitionSchedule7Interrupt);
    #endif
    #if (IGN_CHANNELS >= 8)
    attachIntHandleOC(&HardwareTimers_4, ignitionSchedule8Interrupt, 4, 0);
    //Timer4.attachInterrupt(4, ignitionSchedule8Interrupt);
    #endif
    
    setTimerPrescalerRegister(&HardwareTimers_2, (uint32_t)(getTimerClkFreq(HardwareTimers_2.timer) / (250000)) - 1);
    setTimerPrescalerRegister(&HardwareTimers_3, (uint32_t)(getTimerClkFreq(HardwareTimers_3.timer) / (250000)) - 1);
    setTimerPrescalerRegister(&HardwareTimers_4, (uint32_t)(getTimerClkFreq(HardwareTimers_4.timer) / (250000)) - 1);
    setTimerPrescalerRegister(&HardwareTimers_5, (uint32_t)(getTimerClkFreq(HardwareTimers_5.timer) / (250000)) - 1);
}

uint16_t freeRam()
{
    char top = 't';
    return &top - reinterpret_cast<char*>(sbrk(0));
}

//pinmapping the STM32F407 for different boards, at this moment no board is desgined.
//All boards are set to the default just to be sure. 
void setPinMapping(byte boardID)
{
  switch (boardID)
  {
    case 0:
      //Pin mappings as per the v0.4 shield

      //Black F407VE http://wiki.stm32duino.com/index.php?title=STM32F407
      //PC8~PC12 SDio

      //PA9..PA10 Serial1
      //PA11..PA12 USB
      //PA13..PA15 & PB4 SWD(debug) pins
      //PB0 EEPROM CS pin
      //PB1 LCD
      //PB2 BOOT1 
      //PB3..PB5 SPI interface
      //PB6..PB8 NRF interface

      //PD5 & PD6 Serial2
      pinInjector1 = PA1; //Output pin injector 1 is on
      pinInjector2 = PA2; //Output pin injector 2 is on
      pinInjector3 = PA3; //Output pin injector 3 is on
      pinInjector4 = PA4; //Output pin injector 4 is on
      pinInjector5 = PA5; //Output pin injector 5 is on
      pinInjector6 = PD4; //Output pin injector 6 is on
      pinCoil1 = PD7; //Pin for coil 1
      pinCoil2 = PB9; //Pin for coil 2
      pinCoil3 = PA8; //Pin for coil 3
      pinCoil4 = PB10; //Pin for coil 4
      pinCoil5 = PB11; //Pin for coil 5
      pinMAP = PC0; //MAP sensor pin
      pinTPS = PC1;//TPS input pin
      pinIAT = PC2; //IAT sensor pin
      pinCLT = PC3; //CLS sensor pin
      pinO2 = PC5; //O2 Sensor pin
      pinBat = PB1; //Battery reference voltage pin
      pinBaro = PC4;
      pinIdle1 = PB12; //Single wire idle control
      pinIdle2 = PB11; //2 wire idle control
      pinBoost = PC7; //Boost control
      pinVVT_1 = PD3; //Default VVT output
      pinStepperDir = PE0; //Direction pin  for DRV8825 driver
      pinStepperStep = PE1; //Step pin for DRV8825 driver
      pinStepperEnable = PE2; //Enable pin for DRV8825
      pinDisplayReset = PE5; // OLED reset pin
      pinFan = PE6; //Pin for the fan output

      pinFuelPump = PA6; //Fuel pump output
      pinTachOut = PD15; //Tacho output pin
      // pinLaunch = 51; //Can be overwritten below
      // pinResetControl = 43; //Reset control output

      //external interrupt enabled pins
      //external interrupts could be enalbed in any pin, except same port numbers (PA4,PE4)
      pinFlex = PD4; // Flex sensor (Must be external interrupt enabled)
      pinTrigger = PD13; //The CAS pin
      pinTrigger2 = PD14; //The Cam Sensor pin
      break;
    case 1:
      //Pin mappings as per the v0.4 shield

      //Black F407VE http://wiki.stm32duino.com/index.php?title=STM32F407
      //PC8~PC12 SDio

      //PA9..PA10 Serial1
      //PA11..PA12 USB
      //PA13..PA15 & PB4 SWD(debug) pins
      //PB0 EEPROM CS pin
      //PB1 LCD
      //PB2 BOOT1 
      //PB3..PB5 SPI interface
      //PB6..PB8 NRF interface

      //PD5 & PD6 Serial2
      pinInjector1 = PA1; //Output pin injector 1 is on
      pinInjector2 = PA2; //Output pin injector 2 is on
      pinInjector3 = PA3; //Output pin injector 3 is on
      pinInjector4 = PA4; //Output pin injector 4 is on
      pinInjector5 = PA5; //Output pin injector 5 is on
      pinInjector6 = PD4; //Output pin injector 6 is on
      pinCoil1 = PD7; //Pin for coil 1
      pinCoil2 = PB9; //Pin for coil 2
      pinCoil3 = PA8; //Pin for coil 3
      pinCoil4 = PB10; //Pin for coil 4
      pinCoil5 = PB11; //Pin for coil 5
      pinMAP = PC0; //MAP sensor pin
      pinTPS = PC1;//TPS input pin
      pinIAT = PC2; //IAT sensor pin
      pinCLT = PC3; //CLS sensor pin
      pinO2 = PC5; //O2 Sensor pin
      pinBat = PB1; //Battery reference voltage pin
      pinBaro = PC4;
      pinIdle1 = PB12; //Single wire idle control
      pinIdle2 = PB11; //2 wire idle control
      pinBoost = PC7; //Boost control
      pinVVT_1 = PD3; //Default VVT output
      pinStepperDir = PE0; //Direction pin  for DRV8825 driver
      pinStepperStep = PE1; //Step pin for DRV8825 driver
      pinStepperEnable = PE2; //Enable pin for DRV8825
      pinDisplayReset = PE5; // OLED reset pin
      pinFan = PE6; //Pin for the fan output

      pinFuelPump = PA6; //Fuel pump output
      pinTachOut = PD15; //Tacho output pin
      // pinLaunch = 51; //Can be overwritten below
      // pinResetControl = 43; //Reset control output

      //external interrupt enabled pins
      //external interrupts could be enalbed in any pin, except same port numbers (PA4,PE4)
      pinFlex = PD4; // Flex sensor (Must be external interrupt enabled)
      pinTrigger = PD13; //The CAS pin
      pinTrigger2 = PD14; //The Cam Sensor pin
      break;
  
    case 2:
      //Pin mappings as per the v0.4 shield

      //Black F407VE http://wiki.stm32duino.com/index.php?title=STM32F407
      //PC8~PC12 SDio

      //PA9..PA10 Serial1
      //PA11..PA12 USB
      //PA13..PA15 & PB4 SWD(debug) pins
      //PB0 EEPROM CS pin
      //PB1 LCD
      //PB2 BOOT1 
      //PB3..PB5 SPI interface
      //PB6..PB8 NRF interface

      //PD5 & PD6 Serial2
      pinInjector1 = PA1; //Output pin injector 1 is on
      pinInjector2 = PA2; //Output pin injector 2 is on
      pinInjector3 = PA3; //Output pin injector 3 is on
      pinInjector4 = PA4; //Output pin injector 4 is on
      pinInjector5 = PA5; //Output pin injector 5 is on
      pinInjector6 = PD4; //Output pin injector 6 is on
      pinCoil1 = PD7; //Pin for coil 1
      pinCoil2 = PB9; //Pin for coil 2
      pinCoil3 = PA8; //Pin for coil 3
      pinCoil4 = PB10; //Pin for coil 4
      pinCoil5 = PB11; //Pin for coil 5
      pinMAP = PC0; //MAP sensor pin
      pinTPS = PC1;//TPS input pin
      pinIAT = PC2; //IAT sensor pin
      pinCLT = PC3; //CLS sensor pin
      pinO2 = PC5; //O2 Sensor pin
      pinBat = PB1; //Battery reference voltage pin
      pinBaro = PC4;
      pinIdle1 = PB12; //Single wire idle control
      pinIdle2 = PB11; //2 wire idle control
      pinBoost = PC7; //Boost control
      pinVVT_1 = PD3; //Default VVT output
      pinStepperDir = PE0; //Direction pin  for DRV8825 driver
      pinStepperStep = PE1; //Step pin for DRV8825 driver
      pinStepperEnable = PE2; //Enable pin for DRV8825
      pinDisplayReset = PE5; // OLED reset pin
      pinFan = PE6; //Pin for the fan output

      pinFuelPump = PA6; //Fuel pump output
      pinTachOut = PD15; //Tacho output pin
      // pinLaunch = 51; //Can be overwritten below
      // pinResetControl = 43; //Reset control output

      //external interrupt enabled pins
      //external interrupts could be enalbed in any pin, except same port numbers (PA4,PE4)
      pinFlex = PD4; // Flex sensor (Must be external interrupt enabled)
      pinTrigger = PD13; //The CAS pin
      pinTrigger2 = PD14; //The Cam Sensor pin
      break;

    case 3:
      //Pin mappings as per the v0.4 shield

      //Black F407VE http://wiki.stm32duino.com/index.php?title=STM32F407
      //PC8~PC12 SDio

      //PA9..PA10 Serial1
      //PA11..PA12 USB
      //PA13..PA15 & PB4 SWD(debug) pins
      //PB0 EEPROM CS pin
      //PB1 LCD
      //PB2 BOOT1 
      //PB3..PB5 SPI interface
      //PB6..PB8 NRF interface

      //PD5 & PD6 Serial2
      pinInjector1 = PA1; //Output pin injector 1 is on
      pinInjector2 = PA2; //Output pin injector 2 is on
      pinInjector3 = PA3; //Output pin injector 3 is on
      pinInjector4 = PA4; //Output pin injector 4 is on
      pinInjector5 = PA5; //Output pin injector 5 is on
      pinInjector6 = PD4; //Output pin injector 6 is on
      pinCoil1 = PD7; //Pin for coil 1
      pinCoil2 = PB9; //Pin for coil 2
      pinCoil3 = PA8; //Pin for coil 3
      pinCoil4 = PB10; //Pin for coil 4
      pinCoil5 = PB11; //Pin for coil 5
      pinMAP = PC0; //MAP sensor pin
      pinTPS = PC1;//TPS input pin
      pinIAT = PC2; //IAT sensor pin
      pinCLT = PC3; //CLS sensor pin
      pinO2 = PC5; //O2 Sensor pin
      pinBat = PB1; //Battery reference voltage pin
      pinBaro = PC4;
      pinIdle1 = PB12; //Single wire idle control
      pinIdle2 = PB11; //2 wire idle control
      pinBoost = PC7; //Boost control
      pinVVT_1 = PD3; //Default VVT output
      pinStepperDir = PE0; //Direction pin  for DRV8825 driver
      pinStepperStep = PE1; //Step pin for DRV8825 driver
      pinStepperEnable = PE2; //Enable pin for DRV8825
      pinDisplayReset = PE5; // OLED reset pin
      pinFan = PE6; //Pin for the fan output

      pinFuelPump = PA6; //Fuel pump output
      pinTachOut = PD15; //Tacho output pin
      // pinLaunch = 51; //Can be overwritten below
      // pinResetControl = 43; //Reset control output

      //external interrupt enabled pins
      //external interrupts could be enalbed in any pin, except same port numbers (PA4,PE4)
      pinFlex = PD4; // Flex sensor (Must be external interrupt enabled)
      pinTrigger = PD13; //The CAS pin
      pinTrigger2 = PD14; //The Cam Sensor pin
      break;

    case 9:
      //Pin mappings as per the MX5 PNP shield
      pinInjector1 = 11; //Output pin injector 1 is on
      pinInjector2 = 10; //Output pin injector 2 is on
      pinInjector3 = 9; //Output pin injector 3 is on
      pinInjector4 = 8; //Output pin injector 4 is on
      pinInjector5 = 14; //Output pin injector 5 is on
      pinCoil1 = 39; //Pin for coil 1
      pinCoil2 = 41; //Pin for coil 2
      pinCoil3 = 32; //Pin for coil 3
      pinCoil4 = 33; //Pin for coil 4
      pinCoil5 = 34; //Pin for coil 5 PLACEHOLDER value for now
      pinTrigger = 19; //The CAS pin
      pinTrigger2 = 18; //The Cam Sensor pin
      pinTPS = A2;//TPS input pin
      pinMAP = A5; //MAP sensor pin
      pinIAT = A0; //IAT sensor pin
      pinCLT = A1; //CLS sensor pin
      pinO2 = A3; //O2 Sensor pin
      pinBat = A4; //Battery reference voltage pin
      pinDisplayReset = 48; // OLED reset pin
      pinTachOut = 49; //Tacho output pin  (Goes to ULN2803)
      pinIdle1 = 2; //Single wire idle control
      pinBoost = 4;
      pinIdle2 = 4; //2 wire idle control (Note this is shared with boost!!!)
      pinFuelPump = 37; //Fuel pump output
      pinStepperDir = 16; //Direction pin  for DRV8825 driver
      pinStepperStep = 17; //Step pin for DRV8825 driver
      pinFan = 35; //Pin for the fan output
      pinLaunch = 12; //Can be overwritten below
      pinFlex = 3; // Flex sensor (Must be external interrupt enabled)
      pinResetControl = 44; //Reset control output

      break;

    case 10:
      //Pin mappings as per the v0.4 shield

      //Black F407VE http://wiki.stm32duino.com/index.php?title=STM32F407
      //PC8~PC12 SDio

      //PA9..PA10 Serial1
      //PA11..PA12 USB
      //PA13..PA15 & PB4 SWD(debug) pins
      //PB0 EEPROM CS pin
      //PB1 LCD
      //PB2 BOOT1 
      //PB3..PB5 SPI interface
      //PB6..PB8 NRF interface

      //PD5 & PD6 Serial2
      pinInjector1 = PA1; //Output pin injector 1 is on
      pinInjector2 = PA2; //Output pin injector 2 is on
      pinInjector3 = PA3; //Output pin injector 3 is on
      pinInjector4 = PA4; //Output pin injector 4 is on
      pinInjector5 = PA5; //Output pin injector 5 is on
      pinInjector6 = PD4; //Output pin injector 6 is on
      pinCoil1 = PD7; //Pin for coil 1
      pinCoil2 = PB9; //Pin for coil 2
      pinCoil3 = PA8; //Pin for coil 3
      pinCoil4 = PB10; //Pin for coil 4
      pinCoil5 = PB11; //Pin for coil 5
      pinMAP = PC0; //MAP sensor pin
      pinTPS = PC1;//TPS input pin
      pinIAT = PC2; //IAT sensor pin
      pinCLT = PC3; //CLS sensor pin
      pinO2 = PC5; //O2 Sensor pin
      pinBat = PB1; //Battery reference voltage pin
      pinBaro = PC4;
      pinIdle1 = PB12; //Single wire idle control
      pinIdle2 = PB11; //2 wire idle control
      pinBoost = PC7; //Boost control
      pinVVT_1 = PD3; //Default VVT output
      pinStepperDir = PE0; //Direction pin  for DRV8825 driver
      pinStepperStep = PE1; //Step pin for DRV8825 driver
      pinStepperEnable = PE2; //Enable pin for DRV8825
      pinDisplayReset = PE5; // OLED reset pin
      pinFan = PE6; //Pin for the fan output

      pinFuelPump = PA6; //Fuel pump output
      pinTachOut = PD15; //Tacho output pin
      // pinLaunch = 51; //Can be overwritten below
      // pinResetControl = 43; //Reset control output

      //external interrupt enabled pins
      //external interrupts could be enalbed in any pin, except same port numbers (PA4,PE4)
      pinFlex = PD4; // Flex sensor (Must be external interrupt enabled)
      pinTrigger = PD13; //The CAS pin
      pinTrigger2 = PD14; //The Cam Sensor pin
      break;

    case 20:
      //Pin mappings as per the v0.4 shield

      //Black F407VE http://wiki.stm32duino.com/index.php?title=STM32F407
      //PC8~PC12 SDio

      //PA9..PA10 Serial1
      //PA11..PA12 USB
      //PA13..PA15 & PB4 SWD(debug) pins
      //PB0 EEPROM CS pin
      //PB1 LCD
      //PB2 BOOT1 
      //PB3..PB5 SPI interface
      //PB6..PB8 NRF interface

      //PD5 & PD6 Serial2
      pinInjector1 = PA1; //Output pin injector 1 is on
      pinInjector2 = PA2; //Output pin injector 2 is on
      pinInjector3 = PA3; //Output pin injector 3 is on
      pinInjector4 = PA4; //Output pin injector 4 is on
      pinInjector5 = PA5; //Output pin injector 5 is on
      pinInjector6 = PD4; //Output pin injector 6 is on
      pinCoil1 = PD7; //Pin for coil 1
      pinCoil2 = PB9; //Pin for coil 2
      pinCoil3 = PA8; //Pin for coil 3
      pinCoil4 = PB10; //Pin for coil 4
      pinCoil5 = PB11; //Pin for coil 5
      pinMAP = PC0; //MAP sensor pin
      pinTPS = PC1;//TPS input pin
      pinIAT = PC2; //IAT sensor pin
      pinCLT = PC3; //CLS sensor pin
      pinO2 = PC5; //O2 Sensor pin
      pinBat = PB1; //Battery reference voltage pin
      pinBaro = PC4;
      pinIdle1 = PB12; //Single wire idle control
      pinIdle2 = PB11; //2 wire idle control
      pinBoost = PC7; //Boost control
      pinVVT_1 = PD3; //Default VVT output
      pinStepperDir = PE0; //Direction pin  for DRV8825 driver
      pinStepperStep = PE1; //Step pin for DRV8825 driver
      pinStepperEnable = PE2; //Enable pin for DRV8825
      pinDisplayReset = PE5; // OLED reset pin
      pinFan = PE6; //Pin for the fan output

      pinFuelPump = PA6; //Fuel pump output
      pinTachOut = PD15; //Tacho output pin
      // pinLaunch = 51; //Can be overwritten below
      // pinResetControl = 43; //Reset control output

      //external interrupt enabled pins
      //external interrupts could be enalbed in any pin, except same port numbers (PA4,PE4)
      pinFlex = PD4; // Flex sensor (Must be external interrupt enabled)
      pinTrigger = PD13; //The CAS pin
      pinTrigger2 = PD14; //The Cam Sensor pin
      break;

    case 30:
      //Pin mappings as per the v0.4 shield

      //Black F407VE http://wiki.stm32duino.com/index.php?title=STM32F407
      //PC8~PC12 SDio

      //PA9..PA10 Serial1
      //PA11..PA12 USB
      //PA13..PA15 & PB4 SWD(debug) pins
      //PB0 EEPROM CS pin
      //PB1 LCD
      //PB2 BOOT1 
      //PB3..PB5 SPI interface
      //PB6..PB8 NRF interface

      //PD5 & PD6 Serial2
      pinInjector1 = PA1; //Output pin injector 1 is on
      pinInjector2 = PA2; //Output pin injector 2 is on
      pinInjector3 = PA3; //Output pin injector 3 is on
      pinInjector4 = PA4; //Output pin injector 4 is on
      pinInjector5 = PA5; //Output pin injector 5 is on
      pinInjector6 = PD4; //Output pin injector 6 is on
      pinCoil1 = PD7; //Pin for coil 1
      pinCoil2 = PB9; //Pin for coil 2
      pinCoil3 = PA8; //Pin for coil 3
      pinCoil4 = PB10; //Pin for coil 4
      pinCoil5 = PB11; //Pin for coil 5
      pinMAP = PC0; //MAP sensor pin
      pinTPS = PC1;//TPS input pin
      pinIAT = PC2; //IAT sensor pin
      pinCLT = PC3; //CLS sensor pin
      pinO2 = PC5; //O2 Sensor pin
      pinBat = PB1; //Battery reference voltage pin
      pinBaro = PC4;
      pinIdle1 = PB12; //Single wire idle control
      pinIdle2 = PB11; //2 wire idle control
      pinBoost = PC7; //Boost control
      pinVVT_1 = PD3; //Default VVT output
      pinStepperDir = PE0; //Direction pin  for DRV8825 driver
      pinStepperStep = PE1; //Step pin for DRV8825 driver
      pinStepperEnable = PE2; //Enable pin for DRV8825
      pinDisplayReset = PE5; // OLED reset pin
      pinFan = PE6; //Pin for the fan output

      pinFuelPump = PA6; //Fuel pump output
      pinTachOut = PD15; //Tacho output pin
      // pinLaunch = 51; //Can be overwritten below
      // pinResetControl = 43; //Reset control output

      //external interrupt enabled pins
      //external interrupts could be enalbed in any pin, except same port numbers (PA4,PE4)
      pinFlex = PD4; // Flex sensor (Must be external interrupt enabled)
      pinTrigger = PD13; //The CAS pin
      pinTrigger2 = PD14; //The Cam Sensor pin
      break;

    case 40:
       //Pin mappings as per the v0.4 shield

      //Black F407VE http://wiki.stm32duino.com/index.php?title=STM32F407
      //PC8~PC12 SDio

      //PA9..PA10 Serial1
      //PA11..PA12 USB
      //PA13..PA15 & PB4 SWD(debug) pins
      //PB0 EEPROM CS pin
      //PB1 LCD
      //PB2 BOOT1 
      //PB3..PB5 SPI interface
      //PB6..PB8 NRF interface

      //PD5 & PD6 Serial2
      pinInjector1 = PA1; //Output pin injector 1 is on
      pinInjector2 = PA2; //Output pin injector 2 is on
      pinInjector3 = PA3; //Output pin injector 3 is on
      pinInjector4 = PA4; //Output pin injector 4 is on
      pinInjector5 = PA5; //Output pin injector 5 is on
      pinInjector6 = PD4; //Output pin injector 6 is on
      pinCoil1 = PD7; //Pin for coil 1
      pinCoil2 = PB9; //Pin for coil 2
      pinCoil3 = PA8; //Pin for coil 3
      pinCoil4 = PB10; //Pin for coil 4
      pinCoil5 = PB11; //Pin for coil 5
      pinMAP = PC0; //MAP sensor pin
      pinTPS = PC1;//TPS input pin
      pinIAT = PC2; //IAT sensor pin
      pinCLT = PC3; //CLS sensor pin
      pinO2 = PC5; //O2 Sensor pin
      pinBat = PB1; //Battery reference voltage pin
      pinBaro = PC4;
      pinIdle1 = PB12; //Single wire idle control
      pinIdle2 = PB11; //2 wire idle control
      pinBoost = PC7; //Boost control
      pinVVT_1 = PD3; //Default VVT output
      pinStepperDir = PE0; //Direction pin  for DRV8825 driver
      pinStepperStep = PE1; //Step pin for DRV8825 driver
      pinStepperEnable = PE2; //Enable pin for DRV8825
      pinDisplayReset = PE5; // OLED reset pin
      pinFan = PE6; //Pin for the fan output

      pinFuelPump = PA6; //Fuel pump output
      pinTachOut = PD15; //Tacho output pin
      // pinLaunch = 51; //Can be overwritten below
      // pinResetControl = 43; //Reset control output

      //external interrupt enabled pins
      //external interrupts could be enalbed in any pin, except same port numbers (PA4,PE4)
      pinFlex = PD4; // Flex sensor (Must be external interrupt enabled)
      pinTrigger = PD13; //The CAS pin
      pinTrigger2 = PD14; //The Cam Sensor pin

    case 41:
      //Pin mappings as per the v0.4 shield

      //Black F407VE http://wiki.stm32duino.com/index.php?title=STM32F407
      //PC8~PC12 SDio

      //PA9..PA10 Serial1
      //PA11..PA12 USB
      //PA13..PA15 & PB4 SWD(debug) pins
      //PB0 EEPROM CS pin
      //PB1 LCD
      //PB2 BOOT1 
      //PB3..PB5 SPI interface
      //PB6..PB8 NRF interface

      //PD5 & PD6 Serial2
      pinInjector1 = PA1; //Output pin injector 1 is on
      pinInjector2 = PA2; //Output pin injector 2 is on
      pinInjector3 = PA3; //Output pin injector 3 is on
      pinInjector4 = PA4; //Output pin injector 4 is on
      pinInjector5 = PA5; //Output pin injector 5 is on
      pinInjector6 = PD4; //Output pin injector 6 is on
      pinCoil1 = PD7; //Pin for coil 1
      pinCoil2 = PB9; //Pin for coil 2
      pinCoil3 = PA8; //Pin for coil 3
      pinCoil4 = PB10; //Pin for coil 4
      pinCoil5 = PB11; //Pin for coil 5
      pinMAP = PC0; //MAP sensor pin
      pinTPS = PC1;//TPS input pin
      pinIAT = PC2; //IAT sensor pin
      pinCLT = PC3; //CLS sensor pin
      pinO2 = PC5; //O2 Sensor pin
      pinBat = PB1; //Battery reference voltage pin
      pinBaro = PC4;
      pinIdle1 = PB12; //Single wire idle control
      pinIdle2 = PB11; //2 wire idle control
      pinBoost = PC7; //Boost control
      pinVVT_1 = PD3; //Default VVT output
      pinStepperDir = PE0; //Direction pin  for DRV8825 driver
      pinStepperStep = PE1; //Step pin for DRV8825 driver
      pinStepperEnable = PE2; //Enable pin for DRV8825
      pinDisplayReset = PE5; // OLED reset pin
      pinFan = PE6; //Pin for the fan output

      pinFuelPump = PA6; //Fuel pump output
      pinTachOut = PD15; //Tacho output pin
      // pinLaunch = 51; //Can be overwritten below
      // pinResetControl = 43; //Reset control output

      //external interrupt enabled pins
      //external interrupts could be enalbed in any pin, except same port numbers (PA4,PE4)
      pinFlex = PD4; // Flex sensor (Must be external interrupt enabled)
      pinTrigger = PD13; //The CAS pin
      pinTrigger2 = PD14; //The Cam Sensor pin
      break;

    default:
      //Pin mappings as per the v0.4 shield

      //Black F407VE http://wiki.stm32duino.com/index.php?title=STM32F407
      //PC8~PC12 SDio

      //PA9..PA10 Serial1
      //PA11..PA12 USB
      //PA13..PA15 & PB4 SWD(debug) pins
      //PB0 EEPROM CS pin
      //PB1 LCD
      //PB2 BOOT1 
      //PB3..PB5 SPI interface
      //PB6..PB8 NRF interface

      //PD5 & PD6 Serial2
      pinInjector1 = PA1; //Output pin injector 1 is on
      pinInjector2 = PA2; //Output pin injector 2 is on
      pinInjector3 = PA3; //Output pin injector 3 is on
      pinInjector4 = PA4; //Output pin injector 4 is on
      pinInjector5 = PA5; //Output pin injector 5 is on
      pinInjector6 = PD4; //Output pin injector 6 is on
      pinCoil1 = PD7; //Pin for coil 1
      pinCoil2 = PB9; //Pin for coil 2
      pinCoil3 = PA8; //Pin for coil 3
      pinCoil4 = PB10; //Pin for coil 4
      pinCoil5 = PB11; //Pin for coil 5
      pinMAP = PC0; //MAP sensor pin
      pinTPS = PC1;//TPS input pin
      pinIAT = PC2; //IAT sensor pin
      pinCLT = PC3; //CLS sensor pin
      pinO2 = PC5; //O2 Sensor pin
      pinBat = PB1; //Battery reference voltage pin
      pinBaro = PC4;
      pinIdle1 = PB12; //Single wire idle control
      pinIdle2 = PB11; //2 wire idle control
      pinBoost = PC7; //Boost control
      pinVVT_1 = PD3; //Default VVT output
      pinStepperDir = PE0; //Direction pin  for DRV8825 driver
      pinStepperStep = PE1; //Step pin for DRV8825 driver
      pinStepperEnable = PE2; //Enable pin for DRV8825
      pinDisplayReset = PE5; // OLED reset pin
      pinFan = PE6; //Pin for the fan output

      pinFuelPump = PA6; //Fuel pump output
      pinTachOut = PD15; //Tacho output pin
      // pinLaunch = 51; //Can be overwritten below
      // pinResetControl = 43; //Reset control output

      //external interrupt enabled pins
      //external interrupts could be enalbed in any pin, except same port numbers (PA4,PE4)
      pinFlex = PD4; // Flex sensor (Must be external interrupt enabled)
      pinTrigger = PD13; //The CAS pin
      pinTrigger2 = PD14; //The Cam Sensor pin
      break;
  }

  //Setup any devices that are using selectable pins

  if ( (configPage6.launchPin != 0) && (configPage6.launchPin < BOARD_NR_GPIO_PINS) ) { pinLaunch = pinTranslate(configPage6.launchPin); }
  if ( (configPage4.ignBypassPin != 0) && (configPage4.ignBypassPin < BOARD_NR_GPIO_PINS) ) { pinIgnBypass = pinTranslate(configPage4.ignBypassPin); }
  if ( (configPage2.tachoPin != 0) && (configPage2.tachoPin < BOARD_NR_GPIO_PINS) ) { pinTachOut = pinTranslate(configPage2.tachoPin); }
  if ( (configPage4.fuelPumpPin != 0) && (configPage4.fuelPumpPin < BOARD_NR_GPIO_PINS) ) { pinFuelPump = pinTranslate(configPage4.fuelPumpPin); }
  if ( (configPage6.fanPin != 0) && (configPage6.fanPin < BOARD_NR_GPIO_PINS) ) { pinFan = pinTranslate(configPage6.fanPin); }
  if ( (configPage6.boostPin != 0) && (configPage6.boostPin < BOARD_NR_GPIO_PINS) ) { pinBoost = pinTranslate(configPage6.boostPin); }
  if ( (configPage6.vvtPin != 0) && (configPage6.vvtPin < BOARD_NR_GPIO_PINS) ) { pinVVT_1 = pinTranslate(configPage6.vvtPin); }
  if ( (configPage6.useExtBaro != 0) && (configPage6.baroPin < BOARD_NR_GPIO_PINS) ) { pinBaro = configPage6.baroPin + A0; }
  if ( (configPage6.useEMAP != 0) && (configPage10.EMAPPin < BOARD_NR_GPIO_PINS) ) { pinEMAP = configPage10.EMAPPin + A0; }

  //Currently there's no default pin for Idle Up
  pinIdleUp = pinTranslate(configPage2.idleUpPin);

  /* Reset control is a special case. If reset control is enabled, it needs its initial state set BEFORE its pinMode.
     If that doesn't happen and reset control is in "Serial Command" mode, the Arduino will end up in a reset loop
     because the control pin will go low as soon as the pinMode is set to OUTPUT. */
  if ( (configPage4.resetControl != 0) && (configPage4.resetControlPin < BOARD_NR_GPIO_PINS) )
  {
    resetControl = configPage4.resetControl;
    pinResetControl = pinTranslate(configPage4.resetControlPin);
    setResetControlPinState();
    pinMode(pinResetControl, OUTPUT);
  }

  //Finally, set the relevant pin modes for outputs
  pinMode(pinCoil1, OUTPUT);
  pinMode(pinCoil2, OUTPUT);
  pinMode(pinCoil3, OUTPUT);
  pinMode(pinCoil4, OUTPUT);
  pinMode(pinCoil5, OUTPUT);
  pinMode(pinInjector1, OUTPUT);
  pinMode(pinInjector2, OUTPUT);
  pinMode(pinInjector3, OUTPUT);
  pinMode(pinInjector4, OUTPUT);
  pinMode(pinInjector5, OUTPUT);
  pinMode(pinTachOut, OUTPUT);
  pinMode(pinIdle1, OUTPUT);
  pinMode(pinIdle2, OUTPUT);
  pinMode(pinFuelPump, OUTPUT);
  pinMode(pinIgnBypass, OUTPUT);
  pinMode(pinFan, OUTPUT);
  pinMode(pinStepperDir, OUTPUT);
  pinMode(pinStepperStep, OUTPUT);
  pinMode(pinStepperEnable, OUTPUT);
  pinMode(pinBoost, OUTPUT);
  pinMode(pinVVT_1, OUTPUT);

  inj1_pin_port = portOutputRegister(digitalPinToPort(pinInjector1));
  inj1_pin_mask = digitalPinToBitMask(pinInjector1);
  inj2_pin_port = portOutputRegister(digitalPinToPort(pinInjector2));
  inj2_pin_mask = digitalPinToBitMask(pinInjector2);
  inj3_pin_port = portOutputRegister(digitalPinToPort(pinInjector3));
  inj3_pin_mask = digitalPinToBitMask(pinInjector3);
  inj4_pin_port = portOutputRegister(digitalPinToPort(pinInjector4));
  inj4_pin_mask = digitalPinToBitMask(pinInjector4);
  inj5_pin_port = portOutputRegister(digitalPinToPort(pinInjector5));
  inj5_pin_mask = digitalPinToBitMask(pinInjector5);
  inj6_pin_port = portOutputRegister(digitalPinToPort(pinInjector6));
  inj6_pin_mask = digitalPinToBitMask(pinInjector6);
  inj7_pin_port = portOutputRegister(digitalPinToPort(pinInjector7));
  inj7_pin_mask = digitalPinToBitMask(pinInjector7);
  inj8_pin_port = portOutputRegister(digitalPinToPort(pinInjector8));
  inj8_pin_mask = digitalPinToBitMask(pinInjector8);

  ign1_pin_port = portOutputRegister(digitalPinToPort(pinCoil1));
  ign1_pin_mask = digitalPinToBitMask(pinCoil1);
  ign2_pin_port = portOutputRegister(digitalPinToPort(pinCoil2));
  ign2_pin_mask = digitalPinToBitMask(pinCoil2);
  ign3_pin_port = portOutputRegister(digitalPinToPort(pinCoil3));
  ign3_pin_mask = digitalPinToBitMask(pinCoil3);
  ign4_pin_port = portOutputRegister(digitalPinToPort(pinCoil4));
  ign4_pin_mask = digitalPinToBitMask(pinCoil4);
  ign5_pin_port = portOutputRegister(digitalPinToPort(pinCoil5));
  ign5_pin_mask = digitalPinToBitMask(pinCoil5);
  ign6_pin_port = portOutputRegister(digitalPinToPort(pinCoil6));
  ign6_pin_mask = digitalPinToBitMask(pinCoil6);
  ign7_pin_port = portOutputRegister(digitalPinToPort(pinCoil7));
  ign7_pin_mask = digitalPinToBitMask(pinCoil7);
  ign8_pin_port = portOutputRegister(digitalPinToPort(pinCoil8));
  ign8_pin_mask = digitalPinToBitMask(pinCoil8);

  tach_pin_port = portOutputRegister(digitalPinToPort(pinTachOut));
  tach_pin_mask = digitalPinToBitMask(pinTachOut);
  pump_pin_port = portOutputRegister(digitalPinToPort(pinFuelPump));
  pump_pin_mask = digitalPinToBitMask(pinFuelPump);

  //And for inputs
  pinMode(pinMAP, INPUT);
  pinMode(pinO2, INPUT);
  pinMode(pinO2_2, INPUT);
  pinMode(pinTPS, INPUT);
  pinMode(pinIAT, INPUT);
  pinMode(pinCLT, INPUT);
  pinMode(pinBat, INPUT);
  pinMode(pinBaro, INPUT);

  pinMode(pinTrigger, INPUT);
  pinMode(pinTrigger2, INPUT);
  pinMode(pinTrigger3, INPUT);

  //Each of the below are only set when their relevant function is enabled. This can help prevent pin conflicts that users aren't aware of with unused functions
  if(configPage2.flexEnabled > 0)
  {
    pinMode(pinFlex, INPUT); //Standard GM / Continental flex sensor requires pullup, but this should be onboard. The internal pullup will not work (Requires ~3.3k)!
  }
  if(configPage6.launchEnabled > 0)
  {
    if (configPage6.lnchPullRes == true) { pinMode(pinLaunch, INPUT_PULLUP); }
    else { pinMode(pinLaunch, INPUT); } //If Launch Pull Resistor is not set make input float.
  }
  if(configPage2.idleUpEnabled > 0)
  {
    if (configPage2.idleUpPolarity == 0) { pinMode(pinIdleUp, INPUT_PULLUP); } //Normal setting
    else { pinMode(pinIdleUp, INPUT); } //inverted setting
  }
  

  //These must come after the above pinMode statements
  triggerPri_pin_port = portInputRegister(digitalPinToPort(pinTrigger));
  triggerPri_pin_mask = digitalPinToBitMask(pinTrigger);
  triggerSec_pin_port = portInputRegister(digitalPinToPort(pinTrigger2));
  triggerSec_pin_mask = digitalPinToBitMask(pinTrigger2);

  //Set default values
  digitalWrite(pinMAP, HIGH);
  //digitalWrite(pinO2, LOW);
  digitalWrite(pinTPS, LOW);

}
#endif