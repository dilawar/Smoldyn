/*
 * GUI functions.
 * Author: Dilawar Singh <dilawar.s.rajput@gmail.com>
 *
 * Modified from opengl2.c written by Steven Andrews
 */

#include <cfloat>
#include <cmath>
#include <cstdio>
#include <string>

#include <fmt/core.h>

using namespace std;

#include "../libSteve/SimCommand.h"
#include "../Smoldyn/smoldyn.h"
#include "smoldynfuncs.h"

#include "../libSteve/opengl2.h"

#ifdef NSVC_CPP
#include "../NextSubVolume/nsvc.h"
#endif

#include "graphics.h"

constexpr double PI = 3.14159265358979323846;

namespace gui {

// global graphics structure.
struct GraphicsParam gGraphicsParam_;

/**
 * graphicsupdateinit
 */
int GraphicsUpdateInit(simptr sim)
{
    fmt::print("GraphicsUpdateInit\n");

    auto graphss = sim->graphss;
    auto dim = sim->dim;
    auto wlist = sim->wlist;

    if (dim == 1)
        ComputeParams<double>(wlist[0]->pos, wlist[1]->pos, 0, 0, 0, 0);
    else if (dim == 2)
        ComputeParams<double>(
            wlist[0]->pos, wlist[1]->pos, wlist[2]->pos, wlist[3]->pos, 0, 0);
    else {
        ComputeParams<double>(wlist[0]->pos, wlist[1]->pos, wlist[2]->pos,
            wlist[3]->pos, wlist[4]->pos, wlist[5]->pos);
        if (sim->srfss) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
    }
    return 0;
}

/* graphicsupdatelists */
int GraphicsUpdateLists(simptr sim)
{
    graphicsssptr graphss;
    int tflag;
    GLfloat f1[4];

    graphss = sim->graphss;
    tflag = strchr(sim->flags, 't') ? 1 : 0;
    if (tflag || graphss->graphics == 0)
        return 0;

    if (graphss->graphics >= 3) {
        glEnable(GL_LIGHTING);
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT,
            gl2Double2GLfloat(graphss->ambiroom, f1, 4));
        glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    }
    return 0;
}

/* DrawBoxFaceD */
void DrawBoxFaceD(double* pt1, double* pt2, int dim)
{
    if (dim == 2) {
        glBegin(GL_POLYGON);
        glVertex3d(pt1[0], pt1[1], pt1[2]);
        glVertex3d(pt1[0], pt2[1], pt1[2]);
        glVertex3d(pt2[0], pt2[1], pt1[2]);
        glVertex3d(pt2[0], pt1[1], pt1[2]);
        glEnd();
    } else if (dim == 3) {
        glBegin(GL_POLYGON);
        glVertex3d(pt1[0], pt1[1], pt1[2]);
        glVertex3d(pt1[0], pt2[1], pt1[2]);
        glVertex3d(pt2[0], pt2[1], pt1[2]);
        glVertex3d(pt2[0], pt1[1], pt1[2]);
        glEnd();
        glBegin(GL_POLYGON);
        glVertex3d(pt1[0], pt1[1], pt2[2]);
        glVertex3d(pt1[0], pt2[1], pt2[2]);
        glVertex3d(pt2[0], pt2[1], pt2[2]);
        glVertex3d(pt2[0], pt1[1], pt2[2]);
        glEnd();
        glBegin(GL_POLYGON);
        glVertex3d(pt1[0], pt1[1], pt1[2]);
        glVertex3d(pt1[0], pt1[1], pt2[2]);
        glVertex3d(pt2[0], pt1[1], pt2[2]);
        glVertex3d(pt2[0], pt1[1], pt1[2]);
        glEnd();
        glBegin(GL_POLYGON);
        glVertex3d(pt1[0], pt2[1], pt1[2]);
        glVertex3d(pt1[0], pt2[1], pt2[2]);
        glVertex3d(pt2[0], pt2[1], pt2[2]);
        glVertex3d(pt2[0], pt2[1], pt1[2]);
        glEnd();
        glBegin(GL_POLYGON);
        glVertex3d(pt1[0], pt1[1], pt1[2]);
        glVertex3d(pt1[0], pt1[1], pt2[2]);
        glVertex3d(pt1[0], pt2[1], pt2[2]);
        glVertex3d(pt1[0], pt2[1], pt1[2]);
        glEnd();
        glBegin(GL_POLYGON);
        glVertex3d(pt2[0], pt1[1], pt1[2]);
        glVertex3d(pt2[0], pt1[1], pt2[2]);
        glVertex3d(pt2[0], pt2[1], pt2[2]);
        glVertex3d(pt2[0], pt2[1], pt1[2]);
        glEnd();
    }
    return;
}

void DrawBoxD(double* pt1, double* pt2, int dim)
{
    if (dim == 1) {
        glBegin(GL_LINES);
        glVertex3d(pt1[0], pt1[1], pt1[2]);
        glVertex3d(pt2[0], pt1[1], pt1[2]);
        glEnd();
        return;
    }

    if (dim == 2) {
        glBegin(GL_LINE_LOOP);
        glVertex3d(pt1[0], pt1[1], pt1[2]);
        glVertex3d(pt2[0], pt1[1], pt1[2]);
        glVertex3d(pt2[0], pt2[1], pt1[2]);
        glVertex3d(pt1[0], pt2[1], pt1[2]);
        glEnd();
        return;
    }

    glBegin(GL_LINE_STRIP);
    glVertex3d(pt1[0], pt1[1], pt1[2]);
    glVertex3d(pt1[0], pt1[1], pt2[2]);
    glVertex3d(pt1[0], pt2[1], pt2[2]);
    glVertex3d(pt1[0], pt2[1], pt1[2]);
    glVertex3d(pt1[0], pt1[1], pt1[2]);
    glVertex3d(pt2[0], pt1[1], pt1[2]);
    glVertex3d(pt2[0], pt2[1], pt1[2]);
    glVertex3d(pt2[0], pt2[1], pt2[2]);
    glVertex3d(pt2[0], pt1[1], pt2[2]);
    glVertex3d(pt2[0], pt1[1], pt1[2]);
    glEnd();
    glBegin(GL_LINES);
    glVertex3d(pt1[0], pt1[1], pt2[2]);
    glVertex3d(pt2[0], pt1[1], pt2[2]);
    glVertex3d(pt1[0], pt2[1], pt2[2]);
    glVertex3d(pt2[0], pt2[1], pt2[2]);
    glVertex3d(pt1[0], pt2[1], pt1[2]);
    glVertex3d(pt2[0], pt2[1], pt1[2]);
    glEnd();
    return;
}

