import 'dart:async';
import 'dart:ffi';
import 'dart:io';

import 'package:ffi/ffi.dart';

import 'native_opencv_bindings_generated.dart';

const String _libName = 'native_opencv';

/// The dynamic library in which the symbols for [NativeOpencvBindings] can be found.
final DynamicLibrary _dylib = () {
  if (Platform.isMacOS || Platform.isIOS) {
    return DynamicLibrary.process();
    // return DynamicLibrary.open('$_libName.framework/$_libName');
  }
  if (Platform.isAndroid || Platform.isLinux) {
    return DynamicLibrary.open('lib$_libName.so');
  }
  if (Platform.isWindows) {
    return DynamicLibrary.open('$_libName.dll');
  }
  throw UnsupportedError('Unknown platform: ${Platform.operatingSystem}');
}();

/// The bindings to the native functions in [_dylib].
final NativeOpencvBindings _bindings = NativeOpencvBindings(_dylib);

String opencvVersion() => _bindings.opencvVersion().cast<Utf8>().toDartString();

Future<void> gaussianBlur(String imagePath) async {
  _bindings.gausianBlur(imagePath.toNativeUtf8().cast<Char>());
}

Future<void> cannyDetector(
  String imagePath,
  String outputPath, {
  required double threshold,
  double ratio = 3,
}) async {
  _bindings.cannyEdgeDetector(
    imagePath.toNativeUtf8().cast<Char>(),
    outputPath.toNativeUtf8().cast<Char>(),
    threshold,
    ratio,
  );
}

Future<void> removeWhiteBg(
  String imagePath,
  String outputPath, {
  required int threshold,
}) async {
  _bindings.removeWhiteBg(
    imagePath.toNativeUtf8().cast<Char>(),
    outputPath.toNativeUtf8().cast<Char>(),
    threshold,
  );
}

Future<void> sketch(
  String imagePath,
  String outputPath,
) async {
  _bindings.sketch(
    imagePath.toNativeUtf8().cast<Char>(),
    outputPath.toNativeUtf8().cast<Char>(),
  );
}

Future<void> removeBg(
  String imagePath,
  String outputPath,
) async {
  _bindings.removeBg(
    imagePath.toNativeUtf8().cast<Char>(),
    outputPath.toNativeUtf8().cast<Char>(),
  );
}

Future<void> rough(
  String imagePath,
  String outputPath,
) async {
  _bindings.rough(
    imagePath.toNativeUtf8().cast<Char>(),
    outputPath.toNativeUtf8().cast<Char>(),
  );
}

Future<void> sobelEdgeDetector(String imagePath) async {
  _bindings.sobelEdgeDetector(imagePath.toNativeUtf8().cast<Char>());
}

Future<void> cannyDetectorV2(
  String imagePath,
  String outputPath, {
  required int gaussianKernelSize,
  required double gaussianSigma,
  required double cannyLowThresh,
  required double cannyHighThresh,
  required int edgeR,
  required int edgeG,
  required int edgeB,
  required int edgeA,
  required int dilationSize,
}) async {
  _bindings.cannyEdgeDetectorV2(
    imagePath.toNativeUtf8().cast<Char>(),
    outputPath.toNativeUtf8().cast<Char>(),
    gaussianKernelSize,
    gaussianSigma,
    cannyLowThresh,
    cannyHighThresh,
    edgeR,
    edgeG,
    edgeB,
    edgeA,
    dilationSize,
  );
}
