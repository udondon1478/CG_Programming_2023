//g++ main.cpp -framework OpenGL -framework GLUT -Wno-deprecated
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLUT/glut.h>  //OpenGL/GLUT
#include <opencv2/opencv.hpp> //OpenCV
#include <time.h>


#define TILE 50  //床頂点数
#define FMAX 300 //データフレーム数
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
Vec_3D vectorNormalize(Vec_3D vec);

//グローバル変数
Vec_3D fPoint[TILE][TILE];  //床頂点
double fWidth = 5000.0;  //床長
double eDist, eDegX, eDegY;  //視点極座標
int winW, winH;  //ウィンドウサイズ
int mButton, mState, mX, mY;  //マウス情報
double rDisp = 1.0;  //retinaディスプレイ
double f = 30.0;  //フレームレート
Vec_3D jData[FMAX][15]; //座標データ
int fID = 0; //表示フレーム ID

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
    glutInitWindowSize(1024, 640);  //ウィンドウサイズの指定
    glutCreateWindow("CG-13");  //ウィンドウ生成
    
    //コールバック関数指定
    glutDisplayFunc(display);  //ディスプレイコールバック関数（"display"）
    glutReshapeFunc(reshape);  //リサイズコールバック関数（"reshape"）
    glutTimerFunc(1000/f, timer, 0);  //タイマーコールバック関数（"timer", 1000/fミリ秒）
    glutMouseFunc(mouse);  //マウスクリックコールバック関数
    glutMotionFunc(motion);  //マウスドラッグコールバック関数
    glutKeyboardFunc(keyboard);  //キーボードコールバック関数
    
    //各種設定
    glClearColor(0.0, 0.0, 0.2, 1.0);  //ウィンドウクリア色の指定（RGBA値）
    glEnable(GL_DEPTH_TEST);  //デプスバッファ有効化
    glEnable(GL_NORMALIZE);  //ベクトル正規化有効化
    glEnable(GL_BLEND);  //ブレンディング有効化
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  //ブレンディング方法指定（アルファブレンディング）
    
    //陰影付け・光源
    glEnable(GL_LIGHTING);  //陰影付け有効化
    GLfloat col[4];  //光源設定用配列
    //光源0
    glEnable(GL_LIGHT0);  //光源0有効化
    col[0] = 0.8; col[1] = 0.8; col[2] = 0.8; col[3] = 1.0;
    glLightfv(GL_LIGHT0, GL_DIFFUSE, col);  //拡散反射対象
    glLightfv(GL_LIGHT0, GL_SPECULAR, col);  //鏡面反射対象
    col[0] = 0.2; col[1] = 0.2; col[2] = 0.2; col[3] = 1.0;
    glLightfv(GL_LIGHT0, GL_AMBIENT, col);  //環境光対象
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0000001);  //減衰率
    
    //視点極座標
    eDist = 7000.0;  //距離
    eDegX = 20.0; eDegY = 180.0;  //x軸周り角度，y軸周り角度
    
    //床頂点座標
    for (int j=0; j<TILE; j++) {
        for (int i=0; i<TILE; i++) {
            fPoint[i][j].x = -fWidth/2.0+i*fWidth/(TILE-1);
            fPoint[i][j].y = -800.0;
            fPoint[i][j].z = -fWidth/2.0+j*fWidth/(TILE-1);
        }
    }
    
    //座標データ読み込み
     FILE *fp;
     fp = fopen("pdata.txt", "r"); //ファイルオープン
    if(fp == NULL){
        printf("NO move\n");
        exit(0);
    }
        for (int j=0; j<FMAX; j++) { //FMAX フレーム分
                for (int i=0; i<15; i++) { //15 カ所の座標
                        //ファイルから j フレーム目の座標 i の読み込み
                        fscanf(fp, "%lf,%lf,%lf\n", &jData[j][i].x, &jData[j][i].y, &jData[j][i].z);
                        //表示用に z 座標を補正
                        jData[j][i].z -= 2000.0;
                        printf("%f, %f, %f\n", jData[j][i].x, jData[j][i].y, jData[j][i].z);
                    }
                printf("--------------------------\n");
            }
            fclose(fp);
    //座標データ読み込み


    //テクスチャ準備
