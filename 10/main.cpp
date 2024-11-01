//g++ main.cpp -framework OpenGL -framework GLUT -Wno-deprecated
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLUT/glut.h>  //OpenGL/GLUT

#define TILE 50  //床頂点数

//三次元ベクトル構造体: Vec_3D
typedef struct _Vec_3D
{
    double x, y, z;
} Vec_3D;

//関数名の宣言
void initGL();
void display();
void reshape(int w, int h);
void timer(int value);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void keyboard(unsigned char key, int x, int y);
Vec_3D normcrossprod(Vec_3D v1, Vec_3D v2);
Vec_3D crossProduct(Vec_3D v1, Vec_3D v2);
Vec_3D vectorNormalize(Vec_3D vec);
Vec_3D diffVec(Vec_3D v1, Vec_3D v2);
double vecLen(Vec_3D vec);

//グローバル変数
Vec_3D fPoint[TILE][TILE];  //床頂点
double fWidth;  //床長
double eDist, eDegX, eDegY;  //視点極座標
int winW, winH;  //ウィンドウサイズ
int mButton, mState, mX, mY;  //マウス情報
double rDisp = 1.0;  //retinaディスプレイ

//メイン関数
int main(int argc, char *argv[])
{
    glutInit(&argc, argv);  //OpenGL,GLUTの初期化
    
    initGL();  //初期設定
    
    glutMainLoop();  //イベント待ち無限ループ突入
    
    return 0;
}

//初期設定関数
void initGL()
{
    //ウィンドウ生成
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);  //ディスプレイ表示モード指定
    glutInitWindowSize(960, 600);  //ウィンドウサイズの指定（800×600）
    glutCreateWindow("CG");  //ウィンドウ生成
    
    //コールバック関数指定
    glutDisplayFunc(display);  //ディスプレイコールバック関数（"display"）
    glutReshapeFunc(reshape);  //リサイズコールバック関数（"reshape"）
    glutTimerFunc(33, timer, 0);  //タイマーコールバック関数（"timer", 33ミリ秒）
    glutMouseFunc(mouse);  //マウスクリックコールバック関数
    glutMotionFunc(motion);  //マウスドラッグコールバック関数
    glutKeyboardFunc(keyboard);  //キーボードコールバック関数
    
    //各種設定
    glClearColor(0.0, 0.0, 0.2, 1.0);  //ウィンドウクリア色の指定（RGBA値）
    glEnable(GL_DEPTH_TEST);  //デプスバッファ有効化
    glEnable(GL_NORMALIZE);  //ベクトル正規化有効化
    glEnable(GL_BLEND) ;  //ブレンド有効化
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  //ブレンド関数指定
    
    //光源
    GLfloat col[4];
    col[0] = 0.9; col[1] = 0.9; col[2] = 0.9; col[3] = 1.0;
    glEnable(GL_LIGHTING);
    //0
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, col);
    glLightfv(GL_LIGHT0, GL_SPECULAR, col);
    col[0] = 0.1; col[1] = 0.1; col[2] = 0.1; col[3] = 1.0;
    glLightfv(GL_LIGHT0, GL_AMBIENT, col);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0001);
    //1
    glEnable(GL_LIGHT1);
    col[0] = 0.9; col[1] = 0.9; col[2] = 0.9; col[3] = 1.0;
    glLightfv(GL_LIGHT1, GL_DIFFUSE, col);
    glLightfv(GL_LIGHT1, GL_SPECULAR, col);
    col[0] = 0.1; col[1] = 0.1; col[2] = 0.1; col[3] = 1.0;
    glLightfv(GL_LIGHT1, GL_AMBIENT, col);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.0001);

    //床頂点座標
    fWidth = 100.0;
    for (int j=0; j<TILE; j++) {
        for (int i=0; i<TILE; i++) {
            fPoint[i][j].x = -fWidth/2.0+i*fWidth/(TILE-1);
            fPoint[i][j].y = 0.0;
            fPoint[i][j].z = -fWidth/2.0+j*fWidth/(TILE-1);
        }
    }
    
    //視点極座標
    eDist = 120.0;  //距離
    eDegX = 15.0; eDegY = 0.0;  //x軸周り角度，y軸周り角度
}

//リシェイプコールバック関数(w：ウィンドウ幅，h：ウィンドウ高さ)
void reshape(int w, int h)
{
    glViewport(0, 0, w*rDisp, h*rDisp);  //ビューポート設定（ウィンドウ全体を表示領域に設定）
    
    //投影変換
    glMatrixMode(GL_PROJECTION);  //カレント行列の設定（投影変換行列）
    glLoadIdentity();  //カレント行列初期化
    gluPerspective(30.0, (double)w/(double)h, 1.0, 1000.0);  //透視投影のための投影変換行列の生成
    
    winW = w; winH = h;  //ウィンドウサイズをグローバル変数に格納
}

