/**
 * Web server for Smoldyn
 */

#ifndef SMOLDYN_SERVER_HH
#define SMOLDYN_SERVER_HH

#include "mongoose.h"
#include "smoldyn.h"
#include <array>
#include <atomic>
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

std::string simptr_to_svg(simptr sim, size_t width, size_t height) {
  std::stringstream ss;
  ss << "<svg xmlns='http://www.w3.org/2000/svg' width='" << width
     << "' height='" << height << "'>";

  // add time.
  ss << "<text x='20' y='20' fill='black'> time=" << sim->time
     << ", end time=" << sim->tmax << ", dt=" << sim->dt << "</text>";

  std::array<double, 3> pt1 = {0, 0, 0};
  std::array<double, 3> pt2 = {0, 0, 0};
  pt1[0] = sim->wlist[0]->pos;
  pt2[0] = sim->wlist[1]->pos;
  pt1[1] = sim->dim > 1 ? sim->wlist[2]->pos : 0;
  pt2[1] = sim->dim > 1 ? sim->wlist[3]->pos : 0;
  pt1[2] = sim->dim > 2 ? sim->wlist[4]->pos : 0;
  pt2[2] = sim->dim > 2 ? sim->wlist[5]->pos : 0;

  ss << "</svg>";

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
    }
    if (mg_match(hm->uri, mg_str("/svg"), NULL)) {
      std::stringstream ss;
      ss << simptr_to_svg(sim, 600, 600);
      mg_http_reply(c, 200, "", ss.str().c_str(), 0);
    } else {
      mg_http_reply(c, 404, "", "Not found", 0);
    }
  }
}

#endif
