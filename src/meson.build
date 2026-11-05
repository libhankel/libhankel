inc_root = include_directories('..', '../include', '../external_libs')

# ---- Dependencies ----

inc_dirs = [inc_root]

# Separate library on Linux; part of libSystem on macOS and the C runtime on
# Windows, so required: false avoids a build failure on those platforms.
m_dep = cc.find_library('m', required: false)


# ---- Main C library ----
sources = files(
  'hankel_DHT.c',
  'form_factors.c',
  'hankel_DE_quadrature.c',
  'hankel_G0.c',
  'hankel_QWE.c',
  'hankel_transform.c',
  '../external_libs/DE-quadrature/intde.c', 
  '../external_libs/qwe_Chave/qwe_Chave.c', 
  '../external_libs/qwe_Key/qwe_Key.c', 
  'utils/sasfit_integrate.c',
  '../external_libs/utils/tanhsinh.c',
  'utils/analytical_form_factors.c',
  'utils/sf_functions.c',
  'utils/validate_x.c',
  'utils/boost_bessel.cpp',
  'utils/interp_cubic.cpp',
  'utils/interp_linear.c',
  'utils/interp_loglinear.c',
  'utils/tabulated_ff.c',
  'hankel_fftlog.cpp',
  '../external_libs/fft_log/fft_log.cpp'
)

libhankel = shared_library(
  'libhankel',
  sources: sources,
  include_directories: inc_dirs,
  c_args:   ['-DLIBHANKEL_BUILDING_DLL'],
  cpp_args: ['-DLIBHANKEL_BUILDING_DLL'],
  dependencies: [m_dep],
  install: true,
)

