	//	connect to google.com
	/*{
		auto googeResp = maddsua::fetch("www.google.com", "GET", {}, "");

		printf("Connecting to google.com... %i %s", googeResp.statusCode, googeResp.statusText);
		if (googeResp.errors.size()) puts(googeResp.errors.c_str());
		puts(googeResp.body.c_str());

		for (auto header : googeResp.headers) {
			std::cout << header.name << " " << header.value << std::endl;
		}

		std::cout << "Writing to googlecom.bin result: " << maddsua::writeSync("googlecom.bin", &googeResp.body) << std::endl;
	}*/