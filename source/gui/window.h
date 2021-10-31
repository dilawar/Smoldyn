//
//  Smoldyn GUI.
//   Author:  Dilawar Singh (), dilawar@subcom.tech
//

#ifndef WINDOW_H_VQ6821WA
#define WINDOW_H_VQ6821WA

#include <utility>

#include <GLFW/glfw3.h>

using namespace std;

namespace smoldyn {

class Window {

public:
    Window(const char* name);
    ~Window();

    inline GLFWwindow* ref() const { return window_; }

    int simulate(simptr sim);

    int render_molecules();

    int render();

    int init();
    int clear();

private:
    /* data */
    const char* name_;
    simptr sim_;

    GLFWwindow* window_;
    int error_code_;

    size_t width_;
    size_t height_;

    bool initialized_;
};

}

#endif /* end of include guard: WINDOW_H_VQ6821WA */
