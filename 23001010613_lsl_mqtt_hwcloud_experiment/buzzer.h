#ifndef BUZZER_H
#define BUZZER_H

void buzzer_pwm_init(void);
void buzzer_pwm_start(int duty);
void buzzer_pwm_start1(int duty, int freq);
void buzzer_pwm_stop(void);
void buzzer_gpio_init(void);
void buzzer_gpio_start(void);
void buzzer_gpio_stop(void);

#endif
