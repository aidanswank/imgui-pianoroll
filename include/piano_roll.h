struct pianoRoll {
	int sel = -1; // track note index??
	// ImVec2 myvec = {-1,-1};
	Note mynote; // last note
	float ticksPerColum = 2;
	float noteHeight = 8;
};

void SequencerWindow(bool *isOpen, MidiTrack &track, pianoRoll& prdata)
{

	ImGui::Begin("toptoolbar", isOpen, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_HorizontalScrollbar);
	ImGui::SliderFloat("width", &prdata.ticksPerColum, 1.f, 32);
	// ImGui::SameLine();
	ImGui::SliderFloat("height", &prdata.noteHeight, 1.f, 32);
	ImGui::End();

	// ImGui::Begin("sidetoolbar",isOpen,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_HorizontalScrollbar);
	// ImGui::VSliderFloat("height",ImVec2(16,200),&noteHeight,1.f,16);
	// ImGui::End();

	ImGui::Begin("pianoroll", isOpen, ImGuiWindowFlags_HorizontalScrollbar);
static ImGuiAxis toolbar1_axis = ImGuiAxis_X;
DockingToolbar("Toolbar1", &toolbar1_axis);
static ImGuiAxis toolbar2_axis = ImGuiAxis_Y;
DockingToolbar("Toolbar2", &toolbar2_axis);
	// const ImVec2 p = ImGui::GetCursorScreenPos();

	const ImVec2 wp = ImGui::GetWindowPos();

	// ImGui::BeginChild("child",ImVec2(ImGui::GetWindowWidth(),24),true);
    // // ImGui::BeginGroup();
	// ImGui::SliderFloat("width", &ticksPerColum, 1.f, 32);
	// // ImGui::Text("childdd");
	// // ImGui::EndGroup();
	// ImGui::EndChild();

	ImDrawList *draw_list = ImGui::GetWindowDrawList();

	// print(p.x,p.y);
	ImGuiIO &io = ImGui::GetIO();
	ImGuiStyle &style = ImGui::GetStyle();

	// loop through all notes
	ImVec2 p2 = ImGui::GetWindowPos();
	// p2.x += style.WindowPadding.x;
	// p2.y += style.WindowPadding.y;
	p2.x -= ImGui::GetScrollX();
	p2.y -= ImGui::GetScrollY();

	ImVec2 mouse_pos = ImGui::GetMousePos();
	// print(mouse_pos.x,mouse_pos.y-p2.y);

	float relative_mouseY = mouse_pos.y-p2.y;

	// grid grids
	for (int i = 0; i < 200; i++)
	{
		float x = ((float)(track.tpq / prdata.ticksPerColum) / 4) * i;
		draw_list->AddLine(ImVec2(p2.x + x + 32, p2.y + 0), ImVec2(p2.x + x + 32, p2.y + ImGui::GetWindowHeight()+ImGui::GetScrollY()), ImColor(100, 100, 100, 50));
	}

	for (int i = 0; i < track.notes.size(); i++)
	{
		// set up note rectangle dimensions
		float note_w = track.notes[i].duration / prdata.ticksPerColum;
		float note_x = ((track.notes[i].start) / prdata.ticksPerColum) + 32;
		int noteRange = track.maxNote - track.minNote;
		// float note_y = ((noteRange) - (track.notes[i].key - track.minNote)) * noteHeight;
		float note_y = (127-track.notes[i].key) * prdata.noteHeight;

		// float note_y = ((track.maxNote) - (track.notes[i].key - track.minNote)) * noteHeight;

		// turn into imgui vecs
		ImVec2 size = ImVec2(note_w, prdata.noteHeight);
		// ImVec2 pos = ImVec2(style.WindowPadding.x + note_x, style.WindowPadding.y + note_y);
		ImVec2 pos = ImVec2(note_x, note_y);

		// set xy pos were we draw button
		ImGui::SetCursorPos(pos);
		// print(p.x,p.y);

		// id because we are using invisible buttons
		// ImGui::PushID(i);
		// ImGui::InvisibleButton(" ", size);
		// ImGui::PopID();

		// DEBUG button 
		// ImGui::Button(std::to_string(i).c_str(),size);

		// ImGui::SameLine();

		// get color from current loaded style
		// uint32_t mycolor = ImColor(style.Colors[ImGuiCol_Button]);
		// draw_list->AddRectFilled(ImVec2(p.x + note_x, p.y + note_y), ImVec2(p.x + note_x + note_w, p.y + note_y + noteHeight), mycolor, 1.0f, ImDrawCornerFlags_All);

		// draw_list->AddRectFilled(ImVec2(p2.x + note_x, p2.y + note_y), ImVec2(p2.x + note_x + note_w, p2.y + note_y + noteHeight), mycolor, 1.0f, ImDrawCornerFlags_All);

		ImGui::SetCursorPos(ImVec2(note_x,note_y));
		ImVec2 button_size(note_w, prdata.noteHeight);
		ImGui::PushID(i);
		// ImGui::PushStyleColor(ImGuiCol_Button,key_color);
		ImGui::Button(" ",button_size);
		// ImGui::PopStyleColor();
		ImGui::PopID();

		// show outline around button when hovered
		if (prdata.sel==i)
		{
			// print("hovering..");
			draw_list->AddRect(ImVec2(p2.x + note_x, p2.y + note_y), ImVec2(p2.x + note_x + note_w, p2.y + note_y + prdata.noteHeight), ImColor(255, 255, 0), 1.0f, ImDrawCornerFlags_All);
		}

		// selects note
		if (ImGui::IsItemClicked())
		{
			prdata.sel = i;
			// print("isitemclicked", i, " pos ", io.MouseClickedPos[0].x, " ", io.MouseClickedPos[0].y);
			prdata.mynote = track.notes[prdata.sel];
		}
	}

	// handles moving note
	if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
	{
		print("sel", prdata.sel, " ", ImGui::GetMouseDragDelta().x, " ", ImGui::GetMouseDragDelta().y);
		ImVec2 new_pos = ImGui::GetMouseDragDelta();
		// points[i].x = pos.x;
		if (prdata.sel != -1)
		{
			// points[sel].x = myvec.x + new_pos.x;
			// ((float)(track.tpq / ticksPerColum)/4)
			float hey = ((float)(track.tpq) / 4);
			track.notes[prdata.sel].start = prdata.mynote.start + (new_pos.x * prdata.ticksPerColum);
			// snap to grid
			track.notes[prdata.sel].start = round(track.notes[prdata.sel].start / hey) * hey;
			// points[sel].y = round((myvec.y + new_pos.y)/32)*32;
			track.notes[prdata.sel].key = prdata.mynote.key + (round(new_pos.y / prdata.noteHeight) * -1);
		}
	}

	for(int i = 0; i < 128; i++)
	{
		int i2 = 127-i;
		int bw = piano_keys[i%12];
		uint32_t key_color = ImColor(bw,bw,bw);

		ImGui::SetCursorPos(ImVec2(ImGui::GetScrollX(),prdata.noteHeight*i2));
		ImVec2 button_size(32, prdata.noteHeight);
		ImGui::PushID(i);
		ImGui::PushStyleColor(ImGuiCol_Button,key_color);
		ImGui::Button(" ",button_size);
		// if(ImGui::IsItemHovered())
		// {
		// 	// ImGui::PushStyleColor(ImGuiCol_Button,ImColor(255,0,255));
		// 	print("hovering...");
		// 	ImGui::SetCursorPos(ImVec2(ImGui::GetScrollX()+32,noteHeight*i2));
		// 	ImGui::Text(std::to_string(i).c_str());
		// }
		ImGui::PopStyleColor();
		ImGui::PopID();

		// draw_list->AddRectFilled(
		// 		ImVec2(ImGui::GetWindowPos().x,
		// 	p2.y + (i*noteHeight)),
		// 		ImVec2(ImGui::GetWindowPos().x + 32,
		// 	p2.y + noteHeight + (i*noteHeight)),
		// 	key_color, 1.0f, 
		// 	ImDrawCornerFlags_All
		// );

	}



	// note debuggggg
	float nnn = floor(relative_mouseY/prdata.noteHeight);
	int notenum = 127-int(nnn);
	int octave = int(notenum / 12) - 1;
	int note = (notenum % 12);
	// print(note_names[note]);
	ImGui::SetCursorPos(ImVec2(ImGui::GetScrollX()+38,nnn*prdata.noteHeight));
	std::string text = note_names[note] + " " + std::to_string(notenum);
	ImGui::Text(text.c_str());

	// resets note selection
	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	{
		print("ImGuiMouseButton_Left released");
		prdata.sel = -1;
	}

	ImGui::End();
}