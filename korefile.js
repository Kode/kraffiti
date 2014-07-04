var solution = new Solution("kraffiti");
var project = new Project("kraffiti");

solution.setCmd();

project.addExclude('.git/**');
project.addExclude('imageworsener/.git/**');
project.addExclude('build/**');

project.addDefine('HAVE_CONFIG_H');

project.addFile('Sources/**');
project.addFile('imageworsener/src/*.h');
project.addFile('imageworsener/src/*.c');
project.addExclude('imageworsener/src/imagew-cmd.c');

project.addIncludeDir('Sources');

solution.addProject(project);

return solution;
