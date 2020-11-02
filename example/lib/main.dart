import 'package:flutter/material.dart';
import 'dart:async';

import 'package:flutter/services.dart';
import 'package:platform_proxy/platform_proxy.dart';
import 'package:platform_proxy/pigeon_platform_proxy.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  String _platformVersion = 'Unknown';
  int _result = 0;

  @override
  void initState() {
    super.initState();
    initPlatformState();
  }

  // Platform messages are asynchronous, so we initialize in an async method.
  Future<void> initPlatformState() async {
    String platformVersion;
    int result = 0;

    PlatformProxy.init();
    // Platform messages may fail, so we use a try/catch PlatformException.
    try {
      platformVersion = await PlatformProxy.platformVersion;
      result = await PlatformProxy.invokeLinuxMethodFromDart(1, 2.22, '3');
    } on PlatformException {
      platformVersion = 'Failed to get platform version.';
    }

    var pigeon = PigeonPlatformProxy();
    final request = PigeonRequest()
      ..intArg = 1
      ..doubleArg = 2.22
      ..stringArg = '3';
    PigeonReply reply = await pigeon.invokeLinuxMethodByPigeon(request);
    print(
        '[dart] PlatformProxyPigeon#invokeLinuxMethodByPigeon returns ${reply.result}');

    // If the widget was removed from the tree while the asynchronous platform
    // message was in flight, we want to discard the reply rather than calling
    // setState to update our non-existent appearance.
    if (!mounted) return;

    setState(() {
      _platformVersion = platformVersion;
      _result = result;
    });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Plugin example app'),
        ),
        body: Center(
          child: Text('Running on: $_platformVersion\n'
              'invokeLinuxMethodFromDart(1, 2.22f, "3"): $_result\n'),
        ),
      ),
    );
  }
}