//ディスプレイコールバック関数
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  //ウィンドウクリア
    
    //ビューイング変換準備
    glMatrixMode(GL_MODELVIEW);  //カレント行列の設定（モデルビュー変換行列）
    glLoadIdentity();  //行列初期化
    //視点極座標から直交座標へ変換
    Vec_3D e;
    e.x = eDist*cos(eDegX*M_PI/180.0)*sin(eDegY*M_PI/180.0);
    e.y = eDist*sin(eDegX*M_PI/180.0);
    e.z = eDist*cos(eDegX*M_PI/180.0)*cos(eDegY*M_PI/180.0);
    //視点設定・ビューイング変換
    gluLookAt(e.x, e.y, e.z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);  //視点・注視点に基づくビューイング変換行列の生成
    
    //光源配置
    GLfloat lightPos0[] = {-20.0, 15.0, -25.0, 1.0};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
    GLfloat lightPos1[] = {25.0, 20.0, 40.0, 1.0};
    glLightfv(GL_LIGHT1, GL_POSITION, lightPos1);

    GLfloat col[4], spe[4], shi[1];
    
    //----------床パネル----------
    col[0] = 0.0; col[1] = 0.5; col[2] = 0.0; col[3] = 1.0;
    spe[0] = 1.0; spe[1] = 1.0; spe[2] = 1.0; spe[3] = 1.0;
    shi[0] = 50;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);
    glBegin(GL_QUADS);  //図形開始
    for (int j=0; j<TILE-1; j++) {
        for (int i=0; i<TILE-1; i++) {
            Vec_3D v1, v2, n;
            v1 = diffVec(fPoint[i][j+1], fPoint[i][j]);
            v2 = diffVec(fPoint[i+1][j], fPoint[i][j]);
            n = crossProduct(v1, v2);
            glNormal3d(n.x, n.y, n.z);
            glVertex3d(fPoint[i][j].x, fPoint[i][j].y, fPoint[i][j].z);  //頂点座標
            glVertex3d(fPoint[i][j+1].x, fPoint[i][j+1].y, fPoint[i][j+1].z);  //頂点座標
            glVertex3d(fPoint[i+1][j+1].x, fPoint[i+1][j+1].y, fPoint[i+1][j+1].z);  //頂点座標
            glVertex3d(fPoint[i+1][j].x, fPoint[i+1][j].y, fPoint[i+1][j].z);  //頂点座標
        }
    }
    glEnd();  //図形終了
    
    //物体1（長方形パネル1）
    glPushMatrix();  //行列一時保存
    col[0] = 1.0; col[1] = 1.0; col[2] = 1.0; col[3] = 0.5;  //RGBA
    spe[0] = 1.0; spe[1] = 1.0; spe[2] = 1.0; spe[3] = 1.0;
    shi[0] = 50;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);
    glTranslated(0.0, 10.0, -20.0);  //平行移動
    glScaled(20.0, 15.0, 1.0);  //拡大
    glNormal3d(0.0, 0.0, -1.0);  //法線
    glBegin(GL_QUADS);  //四角形
    glVertex3d(-0.5, 0.5, 0.0);  //頂点0
    glVertex3d(-0.5, -0.5, 0.0);  //頂点1
    glVertex3d(0.5, -0.5, 0.0);  //頂点2
    glVertex3d(0.5, 0.5, 0.0);  //頂点3
    glEnd();
    glPopMatrix();  //行列復帰
    
    //物体2（立方体）
    glPushMatrix();  //行列一時保存
    col[0] = 0.0; col[1] = 0.0; col[2] = 1.0; col[3] = 1.0;
    spe[0] = 1.0; spe[1] = 1.0; spe[2] = 1.0; spe[3] = 1.0;
    shi[0] = 50;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);
    glTranslated(0.0, 5.0, 0.0);  //平行移動
    glScaled(10.0, 10.0, 10.0);  //拡大
    glutSolidCube(1.0);  //立方体
    glPopMatrix();  //行列復帰

    //物体3（長方形パネル2）
    glPushMatrix();  //行列一時保存
    col[0] = 1.0; col[1] = 1.0; col[2] = 1.0; col[3] = 1.0;  //RGBA
    spe[0] = 1.0; spe[1] = 1.0; spe[2] = 1.0; spe[3] = 1.0;
    shi[0] = 50;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);
    glTranslated(0.0, 10.0, 20.0);  //平行移動
    glScaled(20.0, 15.0, 1.0);  //拡大
    glNormal3d(0.0, 0.0, 1.0);  //法線
    glBegin(GL_QUADS);  //四角形
    glVertex3d(-0.5, 0.5, 0.0);  //頂点0
    glVertex3d(-0.5, -0.5, 0.0);  //頂点1
    glVertex3d(0.5, -0.5, 0.0);  //頂点2
    glVertex3d(0.5, 0.5, 0.0);  //頂点3
    glEnd();
    glPopMatrix();  //行列復帰

    glutSwapBuffers();  //OpenGLの命令実行
}

