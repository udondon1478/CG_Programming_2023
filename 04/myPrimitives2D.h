//
//  myPrimitives2D.h
//  
//
//  Created by KITASAKATAKAYUKI on 2023/10/19.
//

#ifndef myPrimitives2D_h  // ifndef : IF Not DEFined XXX（XXXが宣言されていなかったら） 二重登録を防ぐための措置
#define myPrimitives2D_h  // XXXを登録

// 塗りつぶし円の描画（中心座標(cx, cy)，半径r）
void glMySolidCircle(double cx, double cy, double r)
{
	// 計算領域の決定
	double xMin = cx - r;
	double xMax = cx + r;
	double yMin = cy - r;
	double yMax = cy + r;

	// 点の描画開始
	glBegin(GL_POINTS);
	// 計算領域内のすべての点を走査（i：xMin〜xMax， j：yMin〜yMax）
	for (int j=yMin; j<=yMax; j++){
		for (int i=xMin; i<=xMax; i++){
			// 中心(cx, cy)，半径rの円を表す陰関数f(x, y)に点(i, j)を代入
			double f = (i-cx)*(i-cx) + (j-cy)*(j-cy) - r*r;
			
			// 計算結果の判定：f<=0 → 点(i, j)は円の境界または内部 → 点を描画
			if(f <= 0.0){
				glVertex2d(i, j);
			}
		}
	}
	glEnd();   //描画終了
}

// 塗りつぶし無し円の描画（中心座標(cx, cy)，半径r，線幅w）
void glMyWireCircle(double cx, double cy, double r, double w)
{
	//計算領域の決定
	double xMin = cx - r;
	double xMax = cx + r;
	double yMin = cy - r;
	double yMax = cy + r;
	
	// 点の描画開始
	glBegin(GL_POINTS);
	// 計算領域内のすべての点を走査（i：xMin〜xMax， j：yMin〜yMax）
	for (int j=yMin; j<=yMax; j++){
		for (int i=xMin; i<=xMax; i++){
			// 中心(cx, cy)，半径(r-w/2)の円を表す陰関数f1(x, y)に点(i, j)を代入
			double f1 = (i-cx)*(i-cx) + (j-cy)*(j-cy) - (r-w/2.0)*(r-w/2.0);
			// 中心(cx, cy)，半径(r+w/2)の円を表す陰関数f2(x, y)に点(i, j)を代入
			double f2 = (i-cx)*(i-cx) + (j-cy)*(j-cy) - (r+w/2.0)*(r+w/2.0);

			//計算結果の判定：f1<=0 かつ f2>=0 → 点(i, j)は線幅wの円周上 → 点を描画
			if(f1 <= 0.0 && f2 >= 0.0){
				glVertex2d(i, j);
			}
		}
	}
	glEnd();   //描画終了
}

// 塗り潰し円描画関数（多角形近似バージョン）
// 中心座標(cx, cy)，半径r，線幅w，分割数p
void glMySolidCircle2(double cx, double cy, double r, double w, int p)
{
	double delta = 2.0*M_PI / p;  // 変位量Δ, delta=2π/p

	glLineWidth(w);
	
	glBegin(GL_POLYGON);
	for(int i=0; i<p; i++){
		double t = i*delta;  // 角度 t
		// 円周上の点（x, y）
		double x = r*cos(t) + cx;
		double y = r*sin(t) + cy;
		glVertex2d(x, y);
	}
	glEnd();
}

// 塗り潰しなしの円描画関数（多角形近似バージョン）
// 中心座標(cx, cy)，半径r，線幅w，分割数p
void glMyWireCircle2(double cx, double cy, double r, double w, int p)
{
	double delta = 2.0*M_PI/p;  //変位量Δ, delta=2π/p
	
	glLineWidth(w);
	
	glBegin(GL_LINE_LOOP);
	for(int i=0; i<p; i++){
		double t = i*delta;  //角度 t
		// 円周上の点（x, y）
		double x = r*cos(t) + cx;
		double y = r*sin(t) + cy;
		glVertex2d(x, y);
	}
	glEnd();
}

// 2次ベジエ曲線
// 始点：(sx, sy)，終点：(ex, ey)，制御点：(tx, ty)，分割数p（default:1000）
void glBezier(double sx, double sy, double ex, double ey, double tx, double ty, int p=1000)
{
	glBegin(GL_LINE_STRIP);
	for(int i=0; i<=p; i++){
		double t = i*1.0/double(p);  // 媒介変数（パラメータ）t [0 <= t <= 1], TYPE(X): 変数Xを型TYPEに強制変換．キャスト（cast）と言う
		// ベジェ曲線上の点（px, py）
		double px = (sx+ex-2.0*tx)*t*t + 2.0*(tx-sx)*t + sx;
		double py = (sy+ey-2.0*ty)*t*t + 2.0*(ty-sy)*t + sy;
		glVertex2d(px, py);
	}
	glEnd();
}

