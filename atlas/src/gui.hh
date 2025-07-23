{
	auto drawStenotype = [] (steno::Stroke stroke) {
		ImVec4 const* Colors = ImGui::GetStyle().Colors;
		ImVec2 const Pos = ImGui::GetCursorScreenPos();

		ImVec2 const box {12, 15};
		float const pad = 2;
		float const radius = box.x / 2.0;
		ImGui::Dummy(ImVec2 {10*box.x - pad, 3*box.y - pad});
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		auto drawKey = [&] (steno::Key key, ImVec2 xy, ImVec2 wh, bool round) {
			ImColor const KeyColors[2] = {
				Colors[ImGuiCol_FrameBg],
				Colors[ImGuiCol_ButtonActive],
			};
			bool const pressed = stroke.get(key);
			ImVec2 const pos {box.x*xy.x + Pos.x, box.y*xy.y + Pos.y};
			ImVec2 const scl {box.x*wh.x - pad  , box.y*wh.y - pad  };
			drawList->AddRectFilled(
				ImVec2 {pos.x        , pos.y        },
				ImVec2 {pos.x + scl.x, pos.y + scl.y},
				KeyColors[pressed],
				round? radius: 0.0,
				round? ImDrawFlags_RoundCornersBottom: ImDrawFlags_None
			);
		};
		drawKey(steno::Key::S_, ImVec2 {0, 0}, ImVec2 {1, 2}, true);
		drawKey(steno::Key::T_, ImVec2 {1, 0}, ImVec2 {1, 1}, false);
		drawKey(steno::Key::K_, ImVec2 {1, 1}, ImVec2 {1, 1}, true);
		drawKey(steno::Key::P_, ImVec2 {2, 0}, ImVec2 {1, 1}, false);
		drawKey(steno::Key::W_, ImVec2 {2, 1}, ImVec2 {1, 1}, true);
		drawKey(steno::Key::H_, ImVec2 {3, 0}, ImVec2 {1, 1}, false);
		drawKey(steno::Key::R_, ImVec2 {3, 1}, ImVec2 {1, 1}, true);
		drawKey(steno::Key::A, ImVec2 {2.3, 2}, ImVec2 {1, 1}, true);
		drawKey(steno::Key::O, ImVec2 {3.3, 2}, ImVec2 {1, 1}, true);
		drawKey(steno::Key::E, ImVec2 {4.7, 2}, ImVec2 {1, 1}, true);
		drawKey(steno::Key::U, ImVec2 {5.7, 2}, ImVec2 {1, 1}, true);
		drawKey(steno::Key::x , ImVec2 {4, 0}, ImVec2 {1, 2}, true);
		drawKey(steno::Key::_F, ImVec2 {5, 0}, ImVec2 {1, 1}, false);
		drawKey(steno::Key::_R, ImVec2 {5, 1}, ImVec2 {1, 1}, true);
		drawKey(steno::Key::_P, ImVec2 {6, 0}, ImVec2 {1, 1}, false);
		drawKey(steno::Key::_B, ImVec2 {6, 1}, ImVec2 {1, 1}, true);
		drawKey(steno::Key::_L, ImVec2 {7, 0}, ImVec2 {1, 1}, false);
		drawKey(steno::Key::_G, ImVec2 {7, 1}, ImVec2 {1, 1}, true);
		drawKey(steno::Key::_T, ImVec2 {8, 0}, ImVec2 {1, 1}, false);
		drawKey(steno::Key::_S, ImVec2 {8, 1}, ImVec2 {1, 1}, true);
		drawKey(steno::Key::_D, ImVec2 {9, 0}, ImVec2 {1, 1}, false);
		drawKey(steno::Key::_Z, ImVec2 {9, 1}, ImVec2 {1, 1}, true);
	};

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
				ImGui::Text(
					"Drag & drop steno dictionaries to get started!\n"
					"Accepted: (RTF, JSON, or TXT)\n"
					"\n\n\n"
				);
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Or,");
				ImGui::SameLine();
				if (ImGui::Button("browse Plover's default dictionary.")) state.downloadDefaultDictionary();
			}
			else {
				ImGui::Text("Number of dictionaries loaded: %zu", state.dictionaries.size());
				for (int i=0; i<state.dictionaries.size(); i++) {
					auto const& dict = state.dictionaries[i];
					ImGui::PushID(i);
					if (ImGui::Button(dict.name.c_str(), ImVec2 {-FLT_MIN, 40})) {
						state.selectDictionary(i);
					}
					ImGui::PopID();
				}
				auto const* dict = state.selectedDictionary();
				ImGui::SeparatorText(dict->name.c_str());
				auto const count = dict->atlas.getCount();
				auto const viewName = dict->atlas.getMapping()->name();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("%u entries displayed by %s.", count, viewName.c_str());
				ImGui::SameLine();
				if (ImGui::Button("Save PNG")) dict->save();
//				ImGui::Text("%u entries\t%s:", count, (count > 1000)? " 1 - 1,000": "");
//				auto const flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter;
//				if (ImGui::BeginTable("Entries", 2, flags, ImVec2 {400, 160})) {
//					int const limit = 1'000;
//					for (int i=0; auto const& [strokes, text] : dict.entries) {
//						if (strokes.list.size() != 1) continue;
//						if (strokes.list[0].keys.Num) continue;
//						if (i++ == limit) break;
//						ImGui::TableNextRow();
//						ImGui::TableSetColumnIndex(0);
//						ImGui::Text("%s", toString(strokes).c_str());
//						ImGui::TableSetColumnIndex(1);
//						ImGui::Text("%s", text.c_str());
//					}
//					ImGui::EndTable();
//				}
				{ // Instructions
					auto const instructions = 
						"Left Click: \tPan\n"
						"Right Click:\tDisplay stroke\n"
						"Scroll:     \tZoom\n"
						"1 or 2:     \tSelect alternate view\n"
					;
					auto const size = ImGui::CalcTextSize(instructions);
					auto const avail = ImGui::GetContentRegionAvail();
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + avail.y - size.y);
					ImGui::BeginChild("ChildLeftBottom", ImVec2 {0, 0});
					ImGui::TextColored(ImVec4 {0.7, 0.7, 0.7, 1.0}, instructions);
					ImGui::EndChild();
				}
			}
		}
		ImGui::EndChild();

		ImGui::SameLine();
		ImGui::BeginChild("ChildRight", ImVec2 {0, 0}, ImGuiChildFlags_Borders/*, ImGuiWindowFlags_MenuBar*/);
		{
			ImGui::SeparatorText("Atlas");
			if (auto const* dict = state.selectedDictionary()) {
				canvas.setAtlas(dict->atlas.getTexture());
				auto const corner = ImGui::GetCursorScreenPos();
				auto const avail = ImGui::GetContentRegionAvail();
				canvas.rescale(avail.x, avail.y);
				ImGui::Image(canvas.getTexture(), avail);

				if (ImGui::IsMousePosValid()) {
					ImGuiIO const& io = ImGui::GetIO();
					auto atlasPos = atlasCoordinates(avail, ImVec2 {
						io.MousePos.x - corner.x - ImGui::GetScrollX(),
						io.MousePos.y - corner.y - ImGui::GetScrollY(),
					});
					if (atlasPos) {
						auto [x, y] = *atlasPos;
						steno::Stroke stroke = dict->atlas.getMapping()->toStrokes({x, y}).list[0];
						auto const entry = dict->entries.find(stroke);
						auto const NoEntry = dict->entries.end();
						if (entry != NoEntry || ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
							ImGui::BeginTooltip();
							if (entry != NoEntry) ImGui::Text("%s", entry->second.c_str());
							drawStenotype(stroke);
							ImGui::Text("%s", steno::toString(stroke).c_str());
							ImGui::Text("%u, %u", x, y);
							ImGui::EndTooltip();
						}
					}
				}
			}
		}
		ImGui::EndChild();
		ImGui::End();
	}

	{ // Update
		if (state.showDemoWindow) ImGui::ShowDemoWindow();
	}
}
