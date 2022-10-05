#ifndef __FRAMEWORK_IMAGE_HPP
#define __FRAMEWORK_IMAGE_HPP

#include <stdint.h>

class Image {
public:
    int m_width;
    int m_height;
    uint8_t* m_data;

    Image(char const* filename);
    ~Image();
    Image(Image const&) = delete;
};

#endif // __FRAMEWORK_IMAGE_HPP
