#define PKT_SENSOR        0x01
#define PKT_SENSOR_LEN    11      // 1 type + 5 x 2 bytes (format not yet decided)

#define PKT_STEERING      0x10
#define PKT_STEERING_LEN  5       // 1 type + speed (2) + angle (2)