// 塗り潰し2次ベジエ曲線
// 始点：(sx, sy)，終点：(ex, ey)，制御点：(tx, ty)，分割数p（default:1000）
void glSolidBezier(double sx, double sy, double ex, double ey, double tx, double ty, int p=1000)
{
	glBegin(GL_POLYGON);
	for(int i=0.0; i<=p; i++){
		double t = i*1.0/double(p);  // 媒介変数（パラメータ）t [0 <= t <= 1], TYPE(X): 変数Xを型TYPEに強制変換．キャスト（cast）と言う
		// ベジェ曲線上の点（px, py）
		double px = (sx+ex-2.0*tx)*t*t + 2.0*(tx-sx)*t + sx;
		double py = (sy+ey-2.0*ty)*t*t + 2.0*(ty-sy)*t + sy;
		glVertex2d(px, py);
	}
	glEnd();
}

// 塗りつぶし楕円の描画（中心座標(cx, cy)，長径A，短径B）
void glMySolidEllipse(double cx, double cy, double A, double B)
{
	// 計算領域の決定
	double xMin = cx-A;
	double xMax = cx+A;
	double yMin = cy-B;
	double yMax = cy+B;
	
	// 点の描画開始
	glBegin(GL_POINTS);
	// 計算領域内のすべての点を走査（i：xMin〜xMax， j：yMin〜yMax）
	for (int j=yMin; j<=yMax; j++){
		for (int i=xMin; i<=xMax; i++){
			// 中心(cx, cy)，長径A，短径Bの楕円を表す陰関数f(x, y)に点(i, j)を代入
			double f = B*B*(i-cx)*(i-cx) + A*A*(j-cy)*(j-cy) - A*A*B*B;
			
			// 計算結果の判定：f<=0 → 点(i, j)は楕円の境界または内部 → 点を描画
			if(f <= 0.0){
				glVertex2d(i, j);
			}
		}
	}
	glEnd();   // 描画終了
}

// 塗りつぶし無し楕円の描画（中心座標(cx, cy)，長径A，短径B，線幅w）
void glMyWireEllipse(double cx, double cy, double A, double B, double w)
{
	// 計算領域の決定
	double xMin = cx-A;
	double xMax = cx+A;
	double yMin = cy-B;
	double yMax = cy+B;

	// 点の描画開始
	glBegin(GL_POINTS);
	// 計算領域内のすべての点を走査（i：xMin〜xMax， j：yMin〜yMax）
	for (int j=yMin; j<=yMax; j++){
		for (int i=xMin; i<=xMax; i++){
            // 中心(cx, cy)，長径A，短径Bの楕円を表す陰関数f(x, y)に点(i, j)を代入
			double f1 = B*B*(i-cx)*(i-cx) + A*A*(j-cy)*(j-cy) - (A-w/2.0)*(A-w/2.0)*(B-w/2.0)*(B-w/2.0);
            // 中心(cx, cy)，長径A，短径Bの楕円を表す陰関数f(x, y)に点(i, j)を代入
			double f2 = B*B*(i-cx)*(i-cx) + A*A*(j-cy)*(j-cy) - (A+w/2.0)*(A+w/2.0)*(B+w/2.0)*(B+w/2.0);

			// 計算結果の判定：f1<=0 かつ f2>=0 → 点(i, j)は線幅wの楕円周上 → 点を描画
			if(f1<=0 && f2>=0){
				glVertex2d(i, j);
			}
		}
	}
	glEnd();   // 描画終了
}

// 塗り潰し楕円描画関数（多角形近似バージョン）
// 中心座標(cx, cy)，長径A，短径B，分割数p
void glMySolidEllipse2(double cx, double cy, double A, double B, int p)
{
	double delta = 2.0*M_PI/p;  // 変位量Δ
	
	glBegin(GL_POLYGON);
	for(int i=0; i<p; i++){
		double t = i*delta;  // 角度 t
		// だ円周上の点（x, y）
		double x = A*cos(t) + cx;
		double y = B*sin(t) + cy;
		glVertex2d(x, y);
	}
	glEnd();
}

// 塗り潰しなしの楕円描画関数（多角形近似バージョン）
// 中心座標(cx, cy)，長径A，短径B，線幅w，分割数p
void glMyWireEllipse2(double cx, double cy, double A, double B, double w, int p)
{
	double delta = 2.0*M_PI/p;  // 変位量Δ

	glLineWidth(w);
	
	glBegin(GL_LINE_LOOP);
	for(int i=0; i<p; i++){
		double t = i*delta;  //角度 t
		// だ円周上の点（x, y）
		double x = A*cos(t) + cx;
		double y = B*sin(t) + cy;
		glVertex2d(x, y);
	}
	glEnd();
}

#endif /* myPrimitives2D_h */