//タイマーコールバック関数
void timer(int value)
{
    glutPostRedisplay();  //ディスプレイイベント強制発生
    glutTimerFunc(33, timer, 0);  //タイマー再設定
}

//マウスクリックコールバック関数
void mouse(int button, int state, int x, int y)
{
    //マウス情報をグローバル変数に格納
    mButton = button; mState = state; mX = x; mY = y;
}

//マウスドラッグコールバック関数
void motion(int x, int y)
{
    //左ボタンドラッグでチルト・パン変更
    if (mButton==GLUT_RIGHT_BUTTON){
        eDegY += (mX - x)*0.2;  //マウス横方向→水平角
        eDegX += (y - mY)*0.2;  //マウス縦方向→垂直角
    }
    //右ボタンドラッグでズーム変更
    else if (mButton==GLUT_LEFT_BUTTON){
        GLfloat winX, winY, winZ;  //ウィンドウ座標
        GLdouble objX, objY, objZ;  //ワールド座標
        GLdouble model[16], proj[16];  //モデルビュー変換行列，投影変換行列格納用
        GLint view[4];  //ビューポート設定格納用
        
        //マウス座標→ウィンドウ座標
        winX = x;
        winY = winH-y;
        glReadPixels(winX*rDisp, winY*rDisp, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
        
        //変換行列取り出し
        glGetDoublev(GL_MODELVIEW_MATRIX, model);  //モデルビュー変換行列格納
        glGetDoublev(GL_PROJECTION_MATRIX, proj);  //投影変換行列格納
        glGetIntegerv(GL_VIEWPORT, view);  //ビューポート設定格納
        for (int i=0; i<4; i++) {
            view[i] /= rDisp;
        }
        
        //ウィンドウ座標→CG空間座標（ワールド座標）（objX, objY, objZ）
        gluUnProject(winX, winY, winZ, model, proj, view, &objX, &objY, &objZ);
        
        double lenMax = 5.0;
        for (int j=0; j<TILE; j++) {
            for (int i=0; i<TILE; i++) {
                //マウスポインタが指すCG空間座標(objX, objY, objZ)と床面格子点(fPoint[i][j].x, fPoint[i][j].y, fPoint[i][j].z)との距離len
                double len = sqrt(pow(objX-fPoint[i][j].x,2.0)+pow(objY-fPoint[i][j].y,2.0)+pow(objZ-fPoint[i][j].z,2.0));
                //lenがlenMaxより小さい場合，(lenMax-len)に比例する値だけ床面格子点を盛り上げる => マウスポインタ点に近いほど盛り上がる量が大きい
                if (len<lenMax) {
                    fPoint[i][j].y += (lenMax-len)*0.05;
                }
            }
        }
    }
    
    //マウス座標をグローバル変数に保存
    mX = x; mY = y;
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
        case 27:  //[ESC]キー
            exit(0);  //プロセス終了
            break;
            
        case 'r':
            //床面の高さリセット
            for (int j=0; j<TILE; j++) {
                for (int i=0; i<TILE; i++) {
                    fPoint[i][j].y = 0.0;
                }
            }
            break;
            
        default:
            break;
    }
}

//ベクトルの差
Vec_3D diffVec(Vec_3D v1, Vec_3D v2)
{
    Vec_3D out;
    
    out.x = v1.x-v2.x;
    out.y = v1.y-v2.y;
    out.z = v1.z-v2.z;
    
    return out;
}

//ベクトルの長さ
double vecLen(Vec_3D vec)
{
    double len = sqrt(pow(vec.x,2)+pow(vec.y,2)+pow(vec.z,2));
    return len;
}

//v1とv2の外積計算
Vec_3D crossProduct(Vec_3D v1, Vec_3D v2)
{
    Vec_3D out;  //戻り値用
    
    //外積各成分の計算
    out.x = v1.y*v2.z-v1.z*v2.y;
    out.y = v1.z*v2.x-v1.x*v2.z;
    out.z = v1.x*v2.y-v1.y*v2.x;
    
    //正規化
    out = vectorNormalize(out);
    
    //戻り値
    return out;
}

//ベクトル正規化
Vec_3D vectorNormalize(Vec_3D vec)
{
    //ベクトル長
    double len = vecLen(vec);
    
    //各成分をベクトル長で割る
    if (len>0) {
        vec.x /= len;
        vec.y /= len;
        vec.z /= len;
    }
    
    //戻り値
    return vec;
}
