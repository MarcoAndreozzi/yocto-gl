//
// # Yocto/Image: Tiny C++ Library of Image Utilities
//
// Yocto/Image is a collection of utilities for image loading and saving,
// example image creation, tonempping, image resizing, and sunsky procedural
// images. The library contains only color conversion utilities.
// Yocto/Image is written to support the need of a minimal, but
// fully-featured, global illumination renderer, rather than the need of
// generic image editing. Image IO and resizing are wrappers of `stb_image`
// and `tinyexr` libraries.
//
// Yocto/Img support only two image types, `image4b` for low-dynamic range
// images and `image4f` for high-dynamic range images. Pixels are represented
// as RGBA values of either 8-bytes ints (LDRs) or float (HDRs) types.
//
// ## Usage
//
// 1. create images with `make_image4b()` and `make_image4f()`
// 1. load images with `load_image4b()` or `load_image4f()`
// 2. save images with `save_image4b()` or `save_image4f()`
// 3. resize images with `resize_image()`
// 4. tonemap images with `tonemap_image()`
// 5. make various image examples with the `make_XXX_image()` functions
// 6. create procedural sun-sky images with `make_sunsky_image()`
// 7. we also support low level image IO with `load_XXX()` and `save_XXX()`
//    functions.
// 8. see list below for the color conversion utilities supported
//

//
// LICENSE:
//
// Copyright (c) 2016 -- 2018 Fabio Pellacini
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//

#ifndef _YGL_IMAGE_H_
#define _YGL_IMAGE_H_

// -----------------------------------------------------------------------------
// COMPILATION OPTIONS AND INCLUDES
// -----------------------------------------------------------------------------

#include "yocto_math.h"

#include <string>
#include <vector>

// -----------------------------------------------------------------------------
// COLOR CONVERSION UTILITIES
// -----------------------------------------------------------------------------
namespace ygl {

// Element-wise float to byte conversion.
inline vec4b float_to_byte(const vec4f& a) {
    return {(byte)clamp(int(a.x * 256), 0, 255),
        (byte)clamp(int(a.y * 256), 0, 255),
        (byte)clamp(int(a.z * 256), 0, 255),
        (byte)clamp(int(a.w * 256), 0, 255)};
}
inline vec4f byte_to_float(const vec4b& a) {
    return {a.x / 255.0f, a.y / 255.0f, a.z / 255.0f, a.w / 255.0f};
}

// Conversion between linear and gamma-encoded images.
inline vec3f gamma_to_linear(const vec3f& srgb, float gamma) {
    return {pow(srgb.x, gamma), pow(srgb.y, gamma), pow(srgb.z, gamma)};
}
inline vec3f linear_to_gamma(const vec3f& lin, float gamma) {
    return {
        pow(lin.x, 1 / gamma), pow(lin.y, 1 / gamma), pow(lin.z, 1 / gamma)};
}

// Approximate luminance estimate
inline float luminance(const vec3f& a) { return (a.x + a.y + a.z) / 3; }

// Converts HSV to RGB.
vec3f hsv_to_rgb(const vec3f& hsv);
vec3f rgb_to_hsv(const vec3f& rgb);
// Convert between CIE XYZ and xyY
vec3f xyz_to_xyY(const vec3f& xyz);
vec3f xyY_to_xyz(const vec3f& xyY);
// Convert between CIE XYZ and RGB
vec3f xyz_to_rgb(const vec3f& xyz);
vec3f rgb_to_xyz(const vec3f& rgb);

}  // namespace ygl

// -----------------------------------------------------------------------------
// IMAGE UTILITIES
// -----------------------------------------------------------------------------
namespace ygl {

// Conversion between linear and gamma-encoded images.
std::vector<vec4f> gamma_to_linear(
    const std::vector<vec4f>& srgb, float gamma = 2.2f);
std::vector<vec4f> linear_to_gamma(
    const std::vector<vec4f>& lin, float gamma = 2.2f);

// Conversion from/to floats.
std::vector<vec4f> byte_to_float(const std::vector<vec4b>& bt);
std::vector<vec4b> float_to_byte(const std::vector<vec4f>& fl);

// Conversion between different number of channels
std::vector<vec4f> rgb_to_rgba(const std::vector<vec3f>& rgb);
std::vector<vec3f> rgba_to_rgb(const std::vector<vec4f>& rgba);
std::vector<float> rgba_to_red(const std::vector<vec4f>& rgba);
std::vector<float> rgba_to_green(const std::vector<vec4f>& rgba);
std::vector<float> rgba_to_blue(const std::vector<vec4f>& rgba);
std::vector<float> rgba_to_alpha(const std::vector<vec4f>& rgba);
std::vector<float> rgba_to_luminance(const std::vector<vec4f>& rgba);
std::vector<vec4f> luminance_to_rgba(const std::vector<float>& lum);

// Apply exposure and filmic tone mapping
std::vector<vec4f> expose_image(const std::vector<vec4f>& hdr, float exposure);
std::vector<vec4f> filmic_tonemap_image(const std::vector<vec4f>& hdr);

// Filter type and edge mode for resizing.
enum struct resize_filter {
    def,
    box,
    triangle,
    cubic_spline,
    catmull_rom,
    mitchell
};
enum struct resize_edge { def, clamp, reflect, wrap, zero };

// Resize an image.
std::vector<vec4f> resize_image(int width, int height,
    const std::vector<vec4f>& img, int res_width, int res_height,
    resize_filter filter = resize_filter::def,
    resize_edge edge = resize_edge::def, bool premultiplied_alpha = false);
std::vector<vec4b> resize_image(int width, int height,
    const std::vector<vec4b>& img, int res_width, int res_height,
    resize_filter filter = resize_filter::def,
    resize_edge edge = resize_edge::def, bool premultiplied_alpha = false);

}  // namespace ygl

