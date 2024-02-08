#include <iostream>
#include <vector>

#include "../../lambda.hpp"

using namespace Lambda::Storage;

const std::vector<std::pair<std::string, std::string>> dataset = {
	{ "test.txt", "hello world" },
	{ "index.html", "<h1>hello world</h1>" },
	{ "story.html", "Strolling through the park on a crisp autumn day, the leaves crunching underfoot. The scent of fresh coffee wafting from a nearby café fills the air. Birds chirp merrily in the distance, adding to the peaceful ambiance. Nature's simple beauty, a serene escape from the chaos of everyday life." },
	{ "big_story.html", "<p>The sun begins its descent, painting the sky in hues of orange and pink. The city comes alive with the gentle buzz of activity as people make their way home after a long day's work. Street vendors pack up their stalls, their laughter blending with the distant sound of traffic. A gentle breeze carries the scent of blooming flowers from a nearby garden.</p><p>As twilight falls, the first stars appear, twinkling brightly in the darkening sky. A sense of calm settles over the city, a brief respite before the vibrant nightlife takes over. The distant sound of music drifts through the air, mingling with the chatter of passersby.</p><p>In this moment, time seems to slow down, allowing for reflection and appreciation of the simple joys in life. The beauty of the world is evident in the smallest details – a delicate blossom blooming in a crack in the pavement, a child's laughter echoing down the street.</p><p>As the day fades into night, the city transforms into a different entity, pulsing with energy and life. Each corner holds a story, each person a journey. Amidst the chaos and noise, there is a unique kind of beauty that can only be found in the heart of the city.</p>" },
	{ "Users/Administrator/Documents/Projects/Company-Name/Client-Name/2024/Financial-Reports/Q1/Drafts/Final/Analysis/2024-Q1-ClientName-Financial-Report-Final.docx", "You didn't think it would be a real docx file did you"}
};

int main(int argc, char const *argv[]) {

	const std::string tarfileloc = "test.tar";

	Lambda::VirtualFilesystem vfs;

	for (const auto& item : dataset) {
		vfs.write(item.first, item.second);
	}

	vfs.saveSnapshot(tarfileloc);

	Lambda::VirtualFilesystem vfs2;
	vfs2.loadSnapshot(tarfileloc);

	auto entries = vfs2.listAll();

	std::cout << "size of size_t " << sizeof(size_t) << std::endl;

	/*for (const auto& item : entries) {
		printf("%s | %llu\n", item.name.c_str(), item.size);
	}*/

	for (const auto& item : dataset) {

		auto restored = vfs2.read(item.first);
		if (!restored.has_value()) {
			throw std::runtime_error("VFS failed to restore file \"" + item.first + '\"');
		}

		if (item.second.size() != restored.value().size()) {
			throw std::runtime_error("File size mismatch for \"" + item.first + '\"');
		}

		if (item.second != restored.value().text()) {
			throw std::runtime_error("Content mismatch of \"" + item.first + '\"');
		}
	}

	return 0;
}
