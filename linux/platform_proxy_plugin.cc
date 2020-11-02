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
  FlMethodChannel* method_channel;
  FlEventChannel* event_channel;
  FlBasicMessageChannel* basic_message_channel;
};

G_DEFINE_TYPE(PlatformProxyPlugin, platform_proxy_plugin, g_object_get_type())

static bool is_send_events = false;

static FlMethodResponse* get_platform_version(PlatformProxyPlugin* self,
                                              FlValue* args) {
  struct utsname uname_data = {};

  uname(&uname_data);

  g_autofree gchar* version = g_strdup_printf("Linux %s", uname_data.version);
  g_autoptr(FlValue) result = fl_value_new_string(version);
  return FL_METHOD_RESPONSE(fl_method_success_response_new(result));
}

static FlMethodResponse* invoke_linux_method_from_dart(
    PlatformProxyPlugin* self, FlValue* args) {
  FlValue* fl_int_arg = fl_value_lookup_string(args, "intArg");
  FlValue* fl_double_arg = fl_value_lookup_string(args, "doubleArg");
  FlValue* fl_string_arg = fl_value_lookup_string(args, "stringArg");
  int int_arg = fl_value_get_int(fl_int_arg);
  double double_arg = fl_value_get_float(fl_double_arg);
  const gchar* string_arg = fl_value_get_string(fl_string_arg);

  fprintf(stderr,
          "[linux] invokeLinuxMethodFromDart called with {intArg: %d, "
          "doubleArg: %f, stringArg: %s}\n",
          int_arg, double_arg, string_arg);
  int result = int_arg + double_arg + strtol(string_arg, NULL, 10);
  fprintf(stderr, "[linux] invokeLinuxMethodFromDart returns %d\n", result);

  g_autoptr(FlValue) fl_result = fl_value_new_int(result);
  return FL_METHOD_RESPONSE(fl_method_success_response_new(fl_result));
}

static void platform_proxy_plugin_handle_method_call(
    PlatformProxyPlugin* self, FlMethodCall* method_call) {
  g_autoptr(FlMethodResponse) response = nullptr;

  const gchar* method = fl_method_call_get_name(method_call);
  FlValue* args = fl_method_call_get_args(method_call);

  if (strcmp(method, "getPlatformVersion") == 0) {
    response = get_platform_version(self, args);
  } else if (strcmp(method, "invokeLinuxMethodFromDart") == 0) {
    response = invoke_linux_method_from_dart(self, args);
  } else {
    response = FL_METHOD_RESPONSE(fl_method_not_implemented_response_new());
  }

  fl_method_call_respond(method_call, response, nullptr);
}

static void platform_proxy_plugin_dispose(GObject* object) {
  PlatformProxyPlugin* self = PLATFORM_PROXY_PLUGIN(object);
  g_clear_object(&self->event_channel);
  g_clear_object(&self->method_channel);
  G_OBJECT_CLASS(platform_proxy_plugin_parent_class)->dispose(object);
}

static void platform_proxy_plugin_class_init(PlatformProxyPluginClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = platform_proxy_plugin_dispose;
}

static void platform_proxy_plugin_init(PlatformProxyPlugin* self) {}

static void method_call_cb(FlMethodChannel* channel, FlMethodCall* method_call,
                           gpointer user_data) {
  PlatformProxyPlugin* self = PLATFORM_PROXY_PLUGIN(user_data);
  platform_proxy_plugin_handle_method_call(self, method_call);
}

