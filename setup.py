from setuptools import Extension, setup

ext = Extension(
    name='rigging',
    sources=['rigging.cpp'],
)

setup(
    name='rigging',
    version='0.1.0',
    ext_modules=[ext],
)
