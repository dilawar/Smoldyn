/**
 * Web server for Smoldyn
 */

#ifndef SMOLDYN_SERVER_HH
#define SMOLDYN_SERVER_HH

#include "mongoose.h"
#include "smoldyn.h"
#include <atomic>
#include <iostream>
#include <sstream>
#include <string>

std::atomic<bool> g_stop_ui(false);

std::string welcome_page() {
  std::stringstream ss;

  // head
  auto htmx_cdn =
      "https://cdn.jsdelivr.net/npm/htmx.org@2.0.8/dist/htmx.min.js";
  ss << "<!DOCTYPE html><html><head><title>Smoldyn</title>";
  ss << "<script src=\"" << htmx_cdn << "\"></script>";
  ss << "</head>";

  // body
  ss << "<body>";
  ss << "<h1>Welcome to Smoldyn!</h1>";

  // rest of the page goes here.
  ss << "<div hx-get='/svg' hx-trigger='every 500ms'"
     << "style='width:600px;height:600px;border:1px dotted;'></div>";

  // close.
  ss << "</body></html>";

  return ss.str();
}

void server_event_handler(struct mg_connection *c, int ev, void *ev_data) {
  simptr sim = NULL;
  if (c->fn_data) {
    sim = static_cast<simptr>(c->fn_data);
  }

  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *)ev_data;
    if (mg_match(hm->uri, mg_str("/"), NULL)) {
      mg_http_reply(c, 200, "", welcome_page().c_str(), 1);
    } else {
      struct mg_http_serve_opts opts = {.root_dir = ".", .fs = &mg_fs_posix};
      mg_http_serve_dir(c, hm, &opts);
    }
  }
}

#endif
