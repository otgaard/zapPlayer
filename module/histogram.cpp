/* Created by Darren Otgaar on 2016/12/04. http://www.github.com/otgaard/zap */
#include "histogram.hpp"
#define LOGGING_ENABLED
#include <zap/tools/log.hpp>
#include <zap/engine/program.hpp>
#include <zap/graphics2/g2_types.hpp>
#include <zap/renderer/camera.hpp>

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

struct histogram::state_t {
    vbuf_t vbuf;
    mesh_t mesh;
    program prog;

    int width, height;
};

histogram::histogram() : state_(new state_t()), s(*state_.get()) {
}

histogram::~histogram() = default;

bool histogram::initialise() {
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
    s.vbuf.initialise(6 * 128); // 6 vertices per quad
    s.mesh.release();

    return true;
}

void histogram::resize(int width, int height) {
    s.width = width; s.height = height;
    if(s.prog.is_linked()) {

        s.prog.bind();
        s.prog.bind_uniform("height", float(height));
        s.prog.release();
    }
}

void histogram::update(float dt, const std::vector<float>& bins) {
    s.vbuf.bind();
    if(s.vbuf.map(buffer_access::BA_WRITE_ONLY)) {
        const size_t samples = std::max<size_t>(bins.size(), 128);
        float inc = s.width/float(samples-1);

        for(int i = 0; i != samples; ++i) {
            auto idx = 6*i;
            auto A = i*inc;
            auto B = A+inc;
            s.vbuf[idx+0].position.set(A, 0.f);
            s.vbuf[idx+1].position.set(B, 0.f);
            s.vbuf[idx+2].position.set(B, bins[i]);
            s.vbuf[idx+3].position.set(A, 0.f);
            s.vbuf[idx+4].position.set(B, bins[i]);
            s.vbuf[idx+5].position.set(A, bins[i]);
        }
        s.vbuf.unmap();
    }
    s.vbuf.release();
}

void histogram::draw(const zap::renderer::camera& cam) {
    s.prog.bind();
    s.prog.bind_uniform("pvm_matrix", cam.proj_view());
    s.mesh.bind();
    s.mesh.draw();
    s.mesh.release();
    s.prog.release();
}
