#include <stdint.h>
#include <string.h> //memcpy
#include "sdp.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <errno.h>

/*
 * A real SDP implementation is totally unecessary for this purpose.
 * The Wii simply checks for the Wiimote service, verifies its attributes, and
 * moves on. This bare minimum implementation provides the expected responses.
 */

static int state = -1;
static uint32_t sdp_record_handle;
static const uint32_t wiimote_hid_record_handle = 0x10000;

static const uint8_t resp0[14] = 
{ 0x03, 0x00, 0x00, 0x00, 0x09, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00 };

static const uint8_t resp1[128] = 
{
    0x05, 0x00, 0x01, 0x00, 0x7B, 0x00, 0x76, 0x36, 0x01, 0xCC, 0x09, 0x00, 0x00, 
    0x0A, 0x00, 0x01, 0x00, 0x00, 0x09, 0x00, 0x01, 0x35, 0x03, 0x19, 0x11, 0x24,
    0x09, 0x00, 0x04, 0x35, 0x0D, 0x35, 0x06, 0x19, 0x01, 0x00, 0x09, 0x00, 0x11, 
    0x35, 0x03, 0x19, 0x00, 0x11, 0x09, 0x00, 0x05, 0x35, 0x03, 0x19, 0x10, 0x02,
    0x09, 0x00, 0x06, 0x35, 0x09, 0x09, 0x65, 0x6E, 0x09, 0x00, 0x6A, 0x09, 0x01,
    0x00, 0x09, 0x00, 0x09, 0x35, 0x08, 0x35, 0x06, 0x19, 0x11, 0x24, 0x09, 0x01,
    0x00, 0x09, 0x00, 0x0D, 0x35, 0x0F, 0x35, 0x0D, 0x35, 0x06, 0x19, 0x01, 0x00,
    0x09, 0x00, 0x13, 0x35, 0x03, 0x19, 0x00, 0x11, 0x09, 0x01, 0x00, 0x25, 0x13,
    0x4E, 0x69, 0x6E, 0x74, 0x65, 0x6E, 0x64, 0x6F, 0x20, 0x52, 0x56, 0x4C, 0x2D,
    0x43, 0x4E, 0x54, 0x2D, 0x30, 0x31, 0x09, 0x01, 0x02, 0x00, 0x76
};

static const uint8_t resp2[128] = 
{
    0x05, 0x00, 0x02, 0x00, 0x7B, 0x00, 0x76, 0x01, 0x25, 0x13, 0x4E, 0x69, 0x6E, 
    0x74, 0x65, 0x6E, 0x64, 0x6F, 0x20, 0x52, 0x56, 0x4C, 0x2D, 0x43, 0x4E, 0x54, 
    0x2D, 0x30, 0x31, 0x09, 0x01, 0x02, 0x25, 0x08, 0x4E, 0x69, 0x6E, 0x74, 0x65,
    0x6E, 0x64, 0x6F, 0x09, 0x02, 0x00, 0x09, 0x01, 0x00, 0x09, 0x02, 0x01, 0x09,
    0x01, 0x11, 0x09, 0x02, 0x02, 0x08, 0x04, 0x09, 0x02, 0x03, 0x08, 0x33, 0x09,
    0x02, 0x04, 0x28, 0x00, 0x09, 0x02, 0x05, 0x28, 0x01, 0x09, 0x02, 0x06, 0x35,
    0xDF, 0x35, 0xDD, 0x08, 0x22, 0x25, 0xD9, 0x05, 0x01, 0x09, 0x05, 0xA1, 0x01,
    0x85, 0x10, 0x15, 0x00, 0x26, 0xFF, 0x00, 0x75, 0x08, 0x95, 0x01, 0x06, 0x00,
    0xFF, 0x09, 0x01, 0x91, 0x00, 0x85, 0x11, 0x95, 0x01, 0x09, 0x01, 0x91, 0x00,
    0x85, 0x12, 0x95, 0x02, 0x09, 0x01, 0x91, 0x00, 0x02, 0x00, 0xEC
};

