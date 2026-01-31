
project-1 location = kartix@fedora:~/Projects/esp/esp-idf/projects/blinky$ 

idf.py set-target esp32
idf.py build //  to create the project

skdconfig file is created containing all the parameters

files required for flashing{
build/app.bin //for flashing to the board
build/app.elf // contains the symbols for debugging
build/bootloader  //is a second stage bootloader that contains a bootloader.bin file which tells the processor where to go to run our application

build/bootloader/partiton_table/partition-table.bin // contains memory information}

static const gpio_num_t led = gpi_num_2 -- default blue led on the board

for specifing sleep time/delay{
sleep_time = 1000
portTICK_PERIOD_MS // defines the internal clock tick 


for time -- sleep_time/portTICK_PERIOD_MS
vTaskDelay(time) // in loop where time is mathematically equal to the sleep_time in ms

}

gpio_set_level(led_pin, led_state) //to set the level of the led_pin to -- High, --Low

for printing statements{
ESP_LOGE;
ESP_LOGW;
 and much more
 }