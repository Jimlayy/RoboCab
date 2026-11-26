#define PKT_SENSOR        0x01
#define PKT_SENSOR_LEN    24      // 1 type + 5 x 2 bytes (format not yet decided)

#define PKT_MAN_STEERING      1
#define PKT_MAN_STEERING_LEN  6       // 1 type + speed (2) + angle (2)
#define PKT_AUT_STEERING    0x11
#define PKT_AUT_STEERING_LEN 5