static const uint8_t resp3[128] = 
{
    0x05, 0x00, 0x03, 0x00, 0x7B, 0x00, 0x76, 0x85, 0x13, 0x95, 0x01, 0x09, 0x01,
    0x91, 0x00, 0x85, 0x14, 0x95, 0x01, 0x09, 0x01, 0x91, 0x00, 0x85, 0x15, 0x95,
    0x01, 0x09, 0x01, 0x91, 0x00, 0x85, 0x16, 0x95, 0x15, 0x09, 0x01, 0x91, 0x00,
    0x85, 0x17, 0x95, 0x06, 0x09, 0x01, 0x91, 0x00, 0x85, 0x18, 0x95, 0x15, 0x09,
    0x01, 0x91, 0x00, 0x85, 0x19, 0x95, 0x01, 0x09, 0x01, 0x91, 0x00, 0x85, 0x1A,
    0x95, 0x01, 0x09, 0x01, 0x91, 0x00, 0x85, 0x20, 0x95, 0x06, 0x09, 0x01, 0x81,
    0x00, 0x85, 0x21, 0x95, 0x15, 0x09, 0x01, 0x81, 0x00, 0x85, 0x22, 0x95, 0x04,
    0x09, 0x01, 0x81, 0x00, 0x85, 0x30, 0x95, 0x02, 0x09, 0x01, 0x81, 0x00, 0x85,
    0x31, 0x95, 0x05, 0x09, 0x01, 0x81, 0x00, 0x85, 0x32, 0x95, 0x0A, 0x09, 0x01,
    0x81, 0x00, 0x85, 0x33, 0x95, 0x11, 0x09, 0x01, 0x02, 0x01, 0x62
};

static const uint8_t resp4[117] = 
{ 
    0x05, 0x00, 0x04, 0x00, 0x70, 0x00, 0x6D, 0x81, 0x00, 0x85, 0x34, 0x95, 0x15, 
    0x09, 0x01, 0x81, 0x00, 0x85, 0x35, 0x95, 0x15, 0x09, 0x01, 0x81, 0x00, 0x85,
    0x36, 0x95, 0x15, 0x09, 0x01, 0x81, 0x00, 0x85, 0x37, 0x95, 0x15, 0x09, 0x01,
    0x81, 0x00, 0x85, 0x3D, 0x95, 0x15, 0x09, 0x01, 0x81, 0x00, 0x85, 0x3E, 0x95,
    0x15, 0x09, 0x01, 0x81, 0x00, 0x85, 0x3F, 0x95, 0x15, 0x09, 0x01, 0x81, 0x00,
    0xC0, 0x09, 0x02, 0x07, 0x35, 0x08, 0x35, 0x06, 0x09, 0x04, 0x09, 0x09, 0x01,
    0x00, 0x09, 0x02, 0x08, 0x28, 0x00, 0x09, 0x02, 0x09, 0x28, 0x01, 0x09, 0x02,
    0x0A, 0x28, 0x01, 0x09, 0x02, 0x0B, 0x09, 0x01, 0x00, 0x09, 0x02, 0x0C, 0x09,
    0x0C, 0x80, 0x09, 0x02, 0x0D, 0x28, 0x00, 0x09, 0x02, 0x0E, 0x28, 0x00, 0x00
};

