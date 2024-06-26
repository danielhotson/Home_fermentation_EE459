#ifndef RTC_H
#define RTC_H

void rtc_init(void);
void rtc_load(unsigned char seconds, unsigned char minutes, unsigned char hours, unsigned char days);
void start_timer(void);
uint8_t rtc_read_seconds(void);
uint8_t rtc_read_minutes(void);
uint8_t rtc_read_hours(void);
uint8_t rtc_read_days(void);
int bcd_to_decimal(unsigned char x);

#endif