cv::Mat textureImage; //テクスチャ画像格納用
//テクスチャ 0 作成
textureImage = cv::imread("image1.jpg", cv::IMREAD_UNCHANGED); //画像読み込み 
glBindTexture(GL_TEXTURE_2D, 0); //テクスチャ#0
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //テクスチャ拡大時の補間方法 
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //テクスチャ縮小時の補間方法 
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureImage.cols, textureImage.rows, 0, GL_BGR,GL_UNSIGNED_BYTE, textureImage.data); //画像をテクスチャに割り当て} 



//テクスチャ １ 作成
textureImage = cv::imread("image2.png", cv::IMREAD_UNCHANGED); //画像読み込み
glBindTexture(GL_TEXTURE_2D, 1); //テクスチャ#0
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //テクスチャ拡大時の補間方法 
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //テクスチャ縮小時の補間方法 
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureImage.cols, textureImage.rows, 0, GL_BGRA,GL_UNSIGNED_BYTE, textureImage.data); //画像をテクスチャに割り当て}

}

//リシェイプコールバック関数(w：ウィンドウ幅，h：ウィンドウ高さ)
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);  //ウィンドウ内の描画領域(ビューポート)の指定

    //投影変換
    glMatrixMode(GL_PROJECTION);  //カレント行列の設定
    glLoadIdentity();  //カレント行列初期化
    gluPerspective(30.0, (double)w/(double)h, 1.0, 20000.0);  //投影変換行列生成
    
    winW = w; winH = h;  //ウィンドウサイズをグローバル変数に格納
}

