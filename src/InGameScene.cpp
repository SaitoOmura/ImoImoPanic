#define _USE_MATH_DEFINES
#include "DxLib.h"
#include "IngameScene.h"
#include "../../Input/InputManager.h"
#include "../../Input/XInputManager.h"
#include "../../File/AssetsContainer.h"
#include "../../Objects/Jagaimo.h"
#include "../../Objects/Satumaimo.h"
#include "../../Objects/Yamaimo.h"
#include "../../Objects/Potate.h"
#include "../../Objects/MoleHole.h"
#include "../../Objects/Mole.h"
#include "../../Objects/Boar.h"
#include "../../Utility/Vector.h"
#include "../../Data/HighScore.h"
#include "../../Data/DetailScore.h"
#include "../../System/ProjectConfig.h"
#include "../../Utility/ScreenLimit.h"
#include "../../Objects/ScoreEffect.h"
#include <math.h>
#include <time.h>

InGameScene::InGameScene()
	: choice(IconState::eJagaimo),
	choice_num(0),
	money(0),
	delta_second(0),
	count(0),
	rain_flag(FALSE),
	rain_anim_num(0),
	boar_caution_time(0),
	boar_spawn(),
	caution_flag(false),
	count_potato(0),
	cursor(nullptr),
	state(SceneState::start),
	water_wait(0),
	start_flag(FALSE),
	dig_flag(false),
	dig_time(0),
	icon_blinking_count(0),
	icon_blinking_color(0),
	icon_blinking_flag(FALSE),
	jagaimo_count(0),
	satumaimo_count(0),
	yamaimo_count(0),
	mole_defeat_count(0),
	boar_defeat_count(0),
	pause_flag(),
	pause_cursor(),
	result_count()
{

	//インスタンス生成
	am = ActorManager::GetInstance();
	assets = AssetsContainer::GetInstance();
	xinput = XInputManager::GetInstance();

	//画像読み込み
	div_img["icon"] = assets->ImagesReference("icon");
	div_img["number"] = assets->ImagesReference("number");
	div_img["rain"] = assets->ImagesReference("rain");
	img["background"] = assets->ImageReference("ingame_background");
	img["start"] = assets->ImageReference("start");
	img["end"] = assets->ImageReference("end");
	img["caution"] = assets->ImageReference("boar_caution");
	img["count"] = assets->ImageReference("count");
	img["key"] = assets->ImageReference("key");
	img["money"] = assets->ImageReference("money");
	img["yen"] = assets->ImageReference("en");
	img["pause_start"] = assets->ImageReference("pause_start");
	img["pause_title"] = assets->ImageReference("pause_title");
	img["pause_font"] = assets->ImageReference("pause_font");
	div_img["LBRB"] = assets->ImagesReference("LBRB");

	//BGM読み込み
	bgm["bgm"] = assets->SoundReference("ingame");
	//SE読み込み
	se["start"] = assets->SoundReference("start");
	se["move_cursor"] = assets->SoundReference("move_cursor");
	se["planting"] = assets->SoundReference("planting");
	se["give_water"] = assets->SoundReference("give_water");
	se["growth"] = assets->SoundReference("growth");
	se["dig_scoop"] = assets->SoundReference("dig_scoop");
	se["score_plus"] = assets->SoundReference("score_plus");
	se["pico"] = assets->SoundReference("pico");
	se["defeat"] = assets->SoundReference("defeat");
	se["mole_appearance"] = assets->SoundReference("mole_appearance");
	se["warning"] = assets->SoundReference("warning");
	se["wild_boar_run"] = assets->SoundReference("wild_boar_run");
	se["dig_mole"] = assets->SoundReference("dig_mole");
	se["destroy_poteto"] = assets->SoundReference("destroy_poteto");
	se["rain"] = assets->SoundReference("rain");
	se["timelimit1"] = assets->SoundReference("timelimit1");
	se["timelimit2"] = assets->SoundReference("timelimit2");
	se["end"] = assets->SoundReference("end");
	se["pause_select"] = assets->SoundReference("A");
}

InGameScene::~InGameScene()
{

}

void InGameScene::Initialize()
{
	input = InputManager::GetInstance();
	srand((unsigned int)time(NULL)); // 現在時刻でシード初期化
	//アクターの生成
	Vector2D location;
	location.x = WINDOW_SIZE_X / 2;
	location.y = WINDOW_SIZE_Y / 2;
	cursor = am->CreateObject<Cursor>(location);
	srand((unsigned int)time(NULL));
	for (int i = 0; i < 3; i++)
	{
		location.x = MOLEHOLE_POSX + i * 400;
		location.y = rand() % 451 + 70;
		am->CreateObject<MoleHole>(location);
	}
	am->CheckCreateObject();

	//変数の初期化
	money = 3000;
	count = 2;
	CountStorage();
	MoneyStorage();
	rain_flag = FALSE;
	rain_anim_num = 0;
	start_flag = FALSE;
	icon_blinking_count = 0;
	icon_blinking_color = GetColor(0,240,240);
	icon_blinking_flag = FALSE;
	jagaimo_count = 0;
	satumaimo_count = 0;
	yamaimo_count = 0;
	mole_defeat_count = 0;
	boar_defeat_count = 0;
	pause_flag = false;
}

