import 'dart:async';

import 'package:flutter/services.dart';

class PlatformProxy {
  static const MethodChannel _channel =
      const MethodChannel('xyz.takeoverjp.example/platform_proxy');

  static Future<String> get platformVersion async {
    final String version = await _channel.invokeMethod('getPlatformVersion');
    return version;
  }

  static Future<int> invokeLinuxMethodFromDart(
      int int_arg, double double_arg, String string_arg) async {
    final int result = await _channel.invokeMethod(
        'invokeLinuxMethodFromDart', <String, dynamic>{
      'int_arg': int_arg,
      'double_arg': double_arg,
      'string_arg': string_arg
    });
    return result;
  }
}