static const uint8_t attributes[463] =
{
    0x36, 0x01, 0xCC, 0x09, 0x00, 0x00, 
    0x0A, 0x00, 0x01, 0x00, 0x00, 0x09, 0x00, 0x01, 0x35, 0x03, 0x19, 0x11, 0x24,
    0x09, 0x00, 0x04, 0x35, 0x0D, 0x35, 0x06, 0x19, 0x01, 0x00, 0x09, 0x00, 0x11, 
    0x35, 0x03, 0x19, 0x00, 0x11, 0x09, 0x00, 0x05, 0x35, 0x03, 0x19, 0x10, 0x02,
    0x09, 0x00, 0x06, 0x35, 0x09, 0x09, 0x65, 0x6E, 0x09, 0x00, 0x6A, 0x09, 0x01,
    0x00, 0x09, 0x00, 0x09, 0x35, 0x08, 0x35, 0x06, 0x19, 0x11, 0x24, 0x09, 0x01,
    0x00, 0x09, 0x00, 0x0D, 0x35, 0x0F, 0x35, 0x0D, 0x35, 0x06, 0x19, 0x01, 0x00,
    0x09, 0x00, 0x13, 0x35, 0x03, 0x19, 0x00, 0x11, 0x09, 0x01, 0x00, 0x25, 0x13,
    0x4E, 0x69, 0x6E, 0x74, 0x65, 0x6E, 0x64, 0x6F, 0x20, 0x52, 0x56, 0x4C, 0x2D,
    0x43, 0x4E, 0x54, 0x2D, 0x30, 0x31, 0x09, 0x01,
    0x01, 0x25, 0x13, 0x4E, 0x69, 0x6E, 
    0x74, 0x65, 0x6E, 0x64, 0x6F, 0x20, 0x52, 0x56, 0x4C, 0x2D, 0x43, 0x4E, 0x54, 
    0x2D, 0x30, 0x31, 0x09, 0x01, 0x02, 0x25, 0x08, 0x4E, 0x69, 0x6E, 0x74, 0x65,
    0x6E, 0x64, 0x6F, 0x09, 0x02, 0x00, 0x09, 0x01, 0x00, 0x09, 0x02, 0x01, 0x09,
    0x01, 0x11, 0x09, 0x02, 0x02, 0x08, 0x04, 0x09, 0x02, 0x03, 0x08, 0x33, 0x09,
    0x02, 0x04, 0x28, 0x00, 0x09, 0x02, 0x05, 0x28, 0x01, 0x09, 0x02, 0x06, 0x35,
    0xDF, 0x35, 0xDD, 0x08, 0x22, 0x25, 0xD9, 0x05, 0x01, 0x09, 0x05, 0xA1, 0x01,
    0x85, 0x10, 0x15, 0x00, 0x26, 0xFF, 0x00, 0x75, 0x08, 0x95, 0x01, 0x06, 0x00,
    0xFF, 0x09, 0x01, 0x91, 0x00, 0x85, 0x11, 0x95, 0x01, 0x09, 0x01, 0x91, 0x00,
    0x85, 0x12, 0x95, 0x02, 0x09, 0x01, 0x91, 0x00,
    0x85, 0x13, 0x95, 0x01, 0x09, 0x01,
    0x91, 0x00, 0x85, 0x14, 0x95, 0x01, 0x09, 0x01, 0x91, 0x00, 0x85, 0x15, 0x95,
    0x01, 0x09, 0x01, 0x91, 0x00, 0x85, 0x16, 0x95, 0x15, 0x09, 0x01, 0x91, 0x00,
    0x85, 0x17, 0x95, 0x06, 0x09, 0x01, 0x91, 0x00, 0x85, 0x18, 0x95, 0x15, 0x09,
    0x01, 0x91, 0x00, 0x85, 0x19, 0x95, 0x01, 0x09, 0x01, 0x91, 0x00, 0x85, 0x1A,
    0x95, 0x01, 0x09, 0x01, 0x91, 0x00, 0x85, 0x20, 0x95, 0x06, 0x09, 0x01, 0x81,
    0x00, 0x85, 0x21, 0x95, 0x15, 0x09, 0x01, 0x81, 0x00, 0x85, 0x22, 0x95, 0x04,
    0x09, 0x01, 0x81, 0x00, 0x85, 0x30, 0x95, 0x02, 0x09, 0x01, 0x81, 0x00, 0x85,
    0x31, 0x95, 0x05, 0x09, 0x01, 0x81, 0x00, 0x85, 0x32, 0x95, 0x0A, 0x09, 0x01,
    0x81, 0x00, 0x85, 0x33, 0x95, 0x11, 0x09, 0x01,
    0x81, 0x00, 0x85, 0x34, 0x95, 0x15, 
    0x09, 0x01, 0x81, 0x00, 0x85, 0x35, 0x95, 0x15, 0x09, 0x01, 0x81, 0x00, 0x85,
    0x36, 0x95, 0x15, 0x09, 0x01, 0x81, 0x00, 0x85, 0x37, 0x95, 0x15, 0x09, 0x01,
    0x81, 0x00, 0x85, 0x3D, 0x95, 0x15, 0x09, 0x01, 0x81, 0x00, 0x85, 0x3E, 0x95,
    0x15, 0x09, 0x01, 0x81, 0x00, 0x85, 0x3F, 0x95, 0x15, 0x09, 0x01, 0x81, 0x00,
    0xC0, 0x09, 0x02, 0x07, 0x35, 0x08, 0x35, 0x06, 0x09, 0x04, 0x09, 0x09, 0x01,
    0x00, 0x09, 0x02, 0x08, 0x28, 0x00, 0x09, 0x02, 0x09, 0x28, 0x01, 0x09, 0x02,
    0x0A, 0x28, 0x01, 0x09, 0x02, 0x0B, 0x09, 0x01, 0x00, 0x09, 0x02, 0x0C, 0x09,
    0x0C, 0x80, 0x09, 0x02, 0x0D, 0x28, 0x00, 0x09, 0x02, 0x0E, 0x28, 0x00
};