// -----------------------------------------------------------------------------
// IMAGE IO
// -----------------------------------------------------------------------------
namespace ygl {

// Check if an image is HDR based on filename.
bool is_hdr_filename(const std::string& filename);

// Loads/saves a 4 channel ldr/hdr image.
std::vector<vec4b> load_image4b(
    const std::string& filename, int& width, int& height);
std::vector<vec4f> load_image4f(const std::string& filename, int& width,
    int& height, float ldr_gamma = 2.2f);
bool save_image4b(const std::string& filename, int width, int height,
    const std::vector<vec4b>& img);
bool save_image4f(const std::string& filename, int width, int height,
    const std::vector<vec4f>& img, float ldr_gamma = 2.2f);
std::vector<vec4b> load_image4b_from_memory(
    const byte* data, int data_size, int& width, int& height);
std::vector<vec4f> load_image4f_from_memory(const byte* data, int data_size,
    int& width, int& height, float ldr_gamma = 2.2f);

}  // namespace ygl

// -----------------------------------------------------------------------------
// EXAMPLE IMAGES
// -----------------------------------------------------------------------------
namespace ygl {

// Make example images.
std::vector<vec3f> make_grid_image(int width, int height, int tile = 8,
    const vec3f& c0 = {0.5f, 0.5f, 0.5f}, const vec3f& c1 = {0.8f, 0.8f, 0.8f});
std::vector<vec3f> make_checker_image(int width, int height, int tile = 8,
    const vec3f& c0 = {0.5f, 0.5f, 0.5f}, const vec3f& c1 = {0.8f, 0.8f, 0.8f});
std::vector<vec3f> make_bumpdimple_image(int width, int height, int tile = 8);
std::vector<vec3f> make_ramp_image(int width, int height, const vec3f& c0,
    const vec3f& c1, float srgb = false);
std::vector<vec3f> make_gammaramp_image(int width, int height);
std::vector<vec3f> make_uv_image(int width, int height);
std::vector<vec3f> make_uvgrid_image(
    int width, int height, int tile = 8, bool colored = true);

// Comvert a bump map to a normal map.
std::vector<vec3f> bump_to_normal_map(
    int width, int height, const std::vector<vec3f>& img, float scale = 1);

// Make a sunsky HDR model with sun at theta elevation in [0,pi/2], turbidity
// in [1.7,10] with or without sun.
std::vector<vec3f> make_sunsky_image(int width, int height, float thetaSun,
    float turbidity = 3, bool has_sun = false,
    const vec3f& ground_albedo = {0.7f, 0.7f, 0.7f});
// Make an image of multiple lights.
std::vector<vec3f> make_lights_image(int width, int height,
    const vec3f& le = {1, 1, 1}, int nlights = 4, float langle = pi / 4,
    float lwidth = pi / 16, float lheight = pi / 16);

// Make a noise image. Wrap works only if both resx and resy are powers of two.
std::vector<vec3f> make_noise_image(
    int width, int height, float scale = 1, bool wrap = true);
std::vector<vec3f> make_fbm_image(int width, int height, float scale = 1,
    float lacunarity = 2, float gain = 0.5f, int octaves = 6, bool wrap = true);
std::vector<vec3f> make_ridge_image(int width, int height, float scale = 1,
    float lacunarity = 2, float gain = 0.5f, float offset = 1.0f,
    int octaves = 6, bool wrap = true);
std::vector<vec3f> make_turbulence_image(int width, int height, float scale = 1,
    float lacunarity = 2, float gain = 0.5f, int octaves = 6, bool wrap = true);

}  // namespace ygl

#endif