void DrawGridD(double* pt1, double* pt2, int* n, int dim)
{
    double delta1, delta2;
    int i, j;

    if (dim == 1) {
        glBegin(GL_POINTS);
        delta1 = (pt2[0] - pt1[0]) / n[0];
        for (i = 0; i <= n[0]; i++)
            glVertex3d(pt1[0] + i * delta1, pt1[1], pt1[2]);
        glEnd();
        return;
    }

    if (dim == 2) {
        glBegin(GL_LINES);
        delta1 = (pt2[1] - pt1[1]) / n[1];
        for (i = 0; i <= n[1]; i++) {
            glVertex3d(pt1[0], pt1[1] + i * delta1, pt1[2]);
            glVertex3d(pt2[0], pt1[1] + i * delta1, pt1[2]);
        }
        delta1 = (pt2[0] - pt1[0]) / n[0];
        for (i = 0; i <= n[0]; i++) {
            glVertex3d(pt1[0] + i * delta1, pt1[1], pt1[2]);
            glVertex3d(pt1[0] + i * delta1, pt2[1], pt1[2]);
        }
        glEnd();
        return;
    }

    if (dim == 3) {
        glBegin(GL_LINES);
        delta1 = (pt2[1] - pt1[1]) / n[1];
        delta2 = (pt2[2] - pt1[2]) / n[2];
        for (i = 0; i <= n[1]; i++)
            for (j = 0; j <= n[2]; j++) {
                glVertex3d(pt1[0], pt1[1] + i * delta1, pt1[2] + j * delta2);
                glVertex3d(pt2[0], pt1[1] + i * delta1, pt1[2] + j * delta2);
            }
        delta1 = (pt2[0] - pt1[0]) / n[0];
        delta2 = (pt2[2] - pt1[2]) / n[2];
        for (i = 0; i <= n[0]; i++)
            for (j = 0; j <= n[2]; j++) {
                glVertex3d(pt1[0] + i * delta1, pt1[1], pt1[2] + j * delta2);
                glVertex3d(pt1[0] + i * delta1, pt2[1], pt1[2] + j * delta2);
            }
        delta1 = (pt2[0] - pt1[0]) / n[0];
        delta2 = (pt2[1] - pt1[1]) / n[1];
        for (i = 0; i <= n[0]; i++)
            for (j = 0; j <= n[1]; j++) {
                glVertex3d(pt1[0] + i * delta1, pt1[1] + j * delta2, pt1[2]);
                glVertex3d(pt1[0] + i * delta1, pt1[1] + j * delta2, pt2[2]);
            }
        glEnd();
    }
    return;
}

/* graphicsupdateparams */
int GraphicsUpdateParams(simptr sim)
{
    graphicsssptr graphss;
    int lt, tflag;
    GLenum gllightnum;
    GLfloat glf1[4];

    graphss = sim->graphss;
    tflag = strchr(sim->flags, 't') ? 1 : 0;
    if (tflag || graphss->graphics == 0)
        return 0;

    // fmt::print("Updating BgColor: {}\n", graphss->backcolor);
    glClearColor((GLclampf)graphss->backcolor[0],
        (GLclampf)graphss->backcolor[1], (GLclampf)graphss->backcolor[2],
        (GLclampf)graphss->backcolor[3]);

    if (graphss->graphics >= 3) {
        for (lt = 0; lt < MAXLIGHTS; lt++)
            if (graphss->lightstate[lt] == LPon) {
                if (lt == 0)
                    gllightnum = GL_LIGHT0;
                else if (lt == 1)
                    gllightnum = GL_LIGHT1;
                else if (lt == 2)
                    gllightnum = GL_LIGHT2;
                else if (lt == 3)
                    gllightnum = GL_LIGHT3;
                else if (lt == 4)
                    gllightnum = GL_LIGHT4;
                else if (lt == 5)
                    gllightnum = GL_LIGHT5;
                else if (lt == 6)
                    gllightnum = GL_LIGHT6;
                else
                    gllightnum = GL_LIGHT7;
                glLightfv(gllightnum, GL_AMBIENT,
                    gl2Double2GLfloat(graphss->ambilight[lt], glf1, 4));
                glLightfv(gllightnum, GL_DIFFUSE,
                    gl2Double2GLfloat(graphss->difflight[lt], glf1, 4));
                glLightfv(gllightnum, GL_SPECULAR,
                    gl2Double2GLfloat(graphss->speclight[lt], glf1, 4));
                glLightfv(gllightnum, GL_POSITION,
                    gl2Double2GLfloat(graphss->lightpos[lt], glf1, 4));
                glEnable(gllightnum);
            }
    }
    return 0;
}

/**
 * Update graphics structure
 */
int GraphicsUpdate(simptr sim)
{
    int er;
    auto graphss = sim->graphss;

    if (graphss) {
        if (graphss->condition == SCinit) {
            er = GraphicsUpdateInit(sim);
            if (er)
                return er;
            graphicssetcondition(graphss, SClists, 1);
        }
        if (graphss->condition == SClists) {
            er = GraphicsUpdateLists(sim);
            if (er)
                return er;
            graphicssetcondition(graphss, SCparams, 1);
        }
        if (graphss->condition == SCparams) {
            er = GraphicsUpdateParams(sim);
            if (er)
                return er;
            graphicssetcondition(graphss, SCok, 1);
        }
    }
    return 0;
}

