/* Created by Darren Otgaar on 2017/05/28. http://www.github.com/otgaard/zap */
#define LOGGING_ENABLED
#include <tools/log.hpp>
#include <maths/io.hpp>

#include <maths/functions.hpp>

#include <graphics3/g3_types.hpp>
#include <engine/program.hpp>
#include <renderer/camera.hpp>
#include "surface.hpp"
#include <generators/noise/perlin.hpp>

#include <graphics2/plotter/plot_sampler.hpp>

using namespace zap;
using namespace zap::maths;
using namespace zap::engine;
using namespace zap::renderer;
using namespace zap::graphics;

const int cols = 257, rows = cols;

const char* const surface_vshdr = GLSL(
    in vec3 position;
    in vec3 normal;
    in float pointsize;

    out vec3 pos;
    out vec3 nor;
    out float psize;

    uniform mat4 pvm;

    void main() {
        pos = position;
        nor = normal;
        psize = pointsize;
        gl_Position = pvm * vec4(position.x, position.y, pointsize, 1.);
    }
);

const char* const surface_fshdr = GLSL(
    const vec3 light_dir = normalize(vec3(0., .707, -.707));

    uniform float fft[16];
    uniform int light_count;
    uniform vec3 light_pos[5];
    uniform vec3 light_col[5];
    uniform mat4 mv;

    in vec3 pos;
    in vec3 nor;
    in float psize;

    out vec4 frag_colour;

    const vec4 colA = vec4(1, 1, 0, 1);
    const vec4 colB = vec4(1, 0, 0, 1);
    const vec4 colC = vec4(0, 1, 0, 1);
    const vec4 colD = vec4(0, 0, 1, 1);

    void main() {
        vec3 P = (mv * vec4(pos, 1.)).xyz;
        vec3 N = (mv * vec4(nor, 0.)).xyz;
        vec3 H = normalize(normalize(-P) + light_dir);
        float spec = pow(max(0., dot(N, H)), 50);
        float diff = max(0., dot(N, light_dir));

        float u = .6*(10.*psize)+.2*fft[1]+.2*fft[2];
        float v = .6*(pos.y+.5)+.2f*fft[2]+.2f*fft[3];

        vec4 col = mix(mix(colA, colB, u), mix(colC, colD, u), v);
        frag_colour = diff * col + diff * spec * vec4(1., 1., 1., 1.);
    }
);

// Tessellates a uniform patch

template <typename Index>
struct patch_tessellator {
    static_assert(std::is_integral<Index>::value, "Index must be integral");

    using type = Index;
    using coord_t = vec2<type>;
    using tri_t = vec3<type>;
    using quad_t = vec4<type>;

    const type cols, rows, quadcols, quadrows;

    patch_tessellator(type cols, type rows) : cols(cols), rows(rows), quadcols(cols-1), quadrows(rows-1) { }

    type operator()(type c, type r) const { return cols * r + c; }
    type index(type c, type r) const { return cols * r + c; }
    type operator()(const coord_t& c) const { return operator()(c.x, c.y); }
    coord_t operator[](type idx) const { return coord_t{idx / cols, idx % cols}; }
    type col(type idx) const { return idx % cols; }
    type row(type idx) const { return idx / cols; }
    type quad_count() const { return quadcols*quadrows; }
    type tri_count() const { return 2*quad_count(); }

    tri_t tri(type idx) const {
        auto quad = idx / 2;
        auto qr = quad / quadcols;
        auto qc = quad % quadcols;
        return idx % 2 == 0 ? tri_t(index(qc,qr), index(qc+1,qr), index(qc+1,qr+1))
                            : tri_t(index(qc,qr), index(qc+1,qr+1), index(qc,qr+1));
    }

    quad_t quad(type idx) const {
        auto qr = idx / quadcols;
        auto qc = idx % quadcols;
        return quad_t(index(qc,qr), index(qc+1,qr), index(qc+1,qr+1), index(qc,qr+1));
    }

    template <typename IBuffer>
    bool tessellate(IBuffer& ibuf) {
        std::vector<type> idx(tri_count()*3);

        for(auto i = 0; i != tri_count(); ++i) {
            const auto t = tri(i);
            for(auto e = 0; e != 3; ++e) idx[3*i+e] = t[e];
        }

        return ibuf.initialise(idx);
    }
};

