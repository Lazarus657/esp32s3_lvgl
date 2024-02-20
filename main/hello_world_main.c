/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "lv_demos.h"
#include "driver/gpio.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define BUTTON 0
#define EC11_GPIO_SCL 6
#define EC11_GPIO_DAT 13

static u_int8_t count=0;
static xQueueHandle gpioEventQueue=NULL;

static void time_out_task(void *arg)
{
    vTaskDelay(200/portTICK_PERIOD_MS);
    count=0;
    vTaskDelete(NULL);
}

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpioEventQueue, &gpio_num, NULL);
}

static void gpio_task_example(void *arg)
{
    uint32_t io_num;
    for(;;){
        if(xQueueReceive(gpioEventQueue,&io_num,portMAX_DELAY))
        {
            if(io_num==EC11_GPIO_SCL)
            {
                if(count==1)
                {
                    count=3;
                    printf("+ turn \n");
                }
                else if(count==0)
                {
                    count=2;
                    xTaskCreate(time_out_task,"time_out_task",2048,NULL,2,NULL);
                }
            }
            else if(io_num==EC11_GPIO_DAT)
            {
                if(count==2)
                {
                    count=3;
                    printf("- turn \n");
                }
                else if(count==0)
                {
                    count=1;
                    xTaskCreate(time_out_task,"time_out_task",2048,NULL,2,NULL);
                }
            }
        }
    }
}
static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        uint32_t id = lv_btnmatrix_get_selected_btn(obj);
        const char * txt = lv_btnmatrix_get_btn_text(obj, id);
 
        LV_LOG_USER("%s was pressed\n", txt);
    }
}
 
 
 
static const char * btnm_map[] = {"1", "2", "3", "4", "5", "\n",
                                  "6", "7", "8", "9", "0", "\n",
                                  "Action1", "Action2", ""
                                 };

void app_main(void)
{
    int mLevel=0,temp=0;
    gpio_set_direction(BUTTON,GPIO_MODE_INPUT);
    gpio_get_level(BUTTON);

    gpio_config_t gpio_6={
        .pin_bit_mask= 1ULL<<EC11_GPIO_SCL,
        .mode=GPIO_MODE_INPUT,
        .intr_type=GPIO_INTR_NEGEDGE,
        .pull_up_en=1,
    };

    gpio_config_t gpio_13={
        .pin_bit_mask= 1ULL<<EC11_GPIO_DAT,
        .mode=GPIO_MODE_INPUT,
        .intr_type=GPIO_INTR_NEGEDGE,
        .pull_up_en=1,
    };
    gpio_config(&gpio_6);
    gpio_config(&gpio_13);
    gpioEventQueue=xQueueCreate(10,sizeof(uint32_t));
    xTaskCreate(gpio_task_example,"gpio_task_example",2048,NULL,1,NULL);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(EC11_GPIO_SCL,gpio_isr_handler,(void *)EC11_GPIO_SCL);
    gpio_isr_handler_add(EC11_GPIO_DAT,gpio_isr_handler,(void *)EC11_GPIO_DAT);

    printf("Hello world!\n");
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();
    lv_group_t *group = lv_group_create();
    lv_indev_set_group(indev_keypad, group);
 
    
    //lv_demo_music();
    // lv_obj_t *scr=lv_disp_get_scr_act(NULL);
    // lv_obj_t *label1=lv_label_create(scr);
    // lv_label_set_text(label1,"hello\nworld");
    // lv_obj_align(label1,LV_ALIGN_CENTER,0,0);
    lv_obj_t * btnm1 = lv_btnmatrix_create(lv_scr_act());
    lv_btnmatrix_set_map(btnm1, btnm_map);
    lv_btnmatrix_set_btn_width(btnm1, 10, 2);        /*Make "Action1" twice as wide as "Action2"*/
    lv_btnmatrix_set_btn_ctrl(btnm1, 10, LV_BTNMATRIX_CTRL_CHECKABLE);
    lv_btnmatrix_set_btn_ctrl(btnm1, 11, LV_BTNMATRIX_CTRL_CHECKED);
    lv_obj_align(btnm1, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(btnm1, event_handler, LV_EVENT_ALL, NULL);
    lv_group_add_obj(group ,btnm1);
    
    while(1) 
    {
        //printf("Hello world!\n");
        lv_task_handler();
        //lvgl心跳
        lv_tick_inc(10);
        // if(mLevel!=(temp=gpio_get_level(BUTTON))){
        //     mLevel=temp;
        //     if(temp){
        //         printf("high level\n");
        //     }else{
        //         printf("low level\n");
        //     }
        // }
        vTaskDelay(10);
    }
}
