#include <config.h>
#include <mendingwall-themes-application.h>

int main(int argc, char* argv[]) {
  MendingwallThemesApplication* app = mendingwall_themes_application_new();
  return g_application_run(G_APPLICATION(app), argc, argv);
}
