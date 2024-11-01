// g++ -O3 karesansui.cpp -framework OpenGL -framework GLUT -Wno-deprecated
// 床頂点の高さに応じて色を変える
// 枯山水のような床を描画するプログラム
// レーキで床をなぞると、床の高さが変化する
// マウスの左ボタンを押しながらドラッグすると、ドラッグした領域の床の高さが変化する
// 数字キーで小石や木を配置するモードを切り替え
// 1:小石 2:木 3:レーキで線を引く
// 4:小石をランダムに配置 5:木をランダムに配置

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLUT/glut.h> //OpenGL/GLUTの使用

// 3次元ベクトル構造体: Vec_3D
typedef struct _Vec_3D {
    double x, y, z;
} Vec_3D;

// 定数宣言
#define FlOOR 10.0 //床の幅の半分
#define TILE 200 //床頂点格子分割数

// 関数名の宣言
void initGL();
void display();
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void keyboard(unsigned char key, int x, int y);
void timer(int value);
Vec_3D normcrossprod(Vec_3D v1, Vec_3D v2);
double vectorNormalize(Vec_3D *vec);
void drawFloor();
void drawStone();
void drawTree();
void drawRake();
void drawRandomStone();
void drawRandomTree();
void drawLine();
void drawString(char *str, int w, int h, int x0, int y0);

// グローバル変数
double eDist, eDegX, eDegY; //視点の位置と向き
Vec_3D floorPoint[TILE][TILE]; //床頂点の位置
Vec_3D floorNormal[TILE][TILE]; //床頂点の法線ベクトル
double floorHeight[TILE][TILE]; //床頂点の高さ
int stone[TILE][TILE]; //小石の配置
int tree[TILE][TILE]; //木の配置
int rake[TILE][TILE]; //レーキでなぞった線の配置
int mode; //モード

int mX, mY, mButton, mState; //マウスの状態
int winW, winH; //ウィンドウのサイズ
Vec_3D boxPos; //視点座標

// メイン関数
int main(int argc, char *argv[]) {
    glutInit(&argc, argv); //GLUTの初期化
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH); //ディスプレイモードの設定
    glutInitWindowSize(800, 600); //ウィンドウサイズの設定
    glutCreateWindow("Karesansui"); //ウィンドウの生成
    glutDisplayFunc(display); //ディスプレイコールバック関数の設定
    glutReshapeFunc(reshape); //リシェイプコールバック関数の設定
    glutMouseFunc(mouse); //マウスコールバック関数の設定
    glutMotionFunc(motion); //マウスモーションコールバック関数の設定
    glutKeyboardFunc(keyboard); //キーボードコールバック関数の設定
    glutTimerFunc(100, timer, 0); //タイマーコールバック関数の設定
    initGL(); //OpenGLの初期化
    glutMainLoop(); //メインループ
    return 0;
}

// OpenGLの初期化
void initGL() {
    glClearColor(0.0, 0.0, 0.0, 1.0); //背景色の設定
    glEnable(GL_DEPTH_TEST); //デプスバッファの有効化
    glEnable(GL_CULL_FACE); //カリングの有効化
    glCullFace(GL_BACK); //裏面をカリング
    glFrontFace(GL_CCW); //反時計回りが表面
    glLineWidth(2.0); //線の太さ
    glPointSize(5.0); //点の大きさ
    glMatrixMode(GL_PROJECTION); //行列モードの設定
    glLoadIdentity(); //単位行列の設定
    gluPerspective(30.0, 1.0, 1.0, 1000.0); //透視投影法の設定
    glMatrixMode(GL_MODELVIEW); //行列モードの設定
    glLoadIdentity(); //単位行列の設定
    eDist = 500.0; //視点の距離
    eDegX = 0.0; //視点のX軸回転角度
    eDegY = 0.0; //視点のY軸回転角度
    mode = 1; //モード
    int i, j;
    for (i = 0; i < TILE; i++) {
        for (j = 0; j < TILE; j++) {
            floorPoint[i][j].x = (i - TILE / 2) * FlOOR / (TILE / 2);
            floorPoint[i][j].y = 0.0;
            floorPoint[i][j].z = (j - TILE / 2) * FlOOR / (TILE / 2);
            floorNormal[i][j].x = 0.0;
            floorNormal[i][j].y = 1.0;
            floorNormal[i][j].z = 0.0;
            floorHeight[i][j] = 0.0;
            stone[i][j] = 0;
            tree[i][j] = 0;
            rake[i][j] = 0;
        }
    }
    boxPos.x = 0.0;
    boxPos.y = 0.0;
    boxPos.z = 0.0;
}

