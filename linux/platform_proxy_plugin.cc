#include "include/platform_proxy/platform_proxy_plugin.h"

#include <flutter_linux/flutter_linux.h>
#include <gtk/gtk.h>
#include <sys/utsname.h>

#include <cstring>

#define PLATFORM_PROXY_PLUGIN(obj)                                     \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), platform_proxy_plugin_get_type(), \
                              PlatformProxyPlugin))

struct _PlatformProxyPlugin {
  GObject parent_instance;
};

G_DEFINE_TYPE(PlatformProxyPlugin, platform_proxy_plugin, g_object_get_type())

// Called when a method call is received from Flutter.
static void platform_proxy_plugin_handle_method_call(
    PlatformProxyPlugin* self, FlMethodCall* method_call) {
  g_autoptr(FlMethodResponse) response = nullptr;

  const gchar* method = fl_method_call_get_name(method_call);
  FlValue* args = fl_method_call_get_args(method_call);

  if (strcmp(method, "getPlatformVersion") == 0) {
    struct utsname uname_data = {};
    uname(&uname_data);
    g_autofree gchar* version = g_strdup_printf("Linux %s", uname_data.version);
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

static void invoke_dart_method_from_linux(FlMethodChannel* fl_channel,
                                          int int_arg, double double_arg,
                                          const gchar* string_arg) {
  g_autoptr(FlValue) args = fl_value_new_map();
  fl_value_set_string_take(args, "int_arg", fl_value_new_int(int_arg));
  fl_value_set_string_take(args, "double_arg", fl_value_new_float(double_arg));
  fl_value_set_string_take(args, "string_arg", fl_value_new_string(string_arg));

  fprintf(stderr, "[linux] invoke invokeDartMethodFromLinux\n");

  fl_method_channel_invoke_method(
      fl_channel, "invokeDartMethodFromLinux", args, nullptr,
      [](GObject* object, GAsyncResult* result, gpointer user_data) {
        g_autoptr(GError) error = NULL;
        g_autoptr(FlMethodResponse) response =
            fl_method_channel_invoke_method_finish(FL_METHOD_CHANNEL(object),
                                                   result, &error);
        if (response == NULL) {
          g_warning("Failed to call method: %s", error->message);
          return;
        }

        g_autoptr(FlValue) value =
            fl_method_response_get_result(response, &error);
        if (response == NULL) {
          g_warning("Method returned error: %s", error->message);
          return;
        }
        fl_value_ref(value);

        fprintf(stderr, "[linux] invokeDartMethodFromLinux returns %ld\n",
                fl_value_get_int(value));
      },
      nullptr);
}

static gboolean timer_callback(gpointer user_data) {
  static int count = 0;
  auto fl_channel = static_cast<FlMethodChannel*>(user_data);
  fprintf(stderr, "[linux] %s: count=%d\n", __func__, count);

  switch (count++) {
    case 0: {
      invoke_dart_method_from_linux(fl_channel, 1, 2.22, "3");
      return TRUE;
    }
    default:
      return FALSE;
  }
}

void platform_proxy_plugin_register_with_registrar(
    FlPluginRegistrar* registrar) {
  PlatformProxyPlugin* plugin = PLATFORM_PROXY_PLUGIN(
      g_object_new(platform_proxy_plugin_get_type(), nullptr));

  g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
  g_autoptr(FlMethodChannel) channel = fl_method_channel_new(
      fl_plugin_registrar_get_messenger(registrar),
      "xyz.takeoverjp.example/platform_proxy", FL_METHOD_CODEC(codec));
  fl_method_channel_set_method_call_handler(
      channel, method_call_cb, g_object_ref(plugin), g_object_unref);
  g_timeout_add_seconds(1, timer_callback, g_object_ref(channel));

  g_object_unref(plugin);
}
