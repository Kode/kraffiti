let project = new Project('kraffiti');

project.kore = false;

project.setCmd();

project.addExclude('.git/**');
project.addExclude('build/**');

project.addDefine('HAVE_CONFIG_H');
project.addDefine("PNG_NO_CONFIG_H");

project.addFile('Sources/**');

project.addFile("zlib/*.h");
project.addFile("zlib/*.c");
project.addExclude("zlib/gzlib.c");
project.addExclude("zlib/gzclose.c");
project.addExclude("zlib/gzwrite.c");
project.addExclude("zlib/gzread.c");

project.addFile("libpng/*.h");
project.addFile("libpng/*.c");
project.addExclude("libpng/pngtest.c");

project.addFile("libjpeg/*.h");
project.addFile("libjpeg/*.c");
project.addExclude("libjpeg/jmemmac.c");
project.addExclude("libjpeg/jmemdos.c");
project.addExclude("libjpeg/ansi2knr.c");
project.addExclude("libjpeg/cdjpeg.c");
project.addExclude("libjpeg/jmemname.c");
project.addExclude("libjpeg/jmemansi.c");
project.addExclude("libjpeg/rdjpgcom.c");
project.addExclude("libjpeg/wrjpgcom.c");
project.addExclude("libjpeg/cjpeg.c");
project.addExclude("libjpeg/ckconfig.c");
project.addExclude("libjpeg/djpeg.c");
project.addExclude("libjpeg/jpegtran.c");
project.addExclude("libjpeg/example.c");

project.addFile('Libraries/lz4x/**');

project.addIncludeDir("Sources");
project.addIncludeDir("zlib");
project.addIncludeDir("libpng");
project.addIncludeDir("libjpeg");

resolve(project);
