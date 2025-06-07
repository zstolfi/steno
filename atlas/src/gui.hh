namespace GUI {
	void initiate() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();
	}

	void MainMenu() {
		ImGui::SetNextWindowSize(ImVec2 {500, 400}, ImGuiCond_FirstUseEver);
		ImGui::Begin("Steno Atlas Pre-Prototype");
		if (State::files.empty()) {
			ImGui::Text("Drag & drop files here to get started!");
		}
		else {
			ImGui::Text("Number of files loaded: %zu", State::files.size());
			for (int i=0; auto file : State::files) {
				ImGui::PushID(i++);
				if (ImGui::TreeNode(file.name.c_str())) {
					ImGui::Text("%zu bytes", file.bytes.size());
					ImGui::SameLine();
					if (ImGui::Button("Print")) file.print();
					if (auto it = State::atlases.find(file); it != State::atlases.end()) {
						auto const& atlas = it->second;
						ImGui::Text("Atlas (default image for now)");
						ImGui::Indent();
						ImGui::Image((ImTextureID)(intptr_t)atlas.texture, ImVec2 {256, 256});
						ImGui::Unindent();
					}
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
		}
		ImGui::End();
	}

	void BottomRightOverlay() {
		auto* viewport = ImGui::GetMainViewport();
		ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
		ImVec2 work_size = viewport->WorkSize;
		ImVec2 windowPos;
		windowPos.x = work_pos.x + work_size.x - 10;
		windowPos.y = work_pos.y + work_size.y - 10;

		ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, ImVec2 {1, 1});
		ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
		ImGui::Begin("Programmer Menu", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
		ImGui::Checkbox("I'm Bored", &State::showDemoWindow);
		ImGui::End();
	}
}
