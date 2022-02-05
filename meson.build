project(
    'libusb', 'cpp',
    version: '1.0.0',
    default_options: [
        'cpp_std=c++11',
    ]
)

inc_lst = [
    include_directories('include'),
]
dep_lst = []
src_lst = []

subdir('src')

libusb = library(
    'libusb',
    src_lst,
    include_directories: inc_lst,
    dependencies: dep_lst,
    install: true
)

libusb_dep = declare_dependency(
    include_directories: inc_lst,
    dependencies: dep_lst,
    link_with: libusb
)

subdir('examples')