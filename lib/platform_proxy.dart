
import 'dart:async';

import 'package:flutter/services.dart';

class PlatformProxy {
  static const MethodChannel _channel =
      const MethodChannel('platform_proxy');

  static Future<String> get platformVersion async {
    final String version = await _channel.invokeMethod('getPlatformVersion');
    return version;
  }
}
