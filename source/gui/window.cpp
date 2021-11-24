//
//   Manage main windows.
//
//  Author:  Dilawar Singh <dilawar.s.rajput@gmail.com>
//

#include "../Smoldyn/smoldyn.h"
#include "../Smoldyn/smoldynfuncs.h"

#include "helper.hpp"
#include "graphics.h"
#include "window.hpp"

#include <vector>
#include <thread>
#include <chrono>

#include "tiffio.h"

namespace smoldyn {

/**
 * Approximate a circle with this many linear segments.
 */
constexpr float NUM_SEGMENT_IN_CIRCLE = 16.0f;

//
// ImGUi callback.
//
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

Window::Window(simptr sim)
    : sim_(sim)
    , paused_(false)
    , matrix_ { 0.f }
    , frame_rate_(20.0f)
    , canvas_({ 720.f, 480.f, 480.f })
{
    // Window doesn't own sim_
}

Window::~Window()
{
    if (ImGui::GetCurrentContext() && isOpenGLEnabled())
        ImGui::DestroyContext();

    if (window_ && isOpenGLEnabled()) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        glfwDestroyWindow(window_);
        glfwTerminate();
    }
}

/**
 * Returns 'true' if opengl is enabled, 'false' otherwise.
 */
bool Window::isOpenGLEnabled() const
{
    if (!sim_)
        return false;
    return (0 != sim_->graphss->graphics);
}

/* changeSize */
void Window::changeSize()
{
    const auto dim = sim_->dim;
    float clipheight, clipwidth;
    float nearold;

    auto w = gui::gGraphicsParam_.CanvasWidth;
    auto h = gui::gGraphicsParam_.CanvasHeight;

    assert(w > 0);
    assert(h > 0);

    gui::gGraphicsParam_.PixWide = w;
    gui::gGraphicsParam_.PixHigh = h;

    if (h == 0)
        h = 1;

    gui::gGraphicsParam_.setViewPort();

    if (gui::gGraphicsParam_.Dimension < 3
        && gui::gGraphicsParam_.Fix2DAspect) {
        if (w <= h) {
            assert(gui::gGraphicsParam_.Zoom != 0);
            clipheight = gui::gGraphicsParam_.ClipSize
                / gui::gGraphicsParam_.Zoom * h / w;
            clipwidth
                = gui::gGraphicsParam_.ClipSize / gui::gGraphicsParam_.Zoom;
        } else {
            clipheight
                = gui::gGraphicsParam_.ClipSize / gui::gGraphicsParam_.Zoom;
            clipwidth = gui::gGraphicsParam_.ClipSize
                / gui::gGraphicsParam_.Zoom * w / h;
        }
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(gui::gGraphicsParam_.ClipLeft,
            gui::gGraphicsParam_.ClipLeft + clipwidth,
            gui::gGraphicsParam_.ClipBot,
            gui::gGraphicsParam_.ClipBot + clipheight,
            gui::gGraphicsParam_.ClipFront, gui::gGraphicsParam_.ClipBack);
        glMatrixMode(GL_MODELVIEW);
        // glLoadIdentity();
    } else if (gui::gGraphicsParam_.Dimension < 3) {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(gui::gGraphicsParam_.ClipLeft, gui::gGraphicsParam_.ClipRight,
            gui::gGraphicsParam_.ClipBot, gui::gGraphicsParam_.ClipTop,
            gui::gGraphicsParam_.ClipFront, gui::gGraphicsParam_.ClipBack);
        glMatrixMode(GL_MODELVIEW);
    } else {
        gui::gGraphicsParam_.Aspect = 1.0 * w / h;

        nearold = gui::gGraphicsParam_.Near;
        if (w >= h)
            gui::gGraphicsParam_.Near = gui::gGraphicsParam_.ClipSize / 2.0
                / ::tan(gui::gGraphicsParam_.FieldOfView * M_PI / 180.0 / 2.0);
        else
            gui::gGraphicsParam_.Near = gui::gGraphicsParam_.ClipSize / 2.0
                / tan(gui::gGraphicsParam_.FieldOfView
                    * gui::gGraphicsParam_.Aspect * M_PI / 180.0 / 2.0);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(gui::gGraphicsParam_.FieldOfView,
            gui::gGraphicsParam_.Aspect, gui::gGraphicsParam_.Near,
            gui::gGraphicsParam_.ClipSize + gui::gGraphicsParam_.Near);

        glMatrixMode(GL_MODELVIEW);
        glGetFloatv(GL_MODELVIEW_MATRIX, matrix_.data());

        // glLoadIdentity();
        glTranslatef(0, 0, nearold - gui::gGraphicsParam_.Near);

        if (dim == 3)
            rotateScene();

        glMultMatrixf(matrix_.data());
    }

    return;
}

