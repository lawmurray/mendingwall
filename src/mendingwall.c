#include <config.h>
#include <resources.h>
#include <mendingwallapplication.h>

int main(int argc, char* argv[]) {
  g_resources_register(mendingwall_get_resource());
  MendingwallApplication* app = mendingwall_application_new();
  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}
