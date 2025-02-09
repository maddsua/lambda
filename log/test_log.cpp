#include "./log.hpp"

int main() {

	Lambda::Log::err("Smol error message");
	Lambda::Log::err("Formatted message; Data: {}", { 42 });

	Lambda::Log::log("Message with no params {}");
	Lambda::Log::log("Message with no provided params {}", {});
	Lambda::Log::log("Message with no params {} again", {});

	Lambda::Log::log("Message #{} with a ton of {} params", { 13, "additional" });

	Lambda::Log::log("{} {} Sample debug message: {}", { "17:54:23 09/02/2025", "DEBUG", "Bruh" });
	Lambda::Log::err("{}}} {} {{{{}}}} {}", { "17:54:23 09/02/2025", "DEBUG", "Bruh" });
	Lambda::Log::err("{}}} {} {{{{}}}} {}", {  });
	Lambda::Log::err("{}}} {} {{{{}}}} {}");

	return 0;
}
