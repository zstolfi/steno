{
	{ // Setup
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();
	}

	{ // Main Menu
		ImGuiViewport const* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::Begin("Steno Atlas Prototype", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);
		auto const avail = ImGui::GetContentRegionAvail();
		ImGui::SetNextWindowSizeConstraints(
			ImVec2 {std::min(340.0f, 0.5f * avail.x), 0},
			ImVec2 {0.5f * avail.x, FLT_MAX}
		);
		ImGui::BeginChild("ChildLeft", ImVec2 {460, 0}, ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX/*, ImGuiWindowFlags_MenuBar*/);
		{
			ImGui::SeparatorText("Dictionaries");
			if (state.dictionaries.empty()) {
				ImGui::Text("Drag & drop steno dictionaries to get started!");
				ImGui::Text("Accepted: (RTF, JSON, or TXT)");
			}
			else {
				ImGui::Text("Number of dictionaries loaded: %zu", state.dictionaries.size());
				for (int i=0; auto const& dict : state.dictionaries) {
					ImGui::PushID(i++);
					if (ImGui::Button(dict.name.c_str(), ImVec2 {-FLT_MIN, 40})) {
						state.selectedDictionary = &dict;
						canvas.setAtlas(dict.texture.get());
					}
					ImGui::PopID();
				}
			}
		}
		ImGui::EndChild();

		ImGui::SameLine();
		ImGui::BeginChild("ChildRight", ImVec2 {0, 0}, ImGuiChildFlags_Borders/*, ImGuiWindowFlags_MenuBar*/);
		{
			ImGui::SeparatorText("Atlas");
			if (auto const* dict = state.selectedDictionary) {
				ImGui::SameLine();
				if (ImGui::Button("Save")) dict->save();
				auto const avail = ImGui::GetContentRegionAvail();
				canvas.rescale(avail.x, avail.y);
				ImGui::Image(canvas.getTexture(), avail);
			}
		}
		ImGui::EndChild();
		ImGui::End();
	}

	{ // Update
		if (state.showDemoWindow) ImGui::ShowDemoWindow();
	}
}
