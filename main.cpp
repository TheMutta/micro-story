#include <stdint.h>
#include <stddef.h>
#include <string>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Dialogue {
	bool IsDialogue;

	size_t ID;
	size_t NextID;

	std::string Text;

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

int main() {
	std::string jsonText =
		R"(
		[{
			"IsDialogue": true,
			"ID": 0,
			"NextID": 1,
			"Text": "Hello, world"
		},
		{
			"IsDialogue": true,
			"ID": 1,
			"NextID": 2,
			"Text": "I am jack, who are you?"
		},
		{
			"IsDialogue": false,
			"ID": 2,
			"Text": "What do you answer?",
			"TotalChoices": 2,
			"Choices": {
				"0": {
					"NextID": 3,
					"Text": "I am Jane"
				},
				"1": {
					"NextID": 4,
					"Text": "Fuck you"
				}
			}
		},
		{
			"IsDialogue": true,
			"ID": 3,
			"NextID": 0,
			"Text": "Hi Jane"
		},
		{
			"IsDialogue": true,
			"ID": 4,
			"NextID": 0,
			"Text": "What? Fuck you!"
		}]
		)";

	json data = json::parse(jsonText);

	std::map<size_t, Dialogue> story;

	for (size_t i = 0; i < 5; ++i) {
		auto element = data[i];

		story[i].ID = element["ID"];
		story[i].Text = element["Text"];

		if(element["IsDialogue"]) {
			story[i].IsDialogue = true;
			story[i].NextID = element["NextID"];
		} else {
			story[i].IsDialogue = false;
			for(size_t j = 0; j < element["TotalChoices"]; ++j) {
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
}
