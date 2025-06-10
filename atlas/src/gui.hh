namespace GUI {
	void initiate() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();
	}

	void MainMenu() {
		ImGui::SetNextWindowSize(ImVec2 {500, 400}, ImGuiCond_FirstUseEver);
		ImGui::Begin("Steno Atlas Pre-Prototype");
		if (State::dictionaries.empty()) {
			ImGui::Text("Drag & drop dictionaries here to get started!");
			ImGui::Text("Accepted formats are RTF, JSON, or Plain-Text.");
			ImGui::End(); return;
		}
		ImGui::Text("Number of dictionaries loaded: %zu", State::dictionaries.size());
		for (int i=0; auto const& dict : State::dictionaries) {
			ImGui::PushID(i++);
			if (ImGui::TreeNode(dict.name.c_str())) {
				ImGui::Image(dict.texture, ImVec2 {256, 256});
				ImGui::Text("%zu entries", dict.entries.size());
				auto const flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter;
				if (ImGui::BeginTable("Entries", 2, flags, ImVec2 {400, 160})) {
					int const limit = 10'000; int i = 0;
					for (auto const& [stroke, text] : dict.entries) {
						if (i++ == limit) break;
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("%s", toString(stroke).c_str());
						ImGui::TableSetColumnIndex(1);
						ImGui::Text("%s", text.c_str());
					}
					ImGui::EndTable();
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
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
