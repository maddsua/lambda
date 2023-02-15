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


#ifndef H_MADDSUA_BASE64
#define H_MADDSUA_BASE64


#include <stdint.h>
#include <string>
#include <vector>


namespace maddsua {

	std::string b64Decode(std::string* data);
	std::string b64Encode(std::string* data);

	bool b64Validate(const std::string* encoded);
	
}

#endif