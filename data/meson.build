conf = configuration_data()
conf.set('bindir', get_option('prefix') / get_option('bindir'))

desktop_file = i18n.merge_file(
        input: configure_file(
                          input: 'org.indii.mendingwall.desktop.in',
                         output: 'org.indii.mendingwall.desktop.configure',
                  configuration: conf,
                        install: false,
               ),
       output: 'org.indii.mendingwall.desktop',
         type: 'desktop',
       po_dir: '../po',
      install: true,
  install_dir: get_option('datadir') / 'applications',
)

restore_autostart_file = i18n.merge_file(
        input: configure_file(
                         input: 'org.indii.mendingwall.watch.desktop.in',
                        output: 'org.indii.mendingwall.watch.desktop.configure',
                 configuration: conf,
                       install: false,
               ),
       output: 'org.indii.mendingwall.watch.desktop',
         type: 'desktop',
       po_dir: '../po',
      install: true,
  install_dir: get_option('datadir') / 'applications',
)

daemon_autostart_file = i18n.merge_file(
        input: configure_file(
                         input: 'org.indii.mendingwall.restore.desktop.in',
                        output: 'org.indii.mendingwall.restore.desktop.configure',
                 configuration: conf,
                       install: false,
               ),
       output: 'org.indii.mendingwall.restore.desktop',
         type: 'desktop',
       po_dir: '../po',
      install: true,
  install_dir: get_option('datadir') / 'applications',
)

appstream_file = i18n.merge_file(
        input: 'org.indii.mendingwall.metainfo.xml.in',
       output: 'org.indii.mendingwall.metainfo.xml',
       po_dir: '../po',
      install: true,
  install_dir: get_option('datadir') / 'metainfo'
)

configure_file(
          input: 'org.indii.mendingwall.service.in',
         output: 'org.indii.mendingwall.service',
  configuration: conf,
    install_dir: get_option('datadir') / 'dbus-1' / 'services'
)

configure_file(
          input: 'org.indii.mendingwall.watch.service.in',
         output: 'org.indii.mendingwall.watch.service',
  configuration: conf,
    install_dir: get_option('datadir') / 'dbus-1' / 'services'
)

configure_file(
          input: 'org.indii.mendingwall.restore.sh.in',
         output: 'org.indii.mendingwall.restore.sh',
  configuration: conf,
    install_dir: get_option('datadir') / 'mendingwall'
)

install_data(
  ['menus.conf', 'themes.conf'],
  install_dir: get_option('datadir') / 'mendingwall'
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

desktop_utils = find_program('desktop-file-validate', required: false)
if desktop_utils.found()
  test('Validate desktop file', desktop_utils,
    args: [
      desktop_file,
      daemon_autostart_file,
      restore_autostart_file
    ]
  )
endif

subdir('icons')
