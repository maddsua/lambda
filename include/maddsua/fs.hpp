/*

	maddsua's
     ___       ________  _____ ______   ________  ________  ________
    |\  \     |\   __  \|\   _ \  _   \|\   __  \|\   ___ \|\   __  \
    \ \  \    \ \  \|\  \ \  \\\__\ \  \ \  \|\ /\ \  \_|\ \ \  \|\  \
     \ \  \    \ \   __  \ \  \\|__| \  \ \   __  \ \  \ \\ \ \   __  \
      \ \  \____\ \  \ \  \ \  \    \ \  \ \  \|\  \ \  \_\\ \ \  \ \  \
       \ \_______\ \__\ \__\ \__\    \ \__\ \_______\ \_______\ \__\ \__\
        \|_______|\|__|\|__|\|__|     \|__|\|_______|\|_______|\|__|\|__|

	A C++ HTTP server framework

	2023 https://github.com/maddsua/lambda
	
*/


#ifndef H_MADDSUA_LAMBDA_FILESYSTEM
#define H_MADDSUA_LAMBDA_FILESYSTEM

#include <string>

#define LAMBDA_FS_READ_CHUNK	(1048576)

namespace lambda {
	namespace fs {

		bool writeSync(const std::string path, const std::string* data);
		bool readSync(const std::string path, std::string* dest);

		//	Create directories recursively
		bool createTree(std::string tree);
		
	}
}

#endif