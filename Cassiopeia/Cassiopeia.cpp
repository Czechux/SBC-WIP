#include "../SDK/PluginSDK.h"
#include "../SDK/EventArgs.h"
#include "../SDK/EventHandler.h"

PLUGIN_API const char PLUGIN_PRINT_NAME[32] = "GCS Cassiopeia";
PLUGIN_API const char PLUGIN_PRINT_AUTHOR[32] = "Czechu7 Gosu7";
PLUGIN_API ChampionId PLUGIN_TARGET_CHAMP = ChampionId::Cassiopeia;

float delay;

namespace Menu
{
	IMenu* MenuInstance = nullptr;
	IMenuElement* Toggle = nullptr;

	namespace Prediction
	{
		IMenuElement* PredictionC = nullptr;
		IMenuElement* Label1 = nullptr;
		IMenuElement* Immobile = nullptr;

	}

	namespace Combo
	{
		IMenuElement* Enabled = nullptr;
		IMenuElement* DisableAA = nullptr;
		//
		IMenuElement* UseQl = nullptr;
		IMenuElement* UseQ = nullptr;
		IMenuElement* UseQH = nullptr;
		IMenuElement* UseQDash = nullptr;

		//
		IMenuElement* UseW = nullptr;
		IMenuElement* UseWl = nullptr;
		IMenuElement* UseWH = nullptr;
		//
		IMenuElement* UseE = nullptr;
		IMenuElement* UseR = nullptr;

	}



	namespace Laneclear
	{
		IMenuElement* Minmana = nullptr;
		IMenuElement* LaneClear = nullptr;
		IMenuElement* UseQ2 = nullptr;
		IMenuElement* UseW2 = nullptr;
		IMenuElement* UseE2 = nullptr;
		IMenuElement* Enabled2 = nullptr;
		IMenuElement* MinMinions = nullptr;


	}
}
namespace Spells
{
	std::shared_ptr<ISpell> Q = nullptr;
	std::shared_ptr<ISpell> W = nullptr;
	std::shared_ptr<ISpell> E = nullptr;
	std::shared_ptr<ISpell> R = nullptr;
}



int CountEnemiesInRange(const Vector& Position, const float Range)
{
	auto Enemies = g_ObjectManager->GetChampions(false);
	int Counter = 0;
	for (auto& Enemy : Enemies)
		if (Enemy->IsVisible() && Enemy->IsValidTarget() && Position.Distance(Enemy->Position()) <= Range)
			Counter++;
	return Counter;
}

int CountEnemiesInRange(const Vector& Position, const float Range, const float Delay)
{
	auto Enemies = g_ObjectManager->GetChampions(false);
	int Counter = 0;
	for (auto& Enemy : Enemies)
	{
		if (Enemy->IsVisible() && Enemy->IsValidTarget())
		{
			auto Out = g_Common->GetPrediction(Enemy, Delay);
			if (Out.Hitchance >= HitChance::Low && Position.Distance(Out.UnitPosition) < Range)
				Counter++;
		}
	}
	return Counter;
}


int CountKillableEnemiesInRange(const Vector& Position, const float Range, const float Delay)
{
	auto Enemies = g_ObjectManager->GetChampions(false);
	int Counter = 0;
	for (auto& Enemy : Enemies)
	{
		auto RDmg = g_Common->GetSpellDamage(g_LocalPlayer, Enemy, SpellSlot::R, false);
		if (Enemy->IsVisible() && Enemy->IsValidTarget() && RDmg >= Enemy->RealHealth(false, true))
		{
			auto Out = g_Common->GetPrediction(Enemy, Delay);
			if (Out.Hitchance >= HitChance::Medium && Position.Distance(Out.UnitPosition) < Range)
				Counter++;
		}
	}
	return Counter;
}

std::pair<Vector, size_t> GetCircularFarmLocation(std::vector<Vector>& Positions, float Range, float Width)
{
	auto MyPos = g_LocalPlayer->Position();
	auto BestPos = Vector();
	auto BestMinionCount = size_t(0);

	Positions.erase(std::remove_if(Positions.begin(), Positions.end(),
		[&MyPos, Range, Width](Vector _p) -> bool
	{
		return _p.Distance(MyPos) > Range + Width;
	}), Positions.end());

	for (auto& Pos : Positions)
	{
		auto MinionCount = static_cast<size_t>(std::count_if(Positions.begin(), Positions.end(), [&Pos, Width](Vector& Pos2) -> bool
		{
			return Pos2.Distance(Pos) <= Width;
		}));

		if (MinionCount > BestMinionCount)
		{
			BestMinionCount = MinionCount;
			BestPos = Pos;
		}
	}

	return std::make_pair(BestPos, BestMinionCount);
}