eSceneType InGameScene::Update()
{
	if (!pause_flag)
	{
		delta_second++;
		if (boar_caution_time > 0.0)
		{
			boar_caution_time--;
		}
		if (delta_second >= 60)
		{
			count--;
			if (water_wait > 0)
			{
				water_wait--;
			}
			if (dig_time > 0)
			{
				dig_time--;
			}
			delta_second = 0;
		}
		switch (state)
		{
		case SceneState::start:
			if (count <= 0)
			{
				state = SceneState::game;
				count = 180;
			}
			if (!(start_flag))
			{
				start_flag = TRUE;
				PlaySoundMem(se.at("start"), DX_PLAYTYPE_BACK);
			}
			break;
		case SceneState::game:
			count_potato = 0;
			money_container.clear();
			draw_count.clear();
			expense.clear();
			CountStorage();
			MoneyStorage();
			if (!(rain_flag))
			{
				if (count <= 60)
				{
					rain_flag = TRUE;
					PlaySoundMem(se.at("rain"), DX_PLAYTYPE_LOOP);
				}
			}

			if ((int)delta_second % 30 == 0)
			{
				if (++rain_anim_num >= 4)
				{
					rain_anim_num = 0;
				}
			}

			if (++icon_blinking_count >= 12)
			{
				if (icon_blinking_flag)
				{
					icon_blinking_color = GetColor(0, 240, 240);
					icon_blinking_flag = FALSE;
				}
				else
				{
					icon_blinking_color = GetColor(255, 255, 30);
					icon_blinking_flag = TRUE;
				}
				icon_blinking_count = 0;
			}

			if (count == 60)
			{
				if (!(CheckSoundMem(se.at("timelimit1"))))
				{
					PlaySoundMem(se.at("timelimit1"), DX_PLAYTYPE_BACK);
				}
			}
			else if (count == 30)
			{
				if (!(CheckSoundMem(se.at("timelimit2"))))
				{
					PlaySoundMem(se.at("timelimit2"), DX_PLAYTYPE_BACK);
				}
			}

			//生成する、または消すアクター確認
			am->CheckCreateObject();
			am->CheckDestroyObject();

			actor_list = am->GetObjectsList();

			for (Actor* actor : actor_list)
			{
				if (actor->ActorTypeReference() == ePoteto)
				{
					count_potato++;
					dynamic_cast<Potate*>(actor)->GetRain(rain_flag);
				}
			}

			//アクターのアップデート
			for (Actor* actor : actor_list)
			{
				actor->Update();
				if (actor->ActorTypeReference() == ActorType::eEnemy)
				{
					EnemyMove(actor);
					EnemyHit(actor);
					if (dynamic_cast<Enemy*>(actor)->DeleteFlagReference())
					{
						if (dynamic_cast<Enemy*>(actor)->EnemyTypeReference() == EnemyType::eBoar)
						{
							if (CheckSoundMem(se.at("wild_boar_run")))
							{
								StopSoundMem(se.at("wild_boar_run"));
							}
						}
						am->DestroyObject(actor);
					}
				}
				else if (actor->ActorTypeReference() == ActorType::eEffect)
				{
					if (dynamic_cast<ScoreEffect*>(actor)->IsDead())
					{
						am->DestroyObject(actor);
					}
				}
			}

			if (count % 5 == 0 && (count != 180 && count != 0))
			{
				int r = rand() % 10 + 1;

				if (r < 5)
				{
					bool create = true;
					for (Actor* mole : actor_list)
					{
						if (mole->ActorTypeReference() == ActorType::eEnemy)
						{
							if (dynamic_cast<Enemy*>(mole)->EnemyTypeReference() == EnemyType::eMole)
							{
								create = false;
								break;
							}
						}
					}
					if (count_potato <= 0)
					{
						create = false;
					}
					int i = rand() % 3;
					for (Actor* actor : actor_list)
					{
						if (actor->ActorTypeReference() == ActorType::eHole)
						{
							if (i == 0)
							{
								if (create)
								{
									am->CreateObject<Mole>(actor->PositionReference());
									PlaySoundMem(se.at("mole_appearance"), DX_PLAYTYPE_BACK);
								}
								break;
							}
							i--;
						}
					}
				}
			}

			if (count % 8 == 0)
			{
				srand((unsigned int)time(NULL)); // 現在時刻でシード初期化
				int r = rand() % 10 + 1;

				if (r < 10)
				{
					bool create = true;
					for (Actor* boar : actor_list)
					{
						if (boar->ActorTypeReference() == ActorType::eEnemy)
						{
							if (dynamic_cast<Enemy*>(boar)->EnemyTypeReference() == EnemyType::eBoar)
							{
								create = false;
								break;
							}
						}
						if (caution_flag) create = false;
						if (create)
						{
							int i = rand() % 2;
							if (i == 0)
							{
								boar_spawn.x = WINDOW_SIZE_X;
								boar_spawn.y = rand() % 401 + SCREEN_LIMIT_UP;
								caution_flag = true;
								boar_caution_time = 120.0f;
								break;
							}
							else
							{
								boar_spawn.x = 0;
								boar_spawn.y = rand() % 401 + SCREEN_LIMIT_UP;
								caution_flag = true;
								boar_caution_time = 120.0f;
								break;
							}
						}
					}
				}
			}

			if (caution_flag)
			{
				if (boar_caution_time % 12 == 4)
				{
					PlaySoundMem(se.at("warning"), DX_PLAYTYPE_BACK);
				}
			}

			cursor->CanPlantCheck(CheckCollision(cursor->PositionReference()));

			if (xinput->GetButtonState(XINPUT_BUTTON_A) == eXInputState::ePressed)
			{
				switch (choice)
				{
				case IconState::eJagaimo:
					if (!CheckCollision(cursor->PositionReference()))
					{
						if (count_potato < 10)
						{

							am->CreateObject<ScoreEffect>(am->CreateObject<Jagaimo>(cursor->PositionReference())->PositionReference())->SetScore(-0);
							PlaySoundMem(se.at("planting"), DX_PLAYTYPE_BACK);
						}
					}
					break;
				case IconState::eSatumaimo:
					if (!CheckCollision(cursor->PositionReference()) && money >= 300)
					{
						if (count_potato < 10)
						{
							am->CreateObject<ScoreEffect>(am->CreateObject<Satumaimo>(cursor->PositionReference())->PositionReference())->SetScore(-300);
							money -= 300;
							PlaySoundMem(se.at("planting"), DX_PLAYTYPE_BACK);
						}
					}
					break;
				case IconState::eYamaimo:
					if (!CheckCollision(cursor->PositionReference()) && money >= 1000)
					{
						if (count_potato < 10)
						{
							am->CreateObject<ScoreEffect>(am->CreateObject<Yamaimo>(cursor->PositionReference())->PositionReference())->SetScore(-1000);
							money -= 1000;
							PlaySoundMem(se.at("planting"), DX_PLAYTYPE_BACK);
						}
					}
					break;
				case IconState::eWater:
					CheckTool(cursor->CollisionReference());
					PlaySoundMem(se.at("give_water"), DX_PLAYTYPE_BACK);
					break;
				case IconState::eScoop:
					CheckTool(cursor->CollisionReference());
					PlaySoundMem(se.at("dig_scoop"), DX_PLAYTYPE_BACK);
					break;
				case IconState::eHammer:
					CheckTool(cursor->CollisionReference());
					PlaySoundMem(se.at("pico"), DX_PLAYTYPE_BACK);
					break;
				default:
					break;
				}
			}

			if (xinput->GetButtonState(XINPUT_BUTTON_START) == eXInputState::ePressed)
			{
				pause_flag = true;
				PlaySoundMem(se.at("pause_select"), DX_PLAYTYPE_BACK);
			}

			if (choice_num == 0)
			{
				if (count_potato >= 10)
				{
					if (water_wait > 0)
					{
						choice_num = 4;
					}
					else
					{
						choice_num = 3;
					}
				}
			}
			if (choice_num == 1)
			{
				if (money < 300)
				{
					if (count_potato >= 10)
					{
						if (water_wait > 0)
						{
							choice_num = 4;
						}
						else
						{
							choice_num = 3;
						}
					}
					else
					{
						choice_num = 0;
					}
				}
				else if (count_potato >= 10)
				{
					if (water_wait > 0)
					{
						choice_num = 4;
					}
					else
					{
						choice_num = 3;
					}
				}
				else
				{
					choice_num = 1;
				}
			}
			if (choice_num == 2)
			{
				if (money < 1000)
				{
					if (money < 300)
					{
						if (count_potato >= 10)
						{
							if (water_wait > 0)
							{
								choice_num = 4;
							}
							else
							{
								choice_num = 3;
							}
						}
						else
						{
							choice_num = 0;
						}
					}
					else if (count_potato >= 10)
					{
						if (water_wait > 0)
						{
							choice_num = 4;
						}
						else
						{
							choice_num = 3;
						}
					}
					else
					{
						choice_num = 1;
					}
				}
				else if (count_potato >= 10)
				{
					if (water_wait > 0)
					{
						choice_num = 4;
					}
					else
					{
						choice_num = 3;
					}
				}
			}
			if (choice_num == 3)
			{
				if (water_wait > 0)
				{
					choice_num = 4;
				}
			}

			//BGM再生
			if (CheckSoundMem(bgm["bgm"]) == 0)
			{
				PlaySoundMem(bgm["bgm"], DX_PLAYTYPE_LOOP);
			}

			if (xinput->GetButtonState(XINPUT_BUTTON_RIGHT_SHOULDER) == eXInputState::ePressed)
			{
				++choice_num;
				PlaySoundMem(se.at("move_cursor"), DX_PLAYTYPE_BACK);
				if (choice_num > 5)
				{
					choice_num = 0;
				}
				if (choice_num == 0)
				{
					if (count_potato >= 10)
					{
						choice_num = 3;
					}
				}
				if (choice_num == 1)
				{
					if (money < 300)
					{
						choice_num = 2;
					}
				}
				if (choice_num == 2)
				{
					if (money < 1000)
					{
						choice_num = 3;
					}
				}
				if (choice_num == 3)
				{
					if (water_wait > 0)
					{
						choice_num = 4;
					}
				}

			}
			if (xinput->GetButtonState(XINPUT_BUTTON_LEFT_SHOULDER) == eXInputState::ePressed)
			{
				--choice_num;
				PlaySoundMem(se.at("move_cursor"), DX_PLAYTYPE_BACK);
				if (choice_num < 0)
				{
					choice_num = 5;
				}
				if (choice_num == 3)
				{
					if (water_wait > 0)
					{
						choice_num = 2;
					}
				}
				if (choice_num == 2)
				{
					if (money < 1000)
					{
						choice_num = 1;
					}
					else if (count_potato >= 10)
					{
						choice_num = 5;
					}
				}
				if (choice_num == 1)
				{
					if (money < 300)
					{
						choice_num = 0;
					}
					else if (count_potato >= 10)
					{
						choice_num = 5;
					}
				}
				if (choice_num == 0)
				{
					if (count_potato >= 10)
					{
						choice_num = 5;
					}
				}
			}

			IntToIconState();

			cursor->GetIconState(choice);

			if (caution_flag && boar_caution_time <= 0)
			{
				caution_flag = false;
				boar_caution_time = 0;
				am->CreateObject<Boar>(boar_spawn)->SetBoarSize(money);
				PlaySoundMem(se.at("wild_boar_run"), DX_PLAYTYPE_LOOP);
			}


			if (count <= 0)
			{
				PlaySoundMem(se.at("end"), DX_PLAYTYPE_BACK);
				StopSoundMem(se.at("rain"));
				StopSoundMem(se.at("wild_boar_run"));
				state = SceneState::end;
				count = 2;
			}
			break;
		case SceneState::end:
			if (count <= 0)
			{
				StopSoundMem(bgm["bgm"]);
				return eSceneType::eResult;
			}
			break;
		default:
			break;
		}
	}
	else
	{
		//上方向入力でカーソル上移動
		if (xinput->GetLstickY() >= 0.3)
		{
			if (pause_cursor > 0)
			{
				pause_cursor--;
				PlaySoundMem(se.at("move_cursor"), DX_PLAYTYPE_BACK);
			}
		}
		else if (xinput->GetButtonState(XINPUT_BUTTON_DPAD_UP) == eXInputState::ePressed)
		{
			if (pause_cursor > 0)
			{
				pause_cursor--;
				PlaySoundMem(se.at("move_cursor"), DX_PLAYTYPE_BACK);
			}
		}
		//下方向入力でカーソル下移動
		if (xinput->GetLstickY() <= -0.3)
		{
			if (pause_cursor < 1)
			{
				pause_cursor++;
				PlaySoundMem(se.at("move_cursor"), DX_PLAYTYPE_BACK);
			}
		}
		else if (xinput->GetButtonState(XINPUT_BUTTON_DPAD_DOWN) == eXInputState::ePressed)
		{
			if (pause_cursor < 1)
			{
				pause_cursor++;
				PlaySoundMem(se.at("move_cursor"), DX_PLAYTYPE_BACK);
			}
		}
		if (xinput->GetButtonState(XINPUT_BUTTON_A) == eXInputState::ePressed)
		{
			switch (pause_cursor)
			{
			case 0:
				StopSoundMem(se.at("rain"));
				StopSoundMem(se.at("wild_boar_run"));
				StopSoundMem(bgm["bgm"]);
				PlaySoundMem(se.at("pause_select"), DX_PLAYTYPE_BACK);
				return eSceneType::eTitle;
				break;
			case 1:
				pause_flag = false;
				PlaySoundMem(se.at("pause_select"), DX_PLAYTYPE_BACK);
				break;
			default:
				break;
			}
		}
		if (xinput->GetButtonState(XINPUT_BUTTON_START) == eXInputState::ePressed)
		{
			pause_flag = false;
			PlaySoundMem(se.at("pause_select"), DX_PLAYTYPE_BACK);
		}
	}
	return GetNowSceneType();
}

