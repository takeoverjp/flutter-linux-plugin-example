// Autogenerated from Pigeon (v0.1.14), do not edit directly.
// See also: https://pub.dev/packages/pigeon
// ignore_for_file: public_member_api_docs, non_constant_identifier_names, avoid_as, unused_import
// @dart = 2.8
import 'dart:async';
import 'package:flutter/services.dart';
import 'dart:typed_data' show Uint8List, Int32List, Int64List, Float64List;

class PigeonReply {
  int result;
  // ignore: unused_element
  Map<dynamic, dynamic> _toMap() {
    final Map<dynamic, dynamic> pigeonMap = <dynamic, dynamic>{};
    pigeonMap['result'] = result;
    return pigeonMap;
  }

  // ignore: unused_element
  static PigeonReply _fromMap(Map<dynamic, dynamic> pigeonMap) {
    final PigeonReply result = PigeonReply();
    result.result = pigeonMap['result'];
    return result;
  }
}

class PigeonRequest {
  int intArg;
  double doubleArg;
  String stringArg;
  // ignore: unused_element
  Map<dynamic, dynamic> _toMap() {
    final Map<dynamic, dynamic> pigeonMap = <dynamic, dynamic>{};
    pigeonMap['intArg'] = intArg;
    pigeonMap['doubleArg'] = doubleArg;
    pigeonMap['stringArg'] = stringArg;
    return pigeonMap;
  }

  // ignore: unused_element
  static PigeonRequest _fromMap(Map<dynamic, dynamic> pigeonMap) {
    final PigeonRequest result = PigeonRequest();
    result.intArg = pigeonMap['intArg'];
    result.doubleArg = pigeonMap['doubleArg'];
    result.stringArg = pigeonMap['stringArg'];
    return result;
  }
}

class PigeonPlatformProxy {
  Future<PigeonReply> invokeLinuxMethodByPigeon(PigeonRequest arg) async {
    final Map<dynamic, dynamic> requestMap = arg._toMap();
    const BasicMessageChannel<dynamic> channel = BasicMessageChannel<dynamic>(
        'dev.flutter.pigeon.PigeonPlatformProxy.invokeLinuxMethodByPigeon',
        StandardMessageCodec());

    final Map<dynamic, dynamic> replyMap = await channel.send(requestMap);
    if (replyMap == null) {
      throw PlatformException(
          code: 'channel-error',
          message: 'Unable to establish connection on channel.',
          details: null);
    } else if (replyMap['error'] != null) {
      final Map<dynamic, dynamic> error = replyMap['error'];
      throw PlatformException(
          code: error['code'],
          message: error['message'],
          details: error['details']);
    } else {
      return PigeonReply._fromMap(replyMap['result']);
    }
  }
}