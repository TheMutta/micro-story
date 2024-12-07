#include <stdint.h>
#include <stddef.h>
#include <string>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Dialogue {
	bool IsDialogue;

	size_t ID;
	size_t NextID;

	std::string Text;

	size_t TotalChoices;
	std::map<size_t, std::string> Choices;
};


void PrintDialogue(size_t id, std::map<size_t, Dialogue> &story) {
	std::cout << story[id].Text << std::endl;

	std::cin.get();
}

bool NextDialogue(size_t &id, std::map<size_t, Dialogue> &story) {
	if (story[id].IsDialogue) {
		if (story[id].NextID == 0) {
			return false;
		}

		id = story[id].NextID;
	} else {
		while (true) {
			for (auto [nextId, text] : story[id].Choices) {
				std::cout << nextId << " -> " << text << std::endl;
			}

			size_t choice;
			std::cin >> choice;

			if(story[id].Choices.find(choice) != story[id].Choices.end()) {
				id = choice;
				break;
			}
		}
	}

	return true;
}

int main(int argc, char **argv) {
	std::string filename = "story.json";

	if (argc == 2) {
		filename = argv[1];
	}

	std::ifstream inputFile(filename);

	json data = json::parse(inputFile);

	std::map<size_t, Dialogue> story;

	for (size_t i = 0;; ++i) {
		auto element = data[i];

		if (element == nullptr) break;

		story[i].ID = element["ID"];
		story[i].Text = element["Text"];

		if(element["IsDialogue"]) {
			story[i].IsDialogue = true;
			story[i].NextID = element["NextID"];
		} else {
			story[i].IsDialogue = false;
			story[i].TotalChoices = element["TotalChoices"];
			for(size_t j = 0; j < story[i].TotalChoices; ++j) {
				size_t nextID = element["Choices"][std::to_string(j)]["NextID"];
				std::string text = element["Choices"][std::to_string(j)]["Text"];
				story[i].Choices[nextID] = text;
			}
		}
	}

	size_t id = 0;
	do {
		PrintDialogue(id, story);
	} while (NextDialogue(id, story));

	std::cout << std::endl << "THE END" << std::endl;
}
