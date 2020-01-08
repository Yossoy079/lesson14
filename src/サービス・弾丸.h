#pragma once
#include <vector>
#include "共通.h"

namespace エンジン {

	class 弾丸サービス;
	class レンダリングサービス;

	struct 弾丸データ{
		float2 位置;
		float2 速度;
		float 半径; //当たり判定
		bool 死んだ;
	};

	// 種類ごと確保
	class 弾丸 {
	public:
		friend 弾丸サービス;
	private:
		unsigned int リソースID_ = 0;
		float2 半サイズ_ = {0.0f, 0.0f};// 幅と高さの半分
		unsigned int 最大数_ = 0;
		unsigned int 個数_ = 0;

		弾丸データ* データ配列_ = nullptr;

		static bool 画面外？(float2 位置, float2 サイズ, float2 画面サイズ);

	public:
		弾丸() {}
		~弾丸() {}

		void 初期化(unsigned int 最大数, unsigned int リソースID, レンダリングサービス& レンダリングサービス);
		void 片付け();
		void リセット();// スタート時など弾を消す際の処理

		int 追加(float2 位置, float2 速度);

		void 更新(float 経過時間, レンダリングサービス& レンダリングサービス);
		void 更新後処理();
		void 描画(レンダリングサービス& レンダリングサービス);
	};


	class 弾丸サービス {
	public:// 定数宣言
		enum 種類 {
			自弾 = 0,
			敵弾,

			最大数,
		};

	private:
		弾丸 弾丸_[(int)(種類::最大数)];
		レンダリングサービス &レンダリングサービス_;

	public:
		弾丸サービス(レンダリングサービス&);
		~弾丸サービス();

		int 追加(弾丸サービス::種類 種類, float2 位置, float2 速度);
		void 更新(float 経過時間);
		void 描画();

	};

}// namespace エンジン