/**
 * Rotation of scene.
 *
 * There are two ways to rotate the scene.
 *
 * 1. Drag and rotate using mouse.
 * 2. Using the sliders from gui.
 *
 * TODO: To make rotate efficient, unless there is a change in angle, we don't
 * rotate.
 */
void Window::rotateScene()
{
    // Mouse drag.
    const float r = 2000.0f;

    const auto del = ImGui::GetMouseDragDelta();

    angles_[0] += gui::rad2deg(del.x / r);
    angles_[1] += gui::rad2deg(del.y / r);

    // slider.
    for (size_t i = 0; i < 3; ++i) {

        angles_[i] = std::fmod(angles_[i], 180.f);

        const float theta = angles_[i];
        if (i == 0)
            glRotatef(theta, 1.f, 0.f, 0.f);
        else if (i == 1)
            glRotatef(theta, 0.f, 1.f, 0.f);
        else
            glRotatef(theta, 0.f, 0.f, 1.f);

        old_angles_[i] = angles_[i];
    }
}

/**
 * RenderSim
 */
void Window::renderSim()
{
    std::array<double, DIMMAX> pt1 = { 0.f }, pt2 = { 0.f };

    auto graphss = sim_->graphss;
    if (!graphss || graphss->graphics == 0)
        return;

    auto dim = sim_->dim;
    auto wlist = sim_->wlist;

    gui::Initialize(sim_);
    gui::gGraphicsParam_.computeOpenGLSize();

    changeSize();

    if (dim == 3)
        gui::RenderMolecs(sim_);

    //
    // Draw bounding box
    //
    if (graphss->framepts) {
        pt1[0] = wlist[0]->pos;
        pt2[0] = wlist[1]->pos;
        pt1[1] = dim > 1 ? wlist[2]->pos : 0;
        pt2[1] = dim > 1 ? wlist[3]->pos : 0;
        pt1[2] = dim > 2 ? wlist[4]->pos : 0;
        pt2[2] = dim > 2 ? wlist[5]->pos : 0;

        // fmt::print("Drawing bounding box: {} {}: {} {}\n", graphss->framepts,
        //     graphss->framecolor, pt1, pt2);

        glColor4fv(&graphss->framecolor[0]);
        glLineWidth((float)graphss->framepts);

        gui::DrawBoxD(pt1.data(), pt2.data(), dim);
    }

    if (graphss->gridpts) {
        pt1[0] = sim_->boxs->min[0];
        pt2[0] = pt1[0] + sim_->boxs->size[0] * sim_->boxs->side[0];
        pt1[1] = dim > 1 ? sim_->boxs->min[1] : 0;
        pt2[1]
            = dim > 1 ? pt1[1] + sim_->boxs->size[1] * sim_->boxs->side[1] : 0;
        pt1[2] = dim > 2 ? sim_->boxs->min[2] : 0;
        pt2[2]
            = dim > 2 ? pt1[2] + sim_->boxs->size[2] * sim_->boxs->side[2] : 0;
        glColor4fv(&graphss->gridcolor[0]);
        if (dim == 1)
            glPointSize((float)graphss->gridpts);
        else
            glLineWidth((float)graphss->gridpts);
        gui::DrawGridD(pt1.data(), pt2.data(), sim_->boxs->side, dim);
    }

    if (dim < 3)
        gui::RenderMolecs(sim_);

    if (sim_->srfss)
        gui::RenderSurfaces(sim_);
    if (sim_->filss)
        gui::RenderFilaments(sim_);
    if (sim_->latticess)
        gui::RenderLattice(sim_);

    return;
}