static void invoke_dart_method_from_linux(FlMethodChannel* channel, int int_arg,
                                          double double_arg,
                                          const gchar* string_arg) {
  g_autoptr(FlValue) args = fl_value_new_map();
  fl_value_set_string_take(args, "intArg", fl_value_new_int(int_arg));
  fl_value_set_string_take(args, "doubleArg", fl_value_new_float(double_arg));
  fl_value_set_string_take(args, "stringArg", fl_value_new_string(string_arg));

  fprintf(stderr, "[linux] invoke invokeDartMethodFromLinux\n");

  fl_method_channel_invoke_method(
      channel, "invokeDartMethodFromLinux", args, nullptr,
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

static void send_event_from_linux(FlEventChannel* channel) {
  if (!is_send_events) {
    return;
  }

  g_autoptr(FlValue) event = fl_value_new_map();
  fl_value_set_string_take(event, "type", fl_value_new_string("FromLinux"));
  fl_value_set_string_take(event, "time", fl_value_new_int(time(nullptr)));

  fl_event_channel_send(channel, event, nullptr, nullptr);
}

static gboolean timer_callback(gpointer user_data) {
  static int count = 0;
  PlatformProxyPlugin* self = PLATFORM_PROXY_PLUGIN(user_data);
  fprintf(stderr, "[linux] %s: count=%d\n", __func__, count);

  switch (count++) {
    case 0:
      invoke_dart_method_from_linux(self->method_channel, 1, 2.22, "3");
      return TRUE;
    case 1:
    case 2:
    case 3:
      send_event_from_linux(self->event_channel);
      return TRUE;
    default:
      g_object_unref(self);
      return FALSE;
  }
}

static FlMethodErrorResponse* event_channel_listen_cb(FlEventChannel* channel,
                                                      FlValue* args,
                                                      gpointer user_data) {
  is_send_events = true;
  return NULL;
}

static FlMethodErrorResponse* event_channel_cancel_cb(FlEventChannel* channel,
                                                      FlValue* args,
                                                      gpointer user_data) {
  is_send_events = false;
  return NULL;
}

static void invoke_linux_method_by_pigeon (FlBasicMessageChannel* channel,
                        FlValue* message,
                        FlBasicMessageChannelResponseHandle* response_handle,
                        gpointer user_data) {
  FlValue* fl_int_arg = fl_value_lookup_string(message, "intArg");
  FlValue* fl_double_arg = fl_value_lookup_string(message, "doubleArg");
  FlValue* fl_string_arg = fl_value_lookup_string(message, "stringArg");
  int int_arg = fl_value_get_int(fl_int_arg);
  double double_arg = fl_value_get_float(fl_double_arg);
  const gchar* string_arg = fl_value_get_string(fl_string_arg);

  fprintf(stderr,
          "[linux] invokeLinuxMethodFromPigeon called with {intArg: %d, "
          "doubleArg: %f, stringArg: %s}\n",
          int_arg, double_arg, string_arg);
  int result = int_arg + double_arg + strtol(string_arg, NULL, 10);
  fprintf(stderr, "[linux] invokeLinuxMethodFromPigeon returns %d\n", result);

  g_autoptr(FlValue) response = fl_value_new_map();
  g_autoptr(FlValue) fl_result = fl_value_new_map();
  fl_value_set_string_take(fl_result, "result", fl_value_new_int(result));
  fl_value_set_string_take(response, "result", fl_value_ref(fl_result));
  g_autoptr(GError) error = NULL;
  if (!fl_basic_message_channel_respond (channel, response_handle, response,
                                         &error)) {
    g_warning ("Failed to send channel response: %s", error->message);
  }
}

void platform_proxy_plugin_register_with_registrar(
    FlPluginRegistrar* registrar) {
  PlatformProxyPlugin* plugin = PLATFORM_PROXY_PLUGIN(
      g_object_new(platform_proxy_plugin_get_type(), nullptr));

  g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
  plugin->method_channel = fl_method_channel_new(
      fl_plugin_registrar_get_messenger(registrar),
      "xyz.takeoverjp.example/platform_proxy_method_channel",
      FL_METHOD_CODEC(codec));
  fl_method_channel_set_method_call_handler(
      plugin->method_channel, method_call_cb, g_object_ref(plugin),
      g_object_unref);

  plugin->event_channel = fl_event_channel_new(
      fl_plugin_registrar_get_messenger(registrar),
      "xyz.takeoverjp.example/platform_proxy_event_channel",
      FL_METHOD_CODEC(codec));
  fl_event_channel_set_stream_handlers(
      plugin->event_channel, event_channel_listen_cb, event_channel_cancel_cb,
      nullptr, nullptr);

  g_autoptr(FlStandardMessageCodec) message_codec = fl_standard_message_codec_new();
  plugin->basic_message_channel = fl_basic_message_channel_new(
      fl_plugin_registrar_get_messenger(registrar),
      "dev.flutter.pigeon.PigeonPlatformProxy.invokeLinuxMethodByPigeon",
      FL_MESSAGE_CODEC(message_codec));
  fl_basic_message_channel_set_message_handler(
      plugin->basic_message_channel, invoke_linux_method_by_pigeon,
      nullptr, nullptr);

  g_timeout_add_seconds(1, timer_callback, g_object_ref(plugin));

  g_object_unref(plugin);
}
