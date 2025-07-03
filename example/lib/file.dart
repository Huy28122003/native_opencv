import 'dart:io';
import 'dart:typed_data';

import 'package:path_provider/path_provider.dart';

class FileUtils {
  FileUtils._();

  static final FileUtils _shared = FileUtils._();

  factory FileUtils() => _shared;

  Future<Directory> createDirectory(
    String directoryPath, {
    bool recursive = false,
  }) {
    return Directory(directoryPath).create(recursive: recursive);
  }

  Future<String> getTemporaryDirPath() async {
    return (await getTemporaryDirectory()).absolute.path;
  }

  Future<Uint8List?> readAsBytes(File? file) async {
    if (file == null) return null;
    return file.readAsBytes();
  }

  Future<File?> saveTempFile(Uint8List? data, String fileName) async {
    if (data == null) return null;

    final tempDirPath = await FileUtils().getTemporaryDirPath();
    final savedImagePath = '$tempDirPath/$fileName';

    final File file = File(savedImagePath);
    await file.writeAsBytes(data);

    return file;
  }
}
