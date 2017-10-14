
#include "Client.h"
#define charenc
//[enc_string_enable /]
//[junk_enable /]

namespace Client
{
	//[swap_lines]
	int	iScreenWidth = 0;
	int	iScreenHeight = 0;
	int buttonmenu = 0;
	float penis;
	int Animatetype;
	int sleepanimate;
	int sleepanimate2;
	int sleepanimate3;
	int pizda = 0;
	int periodicity = 1.f;
	int menustyle = 1;
	float periodicity1 = 0.01f;
	int periodicity2 = 1.f;
	float R3animate = 0.0f;
	float BUDSTER1 = 0.0f;
	float BUDSTER2 = 200.0f;
	float connva = 0.01f;
	string BaseDir = "";
	string LogFile = "";
	string GuiFile = "";
	string IniFile = "";

	vector<string> ConfigList;

	Vector2D	g_vCenterScreen = Vector2D(0.f, 0.f);

	CPlayers*	g_pPlayers = nullptr;
	CRender*	g_pRender = nullptr;
	CGui*		g_pGui = nullptr;

	CAimbot*	g_pAimbot = nullptr;
	CTriggerbot* g_pTriggerbot = nullptr;
	CEsp*		g_pEsp = nullptr;
	CRadar*		g_pRadar = nullptr;
	CKnifebot*	g_pKnifebot = nullptr;
	CSkin*		g_pSkin = nullptr;
	CMisc*		g_pMisc = nullptr;

	bool		bC4Timer = false;
	int			iC4Timer = 40;

	int			iWeaponID = 0;
	int			iWeaponSelectIndex = WEAPON_DEAGLE;
	int			iWeaponSelectSkinIndex = -1;
	//[/swap_lines]

	void ReadConfigs(LPCTSTR lpszFileName)
	{
		if (!strstr(lpszFileName, "gui.ini"))
		{
			ConfigList.push_back(lpszFileName);
		}
	}

	void RefreshConfigs()
	{
		ConfigList.clear();
		string ConfigDir = BaseDir + "\\*.ini";
		SearchFiles(ConfigDir.c_str(), ReadConfigs, FALSE);
	}
	template<size_t N>
	void render_tabs(char* (&names)[N], int& activetab, float w, float h, bool sameline)
	{
		bool values[N] = { false };

		values[activetab] = true;

		for (auto i = 0; i < N; ++i) {
			if (ImGui::ToggleButton(names[i], &values[i], ImVec2{ w, h })) {
				activetab = i;
			}
			if (sameline && i < N - 1)
				ImGui::SameLine();
		}
	}
	bool Initialize(IDirect3DDevice9* pDevice)
	{
		g_pPlayers = new CPlayers();
		g_pRender = new CRender(pDevice);
		g_pGui = new CGui();

		g_pAimbot = new CAimbot();
		g_pTriggerbot = new CTriggerbot();
		g_pEsp = new CEsp();
		g_pRadar = new CRadar();
		g_pKnifebot = new CKnifebot();
		g_pSkin = new CSkin();
		g_pMisc = new CMisc();

		GuiFile = BaseDir + "\\" + "gui.ini";
		IniFile = BaseDir + "\\" + "settings.ini";

		g_pSkin->InitalizeSkins();

		Settings::LoadSettings(IniFile);

		iWeaponSelectSkinIndex = GetWeaponSkinIndexFromPaintKit(g_SkinChangerCfg[iWeaponSelectIndex].nFallbackPaintKit);

		g_pGui->GUI_Init(pDevice);

		RefreshConfigs();

		return true;
	}

	void Shutdown()
	{
		DELETE_MOD(g_pPlayers);
		DELETE_MOD(g_pRender);
		DELETE_MOD(g_pGui);

		DELETE_MOD(g_pAimbot);
		DELETE_MOD(g_pTriggerbot);
		DELETE_MOD(g_pEsp);
		DELETE_MOD(g_pRadar);
		DELETE_MOD(g_pKnifebot);
		DELETE_MOD(g_pSkin);
		DELETE_MOD(g_pMisc);
	}

	void OnRender()
	{
		if (g_pRender && !Interfaces::Engine()->IsTakingScreenshot() && Interfaces::Engine()->IsActiveApp())
		{
			g_pRender->BeginRender();

			if (g_pGui)
				g_pGui->GUI_Draw_Elements();

			Interfaces::Engine()->GetScreenSize(iScreenWidth, iScreenHeight);

			g_vCenterScreen.x = iScreenWidth / 2.f;
			g_vCenterScreen.y = iScreenHeight / 2.f;

			if (!Interfaces::Engine()->IsConnected())
				g_pRender->Text(3, 3, false, true, Color::Aqua(), "Tenderness Reborn|Full");

			if (Client::g_pPlayers && Client::g_pPlayers->GetLocal() && Interfaces::Engine()->IsInGame())
			{
				if (g_pEsp)
					g_pEsp->OnRender();

				if (g_pMisc)
				{
					g_pMisc->OnRender();
					g_pMisc->OnRenderSpectatorList();
				}
			}

			g_pRender->EndRender();
		}
	}

	void OnLostDevice()
	{
		if (g_pRender)
			g_pRender->OnLostDevice();

		if (g_pGui)
			ImGui_ImplDX9_InvalidateDeviceObjects();
	}

	void OnResetDevice()
	{
		if (g_pRender)
			g_pRender->OnResetDevice();

		if (g_pGui)
			ImGui_ImplDX9_CreateDeviceObjects();
	}

	void OnCreateMove(CUserCmd* pCmd)
	{
		if (g_pPlayers && Interfaces::Engine()->IsInGame())
		{
			g_pPlayers->Update();

			if (g_pEsp)
				g_pEsp->OnCreateMove(pCmd);

			if (IsLocalAlive())
			{
				if (!bIsGuiVisible)
				{
					int iWeaponSettingsSelectID = GetWeaponSettingsSelectID();

					if (iWeaponSettingsSelectID >= 0)
						iWeaponID = iWeaponSettingsSelectID;
				}

				if (g_pAimbot)
					g_pAimbot->OnCreateMove(pCmd, g_pPlayers->GetLocal());

				if (g_pTriggerbot)
					g_pTriggerbot->OnCreateMove(pCmd, g_pPlayers->GetLocal());

				if (g_pKnifebot)
					g_pKnifebot->OnCreateMove(pCmd);

				if (g_pMisc)
					g_pMisc->OnCreateMove(pCmd);
			}
		}
	}

	void OnFireEventClientSideThink(IGameEvent* pEvent)
	{
		if (!strcmp(pEvent->GetName(), "player_connect_full") ||
			!strcmp(pEvent->GetName(), "round_start") ||
			!strcmp(pEvent->GetName(), "cs_game_disconnected"))
		{
			if (g_pPlayers)
				g_pPlayers->Clear();

			if (g_pEsp)
				g_pEsp->OnReset();
		}

		if (Interfaces::Engine()->IsConnected())
		{
			if (g_pEsp)
				g_pEsp->OnEvents(pEvent);

			if (g_pSkin)
				g_pSkin->OnEvents(pEvent);
		}
	}

	void OnFrameStageNotify(ClientFrameStage_t Stage)
	{
		if (Interfaces::Engine()->IsInGame())
		{
			Skin_OnFrameStageNotify(Stage);
			Gloves_OnFrameStageNotify(Stage);
		}
	}

	void OnDrawModelExecute(IMatRenderContext* ctx, const DrawModelState_t &state,
		const ModelRenderInfo_t &pInfo, matrix3x4_t *pCustomBoneToWorld)
	{
		if (Interfaces::Engine()->IsInGame() && ctx && pCustomBoneToWorld)
		{
			if (g_pEsp)
				g_pEsp->OnDrawModelExecute(ctx, state, pInfo, pCustomBoneToWorld);

			if (g_pMisc)
				g_pMisc->OnDrawModelExecute();
		}
	}

