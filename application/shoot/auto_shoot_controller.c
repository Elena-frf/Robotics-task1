/*
 * Produces the time-based automatic firing sequence for one Q-key request.
 * Motor feedback validation and current limiting remain owned by the shooter
 * controller; this layer never sends a motor command directly.
 */
#include "auto_shoot_controller.h"

#include <string.h>

/* Values are milliseconds and must be calibrated on the real launcher. */
#define AUTO_SHOOT_PREHEAT_MS 200U
#define AUTO_SHOOT_FEED_DURATION_MS 80U
#define AUTO_SHOOT_FEED_INTERVAL_MS 180U
#define AUTO_SHOOT_COOLDOWN_MS 120U
#define AUTO_SHOOT_BULLET_LIMIT 1U

static bool AutoShootController_QPressed(const RemoteControlMessage *remote)
{
    return remote != NULL &&
           (((uint16_t)remote->key.v & KEY_PRESSED_OFFSET_Q) != 0U);
}

void AutoShootController_Init(AutoShootController *controller)
{
    if (controller == NULL) {
        return;
    }

    AutoShootController_Reset(controller);
    controller->preheat_ms = AUTO_SHOOT_PREHEAT_MS;
    controller->feed_duration_ms = AUTO_SHOOT_FEED_DURATION_MS;
    controller->feed_interval_ms = AUTO_SHOOT_FEED_INTERVAL_MS;
    controller->cooldown_ms = AUTO_SHOOT_COOLDOWN_MS;
    controller->bullet_limit = AUTO_SHOOT_BULLET_LIMIT;
}

void AutoShootController_Reset(AutoShootController *controller)
{
    if (controller == NULL) {
        return;
    }

    controller->state = AUTO_SHOOT_IDLE;
    controller->trigger_latched = false;
    controller->state_started_ms = 0U;
    controller->bullets_fired = 0U;
}

void AutoShootController_Update(AutoShootController *controller,
                                const RemoteControlMessage *remote,
                                uint32_t now_ms,
                                ShootCmd *command)
{
    bool q_pressed;
    bool q_pressed_edge;

    if (controller == NULL || command == NULL) {
        return;
    }

    memset(command, 0, sizeof(*command));
    q_pressed = AutoShootController_QPressed(remote);
    q_pressed_edge = q_pressed && !controller->trigger_latched;
    controller->trigger_latched = q_pressed;

    switch (controller->state) {
        case AUTO_SHOOT_IDLE:
            if (q_pressed_edge) {
                controller->state = AUTO_SHOOT_PREHEAT;
                controller->state_started_ms = now_ms;
                controller->bullets_fired = 0U;
            }
            break;

        case AUTO_SHOOT_PREHEAT:
            command->friction_enabled = true;
            if ((uint32_t)(now_ms - controller->state_started_ms) >=
                controller->preheat_ms) {
                controller->state = AUTO_SHOOT_FEED_ONE;
                controller->state_started_ms = now_ms;
            }
            break;

        case AUTO_SHOOT_FEED_ONE:
            command->friction_enabled = true;
            command->feed_enabled = true;
            if ((uint32_t)(now_ms - controller->state_started_ms) >=
                controller->feed_duration_ms) {
                controller->bullets_fired += 1U;
                controller->state = controller->bullets_fired >=
                                            controller->bullet_limit
                                        ? AUTO_SHOOT_COOLDOWN
                                        : AUTO_SHOOT_INTERVAL;
                controller->state_started_ms = now_ms;
            }
            break;

        case AUTO_SHOOT_INTERVAL:
            command->friction_enabled = true;
            if ((uint32_t)(now_ms - controller->state_started_ms) >=
                controller->feed_interval_ms) {
                controller->state = AUTO_SHOOT_FEED_ONE;
                controller->state_started_ms = now_ms;
            }
            break;

        case AUTO_SHOOT_COOLDOWN:
            command->friction_enabled = true;
            if ((uint32_t)(now_ms - controller->state_started_ms) >=
                controller->cooldown_ms) {
                controller->state = AUTO_SHOOT_IDLE;
            }
            break;

        default:
            AutoShootController_Reset(controller);
            break;
    }
}
