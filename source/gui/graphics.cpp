/* Steven Andrews, started 10/22/2001.
 This is a library of functions for the Smoldyn program.
 See documentation called SmoldynManual.pdf and SmoldynCodeDoc.pdf, and the
 Smoldyn website, which is at www.smoldyn.org. Copyright 2003-2016 by Steven
 Andrews.  This work is distributed under the terms of the Gnu Lesser General
 Public License (LGPL). */

#include <cfloat>
#include <cmath>
#include <cstdio>
#include <string>

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

void draw_sphere(double r, int lats, int longs)
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

    xlo = gl2GetNumber("ClipLeft");
    xhi = gl2GetNumber("ClipRight");
    ylo = gl2GetNumber("ClipBot");
    yhi = gl2GetNumber("ClipTop");
    xpix = gl2GetNumber("PixWide");
    ypix = gl2GetNumber("PixHigh");
    ymid = gl2GetNumber("ClipMidy");
    zmid = gl2GetNumber("ClipMidz");

    if (sim->dim == 1) {
        for (s = 0; s < srfss->nsrf; s++) {
            srf = srfss->srflist[s];
            if (srf->fdrawmode != DMno) {
                glLineWidth((GLfloat)srf->edgepts);
                delta = srf->edgepts * (xhi - xlo) / xpix / 2;
                glColor4fv(ConvertDoubleTo<float>(srf->fcolor, glfvect, 4));
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
                glColor4fv(ConvertDoubleTo<float>(srf->bcolor, glfvect, 4));
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
                glColor4fv(ConvertDoubleTo<float>(srf->fcolor, glfvect, 4));
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
                    glColor4fv(ConvertDoubleTo<float>(srf->bcolor, glfvect, 4));
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

                glColor4fv(ConvertDoubleTo<float>(flcolor, glfvect, 4));
                glLineWidth((GLfloat)srf->edgepts);
                if (graphics >= 2 && srf->edgestipple[1] != 0xFFFF) {
                    glEnable(GL_LINE_STIPPLE);
                    glLineStipple((GLint)srf->edgestipple[0],
                        (GLushort)srf->edgestipple[1]);
                }

                if (graphics >= 3) {
                    glMaterialfv(GL_FRONT, GL_SPECULAR,
                        ConvertDoubleTo<float>(flcolor, glfvect, 4));
                    glMaterialfv(GL_BACK, GL_SPECULAR,
                        ConvertDoubleTo<float>(blcolor, glfvect, 4));
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
                            glNormal3fv(ConvertDoubleTo<float>(
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
                glColor4fv(ConvertDoubleTo<float>(filtype->color, glfvect, 4));
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
                    // glMaterialfv(GL_FRONT,GL_SPECULAR,ConvertDoubleTo<float>(srf->fcolor,glfvect,4));
                    // glMaterialfv(GL_BACK,GL_SPECULAR,ConvertDoubleTo<float>(srf->bcolor,glfvect,4));
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
    ymid = gl2GetNumber("ClipMidy");
    zmid = gl2GetNumber("ClipMidz");

    if (sim->graphss->graphics == 1) {
        for (ll = 0; ll < sim->mols->nlist; ll++)
            if (sim->mols->listtype[ll] == MLTsystem)
                for (m = 0; m < mols->nl[ll]; m++) {
                    mptr = mols->live[ll][m];
                    i = mptr->ident;
                    ms = mptr->mstate;
                    if (mols->display[i][ms] > 0) {
                        glPointSize((GLfloat)mols->display[i][ms]);
                        glColor3fv(ConvertDoubleTo<float>(
                            mols->color[i][ms], glf1, 3));
                        glBegin(GL_POINTS);
                        if (dim == 1)
                            glVertex3d((GLdouble)mptr->pos[0], (GLdouble)ymid,
                                (GLdouble)zmid);
                        else if (dim == 2)
                            glVertex3d((GLdouble)(mptr->pos[0]),
                                (GLdouble)(mptr->pos[1]), (GLdouble)zmid);
                        else
                            glVertex3fv(
                                ConvertDoubleTo<float>(mptr->pos, glf1, 3));
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
                        glColor3fv(ConvertDoubleTo<float>(
                            mols->color[i][ms], glf1, 3));
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
                        draw_sphere((GLdouble)(mols->display[i][ms]), 15, 15);
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
    poslo[0] = poshi[0] = gl2GetNumber("ClipMidx");
    poslo[1] = poshi[1] = gl2GetNumber("ClipMidy");
    poslo[2] = poshi[2] = gl2GetNumber("ClipMidz");
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
                    glColor3fv(ConvertDoubleTo<float>(
                        mols->color[ismol][MSsoln], glf1, 3));
                    gl2DrawBoxFaceD(poslo, poshi, dim == 3 ? 3 : 2);
                }
            }
        }
    }
    return;
}

/* RenderText */
/* RenderSim */
void RenderSim(simptr sim, void* data)
{
    graphicsssptr graphss;
    double pt1[DIMMAX], pt2[DIMMAX];
    int dim;
    wallptr* wlist;
    GLfloat glf1[4];

    graphss = sim->graphss;
    if (!graphss || graphss->graphics == 0)
        return;

    dim = sim->dim;
    wlist = sim->wlist;
    if (dim < 3)
        glClear(GL_COLOR_BUFFER_BIT);
    else
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (dim == 3)
        RenderMolecs(sim);

    if (graphss->framepts) { // draw bounding box
        pt1[0] = wlist[0]->pos;
        pt2[0] = wlist[1]->pos;
        pt1[1] = dim > 1 ? wlist[2]->pos : 0;
        pt2[1] = dim > 1 ? wlist[3]->pos : 0;
        pt1[2] = dim > 2 ? wlist[4]->pos : 0;
        pt2[2] = dim > 2 ? wlist[5]->pos : 0;
        glColor4fv(ConvertDoubleTo<float>(graphss->framecolor, glf1, 4));
        glLineWidth((GLfloat)graphss->framepts);
        gl2DrawBoxD(pt1, pt2, dim);
    }

    if (graphss->gridpts) {
        pt1[0] = sim->boxs->min[0];
        pt2[0] = pt1[0] + sim->boxs->size[0] * sim->boxs->side[0];
        pt1[1] = dim > 1 ? sim->boxs->min[1] : 0;
        pt2[1] = dim > 1 ? pt1[1] + sim->boxs->size[1] * sim->boxs->side[1] : 0;
        pt1[2] = dim > 2 ? sim->boxs->min[2] : 0;
        pt2[2] = dim > 2 ? pt1[2] + sim->boxs->size[2] * sim->boxs->side[2] : 0;
        glColor4fv(ConvertDoubleTo<float>(graphss->gridcolor, glf1, 4));
        if (dim == 1)
            glPointSize((GLfloat)graphss->gridpts);
        else
            glLineWidth((GLfloat)graphss->gridpts);
        gl2DrawGridD(pt1, pt2, sim->boxs->side, dim);
    }

    if (dim < 3)
        RenderMolecs(sim);

    if (sim->srfss)
        RenderSurfaces(sim);
    if (sim->filss)
        RenderFilaments(sim);
    if (sim->latticess)
        RenderLattice(sim);

#if 0
    if (graphss->ntextitems)
        RenderText(sim);
#endif

    return;
}
} // namespace gui
