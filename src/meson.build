add_project_arguments('-Wl,--export-dynamic', language: 'c')

blueprints = custom_target('blueprints',
  input: ['about.blp', 'mendingwall.blp'],
  output: '.',
  command: [find_program('blueprint-compiler'), 'batch-compile', '@OUTPUT@', '@CURRENT_SOURCE_DIR@', '@INPUT@'],
)

resources = gnome.compile_resources('mendingwall-resources',
  'mendingwall.gresource.xml',
  c_name: 'mendingwall',
  dependencies: blueprints
)

# mendingwall program
mendingwall_sources = [
  'mendingwall.c',
]
mendingwall_sources += blueprints
mendingwall_sources += resources
mendingwall_deps = [
  dependency('gtk4'),
  dependency('libadwaita-1', version: '>= 1.4'),
  dependency('gmodule-export-2.0'),
]
executable('mendingwall', mendingwall_sources,
  dependencies: mendingwall_deps,
       install: true,
)

# mendingwalld program
mendingwalld_sources = [
  'mendingwall-background-application.c',
  'mendingwalld-application.c',
  'mendingwalld.c',
]
mendingwalld_deps = [
  dependency('glib-2.0'),
  dependency('gio-2.0'),
]
executable('mendingwalld', mendingwalld_sources,
  dependencies: mendingwalld_deps,
       install: true,
)
