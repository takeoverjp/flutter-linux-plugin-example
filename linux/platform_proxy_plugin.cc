#include "include/platform_proxy/platform_proxy_plugin.h"

#include <flutter_linux/flutter_linux.h>
#include <gtk/gtk.h>
#include <sys/utsname.h>

#include <cstring>

#define PLATFORM_PROXY_PLUGIN(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), platform_proxy_plugin_get_type(), \
                              PlatformProxyPlugin))

struct _PlatformProxyPlugin {
  GObject parent_instance;
};

G_DEFINE_TYPE(PlatformProxyPlugin, platform_proxy_plugin, g_object_get_type())

// Called when a method call is received from Flutter.
static void platform_proxy_plugin_handle_method_call(
    PlatformProxyPlugin* self,
    FlMethodCall* method_call) {
  g_autoptr(FlMethodResponse) response = nullptr;

  const gchar* method = fl_method_call_get_name(method_call);
  FlValue* args = fl_method_call_get_args(method_call);

  if (strcmp(method, "getPlatformVersion") == 0) {
    struct utsname uname_data = {};
    uname(&uname_data);
    g_autofree gchar *version = g_strdup_printf("Linux %s", uname_data.version);
    g_autoptr(FlValue) result = fl_value_new_string(version);
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(result));
  } else if (strcmp(method, "invokeLinuxMethodFromDart") == 0) {
    FlValue* fl_int_arg = fl_value_lookup_string(args, "int_arg");
    FlValue* fl_double_arg = fl_value_lookup_string(args, "double_arg");
    FlValue* fl_string_arg = fl_value_lookup_string(args, "string_arg");
    int int_arg = fl_value_get_int(fl_int_arg);
    double double_arg = fl_value_get_float(fl_double_arg);
    const gchar* string_arg = fl_value_get_string(fl_string_arg);
    int result = int_arg + double_arg + strtol(string_arg, NULL, 10);
    g_autoptr(FlValue) fl_result = fl_value_new_int(result);
    response = FL_METHOD_RESPONSE(fl_method_success_response_new(fl_result));
  } else {
    response = FL_METHOD_RESPONSE(fl_method_not_implemented_response_new());
  }

  fl_method_call_respond(method_call, response, nullptr);
}

static void platform_proxy_plugin_dispose(GObject* object) {
  G_OBJECT_CLASS(platform_proxy_plugin_parent_class)->dispose(object);
}

static void platform_proxy_plugin_class_init(PlatformProxyPluginClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = platform_proxy_plugin_dispose;
}

static void platform_proxy_plugin_init(PlatformProxyPlugin* self) {}

static void method_call_cb(FlMethodChannel* channel, FlMethodCall* method_call,
                           gpointer user_data) {
  PlatformProxyPlugin* plugin = PLATFORM_PROXY_PLUGIN(user_data);
  platform_proxy_plugin_handle_method_call(plugin, method_call);
}

void platform_proxy_plugin_register_with_registrar(FlPluginRegistrar* registrar) {
  PlatformProxyPlugin* plugin = PLATFORM_PROXY_PLUGIN(
      g_object_new(platform_proxy_plugin_get_type(), nullptr));

  g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
  g_autoptr(FlMethodChannel) channel =
      fl_method_channel_new(fl_plugin_registrar_get_messenger(registrar),
                            "xyz.takeoverjp.example/platform_proxy",
                            FL_METHOD_CODEC(codec));
  fl_method_channel_set_method_call_handler(channel, method_call_cb,
                                            g_object_ref(plugin),
                                            g_object_unref);

  g_object_unref(plugin);
}
