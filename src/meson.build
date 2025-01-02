add_project_arguments('-Wl,--export-dynamic', language: 'c')

mendingwall_sources = [
  'main.c',
]

mendingwall_blueprints = files(
  'main.blp',
)

mendingwall_deps = [
  dependency('gtk4'),
  dependency('libadwaita-1', version: '>= 1.4'),
  dependency('gmodule-export-2.0'),
]

blueprints = custom_target('blueprints',
  input: mendingwall_blueprints,
  output: '.',
  command: [find_program('blueprint-compiler'), 'batch-compile', '@OUTPUT@', '@CURRENT_SOURCE_DIR@', '@INPUT@'],
)
mendingwall_sources += blueprints

resources = gnome.compile_resources('mendingwall-resources',
  'mendingwall.gresource.xml',
  c_name: 'mendingwall',
  dependencies: blueprints
)
mendingwall_sources += resources

executable('mendingwall', mendingwall_sources,
  dependencies: mendingwall_deps,
       install: true,
)

mendingwall_monitor_sources = [
  'monitor.c',
]

mendingwall_monitor_deps = [
  dependency('gtk4')
]

executable('mendingwall-monitor', mendingwall_monitor_sources,
  dependencies: mendingwall_monitor_deps,
       install: true,
)

