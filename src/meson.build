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
  'mendingwall-menus-application.c',
]
mendingwall_menus_deps = [
  dependency('gtk4'),
]
executable('mendingwall-menus', mendingwall_menus_sources,
  dependencies: mendingwall_menus_deps,
       install: true,
)

# mendingwall-themes-save program
mendingwall_themes_save_sources = [
  'mendingwall-themes-save.c',
  'mendingwall-themes-application.c',
]
mendingwall_themes_save_deps = [
  dependency('gtk4'),
]
executable('mendingwall-themes-save', mendingwall_themes_save_sources,
  dependencies: mendingwall_themes_save_deps,
       install: true,
)

# mendingwall-themes-restore program
mendingwall_themes_restore_sources = [
  'mendingwall-themes-restore.c',
]
mendingwall_themes_restore_deps = [
  dependency('glib-2.0'),
  dependency('gio-2.0'),
]
executable('mendingwall-themes-restore', mendingwall_themes_restore_sources,
  dependencies: mendingwall_themes_restore_deps,
       install: true,
)
