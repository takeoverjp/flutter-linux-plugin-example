import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:platform_proxy/platform_proxy.dart';

void main() {
  const MethodChannel channel =
      MethodChannel('xyz.takeoverjp.example/platform_proxy');

  TestWidgetsFlutterBinding.ensureInitialized();

  setUp(() {
    channel.setMockMethodCallHandler((MethodCall methodCall) async {
      String method = methodCall.method;
      final args = methodCall.arguments;
      if (method == 'getPlatformVersion') {
        return '42';
      } else if (method == 'invokeLinuxMethodFromDart') {
        return args['int_arg'] +
            args['double_arg'].toInt() +
            int.parse(args['string_arg']);
      }
    });
  });

  tearDown(() {
    channel.setMockMethodCallHandler(null);
  });

  test('getPlatformVersion', () async {
    expect(await PlatformProxy.platformVersion, '42');
  });

  test('invokeLinuxMethodFromDart', () async {
    expect(await PlatformProxy.invokeLinuxMethodFromDart(1, 2.22, "3"), 6);
  });
}