void InGameScene::Draw() const
{
	if (!pause_flag)
	{
		DrawGraph(0, 0, img.at("background"), true);
	}
	else
	{
		SetDrawBright(150, 150, 150);
		DrawGraph(0, 0, img.at("background"), true);
		SetDrawBright(255, 255, 255);
	}

#ifdef _DEBUG
	DrawString(10, 10, "インゲーム画面", GetColor(255, 255, 255));
	DrawString(10, 30, "SPACEキーでリザルト画面に遷移", GetColor(255, 255, 255));
	DrawString(10, 50, "Pキーで効果音", GetColor(255, 255, 255));
	DrawFormatString(1200, 10, 0xffffff, "%d", money);
#endif
	switch (state)
	{
	case SceneState::start:
		if (start_flag)
		{
			DrawRotaGraph(WINDOW_SIZE_X / 2, WINDOW_SIZE_Y / 2, 0.2, 0.0, img.at("start"), true);
		}
		break;
	case SceneState::game:
		if (!pause_flag)
		{
			for (Actor* actor : actor_list)
			{
				if (actor->ActorTypeReference() == ActorType::eHole)
				{
					actor->Draw();
				}
			}
			for (Actor* actor : actor_list)
			{
				if (actor->ActorTypeReference() == ActorType::ePoteto)
				{
					actor->Draw();
				}

			}
			for (Actor* actor : actor_list)
			{
				if (actor->ActorTypeReference() == ActorType::eEnemy)
				{
					actor->Draw();
				}
			}
			for (Actor* actor : actor_list)
			{
				if (actor->ActorTypeReference() == ActorType::eCursor)
				{
					actor->Draw();
				}
			}
			for (Actor* actor : actor_list)
			{
				if (actor->ActorTypeReference() != ActorType::ePoteto && actor->ActorTypeReference() != ActorType::eHole && actor->ActorTypeReference() != ActorType::eCursor && actor->ActorTypeReference() != ActorType::eEnemy)
				{
					actor->Draw();
				}

			}
			if (rain_flag)
			{
				DrawGraph(0, 0, div_img.at("rain")[rain_anim_num], TRUE);
			}

			DrawRotaGraph(FONT_INIT_POSX, TIME_INIT_POSY, 0.1, 0.0, img.at("count"), true);
			for (int j = 0; j < draw_count.size(); j++)
			{
				DrawRotaGraph(TIME_INIT_POSX - j * NUMBER_SIZE, TIME_INIT_POSY, 0.2, 0.0, div_img.at("number")[draw_count[j]], true);
			}
			DrawRotaGraph(MONEY_INIT_POSX + NUMBER_SIZE, TIME_INIT_POSY, 0.1, 0.0, img.at("yen"), true);
			for (int i = 0; i < money_container.size(); i++)
			{
				DrawRotaGraph(MONEY_INIT_POSX - i * NUMBER_SIZE, TIME_INIT_POSY, 0.2, 0.0, div_img.at("number")[money_container[i]], true);
			}
			DrawRotaGraph(MONEY_INIT_POSX - (money_container.size() + 2) * NUMBER_SIZE, TIME_INIT_POSY, 0.1, 0.0, img.at("money"), true);
			if (caution_flag)
			{
				if (boar_caution_time % 12 <= 4)
				{
					if (boar_spawn.x == WINDOW_SIZE_X)
					{
						DrawRotaGraph(boar_spawn.x - CAUTION_SHIFT, boar_spawn.y, 0.1, 0.0, img.at("caution"), true);
					}
					else
					{
						DrawRotaGraph(boar_spawn.x + CAUTION_SHIFT, boar_spawn.y, 0.1, 0.0, img.at("caution"), true);
					}
				}
			}
			switch (choice)
			{
			case IconState::eJagaimo:
				for (int i = 0; i < expense.size(); i++)
				{
					DrawRotaGraph(Init_Posx1 + (choice_num * COST_SHIFT) + (i * COST_SHIFT_X), COST_POS_Y, 0.1, 0.0, div_img.at("number")[expense[i]], true);
				}
				DrawRotaGraph(Init_Posx1 + (choice_num * COST_SHIFT) + (expense.size() * COST_SHIFT_X), COST_POS_Y, 0.05, 0.0, img.at("yen"), true);
				break;
			case IconState::eSatumaimo:
				for (int i = 0; i < expense.size(); i++)
				{
					DrawRotaGraph(Init_Posx1 + (choice_num * COST_SHIFT) + (i * COST_SHIFT_X), COST_POS_Y, 0.1, 0.0, div_img.at("number")[expense[i]], true);
				}
				DrawRotaGraph(Init_Posx1 + (choice_num * COST_SHIFT) + (expense.size() * COST_SHIFT_X), COST_POS_Y, 0.05, 0.0, img.at("yen"), true);
				break;
			case IconState::eYamaimo:
				for (int i = 0; i < expense.size(); i++)
				{
					DrawRotaGraph(Init_Posx1 + (choice_num * COST_SHIFT1) + (i * COST_SHIFT_X), COST_POS_Y, 0.1, 0.0, div_img.at("number")[expense[i]], true);
				}
				DrawRotaGraph(Init_Posx1 + (choice_num * COST_SHIFT1) + (expense.size() * COST_SHIFT_X), COST_POS_Y, 0.05, 0.0, img.at("yen"), true);
				break;
			}
		}
		else
		{

			DrawRotaGraph(WINDOW_SIZE_X / 2, PAUSE_FONT_POS_Y, 0.5, 0.0, img.at("pause_font"), true);
			DrawRotaGraph(WINDOW_SIZE_X / 2, PAUSE_POS_Y, 0.3, 0.0, img.at("pause_title"), true);
			DrawRotaGraph(WINDOW_SIZE_X / 2, PAUSE_POS_Y2, 0.3, 0.0, img.at("pause_start"), true);
			double angle = -90 * M_PI / 180;
			DrawRotaGraph(WINDOW_SIZE_X / 2 - PAUSE_CURSOR_SHIFT, PAUSE_POS_Y + pause_cursor * PAUSE_CURSOR_SHIFT, 0.1, angle, img.at("key"), true);
		}
		break;
	case SceneState::end:
		DrawRotaGraph(WINDOW_SIZE_X / 2, WINDOW_SIZE_Y / 2, 0.2, 0.0, img.at("end"), true);
		break;
	default:
		break;
	}

	if (!pause_flag)
	{
		
		if (state == SceneState::game)
		{
			DrawBox((Init_Posx2 + choice_num * Cursor_size) - 53, Init_Posy2 - 53, (Init_Posx2 + choice_num * Cursor_size) + 53, Init_Posy2 + 53, icon_blinking_color, TRUE);
		}
		DrawRotaGraph(240, 600, 0.15, 0.0, div_img.at("LBRB")[0], TRUE);
		DrawRotaGraph(1040, 600, 0.15, 0.0, div_img.at("LBRB")[1], TRUE);
		for (int i = 0; i < 6; i++)
		{
			switch (i)
			{
			case 0:
				if (count_potato >= 10)
				{
					//灰色で全体描画
					SetDrawBright(150, 150, 150);
					DrawRotaGraph(Init_Posx1 + i * Cursor_size, Init_Posy1, 1.2, 0.0, div_img.at("icon")[i], true);
					SetDrawBright(255, 255, 255);
				}
				else
				{
					DrawRotaGraph(Init_Posx1 + i * Cursor_size, Init_Posy1, 1.2, 0.0, div_img.at("icon")[i], true);
				}
				break;
			case 1:
				if (money < 300 || count_potato >= 10)
				{
					//灰色で全体描画
					SetDrawBright(150, 150, 150);
					DrawRotaGraph(Init_Posx1 + i * Cursor_size, Init_Posy1, 1.2, 0.0, div_img.at("icon")[i], true);
					SetDrawBright(255, 255, 255);
				}
				else
				{
					DrawRotaGraph(Init_Posx1 + i * Cursor_size, Init_Posy1, 1.2, 0.0, div_img.at("icon")[i], true);
				}
				break;
			case 2:
				if (money < 1000 || count_potato >= 10)
				{
					//灰色で全体描画
					SetDrawBright(150, 150, 150);
					DrawRotaGraph(Init_Posx1 + i * Cursor_size, Init_Posy1, 1.2, 0.0, div_img.at("icon")[i], true);
					SetDrawBright(255, 255, 255);
				}
				else
				{
					DrawRotaGraph(Init_Posx1 + i * Cursor_size, Init_Posy1, 1.2, 0.0, div_img.at("icon")[i], true);
				}
				break;
			case 3:
			{
				int x = Init_Posx1 + i * Cursor_size;
				int y = Init_Posy1;

				int handle = div_img.at("icon")[i];

				if (water_wait > 0)
				{
					//灰色で全体描画
					SetDrawBright(128, 128, 128);
					DrawRotaGraph(x, y, 1.2, 0.0, handle, true);
					SetDrawBright(255, 255, 255);

					//復活割合計算
					float rate = 1.0f - (float)water_wait / WATER_WAIT_MAX;
					int imgSize = Cursor_size;

					float scale = 1.2f;

					float fullHeight = imgSize * scale;
					float fullWidth = imgSize * scale;

					float visibleHeight = imgSize * rate;

					if (visibleHeight > 0.0f)
					{
						float drawHeight = visibleHeight * scale;

						float left = x - fullWidth / 2.4f;
						float right = x + fullWidth / 1.7f;

						float bottom = y + fullHeight / 1.7f;
						float top = bottom - drawHeight;

						DrawRectExtendGraph(
							left,
							top,
							right,
							bottom,
							0,
							imgSize - visibleHeight,
							imgSize,
							visibleHeight,
							handle,
							TRUE
						);
					}
				}
				else
				{
					// 通常描画
					DrawRotaGraph(x, y, 1.2, 0.0, handle, true);
				}
			}
			break;
			case 4:
				DrawRotaGraph(Init_Posx1 + i * Cursor_size, Init_Posy1, 1.2, 0.0, div_img.at("icon")[i], true);
				break;
			case 5:
				DrawRotaGraph(Init_Posx1 + i * Cursor_size, Init_Posy1, 1.2, 0.0, div_img.at("icon")[i], true);
				break;
			default:
				break;
			}
		}
	}

}

