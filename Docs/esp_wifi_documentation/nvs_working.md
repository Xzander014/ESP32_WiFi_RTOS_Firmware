the variable to store the nvs_flash_init() function status is esp_err_t { is a enum ,has 5 different states --esp_ok, esp_err, page not avaliable, memory allocation failed, nvs_new_versoin found}

-- nvs_flash_init() is used to initalise the nvs in the flash memory
-- nvs_flash_erase() removes all the junk which already existed in the flash(used if the nvs_flash_init fails or to just have a fresh start)
-- ESP_ERROR_CHECK(i dont know)
