#include <iostream>
#include <GLFW/glfw3.h>

int main (int argc, char* argv[])
{
    using namespace std;

    if (!glfwInit()) {
        cerr << "error initializing glfw" << endl;
        return -1;
    }

    glfwTerminate();
}
