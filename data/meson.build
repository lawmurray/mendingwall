desktop_file = i18n.merge_file(
        input: 'org.indii.mendingwall.desktop.in',
       output: 'org.indii.mendingwall.desktop',
         type: 'desktop',
       po_dir: '../po',
      install: true,
  install_dir: get_option('datadir') / 'applications'
)

menus_autostart_file = i18n.merge_file(
        input: 'org.indii.mendingwall.menus.desktop.in',
       output: 'org.indii.mendingwall.menus.desktop',
         type: 'desktop',
       po_dir: '../po',
      install: true,
  install_dir: get_option('datadir') / 'mendingwall'
)

themes_save_autostart_file = i18n.merge_file(
        input: 'org.indii.mendingwall.themes.save.desktop.in',
       output: 'org.indii.mendingwall.themes.save.desktop',
         type: 'desktop',
       po_dir: '../po',
      install: true,
  install_dir: get_option('datadir') / 'mendingwall'
)

themes_restore_autostart_file = i18n.merge_file(
        input: 'org.indii.mendingwall.themes.restore.desktop.in',
       output: 'org.indii.mendingwall.themes.restore.desktop',
         type: 'desktop',
       po_dir: '../po',
      install: true,
  install_dir: get_option('datadir') / 'mendingwall'
)

desktop_utils = find_program('desktop-file-validate', required: false)
if desktop_utils.found()
  test('Validate desktop file', desktop_utils,
    args: [
      desktop_file,
      menus_autostart_file,
      themes_save_autostart_file,
      themes_restore_autostart_file
    ]
  )
endif

appstream_file = i18n.merge_file(
        input: 'org.indii.mendingwall.metainfo.xml.in',
       output: 'org.indii.mendingwall.metainfo.xml',
       po_dir: '../po',
      install: true,
  install_dir: get_option('datadir') / 'metainfo'
)

appstreamcli = find_program('appstreamcli', required: false, disabler: true)
test('Validate appstream file', appstreamcli,
     args: ['validate', '--no-net', '--explain', appstream_file])

install_data('org.indii.mendingwall.gschema.xml',
  install_dir: get_option('datadir') / 'glib-2.0' / 'schemas'
)

compile_schemas = find_program('glib-compile-schemas', required: false, disabler: true)
test('Validate schema file',
     compile_schemas,
     args: ['--strict', '--dry-run', meson.current_source_dir()])


service_conf = configuration_data()
service_conf.set('bindir', get_option('prefix') / get_option('bindir'))
configure_file(
  input: 'org.indii.mendingwall.service.in',
  output: 'org.indii.mendingwall.service',
  configuration: service_conf,
  install_dir: get_option('datadir') / 'dbus-1' / 'services'
)
configure_file(
  input: 'org.indii.mendingwall.menus.service.in',
  output: 'org.indii.mendingwall.menus.service',
  configuration: service_conf,
  install_dir: get_option('datadir') / 'dbus-1' / 'services'
)
configure_file(
  input: 'org.indii.mendingwall.themes.save.service.in',
  output: 'org.indii.mendingwall.themes.save.service',
  configuration: service_conf,
  install_dir: get_option('datadir') / 'dbus-1' / 'services'
)

install_data(
  ['themes.conf', 'menus.conf', 'mendingwall-themes-restore.sh'],
  install_dir: get_option('datadir') / 'mendingwall'
)

subdir('icons')