//ディスプレイコールバック関数
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  //ウィンドウクリア
    
    //ビューイング変換準備
    glMatrixMode(GL_MODELVIEW);  //カレント行列の設定
    glLoadIdentity();  //行列初期化
    //視点極座標から直交座標へ変換
    Vec_3D e;
    e.x = eDist*cos(eDegX*M_PI/180.0)*sin(eDegY*M_PI/180.0);
    e.y = eDist*sin(eDegX*M_PI/180.0);
    e.z = eDist*cos(eDegX*M_PI/180.0)*cos(eDegY*M_PI/180.0);
    //視点設定・ビューイング変換
    gluLookAt(e.x, e.y, e.z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);  //ビューイング変換行列生成
    
    //光源配置
    GLfloat lightPos0[] = {200.0, 2000.0, -200.0, 1.0};  //光源座標(点光源)
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);  //光源配置
    
    //質感用配列
    GLfloat col[4], spe[4], shi[1];
    
    
    //----------床パネル----------
    col[0] = 0.2; col[1] = 1.0; col[2] = 0.2; col[3] = 1.0;
    spe[0] = 1.0; spe[1] = 1.0; spe[2] = 1.0; spe[3] = 1.0;
    shi[0] = 50;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);
    glBegin(GL_QUADS);  //図形開始
    for (int j=0; j<TILE-1; j++) {
        for (int i=0; i<TILE-1; i++) {
            //法線ベクトル計算
            Vec_3D v1, v2, n;
            v1 = diffVec(fPoint[i][j+1], fPoint[i][j]);
            v2 = diffVec(fPoint[i+1][j], fPoint[i][j]);
            n = crossProduct(v1, v2);  //法線ベクトル
            glNormal3d(n.x, n.y, n.z);  //法線ベクトル適用
            //頂点座標
            glVertex3d(fPoint[i][j].x, fPoint[i][j].y, fPoint[i][j].z);  //頂点座標
            glVertex3d(fPoint[i][j+1].x, fPoint[i][j+1].y, fPoint[i][j+1].z);  //頂点座標
            glVertex3d(fPoint[i+1][j+1].x, fPoint[i+1][j+1].y, fPoint[i+1][j+1].z);  //頂点座標
            glVertex3d(fPoint[i+1][j].x, fPoint[i+1][j].y, fPoint[i+1][j].z);  //頂点座標
        }
    }
    glEnd();  //図形終了
    
    /*
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

    glDepthMask(GL_FALSE);
    
    //物体1（長方形パネル1）
    glEnable(GL_TEXTURE_2D); //テクスチャ有効化 
    glBindTexture(GL_TEXTURE_2D, 0); //テクスチャ指定(#0)
    glPushMatrix();  //行列一時保存
    col[0] = 1.0; col[1] = 1.0; col[2] = 1.0; col[3] = 1.0;  //RGBA
    spe[0] = 1.0; spe[1] = 1.0; spe[2] = 1.0; spe[3] = 1.0;
    shi[0] = 50;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);
    glTranslated(0.0, 10.0, -20.0);  //平行移動
    glScaled(20.0, 15.0, 1.0);  //拡大
    glNormal3d(0.0, 0.0, -1.0);  //法線
    glBegin(GL_QUADS);  //四角形
    glTexCoord2d(0.0, 0.0); //テクスチャ座標0
    glVertex3d(-0.5, 0.5, 0.0);  //頂点0
    glTexCoord2d(0.0, 1.0); //テクスチャ座標1
    glVertex3d(-0.5, -0.5, 0.0);  //頂点1
    glTexCoord2d(1.0, 1.0); //テクスチャ座標2
    glVertex3d(0.5, -0.5, 0.0);  //頂点2
    glTexCoord2d(1.0, 0.0); //テクスチャ座標3
    glVertex3d(0.5, 0.5, 0.0);  //頂点3
    glEnd();
    glPopMatrix();  //行列復帰
    glDisable(GL_TEXTURE_2D); //テクスチャ無効化
    
    

    //物体3（長方形パネル2）
    glEnable(GL_ALPHA_TEST); //アルファテスト有効化
    glEnable(GL_TEXTURE_2D); //テクスチャ有効化 
    glBindTexture(GL_TEXTURE_2D, 1); //テクスチャオブジェクト指定(#1)
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
    glTexCoord2d(0.0, 0.0); //テクスチャ座標0
    glVertex3d(-0.5, 0.5, 0.0);  //頂点0
    glTexCoord2d(0.0, 1.0); //テクスチャ座標0
    glVertex3d(-0.5, -0.5, 0.0);  //頂点1
    glTexCoord2d(1.0, 1.0); //テクスチャ座標0
    glVertex3d(0.5, -0.5, 0.0);  //頂点2
    glTexCoord2d(1.0, 0.0); //テクスチャ座標0
    glVertex3d(0.5, 0.5, 0.0);  //頂点3
    glEnd();
    glPopMatrix();  //行列復帰
    glDisable(GL_TEXTURE_2D); //テクスチャ無効化
    glDisable(GL_ALPHA_TEST); //アルファテスト無効化

    glDepthMask(GL_TRUE);
     */
    
    //----------座標点----------
    //----------座標点----------
     col[0] = 1.0; col[1] = 1.0; col[2] = 0.0; col[3] = 1.0;
     spe[0] = 1.0; spe[1] = 1.0; spe[2] = 1.0; spe[3] = 1.0;
     shi[0] = 50;
     glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
     glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);
     glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);
     for (int i=0; i<15; i++) {
         /*
     //fID フレーム目の i 番目の三次元座標 jData[fID][i]に球を表示
     glPushMatrix();
     glTranslated(jData[fID][i].x, jData[fID][i].y, jData[fID][i].z-0.0);
     glScaled(0.1, 0.1, 0.1);
     glutSolidSphere(1.0, 1, 1);
     glPopMatrix();
          */
     }
  
    
    
    // Draw joints
        GLfloat jointCol[4], jointSpe[4], jointShi[1];
        jointCol[0] = 1.0; jointCol[1] = 1.0; jointCol[2] = 0.0; jointCol[3] = 1.0;
        jointSpe[0] = 1.0; jointSpe[1] = 1.0; jointSpe[2] = 1.0; jointSpe[3] = 1.0;
        jointShi[0] = 50;
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, jointCol);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, jointSpe);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, jointShi);

        for (int i = 0; i < 15; i++) {
            glPushMatrix();
            glTranslated(jData[fID][i].x, jData[fID][i].y, jData[fID][i].z - 0.0);
            glScaled(0.0, 0.0, 0.0);
            glutSolidSphere(1.0, 36, 18);
            glPopMatrix();
        }
    
    
      glPushMatrix();
      glTranslated(jData[fID][0].x, jData[fID][0].y, jData[fID][0].z-0.0);
      glScaled(60, 60, 60);
      glutSolidSphere(1.0, 36, 18);
      glPopMatrix();
      
      glPushMatrix();
      glTranslated(jData[fID][8].x, jData[fID][8].y, jData[fID][8].z-0.0);
      glScaled(30, 30, 30);
      glutSolidSphere(1.0, 36, 18);
      glPopMatrix();
      
      glPushMatrix();
      glTranslated(jData[fID][5].x, jData[fID][5].y, jData[fID][5].z-0.0);
      glScaled(30, 30, 30);
      glutSolidSphere(1.0, 36, 18);
      glPopMatrix();
      
      glPushMatrix();
      glTranslated(jData[fID][14].x, jData[fID][14].y, jData[fID][14].z-0.0);
      glScaled(30, 30, 30);
      glutSolidSphere(1.0, 36, 18);
      glPopMatrix();
      
      glPushMatrix();
      glTranslated(jData[fID][11].x, jData[fID][11].y, jData[fID][11].z-0.0);
      glScaled(30, 30, 30);
      glutSolidSphere(1.0, 36, 18);
      glPopMatrix();
      

        // Connect spheres 0 and 9 with a line
        glBegin(GL_LINES);
        glVertex3d(jData[fID][0].x, jData[fID][0].y, jData[fID][0].z - 0.0);
        glVertex3d(jData[fID][1].x, jData[fID][1].y, jData[fID][1].z - 0.0);
        glEnd();
    //1-3
    glBegin(GL_LINES);
    glVertex3d(jData[fID][1].x, jData[fID][1].y, jData[fID][1].z );
    glVertex3d(jData[fID][3].x, jData[fID][3].y, jData[fID][3].z );
    glEnd();
    
    //1-6
    glBegin(GL_LINES);
    glVertex3d(jData[fID][1].x, jData[fID][1].y, jData[fID][1].z);
    glVertex3d(jData[fID][6].x, jData[fID][6].y, jData[fID][6].z);
    glEnd();
    
    //3-4
    glBegin(GL_LINES);
    glVertex3d(jData[fID][3].x, jData[fID][3].y, jData[fID][3].z);
    glVertex3d(jData[fID][4].x, jData[fID][4].y, jData[fID][4].z - 0);
    glEnd();
    
    //6-7
    glBegin(GL_LINES);
    glVertex3d(jData[fID][6].x, jData[fID][6].y, jData[fID][6].z );
    glVertex3d(jData[fID][7].x, jData[fID][7].y, jData[fID][8].z - 0.0);
    glEnd();
    
    //4-5
    glBegin(GL_LINES);
    glVertex3d(jData[fID][4].x, jData[fID][4].y, jData[fID][4].z - 0.0);
    glVertex3d(jData[fID][5].x, jData[fID][5].y, jData[fID][5].z - 0.0);
    glEnd();
    
    //7-8
    glBegin(GL_LINES);
    glVertex3d(jData[fID][7].x, jData[fID][7].y, jData[fID][7].z - 0.0);
    glVertex3d(jData[fID][8].x, jData[fID][8].y, jData[fID][8].z - 0.0);
    glEnd();
    
    
    
    //9-10
    glBegin(GL_LINES);
    glVertex3d(jData[fID][9].x, jData[fID][9].y, jData[fID][9].z - 0.0);
    glVertex3d(jData[fID][10].x, jData[fID][10].y, jData[fID][10].z - 0.0);
    glEnd();
    
    //10-11
    glBegin(GL_LINES);
    glVertex3d(jData[fID][10].x, jData[fID][10].y, jData[fID][10].z - 0.0);
    glVertex3d(jData[fID][11].x, jData[fID][11].y, jData[fID][11].z - 0.0);
    glEnd();
    
    //12-13
    glBegin(GL_LINES);
    glVertex3d(jData[fID][12].x, jData[fID][12].y, jData[fID][12].z - 0.0);
    glVertex3d(jData[fID][13].x, jData[fID][13].y, jData[fID][13].z - 0.0);
    glEnd();
    
    //13-14
    glBegin(GL_LINES);
    glVertex3d(jData[fID][13].x, jData[fID][13].y, jData[fID][13].z - 0.0);
    glVertex3d(jData[fID][14].x, jData[fID][14].y, jData[fID][14].z - 0.0);
    glEnd();
    //物体3（長方形パネル2）
    glEnable(GL_ALPHA_TEST); //アルファテスト有効化
    glEnable(GL_TEXTURE_2D); //テクスチャ有効化
    glBindTexture(GL_TEXTURE_2D, 1); //テクスチャオブジェクト指定(#1)
    glPushMatrix();  //行列一時保存
    col[0] = 1.0; col[1] = 1.0; col[2] = 1.0; col[3] = 1.0;  //RGBA
    spe[0] = 1.0; spe[1] = 1.0; spe[2] = 1.0; spe[3] = 1.0;
    shi[0] = 50;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);
   
    glNormal3d(0.0, 0.0, 1.0);  //法線
    glBegin(GL_QUADS);  //四角形
    
    glTexCoord2d(0.0, 0.0); //テクスチャ座標0
    glVertex3d(jData[fID][6].x, jData[fID][6].y, jData[fID][6].z - 0.0);
    glTexCoord2d(0.0, 1.0); //テクスチャ座標0
    glVertex3d(jData[fID][12].x, jData[fID][12].y, jData[fID][12].z - 0.0);
    glTexCoord2d(1.0, 1.0); //テクスチャ座標0
    glVertex3d(jData[fID][9].x, jData[fID][9].y, jData[fID][13].z - 0.0);
    glTexCoord2d(1.0, 0.0); //テクスチャ座標0
    glVertex3d(jData[fID][3].x, jData[fID][3].y, jData[fID][3].z - 0.0);
    glEnd();
    glPopMatrix();  //行列復帰
    glDisable(GL_TEXTURE_2D); //テクスチャ無効化
    glDisable(GL_ALPHA_TEST); //アルファテスト無効化

    glDepthMask(GL_TRUE);
    
    

    glutSwapBuffers();  //OpenGLの命令実行
}