void ImmobileCast()
{
	if (Menu::Prediction::Immobile->GetBool() && Spells::Q->IsReady())
	{
		auto Target = g_Common->GetTarget(Spells::Q->Range(), DamageType::Magical);
		if (Target && Target->IsValidTarget() && Target->IsStunned())
		{
			Spells::Q->Cast(Target, HitChance::High);
		}
		return;
	}

}




void HandleLaneClearLogic()
{
	if (g_LocalPlayer->ManaPercent() < Menu::Laneclear::Minmana->GetInt())
		return;
	auto MinMinions = Menu::Laneclear::MinMinions->GetInt();
	if (!MinMinions)
		return;

	std::vector<Vector> Positions;
	for (auto& e : g_ObjectManager->GetMinionsEnemy())
		Positions.push_back(e->Position());

	if (Spells::Q->IsReady())
	{

		auto Result = GetCircularFarmLocation(Positions, Spells::Q->Range(), Spells::W->Range());
		if (Result.second >= MinMinions)
			Spells::Q->Cast(Result.first);
	}
	else if (Spells::W->IsReady())
	{
		auto Result = GetCircularFarmLocation(Positions, Spells::Q->Range(), Spells::W->Range());
		if (Result.second >= MinMinions)
			Spells::W->Cast(Result.first);
	}

}

void HandleComboLogic()
{
	//Very High Chance
	if (Menu::Prediction::PredictionC->GetInt() == 0)
	{

		if (Menu::Combo::UseQ->GetBool() && Spells::Q->IsReady())
		{
			auto Target = g_Common->GetTarget(Spells::Q->Range(), DamageType::Magical);
			if (Target && Target->IsValidTarget())
			{

				Spells::Q->Cast(Target, HitChance::VeryHigh);

			}

		}



		if (Menu::Combo::UseQ->GetBool() && Spells::W->IsReady())
		{
			auto Target = g_Common->GetTarget(Spells::W->Range(), DamageType::Magical);
			if (Target && Target->IsValidTarget())
			{

				Spells::W->Cast(Target, HitChance::VeryHigh);
			}

		}
		/*if (Menu::Combo::UseE->GetBool() && Spells::E->IsReady())
		{
			auto Target = g_Common->GetTarget(Spells::E->Range(), DamageType::Magical);
			if (Target && Target->IsValidTarget())
			{

				Spells::E->Cast(Target);
			}

		}
		*/
	}

   //High Chance
	if (Menu::Prediction::PredictionC->GetInt() == 1)
	{

		if (Menu::Combo::UseQ->GetBool() && Spells::Q->IsReady())
		{
			auto Target = g_Common->GetTarget(Spells::Q->Range(), DamageType::Magical);
			if (Target && Target->IsValidTarget())
			{

				Spells::Q->Cast(Target, HitChance::High);
			}

		}



		if (Menu::Combo::UseQ->GetBool() && Spells::W->IsReady())
		{
			auto Target = g_Common->GetTarget(Spells::W->Range(), DamageType::Magical);
			if (Target && Target->IsValidTarget())
			{

				Spells::W->Cast(Target, HitChance::High);
			}

		}
		if (Menu::Combo::UseE->GetBool() && Spells::E->IsReady())
		{
			auto Target = g_Common->GetTarget(Spells::E->Range(), DamageType::Magical);
			if (Target && Target->IsValidTarget())
			{

				Spells::E->Cast(Target);
			}

		}
	}
	//Medium Chance
	if (Menu::Prediction::PredictionC->GetInt() == 2)
	{

		if (Menu::Combo::UseQ->GetBool() && Spells::Q->IsReady())
		{
			auto Target = g_Common->GetTarget(Spells::Q->Range(), DamageType::Magical);
			if (Target && Target->IsValidTarget())
			{

				Spells::Q->Cast(Target, HitChance::Medium);
			}

		}



		if (Menu::Combo::UseQ->GetBool() && Spells::W->IsReady())
		{
			auto Target = g_Common->GetTarget(Spells::W->Range(), DamageType::Magical);
			if (Target && Target->IsValidTarget())
			{

				Spells::W->Cast(Target, HitChance::Medium);
			}

		}
		if (Menu::Combo::UseE->GetBool() && Spells::E->IsReady())
		{
			auto Target = g_Common->GetTarget(Spells::E->Range(), DamageType::Magical);
			if (Target && Target->IsValidTarget())
			{

				Spells::E->Cast(Target);
			}

		}
	}
	//Low Chance
	if (Menu::Prediction::PredictionC->GetInt() == 3)
	{

		if (Menu::Combo::UseQ->GetBool() && Spells::Q->IsReady())
		{
			auto Target = g_Common->GetTarget(Spells::Q->Range(), DamageType::Magical);
			if (Target && Target->IsValidTarget())
			{

				Spells::Q->Cast(Target, HitChance::Low);
				auto const PrintMsg = "Low";
				g_Common->Log(PrintMsg);
			}

		}



		if (Menu::Combo::UseQ->GetBool() && Spells::W->IsReady())
		{
			auto Target = g_Common->GetTarget(Spells::W->Range(), DamageType::Magical);
			if (Target && Target->IsValidTarget())
			{

				Spells::W->Cast(Target, HitChance::Low);
			}

		}
		if (Menu::Combo::UseE->GetBool() && Spells::E->IsReady())
		{
			auto Target = g_Common->GetTarget(Spells::E->Range(), DamageType::Magical);
			if (Target && Target->IsValidTarget())
			{

				Spells::E->Cast(Target);
			}

		}
	}
}


