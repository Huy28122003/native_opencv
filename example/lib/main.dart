import 'dart:io';
import 'package:flutter/material.dart';
import 'package:native_opencv/native_opencv.dart' as native_opencv;
import 'package:image_picker/image_picker.dart';
import 'package:native_opencv_example/file.dart';

void main() {
  runApp(MaterialApp(
    home: const MyApp(),
  ));
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  final ImagePicker _picker = ImagePicker();
  late String version;
  Image _img = Image.asset('assets/img/default.jpg');
  double _threshold = 50;
  String? _inputPath;
  String? _tempPath;
  int i = 0;

  @override
  void initState() {
    super.initState();
    version = native_opencv.opencvVersion();
    FileUtils().getTemporaryDirPath().then((value) {
      _tempPath = value;
    });
  }

  Future<void> _loadImage() async {
    print('_inputPath: $_inputPath');
    print('_tempPath: $_tempPath');
    if (_inputPath == null || _tempPath == null) return;

    final outputPath = '$_tempPath/output_${i++}.png';

    await native_opencv.cannyDetector(
      _inputPath!,
      outputPath,
      threshold: _threshold,
      ratio: 3,
    );

    setState(() {
      _img = Image.file(File(outputPath));
    });
  }

  @override
  Widget build(BuildContext context) {
    const textStyle = TextStyle(fontSize: 25);
    const spacerSmall = SizedBox(height: 10);
    return Scaffold(
      appBar: AppBar(
        title: const Text('native_opencv example'),
      ),
      body: SingleChildScrollView(
        child: Container(
          padding: const EdgeInsets.all(10),
          color: Colors.amber,
          child: Column(
            children: [
              spacerSmall,
              Text(
                'OpenCV Version: $version',
                style: textStyle,
                textAlign: TextAlign.center,
              ),
              SizedBox(
                height: 20,
              ),
              ElevatedButton(
                onPressed: () async {
                  final imageFile =
                      await _picker.pickImage(source: ImageSource.gallery);
                  if (imageFile == null) return;

                  _inputPath = imageFile.path;

                  print('_inputPath: $_inputPath');
                  await native_opencv.gaussianBlur(_inputPath!);

                  setState(() {
                    _img = Image.file(File(_inputPath!));
                  });
                },
                child: Text("Run GaussianBlur"),
              ),
              SizedBox(
                height: 20,
              ),
              Slider(
                value: _threshold,
                onChanged: (value) {
                  setState(() {
                    _threshold = value;
                  });
                  print('value: $value');
                  _loadImage();
                },
                max: 100,
                min: 0,
                divisions: 10,
              ),
              ElevatedButton(
                onPressed: () async {
                  final imageFile =
                      await _picker.pickImage(source: ImageSource.gallery);
                  if (imageFile == null) return;

                  _inputPath = imageFile.path;

                  _loadImage();
                },
                child: Text("Run CannyDetector"),
              ),
              SizedBox(
                height: 20,
              ),
              ElevatedButton(
                onPressed: () async {
                  final imageFile =
                      await _picker.pickImage(source: ImageSource.gallery);
                  if (imageFile == null) return;

                  _inputPath = imageFile.path;

                  print('_inputPath: $_inputPath');
                  await native_opencv.sobelEdgeDetector(_inputPath!);

                  setState(() {
                    _img = Image.file(File(_inputPath!));
                  });
                },
                child: Text("Run SobelEdgeDetector"),
              ),
              SizedBox(
                height: 20,
              ),
              SizedBox(
                height: 20,
              ),
              Center(child: _img),
            ],
          ),
        ),
      ),
    );
  }
}