void DrawSphere(double r, int lats, int longs)
{
    int i, j;
    for (i = 0; i <= lats; i++) {
        double lat0 = M_PI * (-0.5 + (double)(i - 1) / lats);
        double z0 = sin(lat0);
        double zr0 = cos(lat0);

        double lat1 = M_PI * (-0.5 + (double)i / lats);
        double z1 = sin(lat1);
        double zr1 = cos(lat1);

        glBegin(GL_QUAD_STRIP);
        for (j = 0; j <= longs; j++) {
            double lng = 2 * M_PI * (double)(j - 1) / longs;
            double x = cos(lng);
            double y = sin(lng);

            glNormal3f(x * zr0, y * zr0, z0);
            glVertex3f(r * x * zr0, r * y * zr0, r * z0);
            glNormal3f(x * zr1, y * zr1, z1);
            glVertex3f(r * x * zr1, r * y * zr1, r * z1);
        }
        glEnd();
    }
}

/* RenderSurfaces */
void RenderSurfaces(simptr sim)
{
    surfacessptr srfss;
    surfaceptr srf;
    int s, p, graphics, fdone, bdone, c;
    double **point, *front;
    double xlo, xhi, ylo, yhi, xpix, ypix, ymid, zmid;
    double delta, deltax, deltay, theta, vect[3], vect2[3], axis[3], height;
    double flcolor[4], blcolor[4];
    enum DrawMode fdrawmode, bdrawmode, fdm, bdm;
    GLdouble gldvect[3];
    GLfloat glfvect[4];

    srfss = sim->srfss;
    graphics = sim->graphss->graphics;
    if (!srfss)
        return;

    xlo = gGraphicsParam_.ClipLeft;
    xhi = gGraphicsParam_.ClipRight;
    ylo = gGraphicsParam_.ClipBot;
    yhi = gGraphicsParam_.ClipTop;
    xpix = gGraphicsParam_.PixWide;
    ypix = gGraphicsParam_.PixHigh;
    ymid = gGraphicsParam_.ClipMidy;
    zmid = gGraphicsParam_.ClipMidz;

    if (sim->dim == 1) {
        for (s = 0; s < srfss->nsrf; s++) {
            srf = srfss->srflist[s];
            if (srf->fdrawmode != DMno) {
                glLineWidth((GLfloat)srf->edgepts);
                delta = srf->edgepts * (xhi - xlo) / xpix / 2;
                glColor4fv(ConvertTo<float>(srf->fcolor, glfvect, 4));
                glBegin(GL_LINES);
                for (p = 0; p < srf->npanel[0]; p++) { // 1-D rectangles front
                    point = srf->panels[0][p]->point;
                    front = srf->panels[0][p]->front;
                    glVertex3d((GLdouble)(point[0][0] + front[0] * delta),
                        (GLdouble)(ymid - (yhi - ylo) / 20), (GLdouble)zmid);
                    glVertex3d((GLdouble)(point[0][0] + front[0] * delta),
                        (GLdouble)(ymid + (yhi - ylo) / 20), (GLdouble)zmid);
                }
                for (p = 0; p < srf->npanel[1]; p++) { // 1-D triangles front
                    point = srf->panels[1][p]->point;
                    front = srf->panels[1][p]->front;
                    glVertex3d((GLdouble)(point[0][0] + front[0] * delta),
                        (GLdouble)(ymid - (yhi - ylo) / 20), (GLdouble)zmid);
                    glVertex3d((GLdouble)(point[0][0] + front[0] * delta),
                        (GLdouble)(ymid + (yhi - ylo) / 20), (GLdouble)zmid);
                }
                for (p = 0; p < srf->npanel[2]; p++) { // 1-D spheres front
                    point = srf->panels[2][p]->point;
                    front = srf->panels[2][p]->front;
                    glVertex3d((GLdouble)(point[0][0] + point[1][0]
                                   + front[0] * delta),
                        (GLdouble)(ymid - (yhi - ylo) / 20), (GLdouble)zmid);
                    glVertex3d((GLdouble)(point[0][0] + point[1][0]
                                   + front[0] * delta),
                        (GLdouble)(ymid + (yhi - ylo) / 20), (GLdouble)zmid);
                    glVertex3d((GLdouble)(point[0][0] - point[1][0]
                                   - front[0] * delta),
                        (GLdouble)(ymid - (yhi - ylo) / 20), (GLdouble)zmid);
                    glVertex3d((GLdouble)(point[0][0] - point[1][0]
                                   - front[0] * delta),
                        (GLdouble)(ymid + (yhi - ylo) / 20), (GLdouble)zmid);
                }
                glEnd();

                delta *= -1;
                glColor4fv(ConvertTo<float>(srf->bcolor, glfvect, 4));
                glBegin(GL_LINES);
                for (p = 0; p < srf->npanel[0]; p++) { // 1-D rectangles back
                    point = srf->panels[0][p]->point;
                    front = srf->panels[0][p]->front;
                    glVertex3d((GLdouble)(point[0][0] + front[0] * delta),
                        (GLdouble)(ymid - (yhi - ylo) / 20), (GLdouble)zmid);
                    glVertex3d((GLdouble)(point[0][0] + front[0] * delta),
                        (GLdouble)(ymid + (yhi - ylo) / 20), (GLdouble)zmid);
                }
                for (p = 0; p < srf->npanel[1]; p++) { // 1-D triangles back
                    point = srf->panels[1][p]->point;
                    front = srf->panels[1][p]->front;
                    glVertex3d((GLdouble)(point[0][0] + front[0] * delta),
                        (GLdouble)(ymid - (yhi - ylo) / 20), (GLdouble)zmid);
                    glVertex3d((GLdouble)(point[0][0] + front[0] * delta),
                        (GLdouble)(ymid + (yhi - ylo) / 20), (GLdouble)zmid);
                }
                for (p = 0; p < srf->npanel[2]; p++) { // 1-D spheres back
                    point = srf->panels[2][p]->point;
                    front = srf->panels[2][p]->front;
                    glVertex3d((GLdouble)(point[0][0] + point[1][0]
                                   + front[0] * delta),
                        (GLdouble)(ymid - (yhi - ylo) / 20), (GLdouble)zmid);
                    glVertex3d((GLdouble)(point[0][0] + point[1][0]
                                   + front[0] * delta),
                        (GLdouble)(ymid + (yhi - ylo) / 20), (GLdouble)zmid);
                    glVertex3d((GLdouble)(point[0][0] - point[1][0]
                                   - front[0] * delta),
                        (GLdouble)(ymid - (yhi - ylo) / 20), (GLdouble)zmid);
                    glVertex3d((GLdouble)(point[0][0] - point[1][0]
                                   - front[0] * delta),
                        (GLdouble)(ymid + (yhi - ylo) / 20), (GLdouble)zmid);
                }
                glEnd();
            }
        }
    }

    else if (sim->dim == 2) {
        for (s = 0; s < srfss->nsrf; s++) {
            srf = srfss->srflist[s];
            fdrawmode = srf->fdrawmode;
            bdrawmode = srf->bdrawmode;
            if (fdrawmode != DMno) {
                glColor4fv(ConvertTo<float>(srf->fcolor, glfvect, 4));
                if (fdrawmode & DMedge || fdrawmode & DMface) {
                    glLineWidth((GLfloat)srf->edgepts);
                    deltax = srf->edgepts * (xhi - xlo) / xpix / 2.5;
                    deltay = srf->edgepts * (yhi - ylo) / ypix / 2.5;
                    glBegin(GL_LINES);
                } else {
                    glPointSize((GLfloat)srf->edgepts);
                    deltax = deltay = 0;
                    glBegin(GL_POINTS);
                }

                for (p = 0; p < srf->npanel[0]; p++) { // 2-D rectangles front
                    point = srf->panels[0][p]->point;
                    front = srf->panels[0][p]->front;
                    if (front[1] == 0) {
                        glVertex3d((GLdouble)(point[0][0] + front[0] * deltax),
                            (GLdouble)(point[0][1]), (GLdouble)zmid);
                        glVertex3d((GLdouble)(point[1][0] + front[0] * deltax),
                            (GLdouble)(point[1][1]), (GLdouble)zmid);
                    } else {
                        glVertex3d((GLdouble)(point[0][0]),
                            (GLdouble)(point[0][1] + front[0] * deltay),
                            (GLdouble)zmid);
                        glVertex3d((GLdouble)(point[1][0]),
                            (GLdouble)(point[1][1] + front[0] * deltay),
                            (GLdouble)zmid);
                    }
                }
                for (p = 0; p < srf->npanel[1]; p++) { // 2-D triangles front
                    point = srf->panels[1][p]->point;
                    front = srf->panels[1][p]->front;
                    glVertex3d((GLdouble)(point[0][0] + front[0] * deltax),
                        (GLdouble)(point[0][1] + front[1] * deltay),
                        (GLdouble)zmid);
                    glVertex3d((GLdouble)(point[1][0] + front[0] * deltax),
                        (GLdouble)(point[1][1] + front[1] * deltay),
                        (GLdouble)zmid);
                }
                for (p = 0; p < srf->npanel[3]; p++) { // 2-D cylinders front
                    point = srf->panels[3][p]->point;
                    front = srf->panels[3][p]->front;
                    glVertex3d((GLdouble)(point[0][0] + front[0] * point[2][0]
                                   + front[2] * front[0] * deltax),
                        (GLdouble)(point[0][1] + front[1] * point[2][0]
                            + front[2] * front[1] * deltay),
                        (GLdouble)zmid);
                    glVertex3d((GLdouble)(point[1][0] + front[0] * point[2][0]
                                   + front[2] * front[0] * deltax),
                        (GLdouble)(point[1][1] + front[1] * point[2][0]
                            + front[2] * front[1] * deltay),
                        (GLdouble)zmid);
                    glVertex3d((GLdouble)(point[0][0] - front[0] * point[2][0]
                                   - front[2] * front[0] * deltax),
                        (GLdouble)(point[0][1] - front[1] * point[2][0]
                            - front[2] * front[1] * deltay),
                        (GLdouble)zmid);
                    glVertex3d((GLdouble)(point[1][0] - front[0] * point[2][0]
                                   - front[2] * front[0] * deltax),
                        (GLdouble)(point[1][1] - front[1] * point[2][0]
                            - front[2] * front[1] * deltay),
                        (GLdouble)zmid);
                }
                for (p = 0; p < srf->npanel[5]; p++) { // 2-D disks front
                    point = srf->panels[5][p]->point;
                    front = srf->panels[5][p]->front;
                    glVertex3d((GLdouble)(point[0][0] + point[1][0] * front[1]
                                   + front[0] * deltax),
                        (GLdouble)(point[0][1] - point[1][0] * front[0]
                            + front[1] * deltay),
                        (GLdouble)zmid);
                    glVertex3d((GLdouble)(point[0][0] - point[1][0] * front[1]
                                   + front[0] * deltax),
                        (GLdouble)(point[0][1] + point[1][0] * front[0]
                            + front[1] * deltay),
                        (GLdouble)zmid);
                }
                glEnd();
                for (p = 0; p < srf->npanel[2]; p++) { // 2-D spheres front
                    point = srf->panels[2][p]->point;
                    front = srf->panels[2][p]->front;
                    if (fdrawmode & DMvert)
                        gl2DrawCircleD(
                            point[0], point[1][0], (int)point[1][1], 'v', 2);
                    if (fdrawmode & DMface)
                        gl2DrawCircleD(
                            point[0], point[1][0], (int)point[1][1], 'f', 2);
                    if (fdrawmode & DMedge)
                        gl2DrawCircleD(point[0],
                            point[1][0] + front[0] * deltax, (int)point[1][1],
                            'e', 2);
                }
                for (p = 0; p < srf->npanel[4]; p++) { // 2-D hemispheres front
                    point = srf->panels[4][p]->point;
                    front = srf->panels[4][p]->front;
                    theta = atan2(point[2][1], point[2][0]) + PI / 2.0;
                    if (fdrawmode & DMvert)
                        gl2DrawArcD(point[0], point[1][0], theta, theta + PI,
                            (int)point[1][1], 'v', 2);
                    if (fdrawmode & DMface)
                        gl2DrawArcD(point[0], point[1][0], theta, theta + PI,
                            (int)point[1][1], 'f', 2);
                    if (fdrawmode & DMedge)
                        gl2DrawArcD(point[0], point[1][0] + front[0] * deltax,
                            theta, theta + PI, (int)point[1][1], 'e', 2);
                }

                if (fdrawmode & DMedge || fdrawmode & DMface) {
                    deltax *= -1;
                    deltay *= -1;
                    glColor4fv(ConvertTo<float>(srf->bcolor, glfvect, 4));
                    glBegin(GL_LINES);
                    for (p = 0; p < srf->npanel[0];
                         p++) { // 2-D rectangles back
                        point = srf->panels[0][p]->point;
                        front = srf->panels[0][p]->front;
                        if (front[1] == 0) {
                            glVertex3d(
                                (GLdouble)(point[0][0] + front[0] * deltax),
                                (GLdouble)(point[0][1]), (GLdouble)zmid);
                            glVertex3d(
                                (GLdouble)(point[1][0] + front[0] * deltax),
                                (GLdouble)(point[1][1]), (GLdouble)zmid);
                        } else {
                            glVertex3d((GLdouble)(point[0][0]),
                                (GLdouble)(point[0][1] + front[0] * deltay),
                                (GLdouble)zmid);
                            glVertex3d((GLdouble)(point[1][0]),
                                (GLdouble)(point[1][1] + front[0] * deltay),
                                (GLdouble)zmid);
                        }
                    }
                    for (p = 0; p < srf->npanel[1]; p++) { // 2-D triangles back
                        point = srf->panels[1][p]->point;
                        front = srf->panels[1][p]->front;
                        glVertex3d((GLdouble)(point[0][0] + front[0] * deltax),
                            (GLdouble)(point[0][1] + front[1] * deltay),
                            (GLdouble)zmid);
                        glVertex3d((GLdouble)(point[1][0] + front[0] * deltax),
                            (GLdouble)(point[1][1] + front[1] * deltay),
                            (GLdouble)zmid);
                    }
                    for (p = 0; p < srf->npanel[3]; p++) { // 2-D cylinders back
                        point = srf->panels[3][p]->point;
                        front = srf->panels[3][p]->front;
                        glVertex3d(
                            (GLdouble)(point[0][0] + front[0] * point[2][0]
                                + front[2] * front[0] * deltax),
                            (GLdouble)(point[0][1] + front[1] * point[2][0]
                                + front[2] * front[1] * deltay),
                            (GLdouble)zmid);
                        glVertex3d(
                            (GLdouble)(point[1][0] + front[0] * point[2][0]
                                + front[2] * front[0] * deltax),
                            (GLdouble)(point[1][1] + front[1] * point[2][0]
                                + front[2] * front[1] * deltay),
                            (GLdouble)zmid);
                        glVertex3d(
                            (GLdouble)(point[0][0] - front[0] * point[2][0]
                                - front[2] * front[0] * deltax),
                            (GLdouble)(point[0][1] - front[1] * point[2][0]
                                - front[2] * front[1] * deltay),
                            (GLdouble)zmid);
                        glVertex3d(
                            (GLdouble)(point[1][0] - front[0] * point[2][0]
                                - front[2] * front[0] * deltax),
                            (GLdouble)(point[1][1] - front[1] * point[2][0]
                                - front[2] * front[1] * deltay),
                            (GLdouble)zmid);
                    }
                    for (p = 0; p < srf->npanel[5]; p++) { // 2-D disks back
                        point = srf->panels[5][p]->point;
                        point = srf->panels[5][p]->point;
                        front = srf->panels[5][p]->front;
                        glVertex3d(
                            (GLdouble)(point[0][0] + point[1][0] * front[1]
                                + front[0] * deltax),
                            (GLdouble)(point[0][1] - point[1][0] * front[0]
                                + front[1] * deltay),
                            (GLdouble)zmid);
                        glVertex3d(
                            (GLdouble)(point[0][0] - point[1][0] * front[1]
                                + front[0] * deltax),
                            (GLdouble)(point[0][1] + point[1][0] * front[0]
                                + front[1] * deltay),
                            (GLdouble)zmid);
                    }
                    glEnd();
                    for (p = 0; p < srf->npanel[2]; p++) { // 2-D spheres back
                        point = srf->panels[2][p]->point;
                        front = srf->panels[2][p]->front;
                        if (bdrawmode & DMedge)
                            gl2DrawCircleD(point[0],
                                point[1][0] + front[0] * deltax,
                                (int)point[1][1], 'e', 2);
                    }
                    for (p = 0; p < srf->npanel[4];
                         p++) { // 2-D hemispheres back
                        point = srf->panels[4][p]->point;
                        front = srf->panels[4][p]->front;
                        theta = atan2(point[2][1], point[2][0]) + PI / 2.0;
                        if (bdrawmode & DMedge)
                            gl2DrawArcD(point[0],
                                point[1][0] + front[0] * deltax, theta,
                                theta + PI, (int)point[1][1], 'e', 2);
                    }
                }
            }
        }
    }

    else if (sim->dim == 3) {
        for (s = 0; s < srfss->nsrf; s++) {
            srf = srfss->srflist[s];
            fdrawmode = srf->fdrawmode;
            bdrawmode = srf->bdrawmode;
            for (c = 0; c < 4; c++)
                flcolor[c] = srf->fcolor[c];
            for (c = 0; c < 4; c++)
                blcolor[c] = srf->bcolor[c];

            if (fdrawmode & DMface)
                fdm = DMface;
            else if (fdrawmode & DMedge)
                fdm = DMedge;
            else if (fdrawmode & DMvert)
                fdm = DMvert;
            else
                fdm = DMno;

            if (bdrawmode & DMface)
                bdm = DMface;
            else if (bdrawmode & DMedge)
                bdm = DMedge;
            else if (bdrawmode & DMvert)
                bdm = DMvert;
            else
                bdm = DMno;

            while (fdm || bdm) {
                if (fdm == DMface)
                    glPolygonMode(GL_FRONT, GL_FILL);
                else if (fdm == DMedge)
                    glPolygonMode(GL_FRONT, GL_LINE);
                else if (fdm == DMvert)
                    glPolygonMode(GL_FRONT, GL_POINT);
                else
                    glCullFace(GL_FRONT);

                if (bdm == DMface)
                    glPolygonMode(GL_BACK, GL_FILL);
                else if (bdm == DMedge)
                    glPolygonMode(GL_BACK, GL_LINE);
                else if (bdm == DMvert)
                    glPolygonMode(GL_BACK, GL_POINT);
                else
                    glCullFace(GL_BACK);

                glColor4fv(ConvertTo<float>(flcolor, glfvect, 4));
                glLineWidth((GLfloat)srf->edgepts);
                if (graphics >= 2 && srf->edgestipple[1] != 0xFFFF) {
                    glEnable(GL_LINE_STIPPLE);
                    glLineStipple((GLint)srf->edgestipple[0],
                        (GLushort)srf->edgestipple[1]);
                }

                if (graphics >= 3) {
                    glMaterialfv(GL_FRONT, GL_SPECULAR,
                        ConvertTo<float>(flcolor, glfvect, 4));
                    glMaterialfv(GL_BACK, GL_SPECULAR,
                        ConvertTo<float>(blcolor, glfvect, 4));
                    glMateriali(GL_FRONT, GL_SHININESS, (GLint)srf->fshiny);
                    glMateriali(GL_BACK, GL_SHININESS, (GLint)srf->bshiny);
                }

                if (srf->npanel[PSrect]) {
                    glBegin(GL_QUADS); // 3-D rectangles
                    for (p = 0; p < srf->npanel[PSrect]; p++) {
                        if (graphics >= 3) {
                            gldvect[0] = gldvect[1] = gldvect[2] = 0;
                            front = srf->panels[PSrect][p]->front;
                            gldvect[(int)front[1]] = (GLdouble)front[0];
                            glNormal3dv(gldvect);
                        }
                        point = srf->panels[PSrect][p]->point;
                        glVertex3d((GLdouble)(point[0][0]),
                            (GLdouble)(point[0][1]), (GLdouble)(point[0][2]));
                        glVertex3d((GLdouble)(point[1][0]),
                            (GLdouble)(point[1][1]), (GLdouble)(point[1][2]));
                        glVertex3d((GLdouble)(point[2][0]),
                            (GLdouble)(point[2][1]), (GLdouble)(point[2][2]));
                        glVertex3d((GLdouble)(point[3][0]),
                            (GLdouble)(point[3][1]), (GLdouble)(point[3][2]));
                    }
                    glEnd();
                }

                if (srf->npanel[PStri]) {
                    glBegin(GL_TRIANGLES); // 3-D triangles
                    for (p = 0; p < srf->npanel[PStri]; p++) {
                        if (graphics >= 3)
                            glNormal3fv(ConvertTo<float>(
                                srf->panels[PStri][p]->front, glfvect, 4));
                        point = srf->panels[PStri][p]->point;
                        glVertex3d((GLdouble)(point[0][0]),
                            (GLdouble)(point[0][1]), (GLdouble)(point[0][2]));
                        glVertex3d((GLdouble)(point[1][0]),
                            (GLdouble)(point[1][1]), (GLdouble)(point[1][2]));
                        glVertex3d((GLdouble)(point[2][0]),
                            (GLdouble)(point[2][1]), (GLdouble)(point[2][2]));
                    }
                    glEnd();
                }

                for (p = 0; p < srf->npanel[PSsph]; p++) { // 3-D spheres
                    point = srf->panels[PSsph][p]->point;
                    front = srf->panels[PSsph][p]->front;
                    glMatrixMode(GL_MODELVIEW);
                    glPushMatrix();
                    glTranslated((GLdouble)(point[0][0]),
                        (GLdouble)(point[0][1]), (GLdouble)(point[0][2]));
                    gl2DrawSphere(point[1][0], (int)point[1][1],
                        (int)point[1][2], front[0] > 0 ? 0 : 1,
                        graphics >= 3 ? 1 : 0);
                    glPopMatrix();
                }

                for (p = 0; p < srf->npanel[PScyl]; p++) { // 3-D cylinders
                    point = srf->panels[PScyl][p]->point;
                    front = srf->panels[PScyl][p]->front;
                    glMatrixMode(GL_MODELVIEW);
                    glPushMatrix();
                    glTranslated((GLdouble)(point[0][0]),
                        (GLdouble)(point[0][1]), (GLdouble)(point[0][2]));
                    vect[0] = vect[1] = 0;
                    vect[2] = 1;
                    vect2[0] = point[1][0] - point[0][0];
                    vect2[1] = point[1][1] - point[0][1];
                    vect2[2] = point[1][2] - point[0][2];
                    height = sqrt(vect2[0] * vect2[0] + vect2[1] * vect2[1]
                        + vect2[2] * vect2[2]);
                    normalize<double>(vect2, 3);
                    theta = gl2FindRotateD(vect, vect2, axis);
                    glRotated((GLdouble)theta, (GLdouble)(axis[0]),
                        (GLdouble)(axis[1]), (GLdouble)(axis[2]));
                    gl2DrawCylinder(point[2][0], point[2][0], height,
                        (int)point[2][1], (int)point[2][2],
                        front[0] > 0 ? 0 : 1, graphics >= 3 ? 1 : 0);
                    glPopMatrix();
                }

                for (p = 0; p < srf->npanel[PShemi]; p++) { // 3-D hemispheres
                    point = srf->panels[PShemi][p]->point;
                    front = srf->panels[PShemi][p]->front;
                    glMatrixMode(GL_MODELVIEW);
                    glPushMatrix();
                    glTranslated((GLdouble)(point[0][0]),
                        (GLdouble)(point[0][1]), (GLdouble)(point[0][2]));
                    vect[0] = vect[1] = 0;
                    vect[2] = -1;
                    theta = gl2FindRotateD(vect, point[2], axis);
                    glRotated((GLdouble)theta, (GLdouble)(axis[0]),
                        (GLdouble)(axis[1]), (GLdouble)(axis[2]));
                    gl2DrawHemisphere(point[1][0], (int)point[1][1],
                        (int)point[1][2], front[0] > 0 ? 0 : 1,
                        graphics >= 3 ? 1 : 0);
                    glPopMatrix();
                }

                for (p = 0; p < srf->npanel[PSdisk]; p++) { // 3-D disks
                    point = srf->panels[PSdisk][p]->point;
                    front = srf->panels[PSdisk][p]->front;
                    glMatrixMode(GL_MODELVIEW);
                    glPushMatrix();
                    glTranslated((GLdouble)(point[0][0]),
                        (GLdouble)(point[0][1]), (GLdouble)(point[0][2]));
                    vect[0] = vect[1] = 0;
                    vect[2] = -1;
                    theta = gl2FindRotateD(vect, front, axis);
                    glRotated((GLdouble)theta, (GLdouble)(axis[0]),
                        (GLdouble)(axis[1]), (GLdouble)(axis[2]));
                    vect2[0] = vect2[1] = vect2[2] = 0;
                    gl2DrawCircleD(vect2, point[1][0], (int)point[1][1], 'f',
                        3); //?? 'f' isn't right
                    glPopMatrix();
                }

                fdone = 0;
                if (fdm == DMface && fdrawmode & DMedge)
                    fdm = DMedge;
                else if ((fdm == DMface || fdm == DMedge) && fdrawmode & DMvert)
                    fdm = DMvert;
                else
                    fdone = 1;

                bdone = 0;
                if (bdm == DMface && bdrawmode & DMedge)
                    bdm = DMedge;
                else if ((bdm == DMface || bdm == DMedge) && bdrawmode & DMvert)
                    bdm = DMvert;
                else
                    bdone = 1;

                if (fdone && bdone)
                    fdm = bdm = DMno;
                else {
                    for (c = 0; c < 3; c++)
                        flcolor[c] = blcolor[c] = 0;
                    flcolor[3] = blcolor[3] = 1;
                }
            }

            if ((fdrawmode || bdrawmode) && glIsEnabled(GL_LINE_STIPPLE))
                glDisable(GL_LINE_STIPPLE);
        }
    }
    return;
}