#define DERIVATIVE_NORMAL

template <typename VBuffer, typename VBufferPS, typename Sampler>
bool sample_patch(VBuffer& vbuf, VBufferPS& vbuf_ps, Sampler&& fnc, const vec3f& minP, const vec3f& maxP, int cols, int rows) {
    const float dx = (maxP.x - minP.x)/(cols-1);
    const float dy = (maxP.y - minP.y)/(rows-1);
    const float dx2 = 2.f*dx, dy2 = 2.f*dy;
    const float dxh = .5f*dx, dyh = .5f*dy;
    UNUSED(dx2); UNUSED(dy2);
    UNUSED(dxh); UNUSED(dyh);

    vbuf.bind();
    if(vbuf.map(buffer_access::BA_WRITE_ONLY)) {
        for(int r = 0; r != rows; ++r) {
            const int offset = r * cols;
            const float yoff = minP.y + r * dy;
            for(int c = 0; c != cols; ++c) {
                const float xoff = minP.x + c * dx;
                vbuf[offset + c].position = vec3f{xoff, yoff, 0};
            }
        }

        vbuf.unmap();
    } else {
        gl_error_check();
        vbuf.release();
        return false;
    }
    vbuf.release();

    vbuf_ps.bind();
    if(vbuf_ps.map(buffer_access::BA_WRITE_ONLY)) {
        for(int r = 0; r != rows; ++r) {
            const int offset = r * cols;
            const float yoff = minP.y + r * dy;
            for(int c = 0; c != cols; ++c) {
                const float xoff = minP.x + c * dx;
                auto h = fnc(xoff, yoff);
#ifdef DERIVATIVE_NORMAL
                auto dFdu = fnc(xoff+dx, yoff) - fnc(xoff-dx, yoff);
                auto dFdv = fnc(xoff, yoff+dy) - fnc(xoff, yoff-dy);
                auto U = normalise(vec3f{dx2, 0.f, dFdu});
                auto V = normalise(vec3f{0.f, dy2, dFdv});
                auto N = cross(U,V);
                vbuf_ps[offset + c].normal = N;
#else
                vbuf_ps[offset + c].normal = vec3f{0.f, 0.f, 0.f};
#endif
                vbuf_ps[offset + c].pointsize = h;
            }
        }

#ifndef DERIVATIVE_NORMAL
        patch_tessellator<int> tess{cols, rows};

        vbuf.bind();
        if(vbuf.map(buffer_access::BA_READ_ONLY)) {
            for(int i = 0; i != tess.tri_count(); ++i) {
                const auto tri = tess.tri(i);

                const auto A = vec3f{vbuf[tri[0]].position.x, vbuf[tri[0]].position.y, vbuf_ps[tri[0]].pointsize};
                const auto B = vec3f{vbuf[tri[1]].position.x, vbuf[tri[1]].position.y, vbuf_ps[tri[1]].pointsize};
                const auto C = vec3f{vbuf[tri[2]].position.x, vbuf[tri[2]].position.y, vbuf_ps[tri[2]].pointsize};

                auto e0 = normalise(B - A);
                auto e1 = normalise(C - A);
                auto N = cross(e0, e1);
                for(int t = 0; t != 3; ++t) vbuf_ps[tri[t]].normal += N;
            }
            vbuf.unmap();
        }
        vbuf.release();

        for(int i = 0; i != vbuf_ps.vertex_count(); ++i) {
            vbuf_ps[i].normal.normalise();
        }

        vbuf_ps.bind();
#endif
        vbuf_ps.unmap();
    } else {
        gl_error_check();
        vbuf_ps.release();
        return false;
    }
    vbuf_ps.release();
    return true;
}

