{
	{ // Setup
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();
	}

	{ // Main Menu
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::Begin("Steno Atlas Prototype", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
		if (state.dictionaries.empty()) {
			ImGui::Text("Drag & drop dictionaries here to get started!");
			ImGui::Text("Accepted: (RTF, JSON,or TXT)");
		}
		else {
			ImGui::Text("Number of dictionaries loaded: %zu", state.dictionaries.size());
			for (int i=0; auto const& dict : state.dictionaries) {
				ImGui::PushID(i++);
				if (ImGui::TreeNode(dict.name.c_str())) {
					ImGui::Image(dict.texture, ImVec2 {256, 256});
					ImGui::AlignTextToFramePadding();
					ImGui::SameLine();
					if (ImGui::Button("Save")) {
						std::filesystem::path imgPath {dict.name};
						imgPath.replace_extension("");
						imgPath += " atlas.png";
						stbi_write_png(
							imgPath.c_str(), Atlas::N, Atlas::N,
							4, dict.atlas.image.data(), 4*Atlas::N
						);
						window.offerDownload(imgPath);
					}
					ImGui::Text("%zu entries\t 1 - 1,000:", dict.entries.size());
					auto const flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter;
					if (ImGui::BeginTable("Entries", 2, flags, ImVec2 {400, 160})) {
						int const limit = 1'000; int i = 0;
						for (auto const& [strokes, text] : dict.entries) {
							if (strokes.list.size() != 1) continue;
							if (i++ == limit) break;
							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("%s", toString(strokes).c_str());
							ImGui::TableSetColumnIndex(1);
							ImGui::Text("%s", text.c_str());
						}
						ImGui::EndTable();
					}
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
		}
		ImGui::End();
	}

	{ // Update
		if (state.showDemoWindow) ImGui::ShowDemoWindow();
	}
}