/* RenderFilaments */
void RenderFilaments(simptr sim)
{
    filamentssptr filss;
    filamenttypeptr filtype;
    filamentptr fil;
    int f, vtx, graphics, ft;
    double* point;
    enum DrawMode drawmode;
    GLfloat glfvect[4];

    filss = sim->filss;
    if (!filss)
        return;
    graphics = sim->graphss->graphics;

    for (ft = 0; ft < filss->ntype; ft++) {
        filtype = filss->filtypes[ft];
        drawmode = filtype->drawmode;

        for (f = 0; f < filtype->nfil; f++) {
            fil = filtype->fillist[f];
            if (drawmode == DMno)
                ;

            else if (drawmode & DMvert || drawmode & DMedge) {
                glColor4fv(ConvertTo<float>(filtype->color, glfvect, 4));
                if (graphics >= 2 && filtype->edgestipple[1] != 0xFFFF) {
                    glEnable(GL_LINE_STIPPLE);
                    glLineStipple((GLint)filtype->edgestipple[0],
                        (GLushort)filtype->edgestipple[1]);
                }
                if (drawmode & DMedge) {
                    glLineWidth((GLfloat)filtype->edgepts);
                    glBegin(GL_LINE_STRIP);
                } else {
                    glPointSize((GLfloat)filtype->edgepts);
                    glBegin(GL_POINTS);
                }

                for (vtx = fil->frontbs; vtx < fil->nbs + fil->frontbs; vtx++) {
                    if (filtype->isbead)
                        point = fil->beads[vtx]->xyz;
                    else
                        point = fil->segments[vtx]->xyzfront;
                    glVertex3d((GLdouble)(point[0]), (GLdouble)(point[1]),
                        (GLdouble)(point[2]));
                }
                if (!filtype->isbead) {
                    point = fil->segments[vtx - 1]->xyzback;
                    glVertex3d((GLdouble)(point[0]), (GLdouble)(point[1]),
                        (GLdouble)(point[2]));
                }
                glEnd();
            }

            else if (drawmode & DMface) {
                glPolygonMode(GL_FRONT, GL_FILL);
                glCullFace(GL_BACK);
                if (graphics >= 3) {
                    // glMaterialfv(GL_FRONT,GL_SPECULAR,ConvertTo<float>(srf->fcolor,glfvect,4));
                    // glMaterialfv(GL_BACK,GL_SPECULAR,ConvertTo<float>(srf->bcolor,glfvect,4));
                    glMateriali(GL_FRONT, GL_SHININESS, (GLint)filtype->shiny);
                }
                for (vtx = fil->frontbs; vtx < fil->nbs + fil->frontbs; vtx++)
                    /*gl2drawtwistprism(fil->px[vtx],fil->px[vtx+1],fil->nface,fil->po[vtx],twist,fil->radius,fil->facecolor)*/
                    ;
            }
            if (glIsEnabled(GL_LINE_STIPPLE))
                glDisable(GL_LINE_STIPPLE);
        }
    }

    return;
}

