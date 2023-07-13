// Settings file for the application.
#ifndef SETTINGS_H
#define SETTINGS_H

    #define PICO_W // Uncomment if using a Pico-W board.
    #define CLOCK_SPEED_KHZ 250000 // Overclocked to 250MHz.

    #define I2S_DATA_PIN 13
    #define I2S_CLOCK_PIN 14

    #define APP_TASK_STACK_SIZE (16*1024) // Increase this if you're getting HardFaults.
    #define APP_TASK_PRIORITY ( tskIDLE_PRIORITY + 3 )
    
    #define USB_TASK_STACK_SIZE ( (3*configMINIMAL_STACK_SIZE/2) * (CFG_TUSB_DEBUG ? 2 : 1) )
    #define USB_TASK_PRIORITY ( tskIDLE_PRIORITY + 2 )

    #define CDC_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE)
    #define CDC_TASK_PRIORITY ( tskIDLE_PRIORITY + 1 )

    /* A combination of interfaces must have a unique product id, since PC will save device driver after the first plug.
    * Same VID/PID with different interface e.g MSC (first), then CDC (later) will possibly cause system error on PC.
    *
    * Auto ProductID layout's Bitmap:
    *   [MSB]         HID | MSC | CDC          [LSB]
    */
    #define _PID_MAP(itf, n)  ( (CFG_TUD_##itf) << (n) )
    #define USB_PID           (0x3300 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | \
                            _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4) )

    #define USB_VID   0xBEEF
    #define USB_BCD   0x0200

#endif // SETTINGS_H
