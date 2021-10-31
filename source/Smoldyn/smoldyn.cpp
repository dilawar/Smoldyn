/* Steven Andrews, started 10/22/2001.
 This is the entry point for the Smoldyn program.
 See documentation called SmoldynUsersManual.pdf and SmoldynCodeDoc.pdf, and the Smoldyn
 website, which is at www.smoldyn.org.
 Copyright 2003-2016 by Steven Andrews.  This work is distributed under the terms
 of the Gnu Lesser General Public License (LGPL). */

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
 See documentation called SmoldynUsersManual.pdf and SmoldynCodeDoc.pdf, and the Smoldyn
 website, which is at www.smoldyn.org.
 Copyright 2003-2016 by Steven Andrews.  This work is distributed under the terms
 of the Gnu Lesser General Public License (LGPL). */

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
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <imgui.h>

//
// ImGUi callback.
//
static void
glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}
#endif // USE_IMGUI

/* main */
int
main(int argc, char** argv)
{
    int exitCode = 0;

#ifdef USE_IMGUI

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        fprintf(stderr, "Failed to init GLFW windows.");
        return 1;
    }

    //
    // Initialize ImGui
    //
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    //
    // Create windows
    //
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Smoldyn", NULL, NULL);
    if (window == nullptr) {
        fprintf(stderr, "Failed to create main windows.\n");
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

#endif

    try {
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
            fprintf(
              stderr, "Enter runtime flags (q=quiet, p=parameters only), or '-'=none: ");
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
                  "Follow command line '--define' options with key=replacement\n");
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
        er = simInitAndLoad(
          root, fname, &sim, flags, new SimpleValueProviderFactory(), new SimpleMesh());
#else
        er = simInitAndLoad(root, fname, &sim, flags);
#endif
        if (!er) {
            if (!tflag && sim->graphss && sim->graphss->graphics != 0)
                gl2glutInit(&argc, argv);
            er = simUpdateAndDisplay(sim);
        }
        if (!oflag && !pflag && !er)
            er = scmdopenfiles((cmdssptr)sim->cmds, wflag);
        if (pflag || er) {
            simLog(sim, 4, "%sSimulation skipped\n", er ? "\n" : "");
        } else {
            fflush(stdout);
            fflush(stderr);
            if (tflag || !sim->graphss || sim->graphss->graphics == 0) {
                er = smolsimulate(sim);
                endsimulate(sim, er);
            } else {
                smolsimulategl(sim);
            }
        }
        simfree(sim);
        simfuncfree();
    }

    catch (const char* errmsg) {
        fprintf(stderr, "%s\n", errmsg);
        exitCode = 1;
        goto CLEANUP;
    } catch (...) {
        fprintf(stderr, "unknown error\n");
        exitCode = 1;
        goto CLEANUP;
    }

CLEANUP:
#ifdef USE_IMGUI
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (window)
        glfwDestroyWindow(window);
    glfwTerminate();
#endif

    return exitCode;
}
