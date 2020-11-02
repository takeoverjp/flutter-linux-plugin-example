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
    print('[dart] invoke invokeLinuxMethodFromDart');
    final int result = await _channel.invokeMethod(
        'invokeLinuxMethodFromDart', <String, dynamic>{
      'int_arg': int_arg,
      'double_arg': double_arg,
      'string_arg': string_arg
    });
    print('[dart] invokeLinuxMethodFromDart returns ${result}');
    return result;
  }

  static Future<dynamic> invokeDartMethodFromLinux(dynamic args) async {
    print('[dart] invokeDartMethodFromLinux called with ${args}');
    final int result = args['int_arg'] +
        args['double_arg'].toInt() +
        int.parse(args['string_arg']);
    print('[dart] invokeDartMethodFromLinux returns ${result}');
    return Future.value(result);
    //return Future.error('error message!!');
  }

  static Future<dynamic> _platformCallHandler(MethodCall call) async {
    final args = call.arguments;
    switch (call.method) {
      case 'invokeDartMethodFromLinux':
        return invokeDartMethodFromLinux(args);
      default:
        print('[dart] Unknowm method ${call.method}');
        throw MissingPluginException();
        break;
    }
  }

  static void init() {
    _channel.setMethodCallHandler(_platformCallHandler);
  }
}