void OnGameUpdate()
{
	if (!Menu::Toggle->GetBool() || g_LocalPlayer->IsDead())
		return;



	if (Menu::Combo::Enabled->GetBool() && g_Orbwalker->IsModeActive(eOrbwalkingMode::kModeCombo))
		HandleComboLogic();

	if (Menu::Laneclear::Enabled2->GetBool() && g_Orbwalker->IsModeActive(eOrbwalkingMode::kModeLaneClear))
		HandleLaneClearLogic();


	   ImmobileCast();

}

PLUGIN_API bool OnLoadSDK(IPluginsSDK* plugin_sdk)
{
	DECLARE_GLOBALS(plugin_sdk);

	if (g_LocalPlayer->ChampionId() != ChampionId::Cassiopeia)
		return false;

	using namespace Menu;
	MenuInstance = g_Menu->CreateMenu("GCS Cassiopeia", "Cassiopeia");
	Toggle = MenuInstance->AddCheckBox("Enabled", "global_toggle", true);
	//prediction
	const auto PredictionSubMenu = MenuInstance->AddSubMenu("Prediction Settings", "Prediction");
	Menu::Prediction::PredictionC = PredictionSubMenu->AddComboBox("Prediction", "Pred", std::vector<std::string>{"Hitchance :: Very High", "Hitchance :: High", "Hitchance :: Medium", "Hitchance :: Low"}, 1);
	Menu::Prediction::Label1 = PredictionSubMenu->AddLabel("Focusing immobile target when he is stunned(Auto Cast)", "PredLabelQ", true);
	Menu::Prediction::Immobile = PredictionSubMenu->AddCheckBox("Cast Q on immobile target", "immobile", false);


	//combo
	const auto ComboSubMenu = MenuInstance->AddSubMenu("General/Combo Settings", "Combomenu");
	Menu::Combo::Enabled = ComboSubMenu->AddCheckBox("Toggle combo?", "ComboON", true);
	Menu::Combo::DisableAA = ComboSubMenu->AddCheckBox("Disable AutoAttack", "DisableAA", true);
	Menu::Combo::UseQl = ComboSubMenu->AddLabel("Spell Q Settings", "ComboLabelQ", true);
	Menu::Combo::UseQ = ComboSubMenu->AddCheckBox("Use Q", "UseQ", true);
	Menu::Combo::UseQH = ComboSubMenu->AddCheckBox("Use Q in harass", "UseQH", true);
	Menu::Combo::UseQDash = ComboSubMenu->AddCheckBox("Use Q at dash end position", "UseQdash", true);

	//
	Menu::Combo::UseWl = ComboSubMenu->AddLabel("Spell W Settings", "ComboLabelW", true);
	Menu::Combo::UseW = ComboSubMenu->AddCheckBox("Use W", "UseW", true);
	Menu::Combo::UseWH = ComboSubMenu->AddCheckBox("Use W in harras", "UseWH", true);
	//
	Menu::Combo::UseE = ComboSubMenu->AddCheckBox("Use E", "UseE", true);
	//
	Menu::Combo::UseR = ComboSubMenu->AddCheckBox("Use R", "UseR", true);





	//laneclear
	const auto LaneclearSubMenu = MenuInstance->AddSubMenu("Laneclear Settings", "Laneclear");
	Menu::Laneclear::Enabled2 = LaneclearSubMenu->AddCheckBox("Enabled", "lane_clear_toggle", true);
	Menu::Laneclear::UseQ2 = LaneclearSubMenu->AddCheckBox("Use Q", "lane_clear_use_q", true);
	Menu::Laneclear::UseW2 = LaneclearSubMenu->AddCheckBox("Use W", "lane_clear_use_w", true);
	Menu::Laneclear::UseE2 = LaneclearSubMenu->AddCheckBox("Use E", "lane_clear_use_e", true);
	Menu::Laneclear::Minmana = LaneclearSubMenu->AddSlider("Minimum mana", "LaneclearMinMana", 50, 0, 100);
	Menu::Laneclear::MinMinions = LaneclearSubMenu->AddSlider("Minimum minions", "lane_clear_min_minions", 2, 0, 9);



	/*


            ComboMenu.Add("UseQH", new CheckBox("Use [Q] in Harass"));
            ComboMenu.Add("UseS", new CheckBox("Use [Q] Mana Saver?", false)); -!
            ComboMenu.Add("UseQI", new CheckBox("Use always [Q] if enemy is immobile?")); -!
            ComboMenu.Add("UseQ2", new CheckBox("Try to hit =< 2 champions if can ?")); - !
            ComboMenu.Add("UseQPok", new CheckBox("Use always [Q] if enemy is killable by Poison?")); -!
            ComboMenu.Add("QComboDash", new CheckBox("Always use [Q] on Dash end position?")); -!
            ComboMenu.AddLabel("W Spell Settings");
            ComboMenu.Add("UseW", new CheckBox("Use [W]")); 
            ComboMenu.Add("UseWH", new CheckBox("Use [W] in Harass", false)); <
            ComboMenu.Add("UseW2", new CheckBox("Try to hit =< 2 champions if can ?")); !
            ComboMenu.AddLabel("E Spell Settings");
            ComboMenu.Add("UseE", new CheckBox("Use [E]"));
            ComboMenu.Add("UseEH", new CheckBox("Use [E] in Harass"));
            ComboMenu.Add("UseES", new CheckBox("Use [E] casting speedup ? (animation cancel)"));
            ComboMenu.Add("UseEK", new CheckBox("Use [E] always if enemy is killable?"));
            ComboMenu.AddLabel("R Spell Settings");
            ComboMenu.Add("UseR", new CheckBox("Use [R]"));
            ComboMenu.Add("UseRFace", new CheckBox("Use [R] only on facing enemy ?"));
            ComboMenu.Add("RGapClose", new CheckBox("Try use [R] for Gapclosing enemy ?", false));
            ComboMenu.Add("Rint", new CheckBox("Try use [R] for interrupt enemy ?"));
            ComboMenu.Add("UseRG", new CheckBox("Use [R] use minimum enemys for R ?"));
            ComboMenu.Add("UseRGs", new Slider("Minimum people for R", 1, 1, 5));*/


	//spells
	Spells::Q = g_Common->AddSpell(SpellSlot::Q, 850.f);
	Spells::W = g_Common->AddSpell(SpellSlot::W, 650.f);
	Spells::E = g_Common->AddSpell(SpellSlot::E, 700.f);
	Spells::R = g_Common->AddSpell(SpellSlot::R, 800.f);

	Spells::Q->SetSkillshot(0.2f, 160.f, 20.f, kCollidesWithNothing, kSkillshotCircle);
	Spells::W->SetSkillshot(0.25f, 180.f, 1500.f, kCollidesWithNothing, kSkillshotCircle);
	Spells::R->SetSkillshot(0.5f, 210.f, 0.f, kCollidesWithMinions, kSkillshotCone);





	//events
	EventHandler<Events::GameUpdate>::AddEventHandler(OnGameUpdate);
	//


	auto const PrintMsg = "Cassiopeia";
	g_Common->Log(PrintMsg);

	char ChatBuffer[64] = { 0 };
	sprintf_s(ChatBuffer, "<font color='#00ff00'>%s</font>", PrintMsg);
	g_Common->ChatPrint(ChatBuffer);

	return true;
}


PLUGIN_API void OnUnloadSDK()
{
	//remove events
	EventHandler<Events::GameUpdate>::RemoveEventHandler(OnGameUpdate);

	//remove menu 
	Menu::MenuInstance->Remove();

	g_Common->Log("Cassipeia plugin was unloaded.");
}