/* RenderMolecs */
void RenderMolecs(simptr sim)
{
    molssptr mols;
    moleculeptr mptr;
    int ll, m, i, dim;
    double ymid, zmid;
    enum MolecState ms;
    GLfloat whitecolor[] = { 1, 1, 1, 1 };
    GLfloat glf1[4];

    dim = sim->dim;
    mols = sim->mols;
    if (!mols)
        return;

    ymid = gGraphicsParam_.ClipMidy;
    zmid = gGraphicsParam_.ClipMidz;

    if (sim->graphss->graphics == 1) {
        for (ll = 0; ll < sim->mols->nlist; ll++)
            if (sim->mols->listtype[ll] == MLTsystem)
                for (m = 0; m < mols->nl[ll]; m++) {
                    mptr = mols->live[ll][m];
                    i = mptr->ident;
                    ms = mptr->mstate;
                    if (mols->display[i][ms] > 0) {
                        glPointSize((GLfloat)mols->display[i][ms]);
                        glColor3fv(
                            ConvertTo<float>(mols->color[i][ms], glf1, 3));
                        glBegin(GL_POINTS);
                        if (dim == 1)
                            glVertex3d((GLdouble)mptr->pos[0], (GLdouble)ymid,
                                (GLdouble)zmid);
                        else if (dim == 2)
                            glVertex3d((GLdouble)(mptr->pos[0]),
                                (GLdouble)(mptr->pos[1]), (GLdouble)zmid);
                        else
                            glVertex3fv(ConvertTo<float>(mptr->pos, glf1, 3));
                        glEnd();
                    }
                }
    }

    else if (sim->graphss->graphics >= 2) {
        glMatrixMode(GL_MODELVIEW);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        if (sim->graphss->graphics >= 3) {
            glMaterialfv(GL_FRONT, GL_SPECULAR, whitecolor);
            glMateriali(GL_FRONT, GL_SHININESS, 30);
        }
        for (ll = 0; ll < sim->mols->nlist; ll++) {
            if (sim->mols->listtype[ll] == MLTsystem) {
                for (m = 0; m < mols->nl[ll]; m++) {
                    mptr = mols->live[ll][m];
                    i = mptr->ident;
                    ms = mptr->mstate;
                    if (mols->display[i][ms] > 0) {
                        glColor3fv(
                            ConvertTo<float>(mols->color[i][ms], glf1, 3));
                        glPushMatrix();
                        if (dim == 1)
                            glTranslated((GLdouble)(mptr->pos[0]),
                                (GLdouble)ymid, (GLdouble)zmid);
                        else if (dim == 2)
                            glTranslated((GLdouble)(mptr->pos[0]),
                                (GLdouble)(mptr->pos[1]), (GLdouble)zmid);
                        else
                            glTranslated((GLdouble)(mptr->pos[0]),
                                (GLdouble)(mptr->pos[1]),
                                (GLdouble)(mptr->pos[2]));
                        DrawSphere((GLdouble)(mols->display[i][ms]), 15, 15);
                        glPopMatrix();
                    }
                }
            }
        }
    }

    return;
}

