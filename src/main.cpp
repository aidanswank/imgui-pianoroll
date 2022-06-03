#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/imgui_stdlib.h>
#include <imgui/imgui_impl_sdl.h>
#include <imgui/imgui_impl_opengl3.h>

// #include <iostream>
#include "vprint.h"
// #include <stdio.h>
// #include <stdint.h>
// #include <assert.h>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

// #include "entt/entt.hpp"

#include "MidiFileWrapper.h"

#define WinWidth 320 * 2
#define WinHeight 240 * 2



// int piano_keys[12]={0,255,0,255,0,0,255,0,255,0,255,0};
int piano_keys[12]={255,0,255,0,255,255,0,255,0,255,0,255};
std::string note_names[12]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};


void DockingToolbar(const char* name, ImGuiAxis* p_toolbar_axis)
{
    // [Option] Automatically update axis based on parent split (inside of doing it via right-click on the toolbar)
    // Pros:
    // - Less user intervention.
    // - Avoid for need for saving the toolbar direction, since it's automatic.
    // Cons: 
    // - This is currently leading to some glitches.
    // - Some docking setup won't return the axis the user would expect.
    const bool TOOLBAR_AUTO_DIRECTION_WHEN_DOCKED = true;

    // ImGuiAxis_X = horizontal toolbar
    // ImGuiAxis_Y = vertical toolbar
    ImGuiAxis toolbar_axis = *p_toolbar_axis;

    // 1. We request auto-sizing on one axis
    // Note however this will only affect the toolbar when NOT docked.
    ImVec2 requested_size = (toolbar_axis == ImGuiAxis_X) ? ImVec2(-1.0f, 0.0f) : ImVec2(0.0f, -1.0f);
    ImGui::SetNextWindowSize(requested_size);

    // 2. Specific docking options for toolbars.
    // Currently they add some constraint we ideally wouldn't want, but this is simplifying our first implementation
    ImGuiWindowClass window_class;
    window_class.DockingAllowUnclassed = true;
    window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoCloseButton;
    window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_HiddenTabBar; // ImGuiDockNodeFlags_NoTabBar // FIXME: Will need a working Undock widget for _NoTabBar to work
    window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoDockingSplitMe;
    window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoDockingOverMe;
    window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoDockingOverOther;
    if (toolbar_axis == ImGuiAxis_X)
        window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoResizeY;
    else
        window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoResizeX;
    ImGui::SetNextWindowClass(&window_class);

    // 3. Begin into the window
    const float font_size = ImGui::GetFontSize();
    const ImVec2 icon_size(ImFloor(font_size * 1.7f), ImFloor(font_size * 1.7f));
    ImGui::Begin(name, NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

    // 4. Overwrite node size
    ImGuiDockNode* node = ImGui::GetWindowDockNode();
    if (node != NULL)
    {
        // Overwrite size of the node
        ImGuiStyle& style = ImGui::GetStyle();
        const ImGuiAxis toolbar_axis_perp = (ImGuiAxis)(toolbar_axis ^ 1);
        const float TOOLBAR_SIZE_WHEN_DOCKED = style.WindowPadding[toolbar_axis_perp] * 2.0f + icon_size[toolbar_axis_perp];
        node->WantLockSizeOnce = true;
        node->Size[toolbar_axis_perp] = node->SizeRef[toolbar_axis_perp] = TOOLBAR_SIZE_WHEN_DOCKED;

        if (TOOLBAR_AUTO_DIRECTION_WHEN_DOCKED)
            if (node->ParentNode && node->ParentNode->SplitAxis != ImGuiAxis_None)
                toolbar_axis = (ImGuiAxis)(node->ParentNode->SplitAxis ^ 1);
    }
    
    // 5. Dummy populate tab bar
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5.0f, 5.0f));
    //UndockWidget(icon_size, toolbar_axis);
    for (int icon_n = 0; icon_n < 10; icon_n++)
    {
        char label[32];
        ImFormatString(label, IM_ARRAYSIZE(label), "%02d", icon_n);
        if (icon_n > 0 && toolbar_axis == ImGuiAxis_X)
            ImGui::SameLine();
        ImGui::Button(label, icon_size);
    }
    ImGui::PopStyleVar(2);

    // 6. Context-menu to change axis
    if (node == NULL || !TOOLBAR_AUTO_DIRECTION_WHEN_DOCKED)
    {
        if (ImGui::BeginPopupContextWindow())
        {
            ImGui::TextUnformatted(name);
            ImGui::Separator();
            if (ImGui::MenuItem("Horizontal", "", (toolbar_axis == ImGuiAxis_X)))
                toolbar_axis = ImGuiAxis_X;
            if (ImGui::MenuItem("Vertical", "", (toolbar_axis == ImGuiAxis_Y)))
                toolbar_axis = ImGuiAxis_Y;
            ImGui::EndPopup();
        }
    }

    ImGui::End();

    // Output user stored data
    *p_toolbar_axis = toolbar_axis;
}

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
static void HelpMarker(const char *desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void ShowExampleAppDockSpace(bool *p_open)
{
	// If you strip some features of, this demo is pretty much equivalent to calling DockSpaceOverViewport()!
	// In most cases you should be able to just call DockSpaceOverViewport() and ignore all the code below!
	// In this specific demo, we are not using DockSpaceOverViewport() because:
	// - we allow the host window to be floating/moveable instead of filling the viewport (when opt_fullscreen == false)
	// - we allow the host window to have padding (when opt_padding == true)
	// - we have a local menu bar in the host window (vs. you could use BeginMainMenuBar() + DockSpaceOverViewport() in your code!)
	// TL;DR; this demo is more complicated than what you would normally use.
	// If we removed all the options we are showcasing, this demo would become:
	//     void ShowExampleAppDockSpace()
	//     {
	//         ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
	//     }

	static bool opt_fullscreen = true;
	static bool opt_padding = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		const ImGuiViewport *viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}
	else
	{
		dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
	}

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
	// and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	if (!opt_padding)
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace Demo", p_open, window_flags);
	if (!opt_padding)
		ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	// Submit the DockSpace
	ImGuiIO &io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}
	else
	{
		// ShowDockingDisabledMessage();
	}

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Options"))
		{
			// Disabling fullscreen would allow the window to be moved to the front of other windows,
			// which we can't undo at the moment without finer window depth/z control.
			ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
			ImGui::MenuItem("Padding", NULL, &opt_padding);
			ImGui::Separator();

			if (ImGui::MenuItem("Flag: NoSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0))
			{
				dockspace_flags ^= ImGuiDockNodeFlags_NoSplit;
			}
			if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0))
			{
				dockspace_flags ^= ImGuiDockNodeFlags_NoResize;
			}
			if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0))
			{
				dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode;
			}
			if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0))
			{
				dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar;
			}
			if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen))
			{
				dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode;
			}
			ImGui::Separator();

			if (ImGui::MenuItem("Close", NULL, false, p_open != NULL))
				*p_open = false;
			ImGui::EndMenu();
		}
		HelpMarker(
			"Hello world :P"
			"\n"
			"test test test...");

		ImGui::EndMenuBar();
	}

	ImGui::End();
}

