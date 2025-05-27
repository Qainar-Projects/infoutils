# cpuinfo - CPU Information Utility
# Part of QCO InfoUtils (Qainrat Code Organization InfoUtils)
# Author: AnmiTaliDev
# License: Apache 2.0

project(
  'cpuinfo',
  'cpp',
  version: '1.0',
  license: 'Apache-2.0',
  default_options: [
    'cpp_std=c++17',
    'warning_level=3',
    'werror=false',
    'optimization=2',
    'debug=true'
  ]
)

# Project information
project_description = 'CPU Information Utility'
project_author = 'AnmiTaliDev'
project_organization = 'QCO InfoUtils'

# Compiler setup
cpp_compiler = meson.get_compiler('cpp')

# Required dependencies
filesystem_dep = cpp_compiler.find_library('stdc++fs', required: false)

# Source files
sources = files([
  'cpuinfo.cpp'
])

# Headers
headers = files([
  'cpuinfo.hpp'
])

# Build executable
cpuinfo_exe = executable(
  'cpuinfo',
  sources,
  dependencies: [filesystem_dep],
  install: true,
  install_dir: get_option('bindir')
)

# Summary
summary({
  'Program': 'cpuinfo',
  'Version': meson.project_version(),
  'Author': project_author,
  'Organization': project_organization,
  'License': meson.project_license()[0],
  'C++ Standard': get_option('cpp_std'),
  'Install prefix': get_option('prefix')
}, section: 'Configuration')