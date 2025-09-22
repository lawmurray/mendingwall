add_project_arguments('-Wl,--export-dynamic', language: 'c')

if get_option('buildfor') == 'flatpak'
  add_project_arguments('-DBUILD_FOR_FLATPAK', language : 'c')
elif get_option('buildfor') == 'snap'
  add_project_arguments('-DBUILD_FOR_SNAP', language : 'c')
endif

blueprints = custom_target('blueprints',
  input: ['about.blp', 'main.blp'],
  output: '.',
  command: [find_program('blueprint-compiler'), 'batch-compile', '@OUTPUT@', '@CURRENT_SOURCE_DIR@', '@INPUT@'],
)

#
# glib-compile-resources seems problematic in some containers, e.g. Fedora 41
# ARM has an issue at time of writing. As a workaround for now, the generated
# files are included in the source repository. On a platform with support, the
# following lines can be exchanged to regenerate the files dynamically,
# and check updated versions into the repository if necessary.
#
resources = ['resources.h', 'resources.c']
#resources = gnome.compile_resources('resources',
#  'mendingwall.gresource.xml',
#  c_name: 'mendingwall',
#  dependencies: blueprints
#)

# mendingwall program
mendingwall_sources = [
  'mendingwall.c',
  'mendingwallapplication.c',
  'utility.c',
]
mendingwall_sources += blueprints
mendingwall_sources += resources
mendingwall_deps = [
  dependency('gtk4'),
  dependency('libadwaita-1', version: '>= 1.5'),
  dependency('gmodule-export-2.0'),
]
executable('mendingwall', mendingwall_sources,
  dependencies: mendingwall_deps,
       install: true,
)

# mendingwallcli program
mendingwallcli_sources = [
  'mendingwallcliapplication.c',
  'mendingwallcli.c',
  'utility.c',
]
mendingwallcli_deps = [
  dependency('glib-2.0'),
  dependency('gio-2.0'),
]
executable('mendingwallcli', mendingwallcli_sources,
  dependencies: mendingwallcli_deps,
       install: true,
)

# mendingwalld program
mendingwalld_sources = [
  'mendingwalldapplication.c',
  'mendingwalld.c',
  'utility.c',
]
mendingwalld_deps = [
  dependency('gtk4'),
]
executable('mendingwalld', mendingwalld_sources,
  dependencies: mendingwalld_deps,
       install: true,
)