	void OnPlaySound(const Vector* pOrigin, const char* pszSoundName)
	{
		if (!pszSoundName || !Interfaces::Engine()->IsInGame())
			return;

		if (!strstr(pszSoundName, "bulletLtoR") &&
			!strstr(pszSoundName, "rics/ric") &&
			!strstr(pszSoundName, "impact_bullet"))
		{
			if (g_pEsp && IsLocalAlive() && Settings::Esp::esp_Sound && pOrigin)
			{
				if (!GetVisibleOrigin(*pOrigin))
					g_pEsp->SoundEsp.AddSound(*pOrigin);
			}
		}
	}

	void OnPlaySound(const char* pszSoundName)
	{
		if (g_pMisc)
			g_pMisc->OnPlaySound(pszSoundName);
	}
	void buttonmenu2(int d) {
		buttonmenu = d;
	}
	void OnOverrideView(CViewSetup* pSetup)
	{
		if (g_pMisc)
			g_pMisc->OnOverrideView(pSetup);
	}

	void OnGetViewModelFOV(float& fov)
	{
		if (g_pMisc)
			g_pMisc->OnGetViewModelFOV(fov);
	}

	void OnRenderGUI()
	{
		int width = 0;
		int height = 0;
		int width1 = 0;
		int height2 = 0;
		int width2 = 0;
		int height3 = 0;
		Interfaces::Engine()->GetScreenSize(width, height);
		ImGui::GetStyle().WindowMinSize = ImVec2( 0.f , 0.f );
		if (menustyle == 0) {
			if (Animatetype == 0) {
				ImGui::SetNextWindowSize(ImVec2(720.f, 400.f));
			}
			if (Animatetype == 1) {
				ImGui::SetNextWindowSize(ImVec2(720.f, R3animate));
			}
			if (Animatetype == 2) {
				ImGui::SetNextWindowSize(ImVec2(BUDSTER1, BUDSTER2));
			}

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.86f, 0.93f, 0.89f, connva));
			ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(0.92f, 0.18f, 0.29f, connva));
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.13f, 0.14f, 0.17f, connva));
			ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImVec4(0.20f, 0.22f, 0.27f, connva));
			ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.20f, 0.22f, 0.27f, connva));
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.00f, 0.00f, 0.00f, connva));
			ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0.00f, 0.00f, 0.00f, connva));
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.20f, 0.22f, 0.27f, connva));
			ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.92f, 0.18f, 0.29f, connva));
			ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.92f, 0.18f, 0.29f, connva));
			ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.20f, 0.22f, 0.27f, connva));
			ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.20f, 0.22f, 0.27f, connva));
			ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.92f, 0.18f, 0.29f, connva));
			ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0.20f, 0.22f, 0.27f, connva));
			ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0.20f, 0.22f, 0.27f, connva));
			ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(0.09f, 0.15f, 0.16f, connva));
			ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ImVec4(0.92f, 0.18f, 0.29f, connva));
			ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, ImVec4(0.92f, 0.18f, 0.29f, connva));
			ImGui::PushStyleColor(ImGuiCol_ComboBg, ImVec4(0.20f, 0.22f, 0.27f, connva));
			ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.71f, 0.18f, 0.29f, connva));
			ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.92f, 0.18f, 0.29f, connva));
			ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.92f, 0.18f, 0.29f, connva));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.92f, 0.18f, 0.29f, connva));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.92f, 0.18f, 0.29f, connva));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.92f, 0.18f, 0.29f, connva));
			ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.92f, 0.18f, 0.29f, connva));
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.92f, 0.18f, 0.29f, connva));
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.92f, 0.18f, 0.29f, connva));
			ImGui::PushStyleColor(ImGuiCol_Column, ImVec4(0.15f, 0.00f, 0.00f, connva));
			ImGui::PushStyleColor(ImGuiCol_ColumnHovered, ImVec4(0.92f, 0.18f, 0.29f, connva));
			ImGui::PushStyleColor(ImGuiCol_ColumnActive, ImVec4(0.92f, 0.18f, 0.29f, connva));
			ImGui::PushStyleColor(ImGuiCol_ResizeGrip, ImVec4(0.92f, 0.18f, 0.29f, connva));
			ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, ImVec4(0.92f, 0.18f, 0.29f, connva));
			ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, ImVec4(0.92f, 0.18f, 0.29f, connva));
			ImGui::PushStyleColor(ImGuiCol_CloseButton, ImVec4(1.00f, 1.00f, 1.00f, connva));
			ImGui::PushStyleColor(ImGuiCol_CloseButtonHovered, ImVec4(1.00f, 1.00f, 1.00f, connva));
			ImGui::PushStyleColor(ImGuiCol_CloseButtonActive, ImVec4(1.00f, 1.00f, 1.00f, connva));
			ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.86f, 0.93f, 0.89f, connva));
			ImGui::PushStyleColor(ImGuiCol_PlotLinesHovered, ImVec4(0.92f, 0.18f, 0.29f, connva));
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.86f, 0.93f, 0.89f, connva));
			ImGui::PushStyleColor(ImGuiCol_PlotHistogramHovered, ImVec4(0.92f, 0.18f, 0.29f, connva));
			ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, ImVec4(0.92f, 0.18f, 0.29f, connva));
			ImGui::PushStyleColor(ImGuiCol_ModalWindowDarkening, ImVec4(0.20f, 0.22f, 0.27f, connva));
			if (ImGui::Begin("Tenderness Reborn|Full", &bIsGuiVisible, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
			{
				ImGui::BeginGroup();

				if (Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_FovType > 1)
					Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_FovType = 1;

				if (Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_BestHit > 1)
					Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_BestHit = 1;

				if (Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Spot > 5)
					Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Spot = 5;

				const char* tabNames[] = {
					"Aimbot" , "Visuals" , "Skins" ,
					"Misc" };

				static int tabOrder[] = { 0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 };
				static int tabSelected = 0;
				const bool tabChanged = ImGui::TabLabels(tabNames,
					sizeof(tabNames) / sizeof(tabNames[0]),
					tabSelected, tabOrder);

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				float SpaceLineOne = 120.f;
				float SpaceLineTwo = 220.f;
				float SpaceLineThr = 320.f;
				float SpaceLineFour = 420.f;

				ImGui::PushItemWidth(105.f);

				static int iConfigSelect = 0;
				static int iMenuSheme = 1;
				static char ConfigName[64] = { 0 };

				//ImGui::PopItemWidth();
				//ImGui::EndGroup();
				//ImGui::SameLine();


				//ImGui::ComboBoxArray("Select Config",&iConfigSelect, ConfigList);

				ImGui::PopItemWidth();


				ImGui::PushItemWidth(50.f);

				if (ImGui::Button("Load", ImVec2(62, 0)))
				{
					Settings::LoadSettings(BaseDir + "\\" + ConfigList[iConfigSelect]);
				}
				ImGui::SameLine();
				if (ImGui::Button("Save", ImVec2(62, 0)))
				{
					Settings::SaveSettings(BaseDir + "\\" + ConfigList[iConfigSelect]);
				}
				ImGui::PopItemWidth();
				ImGui::PushItemWidth(185.f);
				if (ImGui::Button("Uppdate List", ImVec2(130, 0)))
				{
					RefreshConfigs();
				}

				if (ImGui::Button("New Config", ImVec2(130, 0)))
				{
					string ConfigFileName = ConfigName;

					if (ConfigFileName.size() < 1)
					{
						ConfigFileName = "New";
					}

					Settings::SaveSettings(BaseDir + "\\" + ConfigFileName + ".ini");
					RefreshConfigs();
				}
				ImGui::PopItemWidth();

				ImGui::EndGroup();

				ImGui::SameLine();
				if (tabSelected == 0) // Aimbot
				{
					ImGui::BeginGroup();
					ImVec2 siz = ImVec2(185, 360 - ImGui::GetCursorPosY() - 40);
					static char* Aimbot_tab_names[] = { "Aimbot", "Trigger" };
					static int   active_Aimbot_tab = 0;

					bool placeholder_true = false;

					auto& style = ImGui::GetStyle();
					bool group_w = -style.WindowPadding.x * 2;
					ImGui::GetWindowSize(); style.WindowPadding.x * 2;
					render_tabs(Aimbot_tab_names, active_Aimbot_tab, group_w / _countof(Aimbot_tab_names), (30.f, 20.f), true);

					//ImGui::BeginGroup();
					if (active_Aimbot_tab == 0)
					{
						ImGui::PushItemWidth(110.f);
						ImGui::Text("Current Weapon: ");
						ImGui::SameLine();
						ImGui::Combo("##AimWeapon", &iWeaponID, pWeaponData, IM_ARRAYSIZE(pWeaponData));
						ImGui::PopItemWidth();

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::Checkbox("Deathmatch", &Settings::Aimbot::aim_Deathmatch);
						ImGui::SameLine(SpaceLineOne);
						ImGui::Checkbox("WallAttack", &Settings::Aimbot::aim_WallAttack);
						ImGui::SameLine(SpaceLineTwo);
						ImGui::Checkbox("CheckSmoke", &Settings::Aimbot::aim_CheckSmoke);

						ImGui::Checkbox("AntiJump", &Settings::Aimbot::aim_AntiJump);
						ImGui::Checkbox("Psilent", &Settings::Aimbot::aim_Psilent);
						ImGui::SameLine(SpaceLineOne);




						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::Checkbox("Active", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Active);

						if (iWeaponID <= 9)
						{
							ImGui::SameLine();
							ImGui::Checkbox("Autopistol", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_AutoPistol);
						}

						ImGui::PushItemWidth(362.f);
						ImGui::SliderInt("Smooth", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Smooth, 1, 180);
						ImGui::SliderInt("Fov", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Fov, 1, 180);
						ImGui::PopItemWidth();

						const char* AimFovType[] = { "Dynamic" , "Static" };
						ImGui::PushItemWidth(362.f);
						ImGui::Combo("Fov Type", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_FovType, AimFovType, IM_ARRAYSIZE(AimFovType));
						ImGui::PopItemWidth();

						const char* BestHit[] = { "Disable" , "Enable" };
						ImGui::PushItemWidth(362.f);
						ImGui::Combo("BestHit", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_BestHit, BestHit, IM_ARRAYSIZE(BestHit));

						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("if disabled then used Aimspot");

						ImGui::PopItemWidth();

						const char* Aimspot[] = { "Head" , "Neck" , "Low Neck" , "Body" , "Thorax" , "Chest" };
						ImGui::PushItemWidth(362.f);
						ImGui::Combo("Aimspot", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Spot, Aimspot, IM_ARRAYSIZE(Aimspot));
						ImGui::PopItemWidth();

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::PushItemWidth(362.f);
						ImGui::SliderInt("Delay", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Delay, 0, 100);
						ImGui::SliderInt("Rcs", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Rcs, 0, 100);


						ImGui::PopItemWidth();

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						if (iWeaponID >= 10 && iWeaponID <= 30)
						{
							ImGui::PushItemWidth(362.f);
							ImGui::SliderInt("Rcs Fov", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_RcsFov, 1, 180);
							ImGui::SliderInt("Rcs Smooth", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_RcsSmooth, 1, 180);
							ImGui::PopItemWidth();

							const char* ClampType[] = { "All Target" , "Shot" , "Shot + Target" };
							ImGui::PushItemWidth(362.f);
							ImGui::Combo("Rcs Clamp", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_RcsClampType, ClampType, IM_ARRAYSIZE(ClampType));
							ImGui::PopItemWidth();

							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Spacing();
						}
						ImGui::EndGroup();

					}

					if (active_Aimbot_tab == 1)
					{
						ImGui::Combo("Weapon", &iWeaponID, pWeaponData, IM_ARRAYSIZE(pWeaponData));

						const char* TriggerEnable[] = { "Disable" , "Fov" , "Trace" };
						ImGui::PushItemWidth(80.f);
						ImGui::Combo("Enable", &Settings::Triggerbot::trigger_Enable, TriggerEnable, IM_ARRAYSIZE(TriggerEnable));
						ImGui::PopItemWidth();
						ImGui::SameLine();

						const char* items1[] = { CVAR_KEY_MOUSE3 , CVAR_KEY_MOUSE4 , CVAR_KEY_MOUSE5 };
						ImGui::PushItemWidth(80.f);
						ImGui::Combo("Key", &Settings::Triggerbot::trigger_Key, items1, IM_ARRAYSIZE(items1));
						ImGui::PopItemWidth();
						ImGui::SameLine();

						const char* items2[] = { "Hold" , "Press" };
						ImGui::PushItemWidth(80.f);
						ImGui::Combo("Key Mode", &Settings::Triggerbot::trigger_KeyMode, items2, IM_ARRAYSIZE(items2));
						ImGui::PopItemWidth();
						ImGui::SameLine();


						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::Checkbox("Deathmatch", &Settings::Triggerbot::trigger_Deathmatch);
						ImGui::SameLine(SpaceLineOne);
						ImGui::Checkbox("WallAttack", &Settings::Triggerbot::trigger_WallAttack);
						ImGui::SameLine(SpaceLineTwo);
						ImGui::Checkbox("FastZoom", &Settings::Triggerbot::trigger_FastZoom);

						ImGui::Checkbox("SmokCheck", &Settings::Triggerbot::trigger_SmokCheck);
						ImGui::SameLine(SpaceLineOne);
						ImGui::Checkbox("DrawFov", &Settings::Triggerbot::trigger_DrawFov);

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();



						ImGui::SliderInt("Fov", &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_Fov, 1, 100);
						ImGui::SliderInt("Delay Before", &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_DelayBefore, 0, 200);
						ImGui::SliderInt("Delay After", &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_DelayAfter, 0, 1000);

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();


					}
				}
				if (tabSelected == 1) // Visuals
				{
					ImGui::BeginGroup();
					ImVec2 siz = ImVec2(185, 360 - ImGui::GetCursorPosY() - 40);
					static char* visuals_tab_names[] = { "Visuals", "Color" , "settings" };
					static int   active_visuals_tab = 0;

					bool placeholder_true = false;

					auto& style = ImGui::GetStyle();
					bool group_w = -style.WindowPadding.x * 2;
					ImGui::GetWindowSize(); style.WindowPadding.x * 2;
					render_tabs(visuals_tab_names, active_visuals_tab, group_w / _countof(visuals_tab_names), (30.f, 20.f), true);

					if (active_visuals_tab == 0)
					{
						if (ImGui::BeginChild(charenc("##esp-child"), siz, false))
						{
							ImGui::Text(charenc("Visuals"));
							//ImGui::Separator();
							string style_1 = "Box";
							string style_2 = "CoalBox";

							const char* items1[] = { style_1.c_str() , style_2.c_str() };

							ImGui::PushItemWidth(82.f);
							ImGui::Combo("Esp Type", &Settings::Esp::esp_Style, items1, IM_ARRAYSIZE(items1));
							ImGui::PopItemWidth();

							string hpbar_1 = "None";
							string hpbar_2 = "Number";
							string hpbar_3 = "Bottom";
							string hpbar_4 = "Left";

							const char* items3[] = { hpbar_1.c_str() , hpbar_2.c_str() , hpbar_3.c_str() , hpbar_4.c_str() };
							ImGui::Combo("Esp Health", &Settings::Esp::esp_Health, items3, IM_ARRAYSIZE(items3));

							ImGui::Checkbox(charenc("Skeleton"), &Settings::Esp::esp_Skeleton);

							ImGui::Checkbox(charenc("Show Enemy"), &Settings::Esp::esp_Enemy);
							ImGui::Checkbox(charenc("Show Teammates"), &Settings::Esp::esp_Team);
							ImGui::Separator();
							ImGui::Text(charenc("Misc"));
							ImGui::Checkbox(charenc("Spectator List"), &Settings::Misc::misc_Spectators);
							ImGui::Checkbox(charenc("Recoil crosshair"), &Settings::Misc::misc_Punch);
							ImGui::Checkbox(charenc("Fov Draw"), &Settings::Aimbot::aim_DrawFov);

						}ImGui::EndChild(); ImGui::SameLine();
						if (ImGui::BeginChild(charenc("##chams-child"), siz, false))
						{
							ImGui::Text(charenc("Chams"));
							//ImGui::Separator();
							string visible_1 = "Enemy";
							string visible_2 = "Team";
							string visible_3 = "all";
							string visible_4 = "Only Visible";

							const char* items2[] = { visible_1.c_str() , visible_2.c_str() , visible_3.c_str() , visible_4.c_str() };

							ImGui::PushItemWidth(82.f);
							ImGui::Combo("Chams type", &Settings::Esp::esp_Visible, items2, IM_ARRAYSIZE(items2));
							//ImGui::Separator();
							string chams_1 = "None";
							string chams_2 = "Flat";
							string chams_3 = "Texture";

							const char* items5[] = { chams_1.c_str() , chams_2.c_str() , chams_3.c_str() };
							ImGui::Combo("Chams", &Settings::Esp::esp_Chams, items5, IM_ARRAYSIZE(items5));
							//ImGui::Checkbox(charenc("Hands"), &MenuSettings.Visuals.Chams.hands);
							ImGui::Separator();
							ImGui::Text(charenc("Removals"));
							ImGui::Checkbox(charenc("Esp Sound"), &Settings::Esp::esp_Sound);
							ImGui::Checkbox(charenc("Esp World Weapon"), &Settings::Esp::esp_WorldWeapons);
							ImGui::Checkbox(charenc("Esp World Grenade"), &Settings::Esp::esp_WorldGrenade);
							ImGui::Checkbox(charenc("NoFlash"), &Settings::Misc::misc_NoFlash);

						}ImGui::EndChild(); ImGui::SameLine();
						if (ImGui::BeginChild(charenc("##info-child"), siz, false))
						{
							ImGui::Text(charenc("Information"));
							ImGui::Separator();
							ImGui::Selectable(charenc("Name"), &Settings::Esp::esp_Name);
							//ImGui::Selectable(charenc("Health"), &Settings::Esp::esp_Health);
							ImGui::Selectable(charenc("Weapon"), &Settings::Esp::esp_Weapon);
							//ImGui::Selectable(charenc("Flashed"), &Settings::Esp::esp_WorldGrenade);
							ImGui::Selectable(charenc("Distance"), &Settings::Esp::esp_Distance);
							ImGui::Selectable(charenc("Rank"), &Settings::Esp::esp_Rank);
							ImGui::Selectable(charenc("Bomb"), &Settings::Esp::esp_Bomb);
						}ImGui::EndChild(); ImGui::NewLine();
					}
					if (active_visuals_tab == 1)
					{
						ImGui::ColorEdit3("Esp  CT", Settings::Esp::esp_Color_CT);
						ImGui::ColorEdit3("Esp  TT", Settings::Esp::esp_Color_TT);
						ImGui::ColorEdit3("Esp  Visible CT", Settings::Esp::esp_Color_VCT);
						ImGui::ColorEdit3("Esp  Visible TT", Settings::Esp::esp_Color_VTT);

						ImGui::ColorEdit3("Color Rad CT", Settings::Radar::rad_Color_CT);
						ImGui::ColorEdit3("Color Rad TT", Settings::Radar::rad_Color_TT);
						ImGui::ColorEdit3("Color Rad V CT", Settings::Radar::rad_Color_VCT);
						ImGui::ColorEdit3("Color Rad V TT", Settings::Radar::rad_Color_VTT);

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::ColorEdit3("chams_CT", Settings::Esp::chams_Color_CT);
						ImGui::ColorEdit3("chams_TT", Settings::Esp::chams_Color_TT);
						ImGui::ColorEdit3("chams_VCT", Settings::Esp::chams_Color_VCT);
						ImGui::ColorEdit3("chams_VTT", Settings::Esp::chams_Color_VTT);

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();
					}
					if (active_visuals_tab == 2)
					{

						ImGui::SliderInt("Esp Size", &Settings::Esp::esp_Size, 0, 10);

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::SliderInt("Esp BombTimer", &Settings::Esp::esp_BombTimer, 0, 65);

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::ComboBoxArray("Select Config", &iConfigSelect, ConfigList);



					}
					ImGui::EndGroup();

				}
				if (tabSelected == 2) // Skins
				{
					ImGui::BeginGroup();
					//[enc_string_disable /]
					const char* knife_models_items[] =
					{
						"Default","Bayonet","Flip","Gut","Karambit" ,"M9 Bayonet",
						"Huntsman","Falchion","Bowie","Butterfly","Shadow Daggers"
					};

					const char* quality_items[] =
					{
						"Normal","Genuine","Vintage","Unusual","Community","Developer",
						"Self-Made","Customized","Strange","Completed","Tournament"
					};

					const char* gloves_listbox_items[25] =
					{
						"default",
						"bloodhound_black_silver","bloodhound_snakeskin_brass","bloodhound_metallic","handwrap_leathery",
						"handwrap_camo_grey","slick_black","slick_military","slick_red","sporty_light_blue","sporty_military",
						"handwrap_red_slaughter","motorcycle_basic_black","motorcycle_mint_triangle","motorcycle_mono_boom",
						"motorcycle_triangle_blue","specialist_ddpat_green_camo","specialist_kimono_diamonds_red",
						"specialist_emerald_web","specialist_orange_white","handwrap_fabric_orange_camo","sporty_purple",
						"sporty_green","bloodhound_guerrilla","slick_snakeskin_yellow"
					};
					//[enc_string_enable /]


					ImGui::Text("Current Weapon: %s", pWeaponData[iWeaponID]);

					ImGui::PushItemWidth(362.f);

					ImGui::Separator();

					ImGui::Combo("Knife CT Model", &Settings::Skin::knf_ct_model, knife_models_items, IM_ARRAYSIZE(knife_models_items));
					ImGui::Combo("Knife TT Model", &Settings::Skin::knf_tt_model, knife_models_items, IM_ARRAYSIZE(knife_models_items));

					ImGui::Separator();

					static int iSelectKnifeCTSkinIndex = -1;
					static int iSelectKnifeTTSkinIndex = -1;

					int iKnifeCTModelIndex = Settings::Skin::knf_ct_model;
					int iKnifeTTModelIndex = Settings::Skin::knf_tt_model;

					static int iOldKnifeCTModelIndex = -1;
					static int iOldKnifeTTModelIndex = -1;

					if (iOldKnifeCTModelIndex != iKnifeCTModelIndex && Settings::Skin::knf_ct_model)
						iSelectKnifeCTSkinIndex = GetKnifeSkinIndexFromPaintKit(Settings::Skin::knf_ct_skin, false);

					if (iOldKnifeTTModelIndex != iKnifeTTModelIndex && Settings::Skin::knf_tt_model)
						iSelectKnifeTTSkinIndex = GetKnifeSkinIndexFromPaintKit(Settings::Skin::knf_ct_skin, true);

					iOldKnifeCTModelIndex = iKnifeCTModelIndex;
					iOldKnifeTTModelIndex = iKnifeTTModelIndex;

					string KnifeCTModel = knife_models_items[Settings::Skin::knf_ct_model];
					string KnifeTTModel = knife_models_items[Settings::Skin::knf_tt_model];

					KnifeCTModel += " Skin##KCT";
					KnifeTTModel += " Skin##KTT";

					ImGui::SliderFloat("Knife CT Wear", &g_SkinChangerCfg[WEAPON_KNIFE].flFallbackWear, 0.001f, 1.f);
					ImGui::Combo("Knife CT Qality", &g_SkinChangerCfg[WEAPON_KNIFE].iEntityQuality, quality_items, IM_ARRAYSIZE(quality_items));
					ImGui::ComboBoxArray(KnifeCTModel.c_str(), &iSelectKnifeCTSkinIndex, KnifeSkins[iKnifeCTModelIndex].SkinNames);

					ImGui::Separator();

					ImGui::SliderFloat("Knife TT Wear", &g_SkinChangerCfg[WEAPON_KNIFE_T].flFallbackWear, 0.001f, 1.f);
					ImGui::Combo("Knife TT Qality", &g_SkinChangerCfg[WEAPON_KNIFE_T].iEntityQuality, quality_items, IM_ARRAYSIZE(quality_items));
					ImGui::ComboBoxArray(KnifeTTModel.c_str(), &iSelectKnifeTTSkinIndex, KnifeSkins[iKnifeTTModelIndex].SkinNames);

					ImGui::Separator();

					static int iOldWeaponID = -1;

					ImGui::Combo("Weapon##WeaponSelect", &iWeaponID, pWeaponData, IM_ARRAYSIZE(pWeaponData));

					iWeaponSelectIndex = pWeaponItemIndexData[iWeaponID];

					if (iOldWeaponID != iWeaponID)
						iWeaponSelectSkinIndex = GetWeaponSkinIndexFromPaintKit(g_SkinChangerCfg[iWeaponSelectIndex].nFallbackPaintKit);

					iOldWeaponID = iWeaponID;

					string WeaponSkin = pWeaponData[iWeaponID];
					WeaponSkin += " Skin";

					ImGui::ComboBoxArray(WeaponSkin.c_str(), &iWeaponSelectSkinIndex, WeaponSkins[iWeaponID].SkinNames);

					ImGui::Combo("Weapon Qality", &g_SkinChangerCfg[pWeaponItemIndexData[iWeaponID]].iEntityQuality, quality_items, IM_ARRAYSIZE(quality_items));
					ImGui::SliderFloat("Weapon Wear", &g_SkinChangerCfg[pWeaponItemIndexData[iWeaponID]].flFallbackWear, 0.001f, 1.f);
					ImGui::InputInt("Weapon StatTrak", &g_SkinChangerCfg[pWeaponItemIndexData[iWeaponID]].nFallbackStatTrak, 1, 100, ImGuiInputTextFlags_CharsDecimal);

					ImGui::Separator();

					ImGui::Combo("Gloves Skin", &Settings::Skin::gloves_skin, gloves_listbox_items,
						IM_ARRAYSIZE(gloves_listbox_items));

					ImGui::Separator();

					ImGui::PopItemWidth();

					if (ImGui::Button("Apply##Skin"))
					{
						if (iWeaponSelectSkinIndex >= 0) {
							g_SkinChangerCfg[iWeaponSelectIndex].nFallbackPaintKit = WeaponSkins[iWeaponID].SkinPaintKit[iWeaponSelectSkinIndex];
						}
						if (iSelectKnifeCTSkinIndex >= 0) {
							Settings::Skin::knf_ct_skin = KnifeSkins[iKnifeCTModelIndex].SkinPaintKit[iSelectKnifeCTSkinIndex];
						}
						if (iSelectKnifeTTSkinIndex >= 0) {
							Settings::Skin::knf_tt_skin = KnifeSkins[iKnifeTTModelIndex].SkinPaintKit[iSelectKnifeTTSkinIndex];
						}

						ForceFullUpdate();
					}
					ImGui::EndGroup();
				}
				if (tabSelected == 3) // Misc
				{
					ImGui::BeginGroup();
					static char* misc_tab_names[] = { "Misc", "Radar", "Knifebot" };
					static int   active_misc_tab = 0;

					bool placeholder_true = false;

					auto& style = ImGui::GetStyle();
					bool group_w = -style.WindowPadding.x * 2;
					ImGui::GetWindowSize(); style.WindowPadding.x * 2;
					render_tabs(misc_tab_names, active_misc_tab, group_w / _countof(misc_tab_names), 20.0f, true);

					if (active_misc_tab == 0)
					{
						ImGui::Checkbox("Bhop", &Settings::Misc::misc_Bhop);
						ImGui::Checkbox("Auto Strafe", &Settings::Misc::misc_AutoStrafe);
						ImGui::Checkbox("Auto Accept", &Settings::Misc::misc_AutoAccept);
						ImGui::Checkbox("Fov Changer", &Settings::Misc::misc_FovChanger);
						ImGui::PushItemWidth(362.f);
						//ImGui::SliderInt("Fov View", &Settings::Misc::misc_FovView, 1, 190);
						ImGui::SliderInt("Fov Model View", &Settings::Misc::misc_FovModelView, 1, 190);

						ImGui::Separator();
						const char* type[] = { "transparency", "R3", "BUDSTER" };
						ImGui::Combo("Animate type", &Animatetype, type, IM_ARRAYSIZE(type));

						if (Animatetype == 0) {
							ImGui::SliderInt("Sleep Animate", &sleepanimate, 1, 100);
							ImGui::SliderFloat("periodicity Animate", &periodicity1, 0.01f, 0.180f);
						}
						if (Animatetype == 1) {
							ImGui::SliderInt("Sleep Animate", &sleepanimate2, 1, 100);
							ImGui::SliderInt("periodicity Animate", &periodicity, 1, 50);
						}
						if (Animatetype == 2) {
							ImGui::SliderInt("Sleep Animate", &sleepanimate3, 1, 100);
							ImGui::SliderInt("periodicity Animate", &periodicity2, 1, 50);
						}

						ImGui::Separator();

					}

					if (active_misc_tab == 1)
					{
						ImGui::Checkbox("Active", &Settings::Radar::rad_Active);
						ImGui::SameLine();
						ImGui::Checkbox("Team", &Settings::Radar::rad_Team);
						ImGui::SameLine();
						ImGui::Checkbox("Enemy", &Settings::Radar::rad_Enemy);
						ImGui::SameLine();
						ImGui::Checkbox("InGame", &Settings::Radar::rad_InGame);

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::PushItemWidth(339.f);
						ImGui::SliderInt("Range", &Settings::Radar::rad_Range, 1, 5000);
						ImGui::SliderInt("Alpha", &Settings::Radar::rad_Alpha, 1, 255);

					}

					if (active_misc_tab == 2)
					{
						ImGui::Checkbox("Active", &Settings::Knifebot::knf_Active);
						ImGui::Checkbox("Attack Team", &Settings::Knifebot::knf_Team);

						ImGui::Separator();

						string attack_1 = "Attack 1";
						string attack_2 = "Attack 2";
						string attack_3 = "Attack 1 + Attack 2";

						const char* items[] = { attack_1.c_str() , attack_2.c_str() , attack_3.c_str() };
						ImGui::Combo("Attack type", &Settings::Knifebot::knf_Attack, items, IM_ARRAYSIZE(items));

						ImGui::Separator();

						ImGui::SliderInt("Dist to attack 1", &Settings::Knifebot::knf_DistAttack, 1, 100);
						ImGui::SliderInt("Dist to attack 2", &Settings::Knifebot::knf_DistAttack2, 1, 100);
					}
					ImGui::EndGroup();

				}

				ImGui::End();
			}

			if (Animatetype == 0) {
				if (connva < 1.00f) {
					Sleep(sleepanimate);
					connva = connva + periodicity1;
				}
			}
			if (Animatetype == 1) {
				if (R3animate < 400.f) {
					Sleep(sleepanimate2);
					R3animate = R3animate + periodicity;
				}
			}
			if (Animatetype == 2) {
				if (BUDSTER1 < 720.00f)
				{
					Sleep(sleepanimate3);
					BUDSTER1 = BUDSTER1 + periodicity2;
				}
				if (BUDSTER1 >= 720.00f) {
					if (BUDSTER2 < 400.00f) {
						Sleep(sleepanimate3);
						BUDSTER2 = BUDSTER2 + periodicity2;
					}
				}
			}
		}
		if (menustyle == 1) {
			if (connva < 1.f) {
				R3animate = 0.f;
				Sleep(sleepanimate);
				connva = connva + periodicity1;
			}
			if (connva < 1.00f) {
				Sleep(sleepanimate);
				connva = connva + periodicity1;
			}

			if (connva >= 1.00f) {
				if (R3animate < 264.f) {
					Sleep(sleepanimate2);
					R3animate = R3animate + periodicity;
				}
			}
			if (buttonmenu > 0) {
				
				if (penis < 675) {
					Sleep(sleepanimate2);
					penis = penis + periodicity;
				}
			}
			if (buttonmenu == 0) {
				penis = 180;
			}
			width1 = (width / 2);
			height2 = (height / 2);

			int width2 = width1 - 90;
			int height3 = height2 - 152;
			if (buttonmenu > 0) {
				if (penis >= 675) {
					pizda = 247;
				}
			}

			ImGui::SetNextWindowSize(ImVec2(penis, 40.f));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.17f, 0.19f, 0.17f, connva));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.17f, 0.19f, 0.17f, connva));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.17f, 0.19f, 0.17f, connva));
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.17f, 0.19f, 0.17f, connva));
			ImGui::SetNextWindowPos(ImVec2(width2 - pizda, height3));
			if (ImGui::Begin("RXCHEATS", &bIsGuiVisible, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar)) {
				ImGui::Button("YouGame.biz", ImVec2(180.f, 40.f));
			}
			ImGui::PopStyleColor();
			//ImGui::SetNextWindowSize(ImVec2(180.f, 264.f));
			ImGui::SetNextWindowSize(ImVec2(180.f, R3animate));
			ImGui::SetNextWindowPos(ImVec2(width2 - pizda, height3 + 40));			
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(3, 0));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.10f, 0.08f, 1.f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.17f, 0.19f, 0.17f, 1.f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.17f, 0.19f, 0.17f, 1.f));
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.17f, 0.19f, 0.17f, 1.f));
			if (ImGui::Begin("RXCHEATSfj1dfg", &bIsGuiVisible, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar)) {
				if (ImGui::Button("AIMBOT", ImVec2(174, 30))) {//1
					buttonmenu2(1);
				}
				if (ImGui::Button("TRIGGERBOT", ImVec2(174, 30))) {//2
					buttonmenu2(2);
				}
				if (ImGui::Button("CHANGER", ImVec2(174, 30))) {//3
					buttonmenu2(3);
				}
				if (ImGui::Button("KnifeBot", ImVec2(174, 30))) {//4
					buttonmenu2(4);
				}
				if (ImGui::Button("CHAMS", ImVec2(174, 30))) {//5
					buttonmenu2(5);
				}
				if (ImGui::Button("VISUALS", ImVec2(174, 30))) {//6
					buttonmenu2(6);
				}
				if (ImGui::Button("RADAR", ImVec2(174, 30))) {//7
					buttonmenu2(7);
				}
				if (ImGui::Button("MISC", ImVec2(174, 30))) {//8
					buttonmenu2(8);
				}
			}
			if (buttonmenu > 0) {
				
				ImGui::SetNextWindowPos(ImVec2((width2 + 185) - pizda, height3 + 45));
				ImGui::SetNextWindowSize(ImVec2(490.f, 259.f));
				if (ImGui::Begin("RXCHEAT‡ÔÔfgskjdfsdf", &bIsGuiVisible, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar)) {
					if (1 == 1) {

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						float SpaceLineOne = 120.f;
						float SpaceLineTwo = 220.f;
						float SpaceLineThr = 320.f;
						float SpaceLineFour = 420.f;

						ImGui::PushItemWidth(105.f);

						ImGui::BeginGroup();
						if (buttonmenu == 1) {
							ImGui::PushItemWidth(110.f);
							ImGui::Text("Current Weapon: ");
							ImGui::SameLine();
							ImGui::Combo("##AimWeapon", &iWeaponID, pWeaponData, IM_ARRAYSIZE(pWeaponData));
							ImGui::PopItemWidth();

							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Spacing();

							ImGui::Checkbox("Deathmatch", &Settings::Aimbot::aim_Deathmatch);
							ImGui::SameLine(SpaceLineOne);
							ImGui::Checkbox("WallAttack", &Settings::Aimbot::aim_WallAttack);
							ImGui::SameLine(SpaceLineTwo);
							ImGui::Checkbox("CheckSmoke", &Settings::Aimbot::aim_CheckSmoke);

							ImGui::Checkbox("AntiJump", &Settings::Aimbot::aim_AntiJump);
							ImGui::Checkbox("Psilent", &Settings::Aimbot::aim_Psilent);
							ImGui::SameLine(SpaceLineOne);




							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Spacing();

							ImGui::Checkbox("Active", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Active);

							if (iWeaponID <= 9)
							{
								ImGui::SameLine();
								ImGui::Checkbox("Autopistol", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_AutoPistol);
							}

							ImGui::PushItemWidth(362.f);
							ImGui::SliderInt("Smooth", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Smooth, 1, 180);
							ImGui::SliderInt("Fov", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Fov, 1, 180);
							ImGui::PopItemWidth();

							const char* AimFovType[] = { "Dynamic" , "Static" };
							ImGui::PushItemWidth(362.f);
							ImGui::Combo("Fov Type", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_FovType, AimFovType, IM_ARRAYSIZE(AimFovType));
							ImGui::PopItemWidth();

							const char* BestHit[] = { "Disable" , "Enable" };
							ImGui::PushItemWidth(362.f);
							ImGui::Combo("BestHit", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_BestHit, BestHit, IM_ARRAYSIZE(BestHit));

							if (ImGui::IsItemHovered())
								ImGui::SetTooltip("if disabled then used Aimspot");

							ImGui::PopItemWidth();

							const char* Aimspot[] = { "Head" , "Neck" , "Low Neck" , "Body" , "Thorax" , "Chest" };
							ImGui::PushItemWidth(362.f);
							ImGui::Combo("Aimspot", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Spot, Aimspot, IM_ARRAYSIZE(Aimspot));
							ImGui::PopItemWidth();

							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Spacing();

							ImGui::PushItemWidth(362.f);
							ImGui::SliderInt("Delay", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Delay, 0, 100);
							ImGui::SliderInt("Rcs", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_Rcs, 0, 100);


							ImGui::PopItemWidth();

							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Spacing();

							if (iWeaponID >= 10 && iWeaponID <= 30)
							{
								ImGui::PushItemWidth(362.f);
								ImGui::SliderInt("Rcs Fov", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_RcsFov, 1, 180);
								ImGui::SliderInt("Rcs Smooth", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_RcsSmooth, 1, 180);
								ImGui::PopItemWidth();

								const char* ClampType[] = { "All Target" , "Shot" , "Shot + Target" };
								ImGui::PushItemWidth(362.f);
								ImGui::Combo("Rcs Clamp", &Settings::Aimbot::weapon_aim_settings[iWeaponID].aim_RcsClampType, ClampType, IM_ARRAYSIZE(ClampType));
								ImGui::PopItemWidth();

								ImGui::Spacing();
								ImGui::Separator();
								ImGui::Spacing();
							}
							ImGui::EndGroup();
						}
						if (buttonmenu == 2) {
							ImGui::Combo("Weapon", &iWeaponID, pWeaponData, IM_ARRAYSIZE(pWeaponData));

							const char* TriggerEnable[] = { "Disable" , "Fov" , "Trace" };
							ImGui::PushItemWidth(80.f);
							ImGui::Combo("Enable", &Settings::Triggerbot::trigger_Enable, TriggerEnable, IM_ARRAYSIZE(TriggerEnable));
							ImGui::PopItemWidth();
							ImGui::SameLine();

							const char* items1[] = { CVAR_KEY_MOUSE3 , CVAR_KEY_MOUSE4 , CVAR_KEY_MOUSE5 };
							ImGui::PushItemWidth(80.f);
							ImGui::Combo("Key", &Settings::Triggerbot::trigger_Key, items1, IM_ARRAYSIZE(items1));
							ImGui::PopItemWidth();
							ImGui::SameLine();

							const char* items2[] = { "Hold" , "Press" };
							ImGui::PushItemWidth(80.f);
							ImGui::Combo("Key Mode", &Settings::Triggerbot::trigger_KeyMode, items2, IM_ARRAYSIZE(items2));
							ImGui::PopItemWidth();
							ImGui::SameLine();


							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Spacing();

							ImGui::Checkbox("Deathmatch", &Settings::Triggerbot::trigger_Deathmatch);
							ImGui::SameLine(SpaceLineOne);
							ImGui::Checkbox("WallAttack", &Settings::Triggerbot::trigger_WallAttack);
							ImGui::SameLine(SpaceLineTwo);
							ImGui::Checkbox("FastZoom", &Settings::Triggerbot::trigger_FastZoom);

							ImGui::Checkbox("SmokCheck", &Settings::Triggerbot::trigger_SmokCheck);
							ImGui::SameLine(SpaceLineOne);
							ImGui::Checkbox("DrawFov", &Settings::Triggerbot::trigger_DrawFov);

							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Spacing();



							ImGui::SliderInt("Fov", &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_Fov, 1, 100);
							ImGui::SliderInt("Delay Before", &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_DelayBefore, 0, 200);
							ImGui::SliderInt("Delay After", &Settings::Triggerbot::weapon_trigger_settings[iWeaponID].trigger_DelayAfter, 0, 1000);

							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Spacing();
						}
						if (buttonmenu == 3) {
							ImGui::BeginGroup();
							//[enc_string_disable /]
							const char* knife_models_items[] =
							{
								"Default","Bayonet","Flip","Gut","Karambit" ,"M9 Bayonet",
								"Huntsman","Falchion","Bowie","Butterfly","Shadow Daggers"
							};

							const char* quality_items[] =
							{
								"Normal","Genuine","Vintage","Unusual","Community","Developer",
								"Self-Made","Customized","Strange","Completed","Tournament"
							};

							const char* gloves_listbox_items[25] =
							{
								"default",
								"bloodhound_black_silver","bloodhound_snakeskin_brass","bloodhound_metallic","handwrap_leathery",
								"handwrap_camo_grey","slick_black","slick_military","slick_red","sporty_light_blue","sporty_military",
								"handwrap_red_slaughter","motorcycle_basic_black","motorcycle_mint_triangle","motorcycle_mono_boom",
								"motorcycle_triangle_blue","specialist_ddpat_green_camo","specialist_kimono_diamonds_red",
								"specialist_emerald_web","specialist_orange_white","handwrap_fabric_orange_camo","sporty_purple",
								"sporty_green","bloodhound_guerrilla","slick_snakeskin_yellow"
							};
							//[enc_string_enable /]


							ImGui::Text("Current Weapon: %s", pWeaponData[iWeaponID]);

							ImGui::PushItemWidth(362.f);

							ImGui::Separator();

							ImGui::Combo("Knife CT Model", &Settings::Skin::knf_ct_model, knife_models_items, IM_ARRAYSIZE(knife_models_items));
							ImGui::Combo("Knife TT Model", &Settings::Skin::knf_tt_model, knife_models_items, IM_ARRAYSIZE(knife_models_items));

							ImGui::Separator();

							static int iSelectKnifeCTSkinIndex = -1;
							static int iSelectKnifeTTSkinIndex = -1;

							int iKnifeCTModelIndex = Settings::Skin::knf_ct_model;
							int iKnifeTTModelIndex = Settings::Skin::knf_tt_model;

							static int iOldKnifeCTModelIndex = -1;
							static int iOldKnifeTTModelIndex = -1;

							if (iOldKnifeCTModelIndex != iKnifeCTModelIndex && Settings::Skin::knf_ct_model)
								iSelectKnifeCTSkinIndex = GetKnifeSkinIndexFromPaintKit(Settings::Skin::knf_ct_skin, false);

							if (iOldKnifeTTModelIndex != iKnifeTTModelIndex && Settings::Skin::knf_tt_model)
								iSelectKnifeTTSkinIndex = GetKnifeSkinIndexFromPaintKit(Settings::Skin::knf_ct_skin, true);

							iOldKnifeCTModelIndex = iKnifeCTModelIndex;
							iOldKnifeTTModelIndex = iKnifeTTModelIndex;

							string KnifeCTModel = knife_models_items[Settings::Skin::knf_ct_model];
							string KnifeTTModel = knife_models_items[Settings::Skin::knf_tt_model];

							KnifeCTModel += " Skin##KCT";
							KnifeTTModel += " Skin##KTT";

							ImGui::SliderFloat("Knife CT Wear", &g_SkinChangerCfg[WEAPON_KNIFE].flFallbackWear, 0.001f, 1.f);
							ImGui::Combo("Knife CT Qality", &g_SkinChangerCfg[WEAPON_KNIFE].iEntityQuality, quality_items, IM_ARRAYSIZE(quality_items));
							ImGui::ComboBoxArray(KnifeCTModel.c_str(), &iSelectKnifeCTSkinIndex, KnifeSkins[iKnifeCTModelIndex].SkinNames);

							ImGui::Separator();

							ImGui::SliderFloat("Knife TT Wear", &g_SkinChangerCfg[WEAPON_KNIFE_T].flFallbackWear, 0.001f, 1.f);
							ImGui::Combo("Knife TT Qality", &g_SkinChangerCfg[WEAPON_KNIFE_T].iEntityQuality, quality_items, IM_ARRAYSIZE(quality_items));
							ImGui::ComboBoxArray(KnifeTTModel.c_str(), &iSelectKnifeTTSkinIndex, KnifeSkins[iKnifeTTModelIndex].SkinNames);

							ImGui::Separator();

							static int iOldWeaponID = -1;

							ImGui::Combo("Weapon##WeaponSelect", &iWeaponID, pWeaponData, IM_ARRAYSIZE(pWeaponData));

							iWeaponSelectIndex = pWeaponItemIndexData[iWeaponID];

							if (iOldWeaponID != iWeaponID)
								iWeaponSelectSkinIndex = GetWeaponSkinIndexFromPaintKit(g_SkinChangerCfg[iWeaponSelectIndex].nFallbackPaintKit);

							iOldWeaponID = iWeaponID;

							string WeaponSkin = pWeaponData[iWeaponID];
							WeaponSkin += " Skin";

							ImGui::ComboBoxArray(WeaponSkin.c_str(), &iWeaponSelectSkinIndex, WeaponSkins[iWeaponID].SkinNames);

							ImGui::Combo("Weapon Qality", &g_SkinChangerCfg[pWeaponItemIndexData[iWeaponID]].iEntityQuality, quality_items, IM_ARRAYSIZE(quality_items));
							ImGui::SliderFloat("Weapon Wear", &g_SkinChangerCfg[pWeaponItemIndexData[iWeaponID]].flFallbackWear, 0.001f, 1.f);
							ImGui::InputInt("Weapon StatTrak", &g_SkinChangerCfg[pWeaponItemIndexData[iWeaponID]].nFallbackStatTrak, 1, 100, ImGuiInputTextFlags_CharsDecimal);

							ImGui::Separator();

							ImGui::Combo("Gloves Skin", &Settings::Skin::gloves_skin, gloves_listbox_items,
								IM_ARRAYSIZE(gloves_listbox_items));

							ImGui::Separator();

							ImGui::PopItemWidth();

							if (ImGui::Button("Apply##Skin"))
							{
								if (iWeaponSelectSkinIndex >= 0) {
									g_SkinChangerCfg[iWeaponSelectIndex].nFallbackPaintKit = WeaponSkins[iWeaponID].SkinPaintKit[iWeaponSelectSkinIndex];
								}
								if (iSelectKnifeCTSkinIndex >= 0) {
									Settings::Skin::knf_ct_skin = KnifeSkins[iKnifeCTModelIndex].SkinPaintKit[iSelectKnifeCTSkinIndex];
								}
								if (iSelectKnifeTTSkinIndex >= 0) {
									Settings::Skin::knf_tt_skin = KnifeSkins[iKnifeTTModelIndex].SkinPaintKit[iSelectKnifeTTSkinIndex];
								}

								ForceFullUpdate();
							}
							ImGui::EndGroup();
						}
						if (buttonmenu == 4) {
							ImGui::Checkbox("Active", &Settings::Knifebot::knf_Active);
							ImGui::Checkbox("Attack Team", &Settings::Knifebot::knf_Team);

							ImGui::Separator();

							string attack_1 = "Attack 1";
							string attack_2 = "Attack 2";
							string attack_3 = "Attack 1 + Attack 2";

							const char* items[] = { attack_1.c_str() , attack_2.c_str() , attack_3.c_str() };
							ImGui::Combo("Attack type", &Settings::Knifebot::knf_Attack, items, IM_ARRAYSIZE(items));

							ImGui::Separator();

							ImGui::SliderInt("Dist to attack 1", &Settings::Knifebot::knf_DistAttack, 1, 100);
							ImGui::SliderInt("Dist to attack 2", &Settings::Knifebot::knf_DistAttack2, 1, 100);
						}
						if (buttonmenu == 5) {
							ImGui::Text(charenc("Chams"));
							//ImGui::Separator();
							string visible_1 = "Enemy";
							string visible_2 = "Team";
							string visible_3 = "all";
							string visible_4 = "Only Visible";

							const char* items2[] = { visible_1.c_str() , visible_2.c_str() , visible_3.c_str() , visible_4.c_str() };

							ImGui::PushItemWidth(82.f);
							ImGui::Combo("Chams type", &Settings::Esp::esp_Visible, items2, IM_ARRAYSIZE(items2));
							//ImGui::Separator();
							string chams_1 = "None";
							string chams_2 = "Flat";
							string chams_3 = "Texture";

							const char* items5[] = { chams_1.c_str() , chams_2.c_str() , chams_3.c_str() };
							ImGui::Combo("Chams", &Settings::Esp::esp_Chams, items5, IM_ARRAYSIZE(items5));
							ImGui::ColorEdit3("chams_CT", Settings::Esp::chams_Color_CT);
							ImGui::ColorEdit3("chams_TT", Settings::Esp::chams_Color_TT);
							ImGui::ColorEdit3("chams_VCT", Settings::Esp::chams_Color_VCT);
							ImGui::ColorEdit3("chams_VTT", Settings::Esp::chams_Color_VTT);

						}
						if (buttonmenu == 6) {
							//ImGui::Separator();
							string style_1 = "Box";
							string style_2 = "CoalBox";

							const char* items1[] = { style_1.c_str() , style_2.c_str() };

							ImGui::PushItemWidth(82.f);
							ImGui::Combo("Esp Type", &Settings::Esp::esp_Style, items1, IM_ARRAYSIZE(items1));
							ImGui::PopItemWidth();

							string hpbar_1 = "None";
							string hpbar_2 = "Number";
							string hpbar_3 = "Bottom";
							string hpbar_4 = "Left";

							const char* items3[] = { hpbar_1.c_str() , hpbar_2.c_str() , hpbar_3.c_str() , hpbar_4.c_str() };
							ImGui::Combo("Esp Health", &Settings::Esp::esp_Health, items3, IM_ARRAYSIZE(items3));

							ImGui::Checkbox(charenc("Skeleton"), &Settings::Esp::esp_Skeleton);

							ImGui::Checkbox(charenc("Show Enemy"), &Settings::Esp::esp_Enemy);
							ImGui::Checkbox(charenc("Show Teammates"), &Settings::Esp::esp_Team);
							ImGui::Separator();
							ImGui::Text(charenc("Misc"));
							ImGui::Checkbox(charenc("Spectator List"), &Settings::Misc::misc_Spectators);
							ImGui::Checkbox(charenc("Recoil crosshair"), &Settings::Misc::misc_Punch);

							ImGui::Checkbox(charenc("Fov Draw"), &Settings::Aimbot::aim_DrawFov);
							ImGui::SliderInt("Esp Size", &Settings::Esp::esp_Size, 0, 10);

							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Spacing();

							ImGui::SliderInt("Esp BombTimer", &Settings::Esp::esp_BombTimer, 0, 65);
							ImGui::ColorEdit3("Esp  CT", Settings::Esp::esp_Color_CT);
							ImGui::ColorEdit3("Esp  TT", Settings::Esp::esp_Color_TT);
							ImGui::ColorEdit3("Esp  Visible CT", Settings::Esp::esp_Color_VCT);
							ImGui::ColorEdit3("Esp  Visible TT", Settings::Esp::esp_Color_VTT);

							ImGui::ColorEdit3("Color Rad CT", Settings::Radar::rad_Color_CT);
							ImGui::ColorEdit3("Color Rad TT", Settings::Radar::rad_Color_TT);
							ImGui::ColorEdit3("Color Rad V CT", Settings::Radar::rad_Color_VCT);
							ImGui::ColorEdit3("Color Rad V TT", Settings::Radar::rad_Color_VTT);

						}
						if (buttonmenu == 7) {
							ImGui::Checkbox("Active", &Settings::Radar::rad_Active);
							ImGui::SameLine();
							ImGui::Checkbox("Team", &Settings::Radar::rad_Team);
							ImGui::SameLine();
							ImGui::Checkbox("Enemy", &Settings::Radar::rad_Enemy);
							ImGui::SameLine();
							ImGui::Checkbox("InGame", &Settings::Radar::rad_InGame);

							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Spacing();

							ImGui::PushItemWidth(339.f);
							ImGui::SliderInt("Range", &Settings::Radar::rad_Range, 1, 5000);
							ImGui::SliderInt("Alpha", &Settings::Radar::rad_Alpha, 1, 255);

							ImGui::Checkbox("Active", &Settings::Radar::rad_Active);
							ImGui::SameLine();
							ImGui::Checkbox("Team", &Settings::Radar::rad_Team);
							ImGui::SameLine();
							ImGui::Checkbox("Enemy", &Settings::Radar::rad_Enemy);
							ImGui::SameLine();
							ImGui::Checkbox("InGame", &Settings::Radar::rad_InGame);

							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Spacing();

							ImGui::PushItemWidth(339.f);
							ImGui::SliderInt("Range", &Settings::Radar::rad_Range, 1, 5000);
							ImGui::SliderInt("Alpha", &Settings::Radar::rad_Alpha, 1, 255);


						}
						if (buttonmenu == 8) {
							ImGui::Checkbox("Bhop", &Settings::Misc::misc_Bhop);
							ImGui::Checkbox("Auto Strafe", &Settings::Misc::misc_AutoStrafe);
							ImGui::Checkbox("Auto Accept", &Settings::Misc::misc_AutoAccept);
							ImGui::Checkbox("Fov Changer", &Settings::Misc::misc_FovChanger);
							ImGui::PushItemWidth(362.f);
							
							ImGui::SliderInt("Fov Model View", &Settings::Misc::misc_FovModelView, 1, 190);

							ImGui::Separator();
							const char* type[] = { "transparency", "R3", "BUDSTER" };
							ImGui::Combo("Animate type", &Animatetype, type, IM_ARRAYSIZE(type));

							if (Animatetype == 0) {
								ImGui::SliderInt("Sleep Animate", &sleepanimate, 1, 100);
								ImGui::SliderFloat("periodicity Animate", &periodicity1, 0.01f, 0.180f);
								ImGui::SliderInt("periodicity Animate #2", &periodicity, 1, 50);
							}
							ImGui::Separator();
							ImGui::SliderInt("periodicity Animatedfsd #2", &periodicity, 1, 650);

							const char* type1[] = { "default", "RX"};
							ImGui::Combo("Menu style", &pizda, type1, IM_ARRAYSIZE(type1));				
						}

					}
				}
			}
			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
		}
}
}
