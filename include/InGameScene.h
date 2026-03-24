#pragma once

#include "../SceneBase.h"
#include "../../Objects/ActorManager.h"
#include "../../Objects/IconState.h"
#include "../../Objects/Cursor.h"
#include "../../Objects/Potate.h"
#include "vector"

#define Init_Posx1 (340)
#define Init_Posx2 (340)
#define Init_Posy1 (650)
#define Init_Posy2 (650)
#define TIME_INIT_POSX (210)
#define TIME_INIT_POSY (20)
#define FONT_INIT_POSX (60)
#define MONEY_INIT_POSX (1200)
#define NUMBER_SIZE (30)
#define CAUTION_SHIFT (50)
#define Cursor_size (120)
#define MOLEHOLE_POSX (210)
#define Init_Many (3000)
#define WATER_WAIT_MAX (15)
#define FLAMERATE (60)
#define PAUSE_FONT_POS_Y (100)
#define PAUSE_POS_Y (400)
#define PAUSE_POS_Y2 (600)
#define PAUSE_CURSOR_SHIFT (200)
#define COST_POS_Y (580)
#define COST_SHIFT_X (15)
#define COST_SHIFT (100)
#define COST_SHIFT1 (105)

enum class SceneState
{
	start,
	game,
	end,
};

class InGameScene : public SceneBase
{
private:
	ActorManager* am;
	int choice_num;	//選択中のアイコン
	IconState choice; //カーソルに渡す際のintから変更用
	int count;	//時間
	float delta_second; //フレーム計算用
	int money;	//お金
	Cursor* cursor;	//カーソル
	std::vector<int> draw_count; //残り秒数を画像で描画する用の格納変数
	SceneState state;	//現在のシーンの状態
	std::vector<Actor*> actor_list;	//オブジェクトのリスト
	int count_potato;	//現在画面上にあるポテトの数
	int water_wait;	//水やりのクールタイム
	int boar_caution_time;	//イノシシ注意の画像を描画する時間
	bool caution_flag;	//イノシシ注意の画像を描画するかどうか
	bool rain_flag;	//雨が降るかどうか
	int rain_anim_num; //雨のアニメーション
	Position2D boar_spawn;	//イノシシの生成する位置
	std::vector<int> money_container;	//所持金を画像で描画する用の格納変数
	int dig_time;	//モグラが芋を掘り返す時間
	bool dig_flag;	//モグラが芋を掘り返しているかどうか
	bool start_flag;	//スタートの文字を出すかどうか
	int icon_blinking_count;	//選択中のアイコンを示す外枠のアニメーション
	int icon_blinking_color;	//選択中のアイコンを示す外枠の色
	bool icon_blinking_flag;	//選択中のアイコンを示す外枠を表示するかどうか
	int jagaimo_count;	//ジャガイモを収穫した数
	int satumaimo_count;	//サツマイモを収穫した数
	int yamaimo_count;	//山芋を収穫した数
	int mole_defeat_count;	//モグラを退治した数
	int boar_defeat_count;	//イノシシを退治した数
	int result_count[5];	//芋類、獣の退治数を格納する変数
	bool pause_flag;	//ポーズするかどうか
	int pause_cursor;	//ポーズ中の選択している物
	std::vector<int> expense; //出費額の表示用

public:
	InGameScene();
	virtual ~InGameScene();

public:
	virtual void Initialize() override;
	virtual eSceneType Update() override;
	virtual void Draw() const override;
	virtual void Finalize() override;

public:
	virtual eSceneType GetNowSceneType() const override
	{
		return eSceneType::eInGame;
	}

private:
	/// <summary>
	/// カウントダウンを画像で描画するために格納する
	/// </summary>
	void CountStorage();

	/// <summary>
	/// 所持金を画像で描画するために格納する
	/// </summary>
	void MoneyStorage();

	/// <summary>
	/// カーソルが置けない場所にいるかどうか
	/// </summary>
	/// <param name="pos">カーソルの座標</param>
	/// <returns>true:おけない場所 false:おける場所</returns>
	bool CheckCollision(const Position2D& pos);

	/// <summary>
	/// ツールを使えるかどうか確認し使う
	/// </summary>
	/// <param name="pos">カーソルの座標</param>
	void CheckTool(const CircleCollision& collision);

	/// <summary>
	/// int型のchoice_numをタイプに変換
	/// </summary>
	void IntToIconState();

	/// <summary>
	/// モグラを動かすための関数
	/// </summary>
	/// <param name="act">モグラのアクタークラス</param>
	void EnemyMove(Actor* act);

	/// <summary>
	/// 敵が芋に当たったかどうか
	/// </summary>
	/// <param name="act">敵のアクタークラス</param>
	void EnemyHit(Actor* act);


	void ExpenseStorage(int num);
};