int Window::initGLFW()
{
    // do nothing when 'graphics none' is set.
    if (!isOpenGLEnabled())
        return 0;

    // Else add callback.
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        fprintf(stderr, "Failed to init GLFW windows.");
        return 1;
    }

    //
    // Initialize ImGui
    //
#if defined(__APPLE__)
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#else
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);

    //
    // Create windows
    //
    window_ = glfwCreateWindow(canvas_[0], canvas_[1],
        fmt::format("Smoldyn {}", VERSION).c_str(), NULL, NULL);

    if (window_ == nullptr) {
        fprintf(stderr, "Failed to create main window.\n");
        return 1;
    }

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1); // Enable vsync

#ifdef IMGUI_IMPL_OPENGL_LOADER_GL3W
    bool err = glewInit() != GLEW_OK;
#else
    bool err = false;
#endif

    if (err) {
        fprintf(stderr, "Failed to initialize OpenGL loaded!\n");
        return 1;
    }

    // Setup Dear ImGui context.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    glfwSwapInterval(1);

    ImGuiIO& io = ImGui::GetIO();

    // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    io.DisplaySize = ImVec2(canvas_[0], canvas_[1]);
    io.DeltaTime = 1.0f / frame_rate_;

    return 0;
}

int Window::graphicsUpdate()
{
    //
    // Initialize GraphicsParam from sim
    //
    assert(sim_);
    assert(sim_->graphss);
    auto graphss = sim_->graphss;
    graphss->condition = SCparams;
    gui::GraphicsUpdate(sim_);
    return 0;
}

/**
 * Simulate: return '0' on success.
 */
