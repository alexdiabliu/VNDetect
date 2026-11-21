/**
 * Convert raw samples into think-facing features (still no hardware I/O).
 */
#ifndef FEATURES_H
#define FEATURES_H

#include <stdint.h>
#include "protocol/types.h"

SensorFeatures features_from_samples(const RgbSample *rgb,
                                     const SensorXSample *sensor_x,
                                     uint32_t now_ms);

#endif /* FEATURES_H */