// ディスプレイコールバック関数
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //バッファのクリア
    glLoadIdentity(); //単位行列の設定
    gluLookAt(eDist * sin(eDegX * M_PI / 180.0) * cos(eDegY * M_PI / 180.0) + boxPos.x,
              eDist * sin(eDegY * M_PI / 180.0) + boxPos.y,
              eDist * cos(eDegX * M_PI / 180.0) * cos(eDegY * M_PI / 180.0) + boxPos.z,
              boxPos.x, boxPos.y, boxPos.z, 0.0, 1.0, 0.0); //視点の設定
    drawFloor(); //床の描画
    drawStone(); //小石の描画
    drawTree(); //木の描画
    drawRake(); //レーキでなぞった線の描画
    drawLine(); //マウスでなぞった線の描画
    char str[256];
    sprintf(str, "Mode: %d", mode);
    drawString(str, winW, winH, 10, 10);
    glutSwapBuffers(); //バッファの入れ替え
}

// リシェイプコールバック関数
void reshape(int w, int h) {
    winW = w;
    winH = h;
    glViewport(0, 0, w, h); //ビューポートの設定
    glMatrixMode(GL_PROJECTION); //行列モードの設定
    glLoadIdentity(); //単位行列の設定
    gluPerspective(30.0, (double) w / (double) h, 1.0, 1000.0); //透視投影法の設定
    glMatrixMode(GL_MODELVIEW); //行列モードの設定
    glLoadIdentity(); //単位行列の設定
}

// マウスコールバック関数
void mouse(int button, int state, int x, int y) {
    mX = x;
    mY = y;
    mButton = button;
    mState = state;
}

// マウスモーションコールバック関数
void motion(int x, int y) {
    if (mButton == GLUT_LEFT_BUTTON && mState == GLUT_DOWN) {
        int i, j;
        double dx = (double) (x - mX) / (double) winW;
        double dy = (double) (y - mY) / (double) winH;
        double d = sqrt(dx * dx + dy * dy);
        if (d > 0.0) {
            dx /= d;
            dy /= d;
            for (i = 0; i < TILE; i++) {
                for (j = 0; j < TILE; j++) {
                    if (rake[i][j] == 1) {
                        floorHeight[i][j] += 0.1 * (dx * (double) (i - TILE / 2) + dy * (double) (j - TILE / 2));
                    }
                }
            }
        }
    }
    mX = x;
    mY = y;
}

// キーボードコールバック関数
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case '1':
            mode = 1;
            break;
        case '2':
            mode = 2;
            break;
        case '3':
            mode = 3;
            break;
        case '4':
            drawRandomStone();
            break;
        case '5':
            drawRandomTree();
            break;
        case 'q':
        case 'Q':
        case '\033':
            exit(0);
            break;
    }
}

// タイマーコールバック関数
void timer(int value) {
    glutPostRedisplay(); //ディスプレイコールバック関数の呼び出し
    glutTimerFunc(100, timer, 0); //タイマーコールバック関数の設定
}

// 外積を計算する関数
Vec_3D normcrossprod(Vec_3D v1, Vec_3D v2) {
    Vec_3D v;
    v.x = v1.y * v2.z - v1.z * v2.y;
    v.y = v1.z * v2.x - v1.x * v2.z;
    v.z = v1.x * v2.y - v1.y * v2.x;
    vectorNormalize(&v);
    return v;
}

// ベクトルの正規化を行う関数
double vectorNormalize(Vec_3D *vec) {
    double d = sqrt(vec->x * vec->x + vec->y * vec->y + vec->z * vec->z);
    if (d > 0.0) {
        vec->x /= d;
        vec->y /= d;
        vec->z /= d;
    }
    return d;
}

