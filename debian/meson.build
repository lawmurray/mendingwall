conf = configuration_data()
conf.set('PROJECT_VERSION', meson.project_version())
conf.set('PACKAGE_YEAR', '2025')
conf.set('PACKAGE_TIMESTAMP', 'Thu, 30 Jan 2025 13:58:12 +0700')

configure_file(
          input: 'changelog.in',
         output: 'changelog',
  configuration: conf
)
configure_file(
          input: 'copyright.in',
         output: 'copyright',
  configuration: conf
)
