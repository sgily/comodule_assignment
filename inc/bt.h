#ifndef BT_H_
#define BT_H_

#include <stddef.h>

#define ADV_SIZE 26
#define DEVICE_NAME "Kou task beacon"

int start_bt(void);
int update_bt_data(char buf[], size_t length);

#endif