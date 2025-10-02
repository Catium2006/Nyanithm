#ifndef __MPR121_THRESHOLDS_STRUCT__
#define __MPR121_THRESHOLDS_STRUCT__

#define MPR121_TOUCH_THRESHOLD_DEFAULT 12  ///< default touch threshold value
#define MPR121_RELEASE_THRESHOLD_DEFAULT 12

#define DT MPR121_TOUCH_THRESHOLD_DEFAULT
#define DR MPR121_RELEASE_THRESHOLD_DEFAULT

struct mpr121Thresholds {
    uint8_t touch[12];
    uint8_t release[12];
};

#define defaultThresholds                                     \
    {                                                         \
        { DT, DT, DT, DT, DT, DT, DT, DT, DT, DT, DT, DT }, { \
            DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR, DR    \
        }                                                     \
    }

#endif