template <typename VBuffer, typename Sampler>
bool advance_patch(VBuffer& trg_vbuf, VBuffer& src_vbuf, Sampler&& fnc, const vec3f& minP, const vec3f& maxP, int cols, int rows) {
    const float dx = (maxP.x - minP.x)/(cols-1);
    const float dy = (maxP.y - minP.y)/(rows-1);
    const float dx2 = 2.f*dx, dy2 = 2.f*dy;
    UNUSED(dx2); UNUSED(dy2);

    const int advance_size = 1;

    auto b = trg_vbuf.copy(src_vbuf, cols*advance_size, 0, cols*rows-(advance_size*cols));
    if(!b || gl_error_check()) {
        LOG("Error copying buffer!");
        return false;
    }

    trg_vbuf.bind();
    if(trg_vbuf.map(buffer_access::BA_READ_WRITE)) {
        for(int r = rows-advance_size; r != rows; ++r) {
            const int offset = r * cols;
            const float yoff = minP.y + r * dy;
            for(int c = 0; c != cols; ++c) {
                const float xoff = minP.x + c * dx;
                auto h = fnc(xoff, yoff);
                auto dFdu = fnc(xoff + dx, yoff) - fnc(xoff - dx, yoff);
                auto dFdv = fnc(xoff, yoff + dy) - fnc(xoff, yoff - dy);
                auto U = normalise(vec3f{dx2, 0.f, dFdu});
                auto V = normalise(vec3f{0.f, dy2, dFdv});
                auto N = cross(U, V);
                trg_vbuf[offset + c].pointsize = h;
                trg_vbuf[offset + c].normal = N;
            }
        }

        trg_vbuf.unmap();
        trg_vbuf.release();
        return true;
    } else {
        trg_vbuf.release();
        return false;
    }
}

struct surface::state_t {
    vbuf_p3_t vbuf;
    vbuf_n3ps1_t vbuf_ps[2];
    int active;                 // ping-pongs between vbuffers
    ibuf_tri4_t ibuf;
    mesh_p3_n3ps1_trii_t mesh;
    program shdr;
    camera cam;
    generators::noise noise;
    float start;
};

surface::surface() : state_(new state_t{}), s(*state_.get()) {
}

surface::~surface() = default;

bool surface::initialise() {
    s.shdr.add_shader(shader_type::ST_VERTEX, surface_vshdr);
    s.shdr.add_shader(shader_type::ST_FRAGMENT, surface_fshdr);
    if(!s.shdr.link()) {
        LOG_ERR("Failed to compile surface shader");
        return false;
    }

    if(!s.mesh.allocate() || !s.vbuf.allocate() || !s.vbuf_ps[0].allocate() || !s.vbuf_ps[1].allocate() || !s.ibuf.allocate()) {
        LOG_ERR("Error allocating surface module resources");
        return false;
    }

    s.mesh.bind(); s.ibuf.bind();
    s.mesh.set_stream(vertex_stream<vbuf_p3_t, vbuf_n3ps1_t>{&s.vbuf, &s.vbuf_ps[0]});
    s.mesh.set_index(&s.ibuf);

    patch_tessellator<unsigned int> tess{cols, rows};
    tess.tessellate(s.ibuf);

    s.noise.initialise();

    auto fnc = [this](float x, float y)->float {
        return .1f*(.5f+generators::noise::fractal<generators::perlin<float>>(4, .5f, 2.f, 10.f*x, 10.f*y));
    };

    s.vbuf.bind();
    if(!s.vbuf.initialise(cols*rows)) {
        LOG_ERR("Failed to initialise vertex buffer");
        return false;
    }

    s.vbuf_ps[0].bind();
    if(!s.vbuf_ps[0].initialise(cols*rows)) {
        LOG_ERR("Failed to initialise vertex buffer");
    }

    s.mesh.release();

    s.vbuf_ps[1].bind();
    if(!s.vbuf_ps[1].initialise(cols*rows)) {
        LOG_ERR("Failed to initialise vertex buffer");
        return false;
    }
    s.vbuf_ps[1].release();

    timer t;
    t.start();
    s.active = 0;
    if(!sample_patch(s.vbuf, s.vbuf_ps[s.active], fnc, vec3f{-.5f, -.5f, 0.f}, vec3f{.5f, .5f, 0.f}, cols, rows)) {
        LOG_ERR("Failed to sample function");
        return false;
    }
    LOG("Time to build:", t.getf());

    s.start = 0.f;
    return true;
}