int Window::simulate()
{
    initGLFW(); // GLFW Window

    int er;
    simLog(sim_, 2, "Simulating\n");
    sim_->clockstt = time(NULL);

    er = simdocommands(sim_);
    if (er)
        return 1;

    while ((er = simulatetimestep(sim_)) == 0) {
        if (!paused_) {
            graphicsUpdate();
            nframe_ += 1;
            sim_->elapsedtime += difftime(time(NULL), sim_->clockstt);
        } else {
            fprintf(stdout, "Paused.\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        renderScene();
    }
    return 0;
}

void Window::updateCanvasSize()
{
    ImGuiIO& io = ImGui::GetIO();
    canvas_[0] = io.DisplaySize.x;
    canvas_[1] = io.DisplaySize.y;
}

size_t Window::getWidth() { return ImGui::GetIO().DisplaySize.x; }

size_t Window::getHeight() { return ImGui::GetIO().DisplaySize.y; }

int Window::renderScene()
{
    bool render = false;

    static int nthframe = 0;
    nthframe += 1; // count the frames to be skipped.

    auto graphss = sim_->graphss;
    if (!graphss || graphss->graphics == 0) {
        return 1;
    }

    //
    // render only every graphss->graphssit frame.
    //
    if (nthframe == graphss->graphicit) {
        nthframe = 0;
        render = true;
    }

    if (!render)
        return -1;

    // Update canvas size.
    updateCanvasSize();

    auto dim = sim_->dim;

    // Poll events.
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    //
    // Drawing starts.
    //
    ImGui::SetNextWindowPos({ canvas_[0], 0 }, 0, { 1.f, 0 });
    ImGui::SetNextWindowSize({ 0.f, canvas_[1] });

    ImGui::Begin("Menu");

    //
    // Pause/Run button.
    //
    ImGui::TextColored(ArrToColorVec(sim_->graphss->backcolor, true),
        _format("Frame={}, Time={} s", nframe_, sim_->elapsedtime).c_str());

    if (!paused_ && ImGui::Button("⏸ Pause"))
        paused_ = true;
    if (paused_ && ImGui::Button(" ⏵ Run "))
        paused_ = false;

    ImGui::SameLine();
    if (ImGui::Button("Quit")) {
        printf("Quit\n");
    }

    ImGui::InputText("", snapshotName_, 40);
    ImGui::SameLine();
    if (ImGui::Button("Snapshot")) {
        writeTIFF(snapshotName_, "OpenGL picture", -1);
        numSnapshots_ += 1;
        strcpy(
            snapshotName_, _format("OpenGL{:05d}.tif", numSnapshots_).c_str());
    }
    ImGui::Separator();

    //
    // Rotation.
    //
    if (dim == 3) {
        ImGui::SliderAngle("Rot X", &angles_[0], -90.f, 90.f);
        ImGui::SliderAngle("Rot Y", &angles_[1], -90.f, 90.f);
        ImGui::SliderAngle("Rot Z", &angles_[2], -90.f, 90.f);
        ImGui::Separator();
    }

    //
    // View / FoV etc.
    //
    ImGui::SliderFloat("Zoom", &gui::gGraphicsParam_.Zoom, 1.0f, 4.0f);
    ImGui::SliderFloat("FoV", &gui::gGraphicsParam_.FieldOfView, -180.f, 180.f);
    ImGui::Separator();

    ImGui::ColorEdit4("Bg Color", sim_->graphss->backcolor);
    ImGui::ColorEdit4("Frame Color", sim_->graphss->framecolor);
    ImGui::ColorEdit4("Text Color", sim_->graphss->textcolor);
    ImGui::ColorEdit4("Grid Color", sim_->graphss->gridcolor);

    gui::gGraphicsParam_.CanvasOffsetX = 10 + ImGui::GetWindowWidth();

    ImGui::Separator();
    ImGui::End();

    // Render the simulation.
    if (!paused_)
        renderSim();

    //
    // Render the window
    //
    ImGui::Render();

    //
    // Clean the window.
    //
    int display_w = 0, display_h = 0;
    glfwGetFramebufferSize(window_, &display_w, &display_h);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window_);

    return 0;
}

/** writeTIFF
 *
 * NOTE: The following code was modified from a program called writetiff.c that
 * was written and copyrighted by Mark Kilgard, 1997.  This function requires
 * the use of the libtiff library that was written by Sam Leffler and can be
 * downloaded from www.libtiff.org.
 */
int Window::writeTIFF(
    const char* filename, const char* description, int compression)
{
    TIFF* file;
    GLubyte *image, *p;

    GLint viewport[4] = { 0 };
    glGetIntegerv(GL_VIEWPORT, viewport);

    auto x = viewport[0];
    auto y = viewport[1];
    auto width = viewport[2];
    auto height = viewport[3];

    if (compression == -1)
        compression = COMPRESSION_PACKBITS;
    file = TIFFOpen(filename, "w");

    if (!file)
        return 1;

    image = (GLubyte*)malloc(width * height * sizeof(GLubyte) * 3);
    if (!image)
        return 1;

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);
    TIFFSetField(file, TIFFTAG_IMAGEWIDTH, (unsigned int)width);
    TIFFSetField(file, TIFFTAG_IMAGELENGTH, (unsigned int)height);
    TIFFSetField(file, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(file, TIFFTAG_COMPRESSION, compression);
    TIFFSetField(file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(file, TIFFTAG_SAMPLESPERPIXEL, 3);
    TIFFSetField(file, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(file, TIFFTAG_ROWSPERSTRIP, 1);
    TIFFSetField(file, TIFFTAG_IMAGEDESCRIPTION, description);
    p = image;

    for (int i = height - 1; i >= 0; i--) {
        if (TIFFWriteScanline(file, p, i, 0) < 0) {
            free(image);
            TIFFClose(file);
            return 1;
        }
        p += width * sizeof(GLubyte) * 3;
    }
    TIFFClose(file);
    free(image);
    return 0;
}

} // namespace smoldyn