void sdp_recv_data(uint8_t * buf, int32_t len)
{
    struct sdp_pdu * header = (struct sdp_pdu *)buf;
    
    //use the transaction id to determine the response to send
    state = header->transaction_id >> 8;
}

int32_t sdp_get_data(uint8_t * buf)
{
    int32_t len = 0;

    if (state >= 0)
    {
        const uint8_t * arr;
        
        switch (state)
        {
            case 0:
                arr = resp0;
                len = sizeof(resp0);
                break;
            case 1:
                arr = resp1;
                len = sizeof(resp1);
                break;
            case 2:
                arr = resp2;
                len = sizeof(resp2);
                break;
            case 3:
                arr = resp3;
                len = sizeof(resp3);
                break;
            case 4:
                arr = resp4;
                len = sizeof(resp4);
                break;
        }

        memcpy(buf, arr, len);
        
        state = -1;
    }

    return len;
}

int remove_existing_sdp_records(sdp_session_t * session)
{

}

int register_wiimote_sdp_record()
{
    int ret;
    sdp_session_t * session;
    session = sdp_connect(BDADDR_ANY, BDADDR_LOCAL, SDP_RETRY_IF_BUSY);

    if (session == NULL)
    {
        printf("failed to get sdp server session\n");
        return -1;
    }

    ret = sdp_device_record_unregister_binary(session, BDADDR_ANY, wiimote_hid_record_handle);
    if (ret < 0 && errno != EINVAL)
    {
        printf("failed to unregister sdp record %s\n", strerror(errno));
        sdp_close(session);
        return -1;
    }

    ret = sdp_device_record_register_binary(session, BDADDR_ANY, 
      (uint8_t *)attributes, sizeof(attributes), SDP_RECORD_PERSIST, &sdp_record_handle);
    if (ret < 0)
    {
        printf("failed to register sdp record %s\n", strerror(errno));
        sdp_close(session);
        return -1;
    }

    sdp_close(session);

    return 0;
}

int unregister_wiimote_sdp_record()
{
    int ret;
    sdp_session_t * session;
    session = sdp_connect(BDADDR_ANY, BDADDR_LOCAL, SDP_RETRY_IF_BUSY);

    if (session == NULL)
    {
        printf("failed to get sdp server session\n");
        return -1;
    }

    ret = sdp_device_record_unregister_binary(session, BDADDR_ANY, sdp_record_handle);
    if (ret < 0)
    {
        printf("failed to unregister sdp record %s\n", strerror(errno));
        sdp_close(session);
        return -1;
    }

    sdp_close(session);

    return 0;
}