// #define SEQUENTITY_IMPLEMENTATION
// #include "Sequentity.h"
// entt::registry registry;

int sel = -1; // track note index??
// ImVec2 myvec = {-1,-1};
Note mynote;
float ticksPerColum = 2;
float noteHeight = 8;

void SequencerWindow(bool *isOpen, MidiTrack &track)
{

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
		float x = ((float)(track.tpq / ticksPerColum) / 4) * i;
		draw_list->AddLine(ImVec2(p2.x + x + 32, p2.y + 0), ImVec2(p2.x + x + 32, p2.y + ImGui::GetWindowHeight()+ImGui::GetScrollY()), ImColor(100, 100, 100, 50));
	}

	for (int i = 0; i < track.notes.size(); i++)
	{
		// set up note rectangle dimensions
		float note_w = track.notes[i].duration / ticksPerColum;
		float note_x = ((track.notes[i].start) / ticksPerColum) + 32;
		int noteRange = track.maxNote - track.minNote;
		// float note_y = ((noteRange) - (track.notes[i].key - track.minNote)) * noteHeight;
		float note_y = (127-track.notes[i].key) * noteHeight;

		// float note_y = ((track.maxNote) - (track.notes[i].key - track.minNote)) * noteHeight;

		// turn into imgui vecs
		ImVec2 size = ImVec2(note_w, noteHeight);
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
		ImVec2 button_size(note_w, noteHeight);
		ImGui::PushID(i);
		// ImGui::PushStyleColor(ImGuiCol_Button,key_color);
		ImGui::Button(" ",button_size);
		// ImGui::PopStyleColor();
		ImGui::PopID();

		// show outline around button when hovered
		if (ImGui::IsItemHovered())
		{
			// print("hovering..");
			draw_list->AddRect(ImVec2(p2.x + note_x, p2.y + note_y), ImVec2(p2.x + note_x + note_w, p2.y + note_y + noteHeight), ImColor(255, 255, 0), 1.0f, ImDrawCornerFlags_All);
		}

		// selects note
		if (ImGui::IsItemClicked())
		{
			sel = i;
			// print("isitemclicked", i, " pos ", io.MouseClickedPos[0].x, " ", io.MouseClickedPos[0].y);
			mynote = track.notes[sel];
		}
	}

	// handles moving note
	if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
	{
		print("sel", sel, " ", ImGui::GetMouseDragDelta().x, " ", ImGui::GetMouseDragDelta().y);
		ImVec2 new_pos = ImGui::GetMouseDragDelta();
		// points[i].x = pos.x;
		if (sel != -1)
		{
			// points[sel].x = myvec.x + new_pos.x;
			// ((float)(track.tpq / ticksPerColum)/4)
			float hey = ((float)(track.tpq) / 4);
			track.notes[sel].start = mynote.start + (new_pos.x * ticksPerColum);
			// snap to grid
			track.notes[sel].start = round(track.notes[sel].start / hey) * hey;
			// points[sel].y = round((myvec.y + new_pos.y)/32)*32;
			track.notes[sel].key = mynote.key + (round(new_pos.y / noteHeight) * -1);
		}
	}

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
		// 	p2.y + (i*noteHeight)),
		// 		ImVec2(ImGui::GetWindowPos().x + 32,
		// 	p2.y + noteHeight + (i*noteHeight)),
		// 	key_color, 1.0f, 
		// 	ImDrawCornerFlags_All
		// );

	}



	// note debuggggg
	float nnn = floor(relative_mouseY/noteHeight);
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
	}

	ImGui::End();
}

