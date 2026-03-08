/**
 * Web server for Smoldyn
 */

#ifndef SMOLDYN_SERVER_HH
#define SMOLDYN_SERVER_HH

#include "mongoose.h"
#include "smoldyn.h"
#include <atomic>
#include <iostream>
#include <thread>

std::atomic<bool> g_stop_ui(false);

void server_event_handler(struct mg_connection *c, int ev, void *ev_data) {
  simptr sim = NULL;
  if (c->fn_data) {
    sim = static_cast<simptr>(c->fn_data);
  }

  if (ev == MG_EV_OPEN && c->is_listening == 1) {
    MG_INFO(("SERVER is listening"));
  } else if (ev == MG_EV_ACCEPT) {
    MG_INFO(("SERVER accepted a connection"));
  } else if (ev == MG_EV_READ) {
    struct mg_iobuf *r = &c->recv;
    MG_INFO(("SERVER got data: %.*s", r->len, r->buf));
    mg_send(c, r->buf, r->len); // echo it back
    r->len = 0;                 // Tell Mongoose we've consumed data
  } else if (ev == MG_EV_CLOSE) {
    MG_INFO(("SERVER disconnected"));
  } else if (ev == MG_EV_ERROR) {
    MG_INFO(("SERVER error: %s", (char *)ev_data));
  }
}

#endif