// 床の描画
void drawFloor() {
    int i, j;
    Vec_3D v1, v2, v3, v4;
    for (i = 0; i < TILE - 1; i++) {
        for (j = 0; j < TILE - 1; j++) {
            v1 = floorPoint[i][j];
            v2 = floorPoint[i + 1][j];
            v3 = floorPoint[i + 1][j + 1];
            v4 = floorPoint[i][j + 1];
            glBegin(GL_POLYGON);
            glNormal3d(floorNormal[i][j].x, floorNormal[i][j].y, floorNormal[i][j].z);
            glVertex3d(v1.x, v1.y + floorHeight[i][j], v1.z);
            glNormal3d(floorNormal[i + 1][j].x, floorNormal[i + 1][j].y, floorNormal[i + 1][j].z);
            glVertex3d(v2.x, v2.y + floorHeight[i + 1][j], v2.z);
            glNormal3d(floorNormal[i + 1][j + 1].x, floorNormal[i + 1][j + 1].y, floorNormal[i + 1][j + 1].z);
            glVertex3d(v3.x, v3.y + floorHeight[i + 1][j + 1], v3.z);
            glNormal3d(floorNormal[i][j + 1].x, floorNormal[i][j + 1].y, floorNormal[i][j + 1].z);
            glVertex3d(v4.x, v4.y + floorHeight[i][j + 1], v4.z);
            glEnd();
        }
    }
}

// 小石の描画
void drawStone() {
    int i, j;
    Vec_3D v1, v2, v3, v4;
    for (i = 0; i < TILE - 1; i++) {
        for (j = 0; j < TILE - 1; j++) {
            if (stone[i][j] == 1) {
                v1 = floorPoint[i][j];
                v2 = floorPoint[i + 1][j];
                v3 = floorPoint[i + 1][j + 1];
                v4 = floorPoint[i][j + 1];
                glBegin(GL_POLYGON);
                glNormal3d(floorNormal[i][j].x, floorNormal[i][j].y, floorNormal[i][j].z);
                glVertex3d(v1.x, v1.y + floorHeight[i][j], v1.z);
                glNormal3d(floorNormal[i + 1][j].x, floorNormal[i + 1][j].y, floorNormal[i + 1][j].z);
                glVertex3d(v2.x, v2.y + floorHeight[i + 1][j], v2.z);
                glNormal3d(floorNormal[i + 1][j + 1].x, floorNormal[i + 1][j + 1].y, floorNormal[i + 1][j + 1].z);
                glVertex3d(v3.x, v3.y + floorHeight[i + 1][j + 1], v3.z);
                glNormal3d(floorNormal[i][j + 1].x, floorNormal[i][j + 1].y, floorNormal[i][j + 1].z);
                glVertex3d(v4.x, v4.y + floorHeight[i][j + 1], v4.z);
                glEnd();
            }
        }
    }
}

// 木の描画
void drawTree() {
    int i, j;
    Vec_3D v1, v2, v3, v4;
    for (i = 0; i < TILE - 1; i++) {
        for (j = 0; j < TILE - 1; j++) {
            if (tree[i][j] == 1) {
                v1 = floorPoint[i][j];
                v2 = floorPoint[i + 1][j];
                v3 = floorPoint[i + 1][j + 1];
                v4 = floorPoint[i][j + 1];
                glBegin(GL_POLYGON);
                glNormal3d(floorNormal[i][j].x, floorNormal[i][j].y, floorNormal[i][j].z);
                glVertex3d(v1.x, v1.y + floorHeight[i][j], v1.z);
                glNormal3d(floorNormal[i + 1][j].x, floorNormal[i + 1][j].y, floorNormal[i + 1][j].z);
                glVertex3d(v2.x, v2.y + floorHeight[i + 1][j], v2.z);
                glNormal3d(floorNormal[i + 1][j + 1].x, floorNormal[i + 1][j + 1].y, floorNormal[i + 1][j + 1].z);
                glVertex3d(v3.x, v3.y + floorHeight[i + 1][j + 1], v3.z);
                glNormal3d(floorNormal[i][j + 1].x, floorNormal[i][j + 1].y, floorNormal[i][j + 1].z);
                glVertex3d(v4.x, v4.y + floorHeight[i][j + 1], v4.z);
                glEnd();
            }
        }
    }
}

