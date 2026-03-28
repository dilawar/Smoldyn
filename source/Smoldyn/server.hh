/**
 * Web server for Smoldyn
 */

#ifndef SMOLDYN_SERVER_HH
#define SMOLDYN_SERVER_HH

#include "mongoose.h"
#include "smoldyn.h"
#include <algorithm>
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

std::string simptr_to_svg(const simptr sim, size_t svgw, size_t svgh) {
  std::stringstream ss;

  // Simulation bounding box from walls.
  double xmin = sim->wlist[0]->pos;
  double xmax = sim->wlist[1]->pos;
  double ymin = (sim->dim > 1) ? sim->wlist[2]->pos : 0.0;
  double ymax = (sim->dim > 1) ? sim->wlist[3]->pos : 1.0;

  double sim_w = xmax - xmin;
  double sim_h = ymax - ymin;

  // Leave a margin so molecules at the boundary are fully visible.
  const double pad = 20.0;
  double usable_w = (double)svgw - 2.0 * pad;
  double usable_h = (double)svgh - 2.0 * pad;

  // Uniform scale to preserve aspect ratio.
  double scale = (sim_w > 0 && sim_h > 0)
                     ? std::min(usable_w / sim_w, usable_h / sim_h)
                     : 1.0;

  // Coordinate transform: sim -> SVG pixels.
  // SVG y-axis points downward, so we flip y.
  auto to_svgx = [&](double x) { return pad + (x - xmin) * scale; };
  auto to_svgy = [&](double y) { return pad + (ymax - y) * scale; };

  ss << "<svg xmlns='http://www.w3.org/2000/svg' width='" << svgw
     << "' height='" << svgh << "'>";

  // White background.
  ss << "<rect width='" << svgw << "' height='" << svgh
     << "' fill='white'/>";

  // Simulation boundary box.
  double box_x = to_svgx(xmin);
  double box_y = to_svgy(ymax);
  double box_w = sim_w * scale;
  double box_h = sim_h * scale;
  ss << "<rect x='" << box_x << "' y='" << box_y << "' width='" << box_w
     << "' height='" << box_h
     << "' fill='none' stroke='black' stroke-width='1'/>";

  // Time label.
  ss << "<text x='5' y='12' font-size='11' fill='black'>"
     << "t=" << sim->time << " / " << sim->tmax << "</text>";

  // Molecules.
  molssptr mols = sim->mols;
  if (mols) {
    for (int ll = 0; ll < mols->nlist; ll++) {
      if (mols->listtype[ll] != MLTsystem)
        continue;
      for (int m = 0; m < mols->nl[ll]; m++) {
        moleculeptr mptr = mols->live[ll][m];
        int i = mptr->ident;
        enum MolecState ms = mptr->mstate;

        double disp = mols->display[i][ms];
        if (disp <= 0.0)
          continue;

        // Color: mols->color[i][ms] is [r, g, b] in 0..1.
        double *rgb = mols->color[i][ms];
        int r = (int)(rgb[0] * 255.0);
        int g = (int)(rgb[1] * 255.0);
        int b = (int)(rgb[2] * 255.0);

        double cx = to_svgx(mptr->pos[0]);
        double cy = (sim->dim > 1) ? to_svgy(mptr->pos[1]) : to_svgy(0.5);
        // Treat display size as radius in pixels directly.
        double r_px = std::max(disp, 2.0);

        ss << "<circle cx='" << cx << "' cy='" << cy << "' r='" << r_px
           << "' fill='rgb(" << r << "," << g << "," << b << ")'/>";
      }
    }
  }

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
      std::string page = welcome_page(sim);
      mg_http_reply(c, 200, "Content-Type: text/html\r\n", "%s", page.c_str());
    } else if (mg_match(hm->uri, mg_str("/svg"), NULL)) {
      std::string svg = simptr_to_svg(sim, 600, 600);
      mg_http_reply(c, 200, "Content-Type: image/svg+xml\r\n",
                    "%s", svg.c_str());
    } else {
      mg_http_reply(c, 404, "", "Not found\n", 0);
    }
  }
}

#endif
