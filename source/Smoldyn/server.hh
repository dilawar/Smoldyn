/**
 * Web server for Smoldyn
 */

#ifndef SMOLDYN_SERVER_HH
#define SMOLDYN_SERVER_HH

#include "mongoose.h"
#include "smoldyn.h"
#include <array>
#include <atomic>
#include <iostream>
#include <sstream>
#include <string>

// Stop the UI server.
std::atomic<bool> g_stop_ui(false);

// Is UI server started?
//
// The simulation thread MAY wait on this to become true before starting the
// simulation. When user opens the browser, the simulation thread will start and
// user won't miss any display.
std::atomic<bool> g_ui_server_started(false);

std::string simptr_to_svg(const simptr sim, size_t width, size_t height) {
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

  std::cout << "111: " << pt1[0] << " " << pt1[1] << " " << pt1[2] << std::endl;
  std::cout << "112: " << pt2[0] << " " << pt2[1] << " " << pt2[2] << std::endl;

  ss << "</svg>";

  return ss.str();
}

std::string welcome_page(const simptr sim) {
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
  const size_t graphic_width = 600;
  ss << "<div hx-get='/svg' hx-trigger='every 50ms'"
     << "style='width:" << graphic_width << "px;height:" << graphic_width
     << "px;border:1px dotted;'></div>";

  ss << simptr_to_svg(sim, graphic_width, graphic_width);

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
    // we have a live connection!
    if (!g_ui_server_started.load()) {
      // std::cout << "Webserver received a request." << std::endl;
      g_ui_server_started.store(true);
    }

    struct mg_http_message *hm = (struct mg_http_message *)ev_data;
    if (mg_match(hm->uri, mg_str("/"), NULL)) {
      mg_http_reply(c, 200, "", welcome_page(sim).c_str(), 1);
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
