/*
 * Sequences one automatic firing request above the manual shooter command.
 * It emits only application-level ShootCmd values and does not access motors,
 * CAN, or remote-control transport details.
 */
#ifndef AUTO_SHOOT_CONTROLLER_H
#define AUTO_SHOOT_CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>

#include "control_messages.h"
#include "remote_messages.h"

typedef enum {
    AUTO_SHOOT_IDLE = 0,
    AUTO_SHOOT_PREHEAT,
    AUTO_SHOOT_FEED_ONE,
    AUTO_SHOOT_INTERVAL,
    AUTO_SHOOT_COOLDOWN,
} AutoShootState_t;

typedef struct {
    AutoShootState_t state;
    bool trigger_latched;
    uint32_t state_started_ms;
    uint32_t bullets_fired;
    uint32_t preheat_ms;
    uint32_t feed_duration_ms;
    uint32_t feed_interval_ms;
    uint32_t cooldown_ms;
    uint32_t bullet_limit;
} AutoShootController;

/* Initialize a controller in IDLE with conservative time-based defaults. */
void AutoShootController_Init(AutoShootController *controller);

/* Cancel a running sequence. The next update emits no automatic output. */
void AutoShootController_Reset(AutoShootController *controller);

/*
 * Advance the automatic sequence at now_ms and write its requested output.
 * A rising Q key starts one sequence. This implementation uses calibrated
 * time windows because the current shooter contract exposes no speed-ready or
 * single-round position feedback; remote loss must reset the controller first.
 */
void AutoShootController_Update(AutoShootController *controller,
                                const RemoteControlMessage *remote,
                                uint32_t now_ms,
                                ShootCmd *command);

#endif /* AUTO_SHOOT_CONTROLLER_H */
