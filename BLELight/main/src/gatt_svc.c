#include "gap.h"
#include "gatt_svc.h"
#include "light.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/util/util.h"
#include "nimble/ble.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"


static const char *TAG = "NimBLE_GATT";


// Function declarations
static int ble_gatt_light_control_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
static int ble_gatt_light_timeout_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
static int ble_gatt_dsc_read_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);

static const ble_uuid128_t custom_uuid1 = BLE_UUID128_INIT(0xe4, 0x0a, 0xf7, 0x12, 0x59, 0xe5, 0x42, 0xd9, 
                                                           0x96, 0x1c, 0xbf, 0x0f, 0xf9, 0xc2, 0x66, 0x57);

static uint16_t light_conn_handle;
static uint16_t light_attr_handle;
static bool light_conn_handle_inited = false;


// GATT services and their characteristics
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        // Light Control Service
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0x1815),
        .characteristics =  (struct ble_gatt_chr_def[]) { 
            {
                .uuid = &custom_uuid1.u,
                .access_cb = ble_gatt_light_control_cb,
                .flags = BLE_GATT_CHR_F_WRITE,
                .val_handle = &light_attr_handle,
                .descriptors = (struct ble_gatt_dsc_def[]) {
                    {
                        .uuid = BLE_UUID16_DECLARE(0x2901), // User Description
                        .access_cb = ble_gatt_dsc_read_cb,
                        .att_flags = BLE_ATT_F_READ,
                        .min_key_size = 0,
                        .arg = "Light Control: Write"
                    },
                    {0,} // No more descriptors
                }
            },
            {
                .uuid = BLE_UUID16_DECLARE(0x2A01),
                .access_cb = ble_gatt_light_timeout_cb,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
            },
            {0,} // No more characteristics 
        }
    },
    {
        // Device Info Service
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0x180A), // Device Information Service
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = BLE_UUID16_DECLARE(0x2A29), // Manufacturer Name
                .access_cb = ble_gatt_light_control_cb,
                .flags = BLE_GATT_CHR_F_READ,
            },
            {0,}// No more characteristics
        }
    },
    {0,} // No more services
};

// Handle lighting commands
static int ble_gatt_light_control_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
        // Copy written data
        uint16_t len = OS_MBUF_PKTLEN(ctxt->om);
        uint8_t message[len + 1]; 
        memset(message, 0, sizeof(message));
        int rc = ble_hs_mbuf_to_flat(ctxt->om, message, len, NULL);
        if (rc != 0) {
            ESP_LOGE(TAG, "Failed to extract data: %d", rc);
            return BLE_ATT_ERR_UNLIKELY;
        }
        ESP_LOGI(TAG, "Received data: %s", message);

        // Toggle LED
        if (strcmp((char *)message, "blue") == 0) {
            blue_toggle();
        } else if (strcmp((char *)message, "white") == 0) {
            white_toggle();
        }

    }
    return 0;
}

void notify_timer_up() {
    if (light_conn_handle_inited) {
        const char *notification = "Timer is up! Turning off lights.";
        struct os_mbuf *om = ble_hs_mbuf_from_flat(notification, strlen(notification));
        int rc = ble_gatts_notify_custom(light_conn_handle, light_attr_handle, om);
        if (rc != 0) {
            ESP_LOGE("BLE_NOTIFY", "Failed to notify client: %d", rc);
        }
        ESP_LOGI(TAG, "User notified that timer is up");
    }  else {
         ESP_LOGI(TAG, "User was not notified that timer is up");
    }

}

// Read time left on timer 
static int ble_gatt_light_timeout_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
        // Send data to be read
        const char *value = "xxx left";
        int rc = os_mbuf_append(ctxt->om, value, strlen(value));
        if (rc != 0) {
            return BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        ESP_LOGI(TAG, "Sent data: %s", value);
    }
    return 0;
}

static int ble_gatt_dsc_read_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    const char *desc = (char *) arg;
    int len = os_mbuf_append(ctxt->om, desc, strlen(desc));
    return (len == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

/*
 *  Handle GATT attribute register events
 *      - Service register event
 *      - Characteristic register event
 *      - Descriptor register event
 */
void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg) {
    /* Local variables */
    char buf[BLE_UUID_STR_LEN];

    /* Handle GATT attributes register events */
    switch (ctxt->op) {

    /* Service register event */
    case BLE_GATT_REGISTER_OP_SVC:
        ESP_LOGD(TAG, "registered service %s with handle=%d",
                 ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                 ctxt->svc.handle);
        break;

    /* Characteristic register event */
    case BLE_GATT_REGISTER_OP_CHR:
        ESP_LOGD(TAG,
                 "registering characteristic %s with "
                 "def_handle=%d val_handle=%d",
                 ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                 ctxt->chr.def_handle, ctxt->chr.val_handle);
        break;

    /* Descriptor register event */
    case BLE_GATT_REGISTER_OP_DSC:
        ESP_LOGD(TAG, "registering descriptor %s with handle=%d",
                 ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                 ctxt->dsc.handle);
        break;

    /* Unknown event */
    default:
        assert(0);
        break;
    }
}

/*
 *  GATT server subscribe event callback
 *      1. Update heart rate subscription status
 */

void gatt_svr_subscribe_cb(struct ble_gap_event *event) {
    /* Check connection handle */
    if (event->subscribe.conn_handle != BLE_HS_CONN_HANDLE_NONE) {
        ESP_LOGI(TAG, "subscribe event; conn_handle=%d attr_handle=%d", event->subscribe.conn_handle, event->subscribe.attr_handle);
    } else {
        ESP_LOGI(TAG, "subscribe by nimble stack; attr_handle=%d", event->subscribe.attr_handle);
    }

    // Subscribed to light timer 
    if (event->subscribe.attr_handle == light_attr_handle) {
        light_conn_handle = event->subscribe.conn_handle;
        light_conn_handle_inited = true;
        ESP_LOGI(TAG, "subscribed to light timer");
        // heart_rate_ind_status = event->subscribe.cur_indicate;
    }
}

/*
 *  GATT server initialization
 *      1. Initialize GATT service
 *      2. Update NimBLE host GATT services counter
 *      3. Add GATT services to server
 */
int gatt_svc_init(void) {
    /* Local variables */
    int rc;

    /* 1. GATT service initialization */
    ble_svc_gatt_init();

    /* 2. Update GATT services counter */
    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    /* 3. Add GATT services */
    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    return 0;
}
