/* Verify automatic firing timing without motor hardware or message dispatch. */
#include "auto_shoot_controller.h"

#include <assert.h>
#include <string.h>

static RemoteControlMessage RemoteWithQ(bool pressed)
{
    RemoteControlMessage remote;

    memset(&remote, 0, sizeof(remote));
    remote.key.v = pressed ? KEY_PRESSED_OFFSET_Q : 0U;
    return remote;
}

int main(void)
{
    AutoShootController controller;
    RemoteControlMessage pressed = RemoteWithQ(true);
    RemoteControlMessage released = RemoteWithQ(false);
    ShootCmd command;

    AutoShootController_Init(&controller);
    AutoShootController_Update(&controller, &pressed, 10U, &command);
    assert(controller.state == AUTO_SHOOT_PREHEAT);
    assert(!command.friction_enabled && !command.feed_enabled);

    AutoShootController_Update(&controller, &released, 210U, &command);
    assert(controller.state == AUTO_SHOOT_FEED_ONE);
    assert(command.friction_enabled && !command.feed_enabled);

    AutoShootController_Update(&controller, &released, 290U, &command);
    assert(controller.state == AUTO_SHOOT_COOLDOWN);
    assert(command.friction_enabled && command.feed_enabled);

    AutoShootController_Update(&controller, &released, 410U, &command);
    assert(controller.state == AUTO_SHOOT_IDLE);
    assert(command.friction_enabled && !command.feed_enabled);

    AutoShootController_Update(&controller, &released, 411U, &command);
    assert(!command.friction_enabled && !command.feed_enabled);
    return 0;
}
