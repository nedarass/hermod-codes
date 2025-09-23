#include "../../include/sensors/optics.h"
#include "../../include/default_pins.h"
#include "../../include/shared_data.h"

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/sync.h"
#include "hardware/irq.h"
#include <string.h>
#include <stdio.h>

volatile uint16_t dma_buffer[NUM_SAMPLES_PER_CHANNEL * 2];  //sadece dma ve core 0 erisir
static int dma_chan;                                        //mutex gerek yok interrupt durdurma yeterli

void optics_configure_dma_channel()
{
    adc_init(); //start adc

    //sensörlerin takildigi pinleri baslat
    adc_gpio_init(OPTICS_0_PIN);
    adc_gpio_init(OPTICS_1_PIN);

    adc_set_round_robin(0x3);

    adc_fifo_setup(
        true, //FIFO etkin
        true, //DMA DREQ etkin
        1, //her ornek geldiginde
        false, //overrun panik yok
        true //12 bit sola kayar
    );
    adc_fifo_drain();
    adc_run(true);


    //DMA kanali ayarla ADC FIFO dan dma_buffer'a aktarilacak.
    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_chan);

    channel_config_set_transfer_data_size(&c, DMA_SIZE_16); //transferler 16 bit
    channel_config_set_read_increment(&c, false);           //FIFO adresi sabit hep ayni yerden oku
    channel_config_set_write_increment(&c, true);           //hedef buffer artimli
    channel_config_set_dreq(&c, DREQ_ADC);            //ADC dreq sinyali

    dma_channel_configure(
        dma_chan,
        &c,
        (void *)dma_buffer,             //hedef dma_buffer
        (void *)&adc_hw->fifo,          //kaynak adc fifo
        NUM_SAMPLES_PER_CHANNEL * 2,    //transfer uzunlugu
        true                            //basla
    );

    channel_config_set_chain_to(&c, dma_chan); //dma tekrar etsin kendine zincirli
}

//ortalama intensity dondurur. her kanaldan
bool read_optics(optics_sensor_data_t* data)
{
    uint16_t local_dma_buffer_copy[NUM_SAMPLES_PER_CHANNEL * 2];

    uint32_t irq_status = save_and_disable_interrupts();                            //dma calisiyor tutarlilik icin interrupts kapa
    memcpy(local_dma_buffer_copy, (const void *)dma_buffer, sizeof(dma_buffer));
    restore_interrupts(irq_status);

    uint32_t sum0 = 0, sum1 = 0;
    int count0 = 0, count1 = 0;
    for (int i = 0; i < NUM_SAMPLES_PER_CHANNEL * 2; i++) {
        uint16_t sample = local_dma_buffer_copy[i] & 0xfff;
        if ((i & 1) == 0) {  // even / kanal 0
            sum0 += sample;
            count0++;
        } else {             // odd / kanal 1
            sum1 += sample;
            count1++;
        }
    }

    if (count0 == 0 || count1 == 0) return false; //herhangi bir kanal icin basarili sensor okumasi olmayabilir yok

    uint16_t avg0 = sum0 / count0;
    uint16_t avg1 = sum1 / count1;

    data->intensity0 = (avg0 / 4096.f);
    data->intensity1 = (avg1 / 4096.f);

    return true;
}

bool optics_htimer_callback(struct repeating_timer *t)
{
    optics_sensor_data_t sensor_data;

    bool is_read_successfull = read_optics(&sensor_data); //timestamp harici sensor_data doldur

    if (is_read_successfull)
    {
        //basarili
        sensor_data.timestamp = time_us_32();
        buffer_shift_and_update(&optics_mutex, (uint8_t*) optics_sensor_buffer, &sensor_data, sizeof(optics_sensor_data_t));
    }
    else
    {
        //basarisiz
    }

    return true; //timer tekrar tekrar calisir (true)
}
