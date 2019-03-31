import json

import GLWindow
import numpy as np

import moderngl
import rigging
from camera import camera

# bones = [
#     [(0.0, 0.0, 10.0), (0.0, 0.0, 0.0, 1.0)],
#     [(-20.0, 0.0, 10.0), (0.0, 0.0, 0.0, 1.0)],
#     [(-50.0, 0.0, 10.0), (0.0, 0.0, 0.0, 1.0)],
#     [(0.0, 20.0, 10.0), (0.0, 0.0, 0.0, 1.0)],
#     [(0.0, 50.0, 10.0), (0.0, 0.0, 0.0, 1.0)],
#     [(20.0, 0.0, 10.0), (0.0, 0.0, 0.0, 1.0)],
#     [(50.0, 0.0, 10.0), (0.0, 0.0, 0.0, 1.0)],
#     [(0.0, -20.0, 10.0), (0.0, 0.0, 0.0, 1.0)],
#     [(0.0, -50.0, 10.0), (0.0, 0.0, 0.0, 1.0)],
# ]

bones = [
    [(0.0, 0.0, 10.0), (-0.0714221, 0.0542916, 0.0853002, 0.992308)],
    [(-19.5911, 3.54087, 8.08873), (-0.0315694, 0.444767, 0.10668, 0.88871)],
    [(-37.0392, 10.0718, -15.4253), (-0.0624765, 0.153864, 0.0920539, 0.981809)],
    [(3.23066, 19.5049, 13.0202), (-0.43896, 0.0823589, 0.0586576, 0.892799)],
    [(4.2037, 37.7373, 36.8242), (-0.404228, 0.206091, 0.459617, 0.763465)],
    [(19.5911, -3.54087, 11.9113), (-0.0414363, 0.35945, 0.103249, 0.926509)],
    [(41.1992, -10.1742, 31.6366), (-0.0784784, 0.0438327, 0.230336, 0.968951)],
    [(-3.23066, -19.5049, 6.97985), (-0.273743, 0.0706564, 0.0723282, 0.956473)],
    [(-6.22095, -44.6949, -9.03647), (-0.838883, 0.100846, 0.00733659, 0.534838)],
]

skin_data = open('data/character.skin', 'rb').read()
index = json.load(open('data/character.index'))

wnd = GLWindow.create_window()
ctx = moderngl.create_context()

prog = ctx.program(
    vertex_shader='''
        #version 330
        uniform mat4 Mvp;
        in vec3 in_vert;
        in vec3 in_norm;
        out vec3 v_vert;
        out vec3 v_norm;
        void main() {
            v_vert = in_vert;
            v_norm = in_norm;
            gl_Position = Mvp * vec4(v_vert, 1.0);
        }
    ''',
    fragment_shader='''
        #version 330
        uniform vec4 Color;
        uniform vec3 Light;
        in vec3 v_vert;
        in vec3 v_norm;
        out vec4 f_color;
        void main() {
            float lum = dot(normalize(v_norm), normalize(Light - v_vert));
            lum = lum * 0.8 + 0.2;
            f_color = vec4(Color.rgb * lum, 1.0);
        }
    ''',
)

light = prog['Light']
color = prog['Color']
mvp = prog['Mvp']

vbo = ctx.buffer(rigging.skin(skin_data, bones))
ibo = ctx.buffer(np.array(index, dtype='i4').tobytes())
vao = ctx.simple_vertex_array(prog, vbo, 'in_vert', 'in_norm', index_buffer=ibo)

while wnd.update():
    width, height = wnd.size
    ctx.viewport = wnd.viewport
    ctx.clear(1.0, 1.0, 1.0)
    ctx.enable(moderngl.DEPTH_TEST)

    light.value = (-140.0, -300.0, 350.0)
    color.value = (1.0, 1.0, 1.0, 0.25)
    mvp.write(camera((-85, -180, 140), (0.0, 0.0, 25.0)).matrix(ratio=width / height))
    vao.render()
