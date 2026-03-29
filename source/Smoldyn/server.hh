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

// Serialize the current simulation frame as JSON consumed by the WebGL client.
// Format:
//   { "t": <time>, "tmax": <tmax>,
//     "box": [xmin, ymin, xmax, ymax],
//     "mols": [ [x, y, r, g, b, radius_px], ... ] }
std::string simptr_to_json(const simptr sim) {
  std::stringstream ss;

  double xmin = sim->wlist[0]->pos;
  double xmax = sim->wlist[1]->pos;
  double ymin = (sim->dim > 1) ? sim->wlist[2]->pos : 0.0;
  double ymax = (sim->dim > 1) ? sim->wlist[3]->pos : 1.0;

  ss << "{";
  ss << "\"t\":" << sim->time << ",";
  ss << "\"tmax\":" << sim->tmax << ",";
  ss << "\"box\":[" << xmin << "," << ymin << "," << xmax << "," << ymax << "],";
  ss << "\"mols\":[";

  bool first = true;
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

        double *rgb = mols->color[i][ms];
        double x = mptr->pos[0];
        double y = (sim->dim > 1) ? mptr->pos[1] : (ymin + ymax) * 0.5;

        if (!first) ss << ",";
        first = false;
        ss << "[" << x << "," << y << ","
           << rgb[0] << "," << rgb[1] << "," << rgb[2] << ","
           << std::max(disp, 2.0) << "]";
      }
    }
  }

  ss << "]}";
  return ss.str();
}