// レーキでなぞった線の描画
void drawRake() {
    int i, j;
    Vec_3D v1, v2, v3, v4;
    for (i = 0; i < TILE - 1; i++) {
        for (j = 0; j < TILE - 1; j++) {
            if (rake[i][j] == 1) {
                v1 = floorPoint[i][j];
                v2 = floorPoint[i + 1][j];
                v3 = floorPoint[i + 1][j + 1];
                v4 = floorPoint[i][j + 1];
                glBegin(GL_POLYGON);
                glNormal3d(floorNormal[i][j].x, floorNormal[i][j].y, floorNormal[i][j].z);
                glVertex3d(v1.x, v1.y + floorHeight[i][j], v1.z);
                glNormal3d(floorNormal[i + 1][j].x, floorNormal[i + 1][j].y, floorNormal[i + 1][j].z);
                glVertex3d(v2.x, v2.y + floorHeight[i + 1][j], v2.z);
                glNormal3d(floorNormal[i + 1][j + 1].x, floorNormal[i + 1][j + 1].y, floorNormal[i + 1][j + 1].z);
                glVertex3d(v3.x, v3.y + floorHeight[i + 1][j + 1], v3.z);
                glNormal3d(floorNormal[i][j + 1].x, floorNormal[i][j + 1].y, floorNormal[i][j + 1].z);
                glVertex3d(v4.x, v4.y + floorHeight[i][j + 1], v4.z);
                glEnd();
            }
        }
    }
}

// マウスでなぞった線の描画
void drawLine() {
    int i, j;
    Vec_3D v1, v2, v3, v4;
    for (i = 0; i < TILE - 1; i++) {
        for (j = 0; j < TILE - 1; j++) {
            if (rake[i][j] == 1) {
                v1 = floorPoint[i][j];
                v2 = floorPoint[i + 1][j];
                v3 = floorPoint[i + 1][j + 1];
                v4 = floorPoint[i][j + 1];
                glBegin(GL_POLYGON);
                glNormal3d(floorNormal[i][j].x, floorNormal[i][j].y, floorNormal[i][j].z);
                glVertex3d(v1.x, v1.y + floorHeight[i][j], v1.z);
                glNormal3d(floorNormal[i + 1][j].x, floorNormal[i + 1][j].y, floorNormal[i + 1][j].z);
                glVertex3d(v2.x, v2.y + floorHeight[i + 1][j], v2.z);
                glNormal3d(floorNormal[i + 1][j + 1].x, floorNormal[i + 1][j + 1].y, floorNormal[i + 1][j + 1].z);
                glVertex3d(v3.x, v3.y + floorHeight[i + 1][j + 1], v3.z);
                glNormal3d(floorNormal[i][j + 1].x, floorNormal[i][j + 1].y, floorNormal[i][j + 1].z);
                glVertex3d(v4.x, v4.y + floorHeight[i][j + 1], v4.z);
                glEnd();
            }
        }
    }
}

// ランダムに小石を配置する関数
void drawRandomStone() {
    int i, j;
    for (i = 0; i < TILE; i++) {
        for (j = 0; j < TILE; j++) {
            if (rand() % 2 == 0) {
                stone[i][j] = 1;
            } else {
                stone[i][j] = 0;
            }
        }
    }
}

// ランダムに木を配置する関数
void drawRandomTree() {
    int i, j;
    for (i = 0; i < TILE; i++) {
        for (j = 0; j < TILE; j++) {
            if (rand() % 2 == 0) {
                tree[i][j] = 1;
            } else {
                tree[i][j] = 0;
            }
        }
    }
}

// 文字列を描画する関数
void drawString(char *str, int w, int h, int x0, int y0) {
    int i;
    glDisable(GL_LIGHTING); //ライティングの無効化
    glDisable(GL_DEPTH_TEST); //デプスバッファの無効化
    glMatrixMode(GL_PROJECTION); //行列モードの設定
    glPushMatrix(); //行列の保存
    glLoadIdentity(); //単位行列の設定
    gluOrtho2D(0, w, 0, h); //正射影の設定
    glMatrixMode(GL_MODELVIEW); //行列モードの設定
    glPushMatrix(); //行列の保存
    glLoadIdentity(); //単位行列の設定
    glColor3d(1.0, 1.0, 1.0); //色の設定
    glRasterPos2i(x0, y0); //ラスタ位置の設定
    for (i = 0; str[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]); //文字の描画
    }
    glPopMatrix(); //行列の復元
    glMatrixMode(GL_PROJECTION); //行列モードの設定
    glPopMatrix(); //行列の復元
    glMatrixMode(GL_MODELVIEW); //行列モードの設定
    glEnable(GL_DEPTH_TEST); //デプスバッファの有効化
    glEnable(GL_LIGHTING); //ライティングの有効化
}