void surface::resize(int width, int height) {
    s.cam.viewport(0, 0, width, height);
    s.cam.frustum(45.f, float(width)/height, .5f, 100.f);
    s.cam.world_pos(vec3f{0.f, 0.f, .9f});
    s.cam.orthogonolise(vec3f{0.f, 0.f, -1.f}, vec3f{0.f, 1.f, 0.f});

    s.shdr.bind();
    auto rot = make_rotation<float>(vec3f{1.f, 0.f, 0.f}, PI/3.f);
    s.shdr.bind_uniform("pvm", s.cam.proj_view()*rot);
    s.shdr.bind_uniform("mv", s.cam.world_to_view()*rot);
    //s.shdr.bind_uniform("eye", s.cam.world_pos());
    s.shdr.release();
}

inline float bias(float b, float x) { const float logp2 = std::logf(.5f); return std::powf(x, std::log(b)/logp2); }

void surface::update(float dt, const std::vector<float>& samples) {
    // Summarise into 16 bins
    std::vector<float> samps(16, 0.f);
    for(int i = 0; i != 64; i += 4) {
        auto idx = i / 4;
        const float bias_factor = clamp(.5f + (idx/4)*1/32.f, 0.f, 1.f);
        for(int j = 0; j != 4; ++j) samps[idx] = bias(bias_factor, samples[i + j]);
    }

    s.shdr.bind();
    s.shdr.bind_uniform("fft", samps);
    s.shdr.release();

    std::vector<float> grid(36, 0.f);
    for(int i = 0; i != 4; ++i) {
        for(int j = 0; j != 4; ++j) {
            auto c = i + 1, r = j + 1;
            grid[6*(5-r)+c] = samps[4*j+i];
        }
    }

    const auto start = s.start;

    auto fnc = [&grid, start](float x, float y)->float {
        x += .5f; y += .5f;
        const float nx = x * 5, ny = y * 5;
        const int c1 = int(nx), r1 = int(ny);
        const int c2 = std::min(c1+1, 5), r2 = std::min(r1+1, 5);
        const float u = nx - float(c1), v = ny - float(r1);
        auto h = bilinear(u, v, grid[6*r1+c1], grid[6*r1+c2], grid[6*r2+c1], grid[6*r2+c2]);
        auto g = .1f*h*(.5f+generators::noise::fractal<generators::perlin<float>>(2, .6f, 2.1f, 10.f*x, 10.f*y+start)) + .01f*grid[7];
        return .12f*clamp(h, 0.f, 1.f) + g;
    };

    s.start += .1f*samps[0] + .1f*samps[1];

    /*
    using fnc_sig = decltype(graphics::interpolators::cubic<float>);
    auto sampler = graphics::sampler1D<float, fnc_sig>(samps, graphics::interpolators::cubic<float>);

    auto fnc = [&sampler](float x, float y)->float {
        //constexpr float sample_width = .5f/128;
        //return .1f*(.5f+generators::noise::fractal<generators::perlin<float>>(4, .5f, 2.f, 10.f*x, 10.f*y+s.start));
        //auto d = std::sqrtf(x*x + y*y);
        //if(d >= 0.5f) return 0.f;
        //auto i = int(d/sample_width);
        //return .2f*lerp(d - i*sample_width, samples[i], samples[i+1]);
        auto d = std::sqrtf(x*x + y*y);
        return d >= .5f ? 0.f : .4f*clamp(sampler(2.f*std::sqrtf(x*x + y*y)), 0.f, 1.f);
    };
    */

    sample_patch(s.vbuf, s.vbuf_ps[s.active], fnc, vec3f{-.5f, -.5f, 0.f}, vec3f{.5f, .5f, 0.f}, cols, rows);
    //advance_patch(s.vbuf_ps[s.active == 0 ? 1 : 0], s.vbuf_ps[s.active], fnc, vec3f{-.5f, -.5f, 0.f}, vec3f{.5f, .5f, 0.f}, cols, rows);
    //s.start += 10.f/cols;
}

void surface::draw(const zap::renderer::camera& c) {
    s.shdr.bind();
    s.mesh.bind();
    s.vbuf.bind();
    s.vbuf_ps[s.active].bind();
    s.mesh.set_stream(vertex_stream<vbuf_p3_t, vbuf_n3ps1_t>{&s.vbuf, &s.vbuf_ps[s.active]});
    s.mesh.draw();
    s.active = s.active == 0 ? 1 : 0;
    s.mesh.release();
    s.shdr.release();
}
