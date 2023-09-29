#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/types.h>

struct k_timer my_timer;
void my_expiry_function(struct k_timer *timer_id);

K_TIMER_DEFINE(my_timer, my_expiry_function, NULL);

void my_expiry_function(struct k_timer *timer_id)
{

 printk("Timer FINISHED\n");

}

void timer_init(int period){
    k_timer_start(&my_timer, K_SECONDS(1), K_SECONDS(1));
}