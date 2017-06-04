/* Created by Darren Otgaar on 2017/06/03. http://www.github.com/otgaard/zap */
#include "texture_mod.hpp"
#define LOGGING_ENABLED
#include <tools/log.hpp>
#include <maths/io.hpp>

#include <maths/functions.hpp>
#include <graphics2/g2_types.hpp>
#include <engine/program.hpp>
#include <renderer/camera.hpp>
#include <generators/noise/perlin.hpp>
#include <engine/framebuffer.hpp>
#include <engine/sampler.hpp>

using namespace zap;
using namespace zap::maths;
using namespace zap::engine;
using namespace zap::renderer;
using namespace zap::graphics;

const char* const texmod_vshdr = GLSL(
    in vec2 position;

    out vec2 pos;
    out highp vec2 texcoord;

    uniform mat4 pvm;
    uniform vec2 dims;

    void main() {
        pos = position * dims;
        texcoord = position;
        gl_Position = pvm * vec4(position.x, position.y, 0., 1.);
    }
);

//#define FAST_PATTERNS
#define HISTOBARS

#if defined(FAST_PATTERNS)
const char* const texmod_fshdr = GLSL(
    float step(float a, float x) { return float(x >= a); }
    float pulse(float a, float b, float x) { return step(a, x) - step(b, x); }

    uniform vec2 dims;
    uniform float fft[16];

    in vec2 pos;

    out vec4 frag_colour;
    void main() {

        float scaleA = 10.*(fft[0]+fft[1]+fft[2]+fft[3]);
        float scaleB = 25.*(fft[4]+fft[5]+fft[6]+fft[7]);
        float scaleC = 50.*(fft[8]+fft[9]+fft[10]+fft[11]);
        float scaleD = 75.*(fft[12]+fft[13]+fft[14]+fft[15]);
        float scaleE = 125.*(fft[0]+fft[1]+fft[2]+fft[3]);

        vec2 P = vec2(pos.x + 1371., pos.y + 4832.);

        float rx = pulse(.0, fft[0], mod(P.x, scaleA)/scaleA);
        float gx = pulse(.0, fft[1], mod(P.x, scaleB)/scaleB);
        float bx = pulse(.0, fft[2], mod(P.x, scaleC)/scaleC);
        float ry = pulse(.0, fft[3], mod(P.y, scaleA)/scaleA);
        float gy = pulse(.0, fft[4], mod(P.y, scaleB)/scaleB);
        float by = pulse(.0, fft[5], mod(P.y, scaleC)/scaleC);
        float rxy = pulse(.0, fft[6], mod(P.y + P.x, scaleC)/scaleC);
        float gxy = pulse(.0, fft[7], mod(P.y + P.x, scaleD)/scaleD);
        float bxy = pulse(.0, fft[8], mod(P.y + P.x, scaleE)/scaleE);
        float ryx = pulse(.0, fft[9], mod(P.y - P.x, scaleC)/scaleC);
        float gyx = pulse(.0, fft[10], mod(P.y - P.x, scaleD)/scaleD);
        float byx = pulse(.0, fft[11], mod(P.y - P.x, scaleE)/scaleE);

        float bass = pulse(.0, fft[0], mod(P.x, scaleE)/scaleE);

        frag_colour = vec4((gx+gy-gxy-gyx+bass-bxy), (rx+ry-rxy-ryx-bass+rxy), (bx+by-bxy-byx+bass-gxy), 1.);
    }
);
#elif defined(HISTOBARS)
const char* const texmod_fshdr = GLSL(
    const float logH = -0.30103;
    float step(float a, float x) { return float(x >= a); }
    float pulse(float a, float b, float x) { return step(a, x) - step(b, x); }
    float gamma(float g, float x) { return pow(x, 1./g); }
    vec4 gamma4(float g, vec4 v) { return vec4(gamma(g, v.x), gamma(g, v.y), gamma(g, v.z), v.w); }
    float bias(float b, float x) { return pow(x, log(b)/logH); }
    float gain(float g, float x) { return x < .5 ? .5*bias(1. - g, 2.*x) : 1 - .5*bias(1. - g, 2 - 2*x); }

    uniform vec2 dims;
    uniform float fft[128];

    in vec2 texcoord;

    out vec4 frag_colour;
    void main() {
        float e = 0.;
        for(int i = 0; i != 128; ++i) {
            e += fft[i] * pulse(i/128., (i+1)/128., texcoord.x) *
                    pulse(0., .8, mod(texcoord.x, 1./128.)*128) *
                    pulse(0., fft[i], texcoord.y) *
                    pulse(0., .8, mod(texcoord.y, 0.02)/0.02);
        }

        frag_colour = gamma4(1.8, e * mix(vec4(1,1,0,1), vec4(1,0,0,1), gain(.8, texcoord.y)));
    }
);
#else

