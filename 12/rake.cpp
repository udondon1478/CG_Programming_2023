//マウスドラッグをすると滑らかなカーブを3本描く
//  Created by 松岡正 on 2019/01/10.
//  Last update 2019/01/10.
//g++ -O3 rake.cpp -framework OpenGL -framework GLUT -Wno-deprecated
#include <GLUT/glut.h>

// マウスの座標を保持する変数
int mouseX, mouseY;

// カーブの描画関数
void drawCurve(float x0, float y0, float x1, float y1, float x2, float y2) {
  // ベジェ曲線の計算
  float t;
  for (t = 0.0; t <= 1.0; t += 0.01) {
    // カーブ上の点の計算
    float x = (1 - 3 * t * t * t) * x0 + (3 * t * t * (1 - t)) * x1 + (3 * t * (1 - t) * t) * x2;
    float y = (1 - 3 * t * t * t) * y0 + (3 * t * t * (1 - t)) * y1 + (3 * t * (1 - t) * t) * y2;

    // 点の描画
    glVertex2f(x, y);
  }
}

// マウスのドラッグイベントの処理関数
void mouseMotion(int x, int y) {
  // マウスの座標を更新する
  mouseX = x;
  mouseY = y;

  // カーブを描き直す
  glClear(GL_COLOR_BUFFER_BIT);
  drawCurve(mouseX, mouseY, mouseX - 100, mouseY, mouseX + 100, mouseY);
  drawCurve(mouseX, mouseY, mouseX, mouseY - 100, mouseX, mouseY + 100);
  drawCurve(mouseX, mouseY, mouseX - 100, mouseY - 100, mouseX + 100, mouseY + 100);
  glutSwapBuffers();
}

// ウィンドウの描画関数
void display() {
  // 背景を黒で塗りつぶす
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
}

// メイン関数
int main(int argc, char** argv) {
  // GLUTの初期化
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
  glutInitWindowSize(500, 500);
  glutCreateWindow("滑らかなカーブ");

  // コールバック関数の登録
  glutDisplayFunc(display);
  glutMotionFunc(mouseMotion);

  // メインループの開始
  glutMainLoop();

  return 0;
}