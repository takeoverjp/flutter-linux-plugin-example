import 'package:pigeon/pigeon.dart';

class PigeonRequest {
  int intArg;
  double doubleArg;
  String stringArg;
}

class PigeonReply {
  int result;
}

@HostApi()
abstract class PigeonPlatformProxy {
  PigeonReply invokeLinuxMethodByPigeon(PigeonRequest request);
}
