#include <bt.h>
#include <bluetooth/bluetooth.h>

//advertising data struct
static struct bt_data adv[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
	BT_DATA_BYTES(BT_DATA_MANUFACTURER_DATA,
		0x0f, 0x02) /* Comodule GmbH */
};

// bluetooth setup callback
static void bt_ready(int err)
{
	if (err) {
			printk("Bluetooth init failed (err %d)\n", err);
			return;
	}
	printk("Bluetooth initialized\n");
	/* Start advertising */
	err = bt_le_adv_start(BT_LE_ADV_NCONN_NAME, adv, ARRAY_SIZE(adv),
							NULL, 0);
	if (err) {
			printk("Advertising failed to start (err %d)\n", err);
			return;
	}
}

int start_bt(void) {
    return bt_enable(bt_ready);
}

int update_bt_data(char buf[], size_t length) {
    adv[1].data = buf;
	adv[1].data_len = length;
	return bt_le_adv_update_data(adv, ARRAY_SIZE(adv),NULL, 0);
}