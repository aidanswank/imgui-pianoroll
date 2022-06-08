// struct pianoRoll {
// 	int sel = -1; // track note index??
// 	// ImVec2 myvec = {-1,-1};
// 	Note mynote; // last note
// 	float ticksPerColum = 2;
// 	float noteHeight = 8;
// };

int snap(float num, int by)
{
    return round(num/by)*by;
}

void SequencerWindow(bool *isOpen, smf::MidiFile& midiFile)
{

    smf::MidiEventList& midiTrack = midiFile[0];

	static int sel = -1; // track note index??
    static int noteleft_dragIdx = -1;
    static int noteright_dragIdx = -1;
	// ImVec2 myvec = {-1,-1};
	static Note mynote; // last note
    static Note lastNote; // last note
    static smf::MidiEvent lastEventClicked;
	static float ticksPerColum = 2;
	static float noteHeight = 8;

    float divsize = (96.f / 4);


	ImGui::Begin("toptoolbar", isOpen, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_HorizontalScrollbar);
	ImGui::SliderFloat("width", &ticksPerColum, 1.f, 32);
	// ImGui::SameLine();
	ImGui::SliderFloat("height", &noteHeight, 1.f, 32);
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

	ImDrawList *draw_list = ImGui::GetWindowDrawList();

	// print(p.x,p.y);
	ImGuiIO &io = ImGui::GetIO();
	ImGuiStyle &style = ImGui::GetStyle();

	// loop through all notes
	ImVec2 relativeWindowPos = ImGui::GetWindowPos();
	// relativeWindowPos.x += style.WindowPadding.x;
	// relativeWindowPos.y += style.WindowPadding.y;
	relativeWindowPos.x -= ImGui::GetScrollX();
	relativeWindowPos.y -= ImGui::GetScrollY();

	// //window + scrollpos
	// ImVec2 relativeWindowPos = ImGui::GetWindowPos();
	// relativeWindowPos.x -= ImGui::GetScrollX();
	// relativeWindowPos.y -= ImGui::GetScrollY();

	ImVec2 mouse_pos = ImGui::GetMousePos();
	// print(mouse_pos.x,mouse_pos.y-relativeWindowPos.y);

	// float relativeMouse.y = mouse_pos.y-relativeWindowPos.y;
	ImVec2 relativeMouse;
	relativeMouse.x = mouse_pos.x-relativeWindowPos.x;
	relativeMouse.y = mouse_pos.y-relativeWindowPos.y;

	// grid grids
	for (int i = 0; i < 200; i++)
	{
		float x = (divsize / ticksPerColum) * i;
		draw_list->AddLine(ImVec2(relativeWindowPos.x + x + 32, relativeWindowPos.y + 0), ImVec2(relativeWindowPos.x + x + 32, relativeWindowPos.y + ImGui::GetWindowHeight()+ImGui::GetScrollY()), ImColor(100, 100, 100, 50));
	}

    for(int i = 0; i < midiTrack.size(); i++)
    {
        // print(midiTrack[i].tick);
        ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(255,0,0,255));
        ImGui::PopStyleColor();

        smf::MidiEvent *event = &midiTrack[i];

        if(event->isNoteOn())
        {
			// print("testttt");
            float duration = event->getTickDuration();
            float tick = event->tick;
            float key = event->getKeyNumber();
            // set up note rectangle dimensions
            float note_w = duration / ticksPerColum;
            float note_x = (tick / ticksPerColum) + 32;
            // int noteRange = track.maxNote - track.minNote;
            // // float note_y = ((noteRange) - (track.notes[i].key - track.minNote)) * noteHeight;
            float note_y = (127-key) * noteHeight;


            // smf::MidiEvent *endNote = track.notes[i].endNote;
            int startX = ((float)event->tick / ticksPerColum) + 32;
            int endX = ((float)event->tick / ticksPerColum) + 32 + note_w - 8;

            // print(dur);
            ImVec2 startbtnsize = ImVec2(8, noteHeight);
            ImVec2 startbtnpos = ImVec2(startX, note_y);
            ImVec2 endbtnpos = ImVec2(endX, note_y);

            ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(255,0,0,255));

            ImGui::SetCursorPos(startbtnpos);
		    ImGui::Button(std::to_string(i).c_str(),startbtnsize);
            // selects note
            if (ImGui::IsItemClicked())
            {
                // print("start??",i);
                noteleft_dragIdx = i;
                lastEventClicked = midiTrack[noteleft_dragIdx];
            }
            ImGui::SetCursorPos(endbtnpos);
    		ImGui::Button(std::to_string(i).c_str(),startbtnsize);
            // selects note
            if (ImGui::IsItemClicked())
            {
                // print("start??",i);
                noteright_dragIdx = i;
                lastEventClicked = *midiTrack[noteright_dragIdx].getLinkedEvent();
            }
            ImGui::PopStyleColor();

            ImVec2 size = ImVec2(note_w, noteHeight);
            ImGui::SetCursorPos(ImVec2(note_x,note_y));
            ImVec2 button_size(note_w, noteHeight);
            ImGui::PushID(i);
            // ImGui::PushStyleColor(ImGuiCol_Button,key_color);
            ImGui::Button(" ",button_size);
            // ImGui::PopStyleColor();
            ImGui::PopID();

            // selects note
            if (ImGui::IsItemClicked())
            {
                sel = i;
                // mynote = track.notes[sel];
                lastEventClicked = midiTrack[sel];
                print("sel",sel);
            }
        };

        // handles moving note
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            // print("sel", sel, " ", ImGui::GetMouseDragDelta().x, " ", ImGui::GetMouseDragDelta().y);
            ImVec2 new_pos = ImGui::GetMouseDragDelta();
            // points[i].x = pos.x;
            if (sel != -1)
            {
                // ((float)(track.tpq / ticksPerColum)/4)
                int offset = (new_pos.x * ticksPerColum);
                int dur = midiTrack[sel].getTickDuration();
                int s = snap(lastEventClicked.tick + offset,divsize);
                midiTrack[sel].tick = s;

                // move other note
                midiTrack[sel].getLinkedEvent()->tick = dur + s;
                // print(midiTrack[sel].tick,midiTrack[sel].getLinkedEvent()->tick);

                int key = lastEventClicked.getKeyNumber() + (round(new_pos.y / noteHeight) * -1);
                midiTrack[sel].setKeyNumber(key);
                midiTrack[sel].getLinkedEvent()->setKeyNumber(key);

            }
            if(noteleft_dragIdx != -1)
            {
                print(new_pos.x,new_pos.y);
                int offset = (new_pos.x * ticksPerColum);
                int s = snap(lastEventClicked.tick + offset,divsize);
                // print(s);
                midiTrack[noteleft_dragIdx].tick = s;
            }
            if(noteright_dragIdx != -1)
            {
                print(new_pos.x,new_pos.y);
                int offset = (new_pos.x * ticksPerColum);
                int s = snap(lastEventClicked.tick + offset,divsize);
                // print(s);
                midiTrack[noteright_dragIdx].getLinkedEvent()->tick = s;
            }
        }
    }

	// for (int i = 0; i < track.notes.size(); i++)
	// {

    //     ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(255,0,0,255));
    //     ImGui::PopStyleColor();

	// 	// set up note rectangle dimensions
	// 	float note_w = track.notes[i].duration / ticksPerColum;
	// 	float note_x = ((track.notes[i].start) / ticksPerColum) + 32;
	// 	int noteRange = track.maxNote - track.minNote;
	// 	// float note_y = ((noteRange) - (track.notes[i].key - track.minNote)) * noteHeight;
	// 	float note_y = (127-track.notes[i].key) * noteHeight;

	// 	// float note_y = ((track.maxNote) - (track.notes[i].key - track.minNote)) * noteHeight;

	// 	// turn into imgui vecs
	// 	// ImVec2 size = ImVec2(note_w, noteHeight);
	// 	// ImVec2 pos = ImVec2(style.WindowPadding.x + note_x, style.WindowPadding.y + note_y);
	// 	// ImVec2 pos = ImVec2(note_x, note_y);
	// 	// ImGui::SetCursorPos(pos);

	// 	// set xy pos were we draw button
	// 	// print(p.x,p.y);

    //     smf::MidiEvent *endNote = track.notes[i].endNote;
    //     int endDur = endNote->getTickDuration();
    //     int endX = ((float)endNote->tick / ticksPerColum) + 32 - 8;
    //     // print(dur);
    //     ImVec2 endbtnsize = ImVec2(8, noteHeight);
    //     ImVec2 endbtnpos = ImVec2(endX, note_y);

	// 	ImGui::SetCursorPos(endbtnpos);
    //     ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(255,0,0,255));
	// 	ImGui::Button(std::to_string(i).c_str(),endbtnsize);
    //     ImGui::PopStyleColor();

    //     smf::MidiEvent *startNote = track.notes[i].startNote;
    //     int startDur = startNote->getTickDuration();
    //     int startX = ((float)startNote->tick / ticksPerColum) + 32;
    //     // print(dur);
    //     ImVec2 startbtnsize = ImVec2(8, noteHeight);
    //     ImVec2 startbtnpos = ImVec2(startX, note_y);

	// 	ImGui::SetCursorPos(startbtnpos);
    //     ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(255,0,0,255));
	// 	ImGui::Button(std::to_string(i).c_str(),startbtnsize);
    //     ImGui::PopStyleColor();

	// 	// id because we are using invisible buttons
	// 	// ImGui::PushID(i);
	// 	// ImGui::InvisibleButton(" ", size);
	// 	// ImGui::PopID();
        

	// 	// DEBUG button 

    //     		// ImGui::PushStyleColor(ImGuiCol_Button,key_color);

	// 	// ImGui::Button(std::to_string(i).c_str(),size);

	// 	// ImGui::SameLine();

	// 	// get color from current loaded style
	// 	// uint32_t mycolor = ImColor(style.Colors[ImGuiCol_Button]);
	// 	// draw_list->AddRectFilled(ImVec2(p.x + note_x, p.y + note_y), ImVec2(p.x + note_x + note_w, p.y + note_y + noteHeight), mycolor, 1.0f, ImDrawCornerFlags_All);

	// 	// draw_list->AddRectFilled(ImVec2(relativeWindowPos.x + note_x, relativeWindowPos.y + note_y), ImVec2(relativeWindowPos.x + note_x + note_w, relativeWindowPos.y + note_y + noteHeight), mycolor, 1.0f, ImDrawCornerFlags_All);

	// 	ImVec2 size = ImVec2(note_w, noteHeight);
	// 	ImGui::SetCursorPos(ImVec2(note_x,note_y));
	// 	ImVec2 button_size(note_w, noteHeight);
	// 	ImGui::PushID(i);
	// 	// ImGui::PushStyleColor(ImGuiCol_Button,key_color);
	// 	ImGui::Button(" ",button_size);
	// 	// ImGui::PopStyleColor();
	// 	ImGui::PopID();

	// 	// show outline around button when hovered
	// 	// if (sel==i)
    //     if (ImGui::IsItemHovered())
	// 	{
	// 		// print("hovering..");
	// 		draw_list->AddRect(ImVec2(relativeWindowPos.x + note_x, relativeWindowPos.y + note_y), ImVec2(relativeWindowPos.x + note_x + note_w, relativeWindowPos.y + note_y + noteHeight), ImColor(255, 255, 0), 1.0f, ImDrawCornerFlags_All);
	// 	}

	// 	// selects note
	// 	if (ImGui::IsItemClicked())
	// 	{
	// 		sel = i;
	// 		// print("isitemclicked", i, " pos ", io.MouseClickedPos[0].x, " ", io.MouseClickedPos[0].y);
	// 		mynote = track.notes[sel];
	// 	}
	// }

	// // handles moving note
	// if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
	// {
	// 	// print("sel", sel, " ", ImGui::GetMouseDragDelta().x, " ", ImGui::GetMouseDragDelta().y);
	// 	ImVec2 new_pos = ImGui::GetMouseDragDelta();
	// 	// points[i].x = pos.x;
	// 	if (sel != -1)
	// 	{
	// 		// points[sel].x = myvec.x + new_pos.x;
	// 		// ((float)(track.tpq / ticksPerColum)/4)
	// 		float hey = ((float)(track.tpq) / 4);
	// 		track.notes[sel].start = mynote.start + (new_pos.x * ticksPerColum);
	// 		// snap to grid
	// 		track.notes[sel].start = round(track.notes[sel].start / hey) * hey;
	// 		// points[sel].y = round((myvec.y + new_pos.y)/32)*32;
	// 		track.notes[sel].key = mynote.key + (round(new_pos.y / noteHeight) * -1);
	// 	}
	// }

    // float hey = (96.f / 4);

    // draw piano buttons
	for(int i = 0; i < 128; i++)
	{
		int i2 = 127-i;
		int bw = piano_keys[i%12];
		uint32_t key_color = ImColor(bw,bw,bw);

		ImGui::SetCursorPos(ImVec2(ImGui::GetScrollX(),noteHeight*i2));
		ImVec2 button_size(32, noteHeight);
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
		// 	relativeWindowPos.y + (i*noteHeight)),
		// 		ImVec2(ImGui::GetWindowPos().x + 32,
		// 	relativeWindowPos.y + noteHeight + (i*noteHeight)),
		// 	key_color, 1.0f, 
		// 	ImDrawCornerFlags_All
		// );

	}



	// note debuggggg
	float nnn = floor(relativeMouse.y/noteHeight);
	int notenum = 127-int(nnn);
	int octave = int(notenum / 12) - 1;
	int note = (notenum % 12);
	// print(note_names[note]);
	ImGui::SetCursorPos(ImVec2(ImGui::GetScrollX()+38,nnn*noteHeight));
	std::string text = note_names[note] + " " + std::to_string(notenum);
	ImGui::Text(text.c_str());

	// resets note selection
	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	{
		print("ImGuiMouseButton_Left released");
		sel = -1;
        noteleft_dragIdx = -1;
        noteright_dragIdx = -1;
	}

	if(sel==-1&&noteleft_dragIdx==-1&&noteright_dragIdx==-1)
	{
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{

			int nnn = (floor(relativeMouse.y/noteHeight)*-1) - 1;

			int tick = (mouse_pos.x-32+ImGui::GetScrollX()-ImGui::GetWindowPos().x) * ticksPerColum;

			// tick = snap(tick,divsize);
			tick = floor(tick/divsize)*divsize;

			smf::MidiEvent eventOn;
			eventOn.tick = tick;
			eventOn.track = 0;
			eventOn.makeNoteOn(0, nnn, 60);
			midiFile.addEvent(eventOn);

			smf::MidiEvent eventOff;
			eventOff.tick = tick+(divsize*1);
			eventOff.track = 0;
			eventOff.makeNoteOff(0,nnn,0);
			midiFile.addEvent(eventOff);

			midiFile.linkNotePairs();

			// event.makeNoteOff();
			// midiFile.addEvent(event);
			print("hey!!!!!!",mouse_pos.x,mouse_pos.y);
		}
	}

	ImGui::End();
}