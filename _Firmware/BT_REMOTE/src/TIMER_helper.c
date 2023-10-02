#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/types.h>

extern struct k_timer shot_timer;
extern void shot_timer_end_cb(struct k_timer *timer_id);
extern void shot_timer_count_cb(struct k_timer *timer_id);


K_TIMER_DEFINE(my_timer, shot_timer_count_cb, shot_timer_end_cb);

void shot_timer_start(int period)
{
    k_timer_start(&shot_timer, K_SECONDS(period), K_SECONDS(period));
}

void shot_timer_stop()
{
    k_timer_stop(&shot_timer);
}