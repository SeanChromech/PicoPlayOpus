/**
 * PicoPlayOpus
 * A simple Opus player for Raspberry Pi Pico.
 * This is mostly intended as a starting point for other projects.
 */

#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"
#include "tusb.h"

#include "settings.h"

#include "pico/audio_i2s.h"
#include "ogg_stripper.h"
#include "ogg_data.h"
#include "opus.h"

#ifdef PICO_W
    #include "pico/cyw43_arch.h"
#endif

#define SAMPLES_PER_BUFFER 1920 // See the comment for opus_decode.  This is 120ms of audio at 16kHz.  I've used less
                                // than this in the past and it was fine.
#define OGG_BUF_LEN 0xFF

// This is the audio init structure.  It's used to set up the audio device.
// This is taken pretty verbatim from the Pico Audio example.
struct audio_buffer_pool *init_audio() {
    static audio_format_t audio_format = {
            .format = AUDIO_BUFFER_FORMAT_PCM_S16,
            .sample_freq = 16000,
            .channel_count = 1,
    };

    static struct audio_buffer_format producer_format = {
            .format = &audio_format,
            .sample_stride = 2
    };

    struct audio_buffer_pool *producer_pool = audio_new_producer_pool(&producer_format, 3, SAMPLES_PER_BUFFER);
    bool __unused ok;
    const struct audio_format *output_format;

    struct audio_i2s_config config = {
            .data_pin = I2S_DATA_PIN,
            .clock_pin_base = I2S_CLOCK_PIN,
            .dma_channel = 0,
            .pio_sm = 0,
    };

    output_format = audio_i2s_setup(&audio_format, &config);
    if (!output_format) {
        panic("PicoAudio: Unable to open audio device.\n");
    }

    ok = audio_i2s_connect(producer_pool);
    assert(ok);
    audio_i2s_set_enabled(true);
    return producer_pool;
}

// Declare the FreeRTOS tasks.
static TaskHandle_t appTaskHandle;
static void App_Task(void * argument);

static TaskHandle_t usbTaskHandle;
static void USB_Task(void * argument);

static TaskHandle_t cdcTaskHandle;
static void CDC_Task(void * argument);

// Set the clock speed, then init the tasks.
void App_Init(void) {
#if CLOCK_SPEED_KHZ != 133000
    set_sys_clock_khz(CLOCK_SPEED_KHZ, true);
#endif

    xTaskCreate( App_Task,             /* The function that implements the task. */
                 "App",                /* The text name assigned to the task - for debug only as it is not used by the kernel. */
                 APP_TASK_STACK_SIZE,  /* The size of the stack to allocate to the task. */
                 NULL,                 /* The parameter passed to the task - not used in this case. */
                 APP_TASK_PRIORITY,    /* The priority assigned to the task. */
                 &appTaskHandle );     /* The task handle is not required, so NULL is passed. */

    xTaskCreate( USB_Task,
                 "USB",
                 USB_TASK_STACK_SIZE,
                 NULL,
                 USB_TASK_PRIORITY,
                 &usbTaskHandle );

    xTaskCreate( CDC_Task,
                 "CDC",
                 CDC_TASK_STACK_SIZE,
                 NULL,
                 CDC_TASK_PRIORITY,
                 &cdcTaskHandle );

}

// This is the main task.  It's responsible for blinking the LED and playing the audio.
static void App_Task(void * argument) {
    (void) argument;  // Unused parameter
    absolute_time_t nextBlink = make_timeout_time_ms(500);
    bool blinkState = true;
    size_t bytesWritten = 0;
    int samplesDecoded = 0;
    int packetsDecoded = 0;
    int decoderError = 0;

#ifdef PICO_W
    cyw43_arch_init();
#else
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
#endif

    struct audio_buffer_pool *ap = init_audio();
    audio_buffer_t *buffer;
    OpusDecoder *decoder = opus_decoder_create(16000, 1, &decoderError);
    uint8_t oggBuf[OGG_BUF_LEN];
    int32_t oggBufBytes = 0;

    OggSetSource(Sample, SAMPLE_LENGTH);
    bool valid = OggPrepareFile();

    vTaskDelay(1000);

    while (1) {
        if (valid) {
            buffer = take_audio_buffer(ap, true);

            oggBufBytes = OggGetNextPacket(oggBuf, OGG_BUF_LEN);
            if (oggBufBytes < 1) {
                printf("Done!\r\n");
                buffer->sample_count = 0;
                valid = false;
            } else {
                buffer->sample_count = opus_decode(decoder, oggBuf, oggBufBytes,
                                                   (int16_t *) buffer->buffer->bytes,
                                                   (int) buffer->max_sample_count, 0);
            }

            give_audio_buffer(ap, buffer);
        }

        if ( to_us_since_boot(nextBlink) < to_us_since_boot( get_absolute_time() ) ) {
            uint32_t lastCore = get_core_num();
            printf("Hello from core %d!\n", lastCore);
#ifdef PICO_W
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, blinkState);
#else
            gpio_put(LED_PIN, !blinkState);
#endif
            blinkState = !blinkState;
            nextBlink = make_timeout_time_ms(500);
        }
        vTaskDelay(1);
    }
}

// This is the USB task.  It's responsible for handling USB events.
static void USB_Task(void * argument) {
    (void) argument;  // Unused parameter

    tud_init(BOARD_TUD_RHPORT);
    stdio_init_all(); // This must be called after tud_init(), otherwise it will hang.

    while (1) {
        tud_task(); // device task
        tud_cdc_write_flush();
    }   
}

// This is the CDC task.  It's responsible for handling CDC events.
static void CDC_Task(void * argument) {
    (void) argument;  // Unused parameter

    while (1) {
        // There are data available
        while ( tud_cdc_available() ) {
            uint8_t buf[64];

            // read and echo back
            uint32_t count = tud_cdc_read(buf, sizeof(buf));
            tud_cdc_write(buf, count);
        }

        tud_cdc_write_flush();
        vTaskDelay(1);
    }
}

// This is the entry point for the program.
// Call init, then start the scheduler to run the tasks.
int main() {
    App_Init();

    vTaskStartScheduler();
    /* If all is well, the scheduler will now be running, and the following
    line will never be reached.  If the following line does execute, then
    there was insufficient FreeRTOS heap memory available for the Idle and/or
    timer tasks to be created.  See the memory management section on the
    FreeRTOS web site for more details on the FreeRTOS heap
    http://www.freertos.org/a00111.html. */
    for( ;; );
}