#endif
struct texture_mod::state_t {
    framebuffer fbuf[2];
    vbuf_p2_t vbuf;
    mesh_p2_tfan_t mesh;
    camera cam;
    program prog;
    float time;
};

texture_mod::texture_mod() : state_(new state_t()), s(*state_.get()) {
}

texture_mod::~texture_mod() = default;

bool texture_mod::initialise() {
    if(!s.mesh.allocate() || !s.vbuf.allocate()) {
        LOG_ERR("Failed to allocate resources");
        return false;
    }

    s.mesh.bind(); s.vbuf.bind();
    s.mesh.set_stream(&s.vbuf);

    std::vector<vtx_p2_t> quad = { {vec2f{0.f, 0.f}}, {vec2f{+1.f, 0.f}}, {vec2f{+1.f, +1.f}}, {vec2f{0.f, +1.f}} };
    if(!s.vbuf.initialise(quad)) {
        LOG_ERR("Failed to initialise quad");
        return false;
    }

    s.cam.set_perspective(false);
    s.prog.add_shader(shader_type::ST_VERTEX, texmod_vshdr);
    s.prog.add_shader(shader_type::ST_FRAGMENT, texmod_fshdr);
    if(!s.prog.link()) {
        LOG_ERR("Failed to build shader program");
        return false;
    }

    if(!s.fbuf[0].allocate() || !s.fbuf[1].allocate()) {
        LOG_ERR("Failed to allocate framebuffers");
        return false;
    }

    if(!s.fbuf[0].initialise(1, 256, 256, pixel_format::PF_RGB, pixel_datatype::PD_FLOAT, false, false) ||
       !s.fbuf[1].initialise(1, 256, 256, pixel_format::PF_RGB, pixel_datatype::PD_FLOAT, false, false)) {
        LOG_ERR("Failed to initialise framebuffers");
        return false;
    }

    s.time = 0.f;

    return true;
}

void texture_mod::resize(int width, int height) {
    s.cam.frustum(0.f, 1.f, 0.f, 1.f, 0.f, 1.f);
    s.cam.viewport(0.f, 0.f, 1.f, 1.f);
    s.cam.orthogonolise(vec3f{0.f, 0.f, -1.f});
    s.cam.world_pos(vec3f{0.f, 0.f, .1f});
    s.prog.bind();
    s.prog.bind_uniform("pvm", s.cam.proj_view());
    s.prog.bind_uniform("dims", vec2f{float(width), float(height)});
    s.prog.release();
}

void texture_mod::update(float dt, const std::vector<float>& samples) {
    s.prog.bind();
    s.time += dt;
#if defined(FAST_PATTERNS)
    std::vector<float> samps(16);
    for(int i = 0; i != 128; ++i) samps[i/8] += samples[i]/8.f;
    s.prog.bind_uniform("fft", samps);
#elif defined(HISTOBARS)
    s.prog.bind_uniform("fft", samples);
#endif
    //s.prog.bind_uniform("time", s.time);
    s.prog.release();
}

void texture_mod::draw(const zap::renderer::camera& cam) {
    s.prog.bind();
    s.mesh.bind();
    s.mesh.draw();
    s.mesh.release();
    s.prog.release();
}
