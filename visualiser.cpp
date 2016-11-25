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
    uniform float height;

    out float value;

    void main() {
        value = position.y;
        gl_Position = pvm_matrix * vec4(position.x, position.y * height, 0, 1);
    }
);

const char* const frg_shdr = GLSL(
    uniform vec4 colour = vec4(1,1,1,1);
    in float value;
    out vec4 frag_colour;
    void main() {
        frag_colour = vec4(1.0 - value, value, 0, 1);
    }
);

using namespace zap;
using namespace zap::maths;
using namespace zap::engine;
using namespace zap::renderer;

using pos2_t = core::position<vec2f>;
using vertex_t = vertex<pos2_t>;
using vbuf_t = vertex_buffer<vertex_t, buffer_usage::BU_STATIC_DRAW>;
using mesh_t = mesh<vertex_stream<vbuf_t>, primitive_type::PT_TRIANGLES>;

struct visualiser::state_t {
    vbuf_t vbuf;
    mesh_t mesh;
    program prog;
    camera cam;

    std::vector<float> bins;

    int width, height;

    state_t(size_t bins) : cam(false), bins(bins, 0.f) { }
};

visualiser::visualiser(size_t bins) : state_(new state_t(bins)), s(*state_.get()) {
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
    s.vbuf.initialise(6 * s.bins.size()); // 6 vertices per quad
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
        s.prog.bind_uniform("height", float(height));
        s.prog.release();

        update(0,0);
    }
}

void visualiser::update(double t, float dt) {
    s.vbuf.bind();
    if(s.vbuf.map(buffer_access::BA_WRITE_ONLY)) {
        float inc = s.width/float(s.bins.size()-1);
        for(int i = 0; i != s.bins.size(); ++i) {
            auto idx = 6*i;
            auto A = i*inc;
            auto B = A+inc;
            s.vbuf[idx+0].position.set(A, 0.f);
            s.vbuf[idx+1].position.set(B, 0.f);
            s.vbuf[idx+2].position.set(B, s.bins[i]);
            s.vbuf[idx+3].position.set(A, 0.f);
            s.vbuf[idx+4].position.set(B, s.bins[i]);
            s.vbuf[idx+5].position.set(A, s.bins[i]);
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