add_project_arguments('-Wl,--export-dynamic', language: 'c')

blueprints = custom_target('blueprints',
  input: ['mendingwall.blp'],
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

# mendingwall-menus program
mendingwall_menus_sources = [
  'mendingwall-menus.c',
]
mendingwall_menus_deps = [
  dependency('glib-2.0'),
  dependency('gio-2.0'),
]
executable('mendingwall-menus', mendingwall_menus_sources,
  dependencies: mendingwall_menus_deps,
       install: true,
)

# mendingwall-themes program
mendingwall_themes_sources = [
  'mendingwall-themes.c',
]
mendingwall_themes_deps = [
  dependency('glib-2.0'),
  dependency('gio-2.0'),
]
executable('mendingwall-themes', mendingwall_themes_sources,
  dependencies: mendingwall_themes_deps,
       install: true,
)