void setStyle()
{
	ImGuiStyle &style = ImGui::GetStyle();
	style.WindowRounding = 5.3f;
	style.FrameRounding = 2.3f;
	style.ScrollbarRounding = 0;

	style.Colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 0.90f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.15f, 1.00f);
	// style.Colors[ImGuiCol_ChildWindowBg]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.10f, 0.85f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.70f, 0.70f, 0.70f, 0.65f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.01f, 1.00f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.90f, 0.80f, 0.80f, 0.40f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.90f, 0.65f, 0.65f, 0.45f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.83f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.40f, 0.40f, 0.80f, 0.20f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.87f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.01f, 0.01f, 0.02f, 0.80f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.55f, 0.53f, 0.55f, 0.51f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.56f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.91f);
	// style.Colors[ImGuiCol_ComboBg]               = ImVec4(0.1f, 0.1f, 0.1f, 0.99f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 0.83f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.70f, 0.70f, 0.70f, 0.62f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.30f, 0.30f, 0.30f, 0.84f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.48f, 0.72f, 0.89f, 0.49f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.50f, 0.69f, 0.99f, 0.68f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.80f, 0.50f, 0.50f, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.30f, 0.69f, 1.00f, 0.53f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.44f, 0.61f, 0.86f, 1.00f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.38f, 0.62f, 0.83f, 1.00f);
	// style.Colors[ImGuiCol_Column]                = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	// style.Colors[ImGuiCol_ColumnHovered]         = ImVec4(0.70f, 0.60f, 0.60f, 1.00f);
	// style.Colors[ImGuiCol_ColumnActive]          = ImVec4(0.90f, 0.70f, 0.70f, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.85f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
	// style.Colors[ImGuiCol_CloseButton]           = ImVec4(0.50f, 0.50f, 0.90f, 0.50f);
	// style.Colors[ImGuiCol_CloseButtonHovered]    = ImVec4(0.70f, 0.70f, 0.90f, 0.60f);
	// style.Colors[ImGuiCol_CloseButtonActive]     = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
	// style.Colors[ImGuiCol_ModalWindowDarkening]  = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
};

