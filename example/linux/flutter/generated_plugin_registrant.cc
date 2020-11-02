//
//  Generated file. Do not edit.
//

#include "generated_plugin_registrant.h"

#include <platform_proxy/platform_proxy_plugin.h>

void fl_register_plugins(FlPluginRegistry* registry) {
  g_autoptr(FlPluginRegistrar) platform_proxy_registrar =
      fl_plugin_registry_get_registrar_for_plugin(registry, "PlatformProxyPlugin");
  platform_proxy_plugin_register_with_registrar(platform_proxy_registrar);
}
