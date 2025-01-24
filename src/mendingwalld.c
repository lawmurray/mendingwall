#include <config.h>
#include <mendingwalldapplication.h>

int main(int argc, char* argv[]) {
  MendingwallDApplication* app = mendingwalld_application_new();
  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}
