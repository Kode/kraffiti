solution = Solution.new("kraffiti")
project = Project.new("kraffiti")

solution:cmd()

project:addExclude(".git/**")
project:addExclude("imageworsener/.git/**")
project:addExclude("build/**")

project:addDefine("HAVE_CONFIG_H")

project:addFile("Sources/**")
project:addFile("imageworsener/src/*.h")
project:addFile("imageworsener/src/*.c")
project:addExclude("imageworsener/src/imagew-cmd.c")

project:addIncludeDir("Sources");

solution:addProject(project)
