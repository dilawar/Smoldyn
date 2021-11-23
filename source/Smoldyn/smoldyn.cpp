/* Steven Andrews, started 10/22/2001.
 This is the entry point for the Smoldyn program.
 See documentation called SmoldynUsersManual.pdf and SmoldynCodeDoc.pdf, and the
 Smoldyn website, which is at www.smoldyn.org. Copyright 2003-2016 by Steven
 Andrews.  This work is distributed under the terms of the Gnu Lesser General
 Public License (LGPL). */

#include "SimCommand.h"
#include "random2.h"
#include "string2.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "smoldyn.h"
#include "smoldynfuncs.h"

#ifdef OPTION_VCELL
#include "SimpleMesh.h"
#include "SimpleValueProvider.h"
#endif

/* Steven Andrews, started 10/22/2001.
 This is the entry point for the Smoldyn program.
 See documentation called SmoldynUsersManual.pdf and SmoldynCodeDoc.pdf, and the
 Smoldyn website, which is at www.smoldyn.org. Copyright 2003-2016 by Steven
 Andrews.  This work is distributed under the terms of the Gnu Lesser General
 Public License (LGPL). */

#include "SimCommand.h"
#include "random2.h"
#include "string2.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "smoldyn.h"
#include "smoldynfuncs.h"

#ifdef OPTION_VCELL
#include "SimpleMesh.h"
#include "SimpleValueProvider.h"
#endif

#include "opengl2.h"

#ifdef USE_IMGUI
#include "../gui/window.hpp"
#endif // USE_IMGUI

/* main */
int main(int argc, char** argv)
{
    simptr sim;
    int i, er, pflag, wflag, tflag, Vflag, oflag;
    char root[STRCHAR], fname[STRCHAR], flags[STRCHAR], *cptr;

    for (i = 0; i < STRCHAR; i++)
        root[i] = fname[i] = flags[i] = '\0';

    er = 0;
    if (argc <= 1) {
        fprintf(stderr, "Welcome to Smoldyn version %s.\n\n", VERSION);
        fprintf(stderr, "Enter name of configuration file: ");
        fgets(root, STRCHAR, stdin);
        if (strchr(root, '\n'))
            *(strchr(root, '\n')) = '\0';
        fprintf(stderr,
            "Enter runtime flags (q=quiet, p=parameters only), or "
            "'-'=none: ");
        fgets(flags, STRCHAR, stdin);
        if (strchr(flags, '\n'))
            *(strchr(flags, '\n')) = '\0';
    }
    if (argc > 1) {
        strncpy(root, argv[1], STRCHAR - 1);
        root[STRCHAR - 1] = '\0';
        argc--;
        argv++;
    }
    er = Parse_CmdLineArg(&argc, argv, NULL);
    if (er) {
        if (er == 1)
            fprintf(stderr, "Out of memory");
        else
            fprintf(stderr,
                "Follow command line '--define' options with "
                "key=replacement\n");
        return 0;
    }
    if (argc > 1) {
        if (argv[1][0] == '-') {
            strncpy(flags, argv[1], STRCHAR - 1);
            flags[STRCHAR - 1] = '\0';
            strcpy(SimFlags, flags);
            argc--;
            argv++;
        } else {
            fprintf(stderr,
                "Command line format: smoldyn [config_file] [-options] "
                "[-OpenGL_options]\n");
            return 0;
        }
    }

    cptr = strrpbrk(root, ":\\/");
    if (cptr)
        cptr++;
    else
        cptr = root;
    strcpy(fname, cptr);
    *cptr = '\0';

    oflag = strchr(flags, 'o') ? 1 : 0;
    pflag = strchr(flags, 'p') ? 1 : 0;
    Vflag = strchr(flags, 'V') ? 1 : 0;
    if (!strcmp(fname, "-V"))
        Vflag = 1;
    wflag = strchr(flags, 'w') ? 1 : 0;
    tflag = strchr(flags, 't') ? 1 : 0;

    if (Vflag) {
        simLog(NULL, 4, "%s\n", VERSION);
        return 0;
    }
    sim = NULL;

#ifdef OPTION_VCELL
    er = simInitAndLoad(root, fname, &sim, flags,
        new SimpleValueProviderFactory(), new SimpleMesh());
#else
    er = simInitAndLoad(root, fname, &sim, flags);
#endif

    if (!er)
        er = simUpdateAndDisplay(sim);

    if (!oflag && !pflag && !er)
        er = scmdopenfiles((cmdssptr)sim->cmds, wflag);

    if (pflag || er) {
        simLog(sim, 4, "%sSimulation skipped\n", er ? "\n" : "");
        fflush(stdout);
        fflush(stderr);
        return 0;
    }

    smoldyn::Window win(sim);
    er = win.simulate();

    // All done.
    simfree(sim);
    simfuncfree();

    return 0;
}