void InGameScene::Finalize()
{
	SaveScore(money);
	SaveDetailScore(jagaimo_count, satumaimo_count, yamaimo_count, mole_defeat_count, boar_defeat_count);
	img.clear();
	div_img.clear();
	bgm.clear();
	se.clear();
	am->DestroyAllObject();
	am->CheckDestroyObject();
}

void InGameScene::CountStorage()
{
	int storage = count;
	while (storage > 0)
	{
		draw_count.push_back(storage % 10);
		storage = storage / 10;
	}
}

void InGameScene::MoneyStorage()
{
	int storage = money;
	if (storage == 0)
	{
		money_container.push_back(storage);
	}
	else while (storage > 0)
	{
		money_container.push_back(storage % 10);
		storage = storage / 10;
	}
}

bool InGameScene::CheckCollision(const Position2D& pos)
{
	std::vector<Actor*> actor_list = am->GetObjectsList();
	for (Actor* actor : actor_list)
	{
		if (cursor == actor) continue;
		CircleCollision col;
		if (actor->ActorTypeReference() == ActorType::ePoteto)
		{
			col = dynamic_cast<Potate*>(actor)->CollisinoReference();
		}
		else if (actor->ActorTypeReference() == ActorType::eHole)
		{
			col = dynamic_cast<MoleHole*>(actor)->CollisionReference();
		}
		else if (actor->ActorTypeReference() == ActorType::eEnemy)
		{
			col = dynamic_cast<Enemy*>(actor)->CollisionReference();
		}
		else continue;
	 
		Vector2D distance = VectorSubtraction(col.center, pos);
		if (col.radius * 2 >= fabs(distance.x) && col.radius * 2 >= fabs(distance.y))
		{
			return true;
		}
	}
	return false;
}