// The HTML page with a WebGL canvas that polls /frame every 50 ms.
static const char *WEBGL_PAGE = R"HTML(<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>Smoldyn</title>
<style>
  body { margin: 0; background: #1a1a2e; color: #eee;
         font-family: monospace; display: flex; flex-direction: column;
         align-items: center; padding: 12px; }
  h1   { margin: 0 0 8px; font-size: 1.1em; letter-spacing: 2px; }
  #info { font-size: 0.75em; margin-bottom: 6px; color: #aaa; }
  canvas { border: 1px solid #444; display: block; }
</style>
</head>
<body>
<h1>Smoldyn</h1>
<div id="info">connecting...</div>
<canvas id="gl" width="640" height="640"></canvas>
<script>
const canvas = document.getElementById('gl');
const info   = document.getElementById('info');
const gl     = canvas.getContext('webgl');
if (!gl) { info.textContent = 'WebGL not supported'; }

// ── shaders ──────────────────────────────────────────────────────────────
const vsrc = `
  attribute vec2 aPos;
  attribute vec3 aColor;
  attribute float aRadius;
  uniform vec2 uScale;   // 2/(xmax-xmin), 2/(ymax-ymin)
  uniform vec2 uOffset;  // -(xmin+xmax)/2, -(ymin+ymax)/2
  varying vec3 vColor;
  varying float vRadius;
  varying vec2 vUV;       // within the quad [-1,1]
  // We expand each molecule into a small quad in JS, passing a UV attribute.
  attribute vec2 aUV;
  void main() {
    vec2 worldPos = aPos + aUV * aRadius;
    vec2 clip = (worldPos + uOffset) * uScale;
    gl_Position = vec4(clip, 0.0, 1.0);
    vColor  = aColor;
    vRadius = aRadius;
    vUV     = aUV;
  }
`;
const fsrc = `
  precision mediump float;
  varying vec3 vColor;
  varying vec2 vUV;
  void main() {
    // Discard corners outside unit circle → draw a disk.
    float d = dot(vUV, vUV);
    if (d > 1.0) discard;
    // Simple shading: bright centre, darker edge.
    float shade = 1.0 - d * 0.4;
    gl_FragColor = vec4(vColor * shade, 1.0);
  }
`;

function compileShader(type, src) {
  const s = gl.createShader(type);
  gl.shaderSource(s, src);
  gl.compileShader(s);
  return s;
}
const prog = gl.createProgram();
gl.attachShader(prog, compileShader(gl.VERTEX_SHADER,   vsrc));
gl.attachShader(prog, compileShader(gl.FRAGMENT_SHADER, fsrc));
gl.linkProgram(prog);
gl.useProgram(prog);

const aPos    = gl.getAttribLocation(prog, 'aPos');
const aColor  = gl.getAttribLocation(prog, 'aColor');
const aRadius = gl.getAttribLocation(prog, 'aRadius');
const aUV     = gl.getAttribLocation(prog, 'aUV');
const uScale  = gl.getUniformLocation(prog, 'uScale');
const uOffset = gl.getUniformLocation(prog, 'uOffset');

const buf = gl.createBuffer();
gl.bindBuffer(gl.ARRAY_BUFFER, buf);

// stride: x,y (pos 2f) | r,g,b (3f) | radius (1f) | uvx,uvy (2f) = 8 floats
const STRIDE = 8 * 4;
gl.enableVertexAttribArray(aPos);
gl.vertexAttribPointer(aPos,    2, gl.FLOAT, false, STRIDE, 0);
gl.enableVertexAttribArray(aColor);
gl.vertexAttribPointer(aColor,  3, gl.FLOAT, false, STRIDE, 2*4);
gl.enableVertexAttribArray(aRadius);
gl.vertexAttribPointer(aRadius, 1, gl.FLOAT, false, STRIDE, 5*4);
gl.enableVertexAttribArray(aUV);
gl.vertexAttribPointer(aUV,     2, gl.FLOAT, false, STRIDE, 6*4);

const ibuf = gl.createBuffer();

// UV corners for a quad (2 triangles)
const UV = [[-1,-1],[1,-1],[1,1],  [-1,-1],[1,1],[-1,1]];

// ── box outline (drawn with canvas 2d overlay) ───────────────────────────
const overlay = document.createElement('canvas');
overlay.width  = canvas.width;
overlay.height = canvas.height;
overlay.style.position = 'absolute';
canvas.parentNode.insertBefore(overlay, canvas.nextSibling);
overlay.style.pointerEvents = 'none';
const ctx2d = overlay.getContext('2d');

let simBox = null;

function drawBox(box) {
  // box = [xmin, ymin, xmax, ymax] in sim coords
  ctx2d.clearRect(0, 0, overlay.width, overlay.height);
  if (!box) return;
  const [x0,y0,x1,y1] = box;
  const sw = x1-x0, sh = y1-y0;
  const pad = 20;
  const scale = Math.min((canvas.width-2*pad)/sw, (canvas.height-2*pad)/sh);
  const ox = pad, oy = pad;
  const bx = ox, by = oy, bw = sw*scale, bh = sh*scale;
  ctx2d.strokeStyle = '#888';
  ctx2d.lineWidth   = 1;
  ctx2d.strokeRect(bx, by, bw, bh);
}

// ── render ────────────────────────────────────────────────────────────────
function render(frame) {
  const [x0,y0,x1,y1] = frame.box;
  const cx = (x0+x1)*0.5, cy = (y0+y1)*0.5;
  const sx = 2.0/(x1-x0), sy = 2.0/(y1-y0);

  gl.uniform2f(uScale,  sx, sy);
  gl.uniform2f(uOffset, -cx, -cy);

  // Build interleaved vertex array (6 verts per molecule quad).
  const mols = frame.mols;
  const vdata = new Float32Array(mols.length * 6 * 8);
  const idata = new Uint16Array(mols.length * 6);
  let vi = 0;
  for (let m = 0; m < mols.length; m++) {
    const [mx, my, r, g, b, radius] = mols[m];
    // Convert radius from pixels to sim units so it looks consistent.
    const rSim = radius / Math.min(
      (canvas.width  - 40) / (x1-x0),
      (canvas.height - 40) / (y1-y0)
    );
    for (const [u,v] of UV) {
      vdata[vi++] = mx; vdata[vi++] = my;
      vdata[vi++] = r;  vdata[vi++] = g; vdata[vi++] = b;
      vdata[vi++] = rSim;
      vdata[vi++] = u;  vdata[vi++] = v;
    }
  }

  gl.bindBuffer(gl.ARRAY_BUFFER, buf);
  gl.bufferData(gl.ARRAY_BUFFER, vdata, gl.DYNAMIC_DRAW);

  gl.clearColor(0.08, 0.08, 0.12, 1.0);
  gl.clear(gl.COLOR_BUFFER_BIT);
  gl.drawArrays(gl.TRIANGLES, 0, mols.length * 6);

  drawBox(frame.box);
  info.textContent = `t = ${frame.t.toFixed(4)} / ${frame.tmax.toFixed(4)}  |  ${mols.length} molecules`;
}

// ── polling loop ──────────────────────────────────────────────────────────
async function poll() {
  try {
    const resp = await fetch('/frame');
    if (resp.ok) {
      const frame = await resp.json();
      render(frame);
    }
  } catch(_) {}
  setTimeout(poll, 50);
}
poll();
</script>
</body>
</html>
)HTML";

std::string welcome_page() {
  return std::string(WEBGL_PAGE);
}

void server_event_handler(struct mg_connection *c, int ev, void *ev_data) {
  simptr sim = NULL;
  if (c->fn_data) {
    sim = static_cast<simptr>(c->fn_data);
  }

  if (ev == MG_EV_HTTP_MSG) {
    if (!g_ui_server_started.load())
      g_ui_server_started.store(true);

    struct mg_http_message *hm = (struct mg_http_message *)ev_data;
    if (mg_match(hm->uri, mg_str("/"), NULL)) {
      std::string page = welcome_page();
      mg_http_reply(c, 200, "Content-Type: text/html\r\n", "%s", page.c_str());
    } else if (mg_match(hm->uri, mg_str("/frame"), NULL)) {
      std::string json = simptr_to_json(sim);
      mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                    "%s", json.c_str());
    } else {
      mg_http_reply(c, 404, "", "Not found\n", 0);
    }
  }
}

#endif
