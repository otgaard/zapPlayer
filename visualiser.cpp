/* Created by Darren Otgaar on 2016/11/19. http://www.github.com/otgaard/zap */
#include "visualiser.hpp"
#include <zap/engine/vertex_buffer.hpp>
#include <zap/engine/mesh.hpp>
#include <zap/maths/algebra.hpp>
#include <zap/engine/program.hpp>
#include <zap/renderer/camera.hpp>

#define LOGGING_ENABLED
#include <zap/tools/log.hpp>

#define GLSL(src) "#version 330 core\n" #src

const char* const vtx_shdr = GLSL(
    in vec2 position;

    uniform mat4 pvm_matrix;

    void main() {
        gl_Position = pvm_matrix * vec4(position, 0, 1);
    }
);

const char* const frg_shdr = GLSL(
    uniform vec4 colour = vec4(1,1,1,1);
    out vec4 frag_colour;
    void main() {
        frag_colour = colour;
    }
);

using namespace zap;
using namespace zap::maths;
using namespace zap::engine;
using namespace zap::renderer;

using pos2_t = core::position<vec2f>;
using vertex_t = vertex<pos2_t>;
using vbuf_t = vertex_buffer<vertex_t, buffer_usage::BU_STATIC_DRAW>;
using mesh_t = mesh<vertex_stream<vbuf_t>, primitive_type::PT_LINE_STRIP>;

struct visualiser::state_t {
    vbuf_t vbuf;
    mesh_t mesh;
    program prog;
    camera cam;

    std::vector<float> bins;

    int width, height;

    state_t() : cam(false), bins(128, 0.f) { }
};

visualiser::visualiser() : state_(new state_t()), s(*state_.get()) {
}

visualiser::~visualiser() = default;

void visualiser::set_frequency_bins(const std::vector<float>& bins) {
    s.bins = bins;
    //LOG(s.bins[0], s.bins[1], s.bins[2], s.bins[3]);
}

bool visualiser::initialise() {
    zap::engine::init();

    s.prog.add_shader(shader_type::ST_VERTEX, vtx_shdr);
    s.prog.add_shader(shader_type::ST_FRAGMENT, frg_shdr);
    if(!s.prog.link()) {
        LOG_ERR("Error initialising visualiser shader");
        return false;
    }

    if(!s.mesh.allocate() || !s.vbuf.allocate()) {
        LOG_ERR("Error allocating mesh or vertex buffer");
        return false;
    }

    s.mesh.set_stream(&s.vbuf);
    s.mesh.bind(); s.vbuf.bind();
    s.vbuf.initialise(128);
    s.mesh.release();

    return true;
}

void visualiser::resize(int width, int height) {
    s.width = width; s.height = height;
    if(s.prog.is_linked()) {
        s.cam.viewport(0, 0, width, height);
        s.cam.frustum(0, width, 0, height, 0, 10.f);

        s.prog.bind();
        s.prog.bind_uniform("pvm_matrix", s.cam.proj_view());
        s.prog.release();

        s.vbuf.bind();
        if(s.vbuf.map(buffer_access::BA_WRITE_ONLY)) {
            float inc = s.width/127.f;
            for(int i = 0; i != 128; ++i) {
                s.vbuf[i].position.set(i*inc, s.bins[i]);
            }
            s.vbuf.unmap();
        }
        s.vbuf.release();
    }
}

void visualiser::update(double t, float dt) {
    s.vbuf.bind();
    if(s.vbuf.map(buffer_access::BA_WRITE_ONLY)) {
        float inc = s.width/127.f;
        for(int i = 0; i != 128; ++i) {
            s.vbuf[i].position.set(i*inc, s.height*s.bins[i]+10);
        }
        s.vbuf.unmap();
    }
    s.vbuf.release();
}

void visualiser::draw() {
    s.prog.bind();
    s.mesh.bind();
    s.mesh.draw();
    s.mesh.release();
    s.prog.release();
}

bool visualiser::is_initialised() const {
    return s.mesh.is_allocated();
}