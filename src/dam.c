
#include "dam.h"
#include "log.h"
#include "mjson.h"
#include "url.h"
#include <string.h>

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
		strncpy(peer->id, dam_val, val_len);
		log_info("dam: peer %s:%d: we're talking with peer id %s",
			 peer->url->host, peer->url->port, peer->id);
	}

	return true;
}
