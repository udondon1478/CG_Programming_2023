#include <GLUT/glut.h>
#include <cmath>

float ballPosition[3] = {0.0f, 0.0f, 0.0f};  // ボールの位置
float mousePosition[3] = {0.0f, 0.0f, 0.0f};  // マウスの位置

void drawBall() {
    glColor3f(1.0f, 0.0f, 0.0f);  // 赤いボール
    glTranslatef(ballPosition[0], ballPosition[1], ballPosition[2]);
    glutSolidSphere(0.5, 20, 20);  // 半径0.5の球
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    drawBall();

    glutSwapBuffers();
}

void mouseMotion(int x, int y) {
    // マウス座標を正規化
    float normalizedX = (float)x / glutGet(GLUT_WINDOW_WIDTH);
    float normalizedY = 1.0f - (float)y / glutGet(GLUT_WINDOW_HEIGHT);

    // マウスの位置を3D座標に変換
    GLdouble modelview[16], projection[16];
    GLint viewport[4];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);
    gluUnProject(normalizedX * viewport[2], normalizedY * viewport[3], 0.5, modelview, projection, viewport, &mousePosition[0], &mousePosition[1], &mousePosition[2]);

    // 引力を計算し、ボールを移動
    float forceScale = 0.01f;  // 引力のスケール
    float force[3] = {
        (mousePosition[0] - ballPosition[0]) * forceScale,
        (mousePosition[1] - ballPosition[1]) * forceScale,
        (mousePosition[2] - ballPosition[2]) * forceScale
    };

    ballPosition[0] += force[0];
    ballPosition[1] += force[1];
    ballPosition[2] += force[2];

    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutCreateWindow("Rolling Ball with Attraction Force");
    glEnable(GL_DEPTH_TEST);

    glutDisplayFunc(display);
    glutMotionFunc(mouseMotion);

    glutMainLoop();
    return 0;
}
