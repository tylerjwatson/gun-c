
#include "dam.h"
#include "gun.h"
#include "log.h"
#include "mjson.h"
#include "url.h"
#include <stdio.h>
#include <string.h>

static inline int clamp(int val, int min, int max)
{
	const int t = val < min ? min : val;
	return t > max ? max : t;
}

bool gun_dam_handle_message(struct gun_peer *peer, const char *msg,
			    size_t msg_len)
{
	char dam_val[32];
	int val_len;

	// {"#":"ypmFCD0m6","dam":"?","pid":"jG3DKrlme"}
	if ((val_len = mjson_get_string(msg, msg_len, "$.dam", dam_val,
					sizeof(dam_val))) != -1 &&
	    strncmp(dam_val, "?", 1) == 0 &&
	    (val_len = mjson_get_string(msg, msg_len, "$.pid", dam_val,
					sizeof(dam_val)))) {
		strncpy(peer->id, dam_val, clamp(val_len, 0, sizeof(dam_val)));
		log_info("dam: peer %s:%d: we're talking with peer idA %s",
			 peer->url->host, peer->url->port, peer->id);
		gun_dam_say_hi(peer);
	}

	return true;
}

int gun_dam_say_hi(const struct gun_peer *peer)
{
	static const char *hi_packet_format =
		"{\"#\":\"%s\",\"dam\":\"hi\",\"@\":\"%s\"}";
	unsigned char packet[GUN_MAX_PACKET_LENGTH] = { 0 };
	int written_len;

	written_len = snprintf((char *)&packet[LWS_PRE],
			       GUN_MAX_PACKET_LENGTH - LWS_PRE,
			       hi_packet_format, peer->context->id, peer->id);

	log_trace("dam: %s", &packet[LWS_PRE]);

	if (lws_write(peer->ws_handle, packet, written_len, LWS_WRITE_TEXT) ==
	    -1) {
		log_error("dam: cannot say hi to %s", peer->id);
		return -1;
	}


	return 0;
}