//タイマーコールバック関数
void timer(int value)
{
    fID = (fID + 1) % FMAX;  // フレームIDを増やし、必要に応じてラップアラウンド
    glutPostRedisplay();  //ディスプレイイベント強制発生
    glutTimerFunc(1000/f, timer, 0);  //タイマー再設定
}

//マウスクリックコールバック関数
void mouse(int button, int state, int x, int y)
{
    GLfloat winX, winY, winZ;  //ウィンドウ座標
    GLdouble objX, objY, objZ;  //ワールド座標
    GLdouble model[16], proj[16];  //モデルビュー変換行列，投影変換行列格納用
    GLint view[4];  //ビューポート設定格納用
    
    //マウス座標→ウィンドウ座標
    winX = x;
    winY = winH-y;
    glReadPixels(winX, winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
    
    //変換行列取り出し
    glGetDoublev(GL_MODELVIEW_MATRIX, model);  //モデルビュー変換行列格納
    glGetDoublev(GL_PROJECTION_MATRIX, proj);  //投影変換行列格納
    glGetIntegerv(GL_VIEWPORT, view);  //ビューポート設定格納
    
    //ウィンドウ座標→ワールド座標（objX, objY, objZ）
    gluUnProject(winX, winY, winZ, model, proj, view, &objX, &objY, &objZ);
    
    //マウス情報をグローバル変数に格納
    mButton = button; mState = state; mX = x; mY = y;
}

//マウスドラッグコールバック関数
void motion(int x, int y)
{
    //左ドラッグで変更
    if(mButton == GLUT_LEFT_BUTTON){
        eDegY += (mX-x)*0.2;  //マウス横方向→水平角
        eDegX += (y-mY)*0.2;  //マウス縦方向→垂直角
        if (eDegX>85) eDegX = 85.0;
        if (eDegX<-85) eDegX = -85.0;
    }else if(mButton == GLUT_RIGHT_BUTTON){
        eDist += (y-mY)*10;  //マウス縦方向→Zoom In/Out
    }

    //マウス座標をグローバル変数に保存
    mX = x; mY = y;
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
        case 27:  //[ESC]キー
        case 'q':  //[q]キー
        case 'Q':  //[Q]キー
            exit(0);  //プロセス終了
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
