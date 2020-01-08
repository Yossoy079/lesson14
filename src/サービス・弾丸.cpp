﻿#include "サービス・弾丸.h"
#include "サービス・レンダリング.h"
#include "DxLib.h"

namespace エンジン {

	/////////////////////////////////////////////////////
	// 弾丸
	/////////////////////////////////////////////////////

	void 弾丸::初期化(unsigned int 最大数, unsigned int リソースID, レンダリングサービス& レンダリングサービス)
	{
		リソースID_ = リソースID;

		int 大きさ[2];
		レンダリングサービス.サイズ取得(リソースID_, 大きさ);
		半サイズ_.x = static_cast<float>(大きさ[0] >> 1);
		半サイズ_.y = static_cast<float>(大きさ[1] >> 1);

		最大数_ = 最大数;
		データ配列_ = new 弾丸データ[最大数];

		リセット();
	}
	void 弾丸::片付け()
	{
		安全DELETE_ARRAY(データ配列_);
		最大数_ = 0;

		リセット();
	}
	void 弾丸::リセット()
	{
		個数_ = 0;
	}

	int 弾丸::追加(float2 位置, float2 速度)
	{
		if (最大数_ <= 個数_) return -1; // プールが余っていない

		弾丸データ &データ = データ配列_[個数_++];
		データ.位置 = 位置;
		データ.速度 = 速度;
		データ.死んだ = false;

		return 0;
	}

	bool 弾丸::画面外？(float2 位置, float2 サイズ, float2 画面サイズ) {
		return
			位置.x < -サイズ.x ||
			位置.y < -サイズ.y ||
			画面サイズ.x + サイズ.x < 位置.x ||
			画面サイズ.y + サイズ.y < 位置.y;
	}

	void 弾丸::更新(float 経過時間, レンダリングサービス& レンダリングサービス)
	{
		const レンダリングサービス::情報& 画面情報 = レンダリングサービス.情報取得();

		float2 画面サイズ(画面情報.画面サイズ[0], 画面情報.画面サイズ[1]);

		for (int i = 0; i < 個数_; i++) {
			弾丸データ& r = データ配列_[i];

			// 移動
			r.位置 += r.速度 * 経過時間;

			// カリング
			if (画面外？(r.位置, 半サイズ_, 画面サイズ)) {
				r.死んだ = true;
			}
		}
	}

	void 弾丸::更新後処理()
	{
		// 死んだ子に対して、後ろからデータをコピーしてくる
		// (コピーが起きるので重いが、リスト管理だとキャッシュに載りにくくなるので、
		// あえてデータを丸ごとコピー)
		for (int i = 0; i < 個数_; i++) 
		{
			弾丸データ& r = データ配列_[i];
			if (!r.死んだ) continue;// 死んでないなら対象外

			弾丸データ* 入れ替え対象;
			do {
				入れ替え対象 = &データ配列_[--個数_];// 後ろから持ってくる
			}while(入れ替え対象->死んだ && 0 < 個数_);// 生きてる最初の物を探す
			if (i < 個数_) {// 個数が追いつく場合は入れ替えられるものがなかったという意味
				r = *入れ替え対象;
				r.死んだ = false;// この処理の後には、動いている物の死んだフラグは全て落ちている
			}
		}
	}

	void 弾丸::描画(レンダリングサービス& レンダリングサービス)
	{
		for (int i = 0; i < 個数_; i++) {
			弾丸データ& r = データ配列_[i];
			レンダリングサービス.描画(リソースID_, 
				static_cast<int>(r.位置.x + 0.5f), 
				static_cast<int>(r.位置.y + 0.5f));
		}
	}

	/////////////////////////////////////////////////////
	// 弾丸サービス
	/////////////////////////////////////////////////////


	弾丸サービス::弾丸サービス(レンダリングサービス& レンダリングサービス)
		: レンダリングサービス_(レンダリングサービス)
	{
		弾丸_[種類::自弾].初期化(100, RID_SHOT, レンダリングサービス);
		弾丸_[種類::敵弾].初期化(1024, RID_BULLET_ANIM, レンダリングサービス);

	}

	弾丸サービス::~弾丸サービス()
	{
		for (auto& 弾丸 : 弾丸_) {
			弾丸.片付け();
		}
	}

	int 弾丸サービス::追加(弾丸サービス::種類 種類, float2 位置, float2 速度)
	{
		switch (種類) {
		case 種類::自弾:
			速度 = float2(0.0f, -500.f);
			break;
		case 種類::敵弾:
			break;
		default:
			return -1;// おかしな引数が来た
			break;
		}

		弾丸_[種類].追加(位置, 速度);

		return 0;
	}

	void 弾丸サービス::更新(float 経過時間)
	{
		for (auto& 弾丸 : 弾丸_) {
			弾丸.更新(経過時間, レンダリングサービス_);
		}
	}

	void 弾丸サービス::描画()
	{
		for (auto& 弾丸 : 弾丸_) {
			弾丸.更新後処理();// 消えた弾のメモリをきれいにしてから描画
			弾丸.描画(レンダリングサービス_);
		}
	}
}// namespace エンジン
