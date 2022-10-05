#include <stdexcept>
#include <stb_image.h>
#include "image.hpp"

Image::Image(char const* filename):
    m_width(0), m_height(0), m_data(nullptr)
{
    int bpp = 0;
    m_data = stbi_load(filename, &m_width, &m_height, &bpp, 4);
    if (m_data == nullptr) {
        throw std::runtime_error("error loading image");
    }
}

Image::~Image() {
    stbi_image_free(m_data);
}
