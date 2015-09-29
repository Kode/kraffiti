const solution = new Solution("kraffiti");
const project = new Project("kraffiti");

const pvrtc = true;

solution.setCmd();

project.addExclude('.git/**');
project.addExclude('build/**');

project.addDefine('HAVE_CONFIG_H');
project.addDefine("PNG_NO_CONFIG_H");

project.addFile('Sources/**');
project.addFile('Libraries/PVRTexTool/Include/**');

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

/*
project.addFile('Libraries/astc-encoder/Source/**');
project.addExclude('Libraries/astc-encoder/Source/exr_to_htga.cpp');
project.addExclude('Libraries/astc-encoder/Source/imgdiff.cpp');
project.addExclude('Libraries/astc-encoder/Source/astc_averages_and_directions_eigenvectors.cpp');
if (platform === Platform.Windows) {
	project.addIncludeDir('Libraries/astc-encoder/Source/win32-2010/astcenc');
}
*/

project.addIncludeDir("Sources");
project.addIncludeDir("zlib");
project.addIncludeDir("libpng");
project.addIncludeDir("libjpeg");

if (pvrtc) {
	project.addDefine('LIB_PVRTC');
	project.addIncludeDir('Libraries/PVRTexTool/Include');
	if (platform === Platform.Windows) {
		project.addLibFor('Win32', 'Libraries/PVRTexTool/Windows_x86_32/Static/PVRTexLib');
		project.addLibFor('x64', 'Libraries/PVRTexTool/Windows_x86_64/Static/PVRTexLib');
	}
	else if (platform === Platform.OSX) {
		project.addLibFor('Libraries/PVRTexTool/OSX_x86/Static/PVRTexLib');
	}
}

solution.addProject(project);

return solution;
