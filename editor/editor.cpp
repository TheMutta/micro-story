// Dear ImGui: standalone example application for SDL2 + SDL_Renderer
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

// Important to understand: SDL_Renderer is an _optional_ component of SDL2.
// For a multi-platform app consider using e.g. SDL+DirectX on Windows and SDL+OpenGL on Linux/OSX.

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "imgui_stdlib.h"

#include <stdio.h>
#include <fstream>
#include <iostream>

#include <SDL2/SDL.h>
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

void LoadStory(std::string filename, std::map<size_t, Dialogue> &story) {
	std::ifstream inputFile(filename);

	json data = json::parse(inputFile);

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
}
					
void SaveStory(std::string filename, std::map<size_t, Dialogue> &story) {
	json data;

	for (size_t i = 0; story.find(i) != story.end(); ++i) {
		auto &element = data[i];

		element["ID"] = story[i].ID;
		element["TextID"] = story[i].Text;

		if(story[i].IsDialogue) {
			element["IsDialogue"] = true;
			element["NextID"] = story[i].NextID;
		} else {
			element["IsDialogue"] = false;
			size_t j = 0;
			element["TotalChoices"] = story[i].TotalChoices;
			for (auto [nextID, text] : story[i].Choices) {
				element["Choices"][std::to_string(j)]["NextID"] = nextID;
				element["Choices"][std::to_string(j)]["Text"] = text;
				++j;
			}
		}
	}

	std::ofstream outputFile(filename);
	outputFile  << std::setw(4) << data << std::endl;

}


#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

// Main code
int main(int, char**) {
	// Setup SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
		printf("Error: %s\n", SDL_GetError());
		return -1;
	}

	// From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

	// Create window with SDL_Renderer graphics context
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_Window* window = SDL_CreateWindow("Story Editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
	if (window == nullptr)	{
		printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
		return -1;
	}
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
	if (renderer == nullptr) {
		SDL_Log("Error creating SDL_Renderer!");
		return -1;
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer2_Init(renderer);

	// Our state
	std::map<size_t, Dialogue> story;
	bool createNodeWindow = false;
	bool removeNodeWindow = false;
	bool addAnswerWindow = false;
	bool removeAnswerWindow = false;
	ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// Main loop
	bool done = false;
	while (!done) {
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_QUIT)
				done = true;
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
				done = true;
		}
		if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
			SDL_Delay(10);
			continue;
		}

		// Start the Dear ImGui frame
		ImGui_ImplSDLRenderer2_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Workshop", &done, ImGuiWindowFlags_MenuBar);
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Open..", "Ctrl+O")) {
					LoadStory("story.json", story);
				}
				if (ImGui::MenuItem("Save", "Ctrl+S")) {
					SaveStory("story.bak", story);
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit")) {
				if (ImGui::MenuItem("Create Node", "Ctrl+N")) {
					createNodeWindow = true;
				}
				if (ImGui::MenuItem("Remove Node", "Ctrl+D")) {
					removeNodeWindow = true;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		static size_t selected = 0;
		{
			ImGui::BeginChild("left pane", ImVec2(150, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);

			for(auto [id, dial] : story) {
				if (ImGui::Selectable(std::to_string(dial.ID).c_str(), selected == id)) {
					selected = id;
				}
			}

			ImGui::EndChild();
		}
		ImGui::SameLine();

		{
			ImGui::BeginGroup();
			ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
			ImGui::Text("ID: %lu", selected);
			ImGui::Separator();
			if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None)) {
				auto &dial = story[selected];
				if (ImGui::BeginTabItem("Info")) {
					if (dial.IsDialogue) {
						ImGui::Text("Next ID: %lu", dial.NextID);
					} else {
						ImGui::TextWrapped("Is a question");
					}

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Text")) {
					ImGui::TextWrapped("%s", dial.Text.c_str());
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Edit Text")) {
					char buffer[1024 * 16];
					strcpy(buffer, dial.Text.c_str());
					if(ImGui::InputTextMultiline("Edit", buffer, IM_ARRAYSIZE(buffer), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_CtrlEnterForNewLine | ImGuiInputTextFlags_EnterReturnsTrue)) {
						dial.Text = std::string(buffer);
					}
					ImGui::EndTabItem();
				}
				if (!dial.IsDialogue) {
					if (ImGui::BeginTabItem("Answers")) {
						for(auto [id, text] : dial.Choices) {
							ImGui::TextWrapped("%lu -> %s", id, text.c_str());
						}

						if (ImGui::Button("Add answer")) {
							addAnswerWindow = true;
						}
						ImGui::SameLine();
						if (ImGui::Button("Remove answer")) {
							removeAnswerWindow = true;
						}

						ImGui::EndTabItem();

					}
				}

				ImGui::EndTabBar();
			}
			ImGui::EndChild();
			ImGui::EndGroup();
		}

		ImGui::End();

		if (addAnswerWindow) {
			static size_t id;
			static std::string data;

			ImGui::Begin("Add Answer", &addAnswerWindow);

			static char buffer[64] = {0};
			if(ImGui::InputText("ID", buffer, IM_ARRAYSIZE(buffer), 0)) {
				id = atoi(buffer);
			}
			
			ImGui::InputText("Answer", &data);

			if (ImGui::Button("Add Answer")) {
				story[selected].Choices[id] = data;
				addAnswerWindow = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				addAnswerWindow = false;
			}

			ImGui::End();
		}

		if (removeAnswerWindow) {
			static size_t id;

			ImGui::Begin("Remove Answer", &removeAnswerWindow);

			static char buffer[64] = {0};
			if(ImGui::InputText("ID", buffer, IM_ARRAYSIZE(buffer), 0)) {
				id = atoi(buffer);
			}
			
			if (ImGui::Button("Remove Answer")) {
				story[selected].Choices.erase(id);
				removeAnswerWindow = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				removeAnswerWindow = false;
			}

			ImGui::End();
		}


		if (createNodeWindow) {
			static Dialogue dialogue;

			ImGui::Begin("Create Node", &createNodeWindow);
			
			static char buffer[64] = {0};
			if(ImGui::InputText("Edit", buffer, IM_ARRAYSIZE(buffer), 0)) {
				dialogue.ID = atoi(buffer);
			}

			ImGui::Checkbox("Is dialogue?", &dialogue.IsDialogue);

			if (ImGui::Button("Create Node")) {
				story[dialogue.ID] = dialogue;
				createNodeWindow = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				createNodeWindow = false;
			}

			ImGui::End();
		}

		if (removeNodeWindow) {
			static size_t id;

			ImGui::Begin("Remove Node", &removeNodeWindow);

			static char buffer[64] = {0};
			if(ImGui::InputText("ID", buffer, IM_ARRAYSIZE(buffer), 0)) {
				id = atoi(buffer);
			}
			
			if (ImGui::Button("Remove Node")) {
				story.erase(id);
				removeNodeWindow = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				removeNodeWindow = false;
			}

			ImGui::End();
		}

		// Rendering
		ImGui::Render();
		SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
		SDL_SetRenderDrawColor(renderer, (Uint8)(clearColor.x * 255), (Uint8)(clearColor.y * 255), (Uint8)(clearColor.z * 255), (Uint8)(clearColor.w * 255));
		SDL_RenderClear(renderer);
		ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
		SDL_RenderPresent(renderer);
	}

	// Cleanup
	ImGui_ImplSDLRenderer2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