void InGameScene::CheckTool(const CircleCollision& collision)
{
	std::vector<Actor*> actor_list = am->GetObjectsList();
	CircleCollision col;
	col.center.x = 0.0;
	col.center.y = 0.0;
	col.radius = 0.0;
	for (Actor* actor : actor_list)
	{
		if (cursor == actor) continue;
		switch (choice)
		{
		case IconState::eWater:
		case IconState::eScoop:
			if (actor->ActorTypeReference() == ActorType::ePoteto)
			{
				col = dynamic_cast<Potate*>(actor)->CollisinoReference();
			}
			else continue;
			break;
		case IconState::eHammer:
			if (actor->ActorTypeReference() == ActorType::eEnemy)
			{
				col = dynamic_cast<Enemy*>(actor)->CollisionReference();
			}
			else continue;
			break;
		default:
			break;
		}
		Vector2D distance = VectorSubtraction(col.center, collision.center);
		bool enemy_near = false;
		if (col.radius + collision.radius >= fabs(distance.x) && col.radius + collision.radius >= fabs(distance.y))
		{
			switch (choice)
			{
			case IconState::eWater:
				if (water_wait <= 0)
				{
					if (dynamic_cast<Potate*>(actor)->GrowthStateReference() != GrowthState::eMature)
					{
						dynamic_cast<Potate*>(actor)->Watering();
						water_wait = WATER_WAIT_MAX;
						PlaySoundMem(se.at("growth"), DX_PLAYTYPE_BACK);
					}
				}
				break;
			case IconState::eScoop:
				if (dynamic_cast<Potate*>(actor)->GrowthStateReference() != GrowthState::eMature)
					break;

				for (Actor* partner : actor_list)
				{
					if (partner->ActorTypeReference() == eEnemy)
					{
						Vector2D dis = VectorSubtraction(
							partner->PositionReference(),
							actor->PositionReference()
						);

						if (fabs(dis.x) <= 2 && fabs(dis.y) <= 2)
						{
							if(!dynamic_cast<Enemy*>(partner)->DefeatFlagReference()) enemy_near = true;
							break;
						}
					}
				}
				if (!enemy_near)
				{
					Potate* potato = dynamic_cast<Potate*>(actor);
					switch (potato->PotatoTypeReference())
					{
					case PotatoType::jagaimo: 
						am->CreateObject<ScoreEffect>(actor->PositionReference())->SetScore(100);  
						money += 100;  
						jagaimo_count++;
						break;
					case PotatoType::satumaimo: 
						am->CreateObject<ScoreEffect>(actor->PositionReference())->SetScore(800);
						money += 800;  
						satumaimo_count++;
						break;
					case PotatoType::yamaimo:   
						am->CreateObject<ScoreEffect>(actor->PositionReference())->SetScore(2000);
						money += 2000; 
						yamaimo_count++;
						break;
					}
					PlaySoundMem(se.at("score_plus"), DX_PLAYTYPE_BACK);

					am->DestroyObject(actor);
				}
				break;
			case IconState::eHammer:
				Enemy* enemy = dynamic_cast<Enemy*>(actor);
				if (!enemy->DefeatFlagReference())
				{
					xinput->Pad_Vibe(100, 500);
					enemy->SubtractionHP();

					if (enemy->HPReference() == 0)
					{
						if (enemy->EnemyTypeReference() == EnemyType::eMole && dig_flag)
						{
							dig_flag = false;
							dig_time = 0;
						}
						PlaySoundMem(se.at("defeat"), DX_PLAYTYPE_BACK);
						if (dynamic_cast<Enemy*>(actor)->EnemyTypeReference() == EnemyType::eMole)
						{
							mole_defeat_count++;
						}
						else if (dynamic_cast<Enemy*>(actor)->EnemyTypeReference() == EnemyType::eBoar)
						{
							boar_defeat_count++;
							if (CheckSoundMem(se.at("wild_boar_run")))
							{
								StopSoundMem(se.at("wild_boar_run"));
							}
						}
					}
				}
				
				break;
			}
		}
	}
}

