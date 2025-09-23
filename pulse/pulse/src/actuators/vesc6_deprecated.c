#include "../../include/actuators/vesc6.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

volatile uint slice1, slice2, slice3, slice4;
volatile uint chan1, chan2, chan3, chan4;

static float pwm_freq = 20000.0f;
static float clk_div = 1.0f;

void vesc6_configure()
{
    gpio_set_function(MOTOR0_PIN, GPIO_FUNC_PWM);
    gpio_set_function(MOTOR1_PIN, GPIO_FUNC_PWM);
    gpio_set_function(MOTOR2_PIN, GPIO_FUNC_PWM);
    gpio_set_function(MOTOR3_PIN, GPIO_FUNC_PWM);


    // her cikis icin bir slice
    slice1 = pwm_gpio_to_slice_num(MOTOR0_PIN);
    chan1  = pwm_gpio_to_channel(MOTOR0_PIN);

    slice2 = pwm_gpio_to_slice_num(MOTOR1_PIN);
    chan2  = pwm_gpio_to_channel(MOTOR1_PIN);

    slice3 = pwm_gpio_to_slice_num(MOTOR2_PIN);
    chan3  = pwm_gpio_to_channel(MOTOR2_PIN);

    slice4 = pwm_gpio_to_slice_num(MOTOR3_PIN);
    chan4  = pwm_gpio_to_channel(MOTOR3_PIN);

    // pwm frekans ve clock divider
    // f_pwm(pwm frekansi) = f_clk / ((wrap + 1) *clk_div)
    // f_pwm = 20 kHz clk_div = 1.0f
    uint32_t sys_clk = clock_get_hz(clk_sys);
    uint16_t wrap = (uint16_t)((sys_clk / (pwm_freq * clk_div)) - 1);

    // her slice icin ayarlari yap
    pwm_set_clkdiv(slice1, clk_div);
    pwm_set_wrap(slice1, wrap);

    pwm_set_clkdiv(slice2, clk_div);
    pwm_set_wrap(slice2, wrap);

    pwm_set_clkdiv(slice3, clk_div);
    pwm_set_wrap(slice3, wrap);

    pwm_set_clkdiv(slice4, clk_div);
    pwm_set_wrap(slice4, wrap);

    // baslangicta calismaz
    uint16_t level = 0; //wrap / 2;
    pwm_set_chan_level(slice1, chan1, level);
    pwm_set_chan_level(slice2, chan2, level);
    pwm_set_chan_level(slice3, chan3, level);
    pwm_set_chan_level(slice4, chan4, level);

    // slice etkinlestir
    pwm_set_enabled(slice1, true);
    pwm_set_enabled(slice2, true);
    pwm_set_enabled(slice3, true);
    pwm_set_enabled(slice4, true);
}

void set_motor_duty_cycle(uint slice, uint channel, uint16_t level)
{
    pwm_set_chan_level(slice1, chan1, level);
}