/* RenderLattice */
void RenderLattice(simptr sim)
{
    latticeptr lattice;
    int lat, n, ilat, ismol, i, dim;
    const int* copy_numbers;
    const double* positions;
    molssptr mols;
    GLfloat glf1[4];
    double poslo[3], poshi[3], deltay;

    mols = sim->mols;
    dim = sim->dim;

    poslo[0] = poshi[0] = gGraphicsParam_.ClipMidx;
    poslo[1] = poshi[1] = gGraphicsParam_.ClipMidy;
    poslo[2] = poshi[2] = gGraphicsParam_.ClipMidz;

    for (lat = 0; lat < sim->latticess->nlattice; lat++) {
        lattice = sim->latticess->latticelist[lat];
        positions = NULL;
        copy_numbers = NULL;
        n = 0;
        for (ilat = 0; ilat < lattice->nspecies;
             ilat++) { // ilat is the species identity
            ismol = lattice->species_index[ilat];
#ifdef NSVC_CPP
            NSV_CALL(n = nsv_get_species_copy_numbers(
                         lattice->nsv, ismol, &copy_numbers, &positions));
#endif
            for (i = 0; i < n;
                 ++i) { // n is the total number of molecules of this species,
                        // copy_numbers is how many in each site, and positions
                        // are the site positions
                if ((mols->display[ismol][MSsoln] > 0)
                    && (copy_numbers[i] > 0)) {
                    poslo[0] = positions[3 * i + 0] - 0.5 * lattice->dx[0];
                    poshi[0] = positions[3 * i + 0] + 0.5 * lattice->dx[0];
                    if (dim == 1) {
                        deltay = 0.025
                            * (gl2GetNumber("ClipTop")
                                - gl2GetNumber("ClipBot"));
                        poslo[1] -= deltay;
                        poshi[1] += deltay;
                    } else if (dim > 1) {
                        poslo[1] = positions[3 * i + 1] - 0.5 * lattice->dx[1];
                        poshi[1] = positions[3 * i + 1] + 0.5 * lattice->dx[1];
                    }
                    if (dim > 2) {
                        poslo[2] = positions[3 * i + 2] - 0.5 * lattice->dx[2];
                        poshi[2] = positions[3 * i + 2] + 0.5 * lattice->dx[2];
                    }
                    glColor3fv(
                        ConvertTo<float>(mols->color[ismol][MSsoln], glf1, 3));
                    gl2DrawBoxFaceD(poslo, poshi, dim == 3 ? 3 : 2);
                }
            }
        }
    }
    return;
}


void Initialize(simptr sim)
{
    //
    // Clear buffers to preset value.
    //
    const auto dim = sim->dim;
    if (dim < 3)
        glClear(GL_COLOR_BUFFER_BIT);
    else
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto wlist = sim->wlist;
    if (dim == 1) {
        ComputeParams(
            (float)wlist[0]->pos, (float)wlist[1]->pos, 0.f, 0.f, 0.f, 0.f);
        return;
    }

    if (dim == 2) {
        ComputeParams((float)wlist[0]->pos, (float)wlist[1]->pos,
            (float)wlist[2]->pos, (float)wlist[3]->pos, 0.f, 0.f);
        return;
    }

    ComputeParams((float)wlist[0]->pos, (float)wlist[1]->pos,
        (float)wlist[2]->pos, (float)wlist[3]->pos, (float)wlist[4]->pos,
        (float)wlist[5]->pos);

    if (sim->srfss) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
}

} // namespace gui