void InGameScene::IntToIconState()
{
	switch (choice_num)
	{
	case 0:
		choice = IconState::eJagaimo;
		ExpenseStorage(0);
		break;
	case 1:
		choice = IconState::eSatumaimo;
		ExpenseStorage(300);
		break;
	case 2:
		choice = IconState::eYamaimo;
		ExpenseStorage(1000);
		break;
	case 3:
		choice = IconState::eWater;
		break;
	case 4:
		choice = IconState::eScoop;
		break;
	case 5:
		choice = IconState::eHammer;
	}
}

void InGameScene::EnemyMove(Actor* act)
{
	Vector2D distance;
	float mostnear = FLT_MAX;
	Actor* most_potato = nullptr;
	switch (dynamic_cast<Enemy*>(act)->EnemyTypeReference())
	{
	case EnemyType::eMole:
		for (Actor* actor : actor_list)
		{
			if (actor->ActorTypeReference() == ActorType::ePoteto)
			{
				distance = VectorSubtraction(act->PositionReference(), actor->PositionReference());
				float distsq = distance.x * distance.x + distance.y * distance.y;
				if (distsq < mostnear)
				{
					most_potato = actor;
					mostnear = distsq;
				}
			}
		}
		if (most_potato == nullptr)
		{
			for (Actor* actor : actor_list)
			{
				if (actor->ActorTypeReference() == ActorType::eHole)
				{
					distance = VectorSubtraction(act->PositionReference(), actor->PositionReference());
					float distsq = distance.x * distance.x + distance.y * distance.y;
					if (distsq < mostnear)
					{
						most_potato = actor;
						mostnear = distsq;
					}
				}
			}

		}
		if (!dig_flag)
		{
			dynamic_cast<Mole*>(act)->Move(most_potato->PositionReference());
		}
		break;
	case EnemyType::eBoar:
		dynamic_cast<Boar*>(act)->Move();
		break;
	default:
		break;
	}

}

