// Settings file for the application.
#ifndef SETTINGS_H
#define SETTINGS_H

    #define CLOCK_SPEED_KHZ 270000 // Overclocked to 270MHz.

    #define APP_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE)
    #define APP_TASK_PRIORITY ( tskIDLE_PRIORITY + 3 )
    
    #define USB_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE)
    #define USB_TASK_PRIORITY ( tskIDLE_PRIORITY + 2 )

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