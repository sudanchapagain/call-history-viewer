project(
  'chv',
  'cpp',
  version: '1.0.0',
  default_options: [
        'c_std=c++11',
  ]
)

tinyxml2 = dependency('tinyxml2')

sources = files(
  'src/main.cpp'
)

out = executable(
  'chv',
  sources,
  dependencies: [tinyxml2],
  install: true
)