void InGameScene::EnemyHit(Actor* act)
{
	Enemy* enemy = dynamic_cast<Enemy*>(act);
	bool flag = false;
	for (Actor* actor : actor_list)
	{
		if (actor->ActorTypeReference() == ActorType::ePoteto)
		{
			Vector2D distance = VectorSubtraction(act->PositionReference(), actor->PositionReference());
			CircleCollision col = enemy->CollisionReference();
			if (enemy->EnemyTypeReference() == EnemyType::eBoar)
			{
				if (fabs(distance.x) <= col.radius && fabs(distance.y) <= col.radius)
				{
					am->DestroyObject(actor);
					count_potato--;
					PlaySoundMem(se.at("destroy_poteto"), DX_PLAYTYPE_BACK);
				}
			}
			else
			{
				if (!enemy->DefeatFlagReference())
				{
					if (fabs(distance.x) <= 2 && fabs(distance.y) <= 2)
					{
						flag = true;
						if (!dig_flag)
						{
							dynamic_cast<Mole*>(act)->DigFlag(true);
							dig_flag = true;
							dig_time = 2;
						}
						else
						{
							if (dig_time <= 0)
							{
								dig_flag = false;
								dig_time = 0;
								am->DestroyObject(actor);
								am->DestroyObject(act);
								count_potato--;
								PlaySoundMem(se.at("destroy_poteto"), DX_PLAYTYPE_BACK);
							}
							else
							{
								if (dynamic_cast<Mole*>(act)->AnimCountReference() % 18 == 0)
								{
									PlaySoundMem(se.at("dig_mole"), DX_PLAYTYPE_BACK);
								}
							}
						}
					}
				}
			}
		}
		else if (actor->ActorTypeReference() == ActorType::eHole)
		{
			if (enemy->EnemyTypeReference() == EnemyType::eMole)
			{
				if (count_potato <= 0)
				{
					Vector2D distance = VectorSubtraction(act->PositionReference(), actor->PositionReference());
					if (fabs(distance.x) <= 2 && fabs(distance.y) <= 2)
					{
						am->DestroyObject(act);
					}
				}
			}
		}
	}
	if (!flag && enemy->EnemyTypeReference() == EnemyType::eMole)
	{
		dig_flag = false;
		dig_time = 0;
		dynamic_cast<Mole*>(act)->DigFlag(false);
	}
}

void InGameScene::ExpenseStorage(int num)
{
	if (num == 0)
	{
		expense.push_back(0);
	}
	else while (num > 0)
	{
		expense.push_back(num % 10);
		num /= 10;
	}
	std::reverse(expense.begin(), expense.end());
}
