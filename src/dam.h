
#pragma once

#include "gun.h"

int gun_dam_say_hi(const struct gun_peer *peer);

bool gun_dam_handle_message(struct gun_peer *peer, const char *msg, size_t msg_len);