int main(void)
{
	// printf("hello world");
	SDL_Init(SDL_INIT_EVERYTHING);
	// SDL_Init(SDL_INIT_VIDEO);

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // to make macos happy
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	// uint32_t WindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI;
	uint32_t WindowFlags = SDL_WINDOW_OPENGL | SDL_RENDERER_PRESENTVSYNC | SDL_WINDOW_RESIZABLE;
	SDL_Window *Window = SDL_CreateWindow("OpenGL Test", 0, 0, WinWidth, WinHeight, WindowFlags);
	// assert(Window);
	if (Window == NULL)
	{
		printf("failed to create window!");
	}

	SDL_GLContext Context = SDL_GL_CreateContext(Window);
	SDL_GL_MakeCurrent(Window, Context);
	SDL_GL_SetSwapInterval(1); // Enable vsync

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;	  // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;	  // Enable Multi-Viewport / Platform Windows
	io.ConfigWindowsMoveFromTitleBarOnly = true;
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	// ImGui::StyleColorsClassic();
	// ImGui::StyleColorsLight();
	ImGuiStyle *style = &ImGui::GetStyle();
	style->FrameBorderSize = 1.0f;

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(Window, Context);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	ImFont *font = io.Fonts->AddFontFromFileTTF("./res/fonts/unifont-14.0.01.ttf", 16.0f);

	setStyle();

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	bool isRunning = true;
	bool isFullScreen = true;

	bool isOpenSequencerWindow = true;
	std::vector<ImVec2> points = {ImVec2(0, 0), ImVec2(32, 32), ImVec2(32, 64), ImVec2(32, 128)};

	MidiFileWrapper mid;
	int err = mid.init("./res/midis/ableton.mid");
	if (err == 0)
	{
		std::cout << "error loading midi!!" << std::endl;
	}
	// mid.printData();
	std::vector<MidiTrack> tracks = mid.makeStructs();

	while (isRunning)
	{
		SDL_Event Event;
		while (SDL_PollEvent(&Event))
		{
			ImGui_ImplSDL2_ProcessEvent(&Event);
			if (Event.type == SDL_KEYDOWN)
			{
				switch (Event.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					isRunning = 0;
					break;
				case 'f':
					isFullScreen = !isFullScreen;
					if (isFullScreen)
					{
						SDL_SetWindowFullscreen(Window, WindowFlags | SDL_WINDOW_FULLSCREEN_DESKTOP);
					}
					else
					{
						SDL_SetWindowFullscreen(Window, WindowFlags);
					}
					break;
				default:
					break;
				}
			}
			else if (Event.type == SDL_QUIT)
			{
				isRunning = 0;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(Window);
		ImGui::NewFrame();

		{

			ImGui::PushFont(font);

			ImGui::Begin("debug");

			// ImGui::SliderFloat("rotation", &rot_amount, 0.0f, 360.0f);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			ImGui::End();

			// SequencerWindow(&isOpenSequencerWindow, points);
			ShowExampleAppDockSpace(&isOpenSequencerWindow);
			SequencerWindow(&isOpenSequencerWindow, tracks[0]);
			ImGui::ShowDemoWindow(&isOpenSequencerWindow);
			ImGui::PopFont();
		}

		// Rendering
		ImGui::Render();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
		//  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			SDL_Window *backup_current_window = SDL_GL_GetCurrentWindow();
			SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
		}

		SDL_GL_SwapWindow(Window);
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(Context);
	SDL_DestroyWindow(Window);
	SDL_Quit();

	return 0;
};