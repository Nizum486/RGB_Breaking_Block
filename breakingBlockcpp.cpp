#include <iostream>
#include <windows.h>
#include <gl/gl.h>
#include <gl/glut.h>
#include <ctime>
#include <mmsystem.h>
#include <Digitalv.h>
#include <time.h>
#pragma comment(lib, "winmm.lib")

MCI_OPEN_PARMS openBgm;
MCI_PLAY_PARMS playBgm;
MCI_OPEN_PARMS openShuffleSound;
MCI_PLAY_PARMS playShuffleSound;

#define    width             505 //505
#define    height            600
#define    PI                3.1415
#define    polygon_num        50

#define BGM "./Aurea Carmina - Kevin MacLeod.mp3"
#define HIT "./Hit.mp3"

using namespace std;

// �� ��ȣ
int     map = 0;

// ���� ���� 3����
int     random_num1;
int     random_num2;
int     random_num3;

// ȭ�� ����, �Ʒ���
int     _left = 0;
int     _bottom = 0;

bool    isStart = false;
bool    gameOver = false;
bool    isClear = false;
bool    print_one = false;
bool    allClear = false;
bool    check[7] = { false, false, false, false, false, false, false };

// ��ƽ �̵� �Ÿ�(10), ��ƽ ��ǥ x, y
float    stick_move = 10;
float    stick_x = 0.0;
float    stick_y = 30.0;

// ���� ù x, y ��ǥ
float   block_x = 5.0;
float   block_y = 580.0;

clock_t finish;
double duration;

int dwID;

// ������� �۵� �Լ�
void playingBgm(void) { 
    openBgm.lpstrElementName = TEXT(BGM); // ���� ��� �Է�
    openBgm.lpstrDeviceType = TEXT("mpegvideo"); //mp3 ����
    mciSendCommand(0, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE, (DWORD)(LPVOID)&openBgm);
    dwID = openBgm.wDeviceID;
    mciSendCommand(dwID, MCI_PLAY, MCI_DGV_PLAY_REPEAT, (DWORD)(LPVOID)&openBgm); //���� �ݺ� ���
}

// �浹 ȿ���� �۵� �Լ�
void playingHitSound(void) {
    openShuffleSound.lpstrElementName = TEXT(HIT); //���� ����
    openShuffleSound.lpstrDeviceType = TEXT("mpegvideo"); //mp3 ����
    mciSendCommand(0, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE, (DWORD)(LPVOID)&openShuffleSound);
    dwID = openShuffleSound.wDeviceID;
    mciSendCommand(dwID, MCI_PLAY, MCI_NOTIFY, (DWORD)(LPVOID)&openShuffleSound); //������ �� �� ���
    Sleep(50); //sleep()�̱� ������ ��� ��ĩ�Ÿ��� ����
    mciSendCommand(dwID, MCI_SEEK, MCI_SEEK_TO_START, (DWORD)(LPVOID)NULL); //�����ġ�� �ʱ���ġ�� ����
}


// �� Ŭ����
class Block {

public:
    float x;                      // ���� x ��ǥ ( ���� �ϴ�)
    float y;                      // ���� y ��ǥ ( ���� �ϴ�)
    bool exist = false;           // ���� ���� ����

    float block_width = 45;       // ���� ��
    float block_height = 25;      // ���� ����

    Block() {}                    // �⺻ ������

    Block(float b_x, float b_y) { // ������

        x = b_x;
        y = b_y;

    }

    void draw_block() {             // �� �׸��� �Լ�
        if (exist) {
            glBegin(GL_POLYGON);
            glVertex2f(x, y);
            glVertex2f(x + block_width, y);
            glVertex2f(x + block_width, y - block_height);
            glVertex2f(x, y - block_height);
            glEnd();
        }
    }

};

// 50�� �� ����
Block block[50];

// �� Ŭ����
class Ball {

public:
    float x;            // ���� x ��ǥ
    float y;            // ���� y ��ǥ
    float radius;       // ���� ������
    float velocity_x;   // ���� x�������� �ӵ�
    float velocity_y;   // ���� y�������� �ӵ�

    Ball(float b_x, float b_y, float b_radius, float velocity) {    // Ball�� ������
        x = b_x;
        y = b_y;
        radius = b_radius;
        velocity_x = velocity;
        velocity_y = velocity;
    }

    // ���� �׸��� �Լ�
    void    Draw_ball(float radius, float x, float y) {             
        float    delta;
        delta = 2 * PI / polygon_num;               // polygon_num = 50
        glBegin(GL_POLYGON);
        for (int i = 0; i < polygon_num; i++)       // �� (x,y) �� �������� 50���� ���� ����
            glVertex2f(x + radius * cos(delta * i), y + radius * sin(delta * i));
        glEnd();
    }

    // ���� ���� �浹 - ���� ��
    void    Main_ball_Detection_to_Wall(void) {
        if (x - radius <= _left || x + radius >= _left + width) {  // �� �� �浹
            velocity_x *= -1.0;                                    // ���Ǽӵ�(x) �ݴ����
        }
        if (y + radius >= _bottom + height) {                      // ���� �浹
            velocity_y *= -1.0;                                    // ���Ǽӵ�(y) �ݴ����
        }
        if (y - radius <= _bottom) {                               // �Ʒ��� �浹
            velocity_y *= -1.0;
            gameOver = true;                                       // ���� ����!
        }
    }

    // ���� ���� �浹 - ���� ��
    void    Collision_Detection_to_Walls(void) {

        if (x - radius <= _left || x + radius >= _left + width) {       // �� �� �浹 
            velocity_x *= -1.0;                                         // ���Ǽӵ�(x) �ݴ����
        }
        if (y - radius <= _bottom || y + radius >= _bottom + height) {  // �� �� �浹
            velocity_y *= -1.0;                                         // ���Ǽӵ�(y) �ݴ����
        }
                                                                        // ���꺼�� ���ӿ��� X
    }


    // ���� ��ƽ�� �浹
    void    Collision_Detection_to_Stick(void) {

        // ���� �浹
        if (stick_x < x && x < stick_x && stick_y > y - radius) {
            velocity_y *= -1.0;
        }

        // �Ʒ��� �浹
        else if (stick_x < x && x < stick_x + 90 && stick_y - 20 < y + radius && y < stick_y + radius) {
            velocity_y *= -1.0;
        }

        // ������ �浹
        else if (stick_x + 90 > x - radius && x > stick_x - radius && stick_y > y && y > stick_y - 20) {
            velocity_y *= -1.0;
        }

        // ���� �浹
        else if (stick_x < x + radius && x < stick_x + radius && stick_y > y && y > stick_y - 20) {
            velocity_y *= -1.0;
        }


    }

    // ���� ���� �浹
    void    Collision_Detection_to_Block(void) {

        bool hit = false;

        for (int i = 0; i < 50; i++) {

            // �Ʒ��� �浹
            if (block[i].x < x && x < block[i].x + 45 && block[i].y - 25 < y + radius && y < block[i].y + radius) {
                velocity_y *= -1.0;         // ���� �ӵ�(y) �ݴ� ����
                block[i].exist = false;     // ���� true -> false
                hit = true;                 // �浹! -> ���� ���
                block[i].x -= 1000;         // ���� exist�� false����, �׸����� ���� �� ���� �װ��� ����
                block[i].y -= 1000;         // ���� ���� �ٸ� ������ �Ű� ������ �ǰڴٶ�� �Ǵ�
                                            // + �� ������� �Ұ�� �𼭸� �浹���� ������ ���� ����
                                            // + ������ ���ɸ鿡�� ���� �ʴٰ� ������
            }

            // ���� �浹 (= �Ʒ��ʰ� ����)
            else if (block[i].x < x && x < block[i].x && block[i].y > y - radius) {
                velocity_y *= -1.0;
                block[i].exist = false;
                hit = true;
                block[i].x -= 1000;
                block[i].y -= 1000;
            }

            // ������ �浹
            else if (block[i].x + 45 > x - radius && x > block[i].x - radius && block[i].y > y && y > block[i].y - 25) {
                velocity_x *= -1.0;     // ���� �ӵ�(x) �ݴ����
                block[i].exist = false;
                hit = true;
                block[i].x -= 1000;     // ���� �ָ� ����������
                block[i].y -= 1000;
            }

            // ���� �浹 (= �����ʰ� ����)
            else if (block[i].x < x + radius && x < block[i].x + radius && block[i].y > y && y > block[i].y - 25) {
                velocity_x *= -1.0;
                block[i].exist = false;
                hit = true;
                block[i].x -= 1000; 
                block[i].y -= 1000;
            }

        }

        if (hit) {
            playingHitSound();          // �� �浹�� �浹 ȿ���� ���
        }
    }

};

// 4���� �� ����
Ball moving_ball(width / 2, height / 4, 10, 0.2);    // ���� �ʱ� ��ġ x, y, radius, ���� �ӵ�(x,y �Ѵ�)
Ball second_ball(width / 4, height / 4, 5, 0.22);
Ball third_ball(width / 6, height / 4, 5, 0.23);
Ball fourth_ball(width / 8, height / 4, 5, 0.24);

// ���� �Լ�
void reset_Rotation(void) {

    srand(time(0));             // ���� ���� 3�� �� �ʱ�ȭ (�缳��)
    random_num1 = rand() % 50;
    random_num2 = rand() % 50;
    random_num3 = rand() % 50;
  

    moving_ball.x = width / 2;  // ���� �� ��ġ �ʱ�ȭ
    moving_ball.y = height / 4;

    second_ball.x = width / 4;  // ���� �� 1 ��ġ �ʱ�ȭ
    second_ball.y = height / 4;

    third_ball.x = width / 6;   // ���� �� 2 ��ġ �ʱ�ȭ
    third_ball.y = height / 4;

    fourth_ball.x = width / 8;  // ���� �� 3 ��ġ �ʱ�ȭ
    fourth_ball.y = height / 4;

    if (moving_ball.velocity_y < 0) moving_ball.velocity_y *= -1.0; // ���� �Ʒ����� ���Ѵٸ� �������� ���ϰ� ��

}

// �� ��ġ ��� ����?
// ��1 �� 50��
void block_basic(void) {

    map = 1;                        // block_basic�� 1����

    float   block_x = 5.0;          // ù ���� x��ǥ
    float   block_y = 580.0;        // ù ���� y��ǥ

    int row = 0;                    // �� ��ȯ�� ���� ���� row ����

    for (int i = 0; i < 50; i++) {  

        if (i % 10 == 0) {          // 10���� ���� �������
            row = 0;                // ���� �ʱ� ��ġ��
            block_y -= 30;          // �� y�� ��ġ�� ��ĭ(30) ����
        }

        block[i].x = block_x + 50 * row;    // ���� x ��ǥ�� ��ĭ ��ﶧ���� 50�� ���
        block[i].y = block_y;               // ������ �࿡�ִ� ������ y��ǥ�� �Ȱ���.

        block[i].exist = true;              // ���� �׸��鼭 exist���� true�� ��

        row++;                              // �� ĭ �׸������� ������ ��ĭ�� �ű�
    }

}

// ��2 �� 45��
void block_stair(void) {

    map = 2;                            // block_stair�� 2����

    float   block_x = 5.0;              // ù ���� x ��ǥ
    float   block_y = 580.0;            // ù ���� y ��ǥ

    int frefix_sum[9] = { 0, };         // ���� �ٲܰ��� ã�� ���� ������� �κ����� �̿��ϱ�� ��
    int sum = 0;
    for (int i = 1; i < 10; i++) {      // �κ��� ���
        frefix_sum[i - 1] = sum;
        sum += i;                       // �� ������ ���ؼ�
    }                                   // frefix_sum���� [1, 3, 6, 10, 15, 21, 28, 36, 45]�� ����Ǿ� ����

    int num = 0;
    for (int i = 0; i < 50; i++) {      // �� ���� ���� 50�� ���� �ݺ�
        for (int j = 0; j < 9; j++) {   // �κ��� �迭�� frefix_sum�� Ž���ϱ� ���� 2�� for��
            if (i == frefix_sum[j]) {   // 50���� �� �����鼭, i�� �κ��� �迭�϶�
                num = 0;                // ���� ù��°�� ���ư�
                block_y -= 30;          // ���� ��ĭ(30) ����
            }
        }

        block[i].x = block_x + 50 * num;    // �� �� �׸������� num * 50�� �ű�
        block[i].y = block_y;               // ������ �࿡ �ִ� ���� y��ǥ�� ����

        num++;                              // �� �� �׸��� ���� ������ ��ĭ�� �ű�
    }

    for (int i = 0; i < 45; i++) {          // ���� 45�� ���� �����ϰ� ����
        block[i].exist = true;
    }
    for (int i = 45; i < 50; i++) {         // ���� �� ó���� �� �浹���� ó�� �ߴ� ��İ� ����ϰ�
        block[i].x = 5000;                  // ���� �ָ� ��������
        block[i].y = 5000;
        block[i].exist = false;             // �׸��� �͵� false�� ��
    }

}

// ��3 �� 40��
void block_heart(void) {

    map = 3;                        // block_heart�� 3�� ���Դϴ�.

    float   block_x = 5.0;          // ù ���� x ��ǥ
    float   block_y = 550.0;        // ù ���� y ��ǥ

                                    // �˰��� ����(���� �����)ó�� �����Ϸ��� �غ�������
                                    // �ᱹ���� ���� �ϳ��� ���� ����

    // ù��
    block[0].x = block_x + 100;     
    block[1].x = block_x + 150;
    block[2].x = block_x + 300;
    block[3].x = block_x + 350;

    for (int i = 0; i < 4; i++) {
        block[i].y = block_y;
    }

    //2,3,4��
    float bb;
    bb = block_x + 50;
    for (int i = 4; i < 12; i++) {
        block[i].x = bb;
        block[i].y = block_y - 30;
        bb += 50;
    }
    bb = block_x + 50;
    for (int i = 12; i < 20; i++) {
        block[i].x = bb;
        block[i].y = block_y - 60;
        bb += 50;
    }
    bb = block_x + 50;
    for (int i = 20; i < 28; i++) {
        block[i].x = bb;
        block[i].y = block_y - 90;
        bb += 50;
    }

    //5��
    float cc;
    cc = block_x + 100;
    for (int i = 28; i < 34; i++) {
        block[i].x = cc;
        block[i].y = block_y - 120;
        cc += 50;
    }

    //6��
    float dd;
    dd = block_x + 150;
    for (int i = 34; i < 38; i++) {
        block[i].x = dd;
        block[i].y = block_y - 150;
        dd += 50;
    }

    //7��(��)
    block[38].x = block_x + 200;
    block[38].y = block_y - 180;

    block[39].x = block_x + 250;
    block[39].y = block_y - 180;

    for (int i = 0; i < 40; i++) { // ���� 40�� ������ �����ϰ� ����
        block[i].exist = true;
    }


    for (int i = 40; i < 50; i++) { // ���� �� ó���� blcok_stair�� ����
        block[i].x = 5000;
        block[i].y = 5000;
        block[i].exist = false;
    }
}

// ��4 �� 26��
void block_face(void) {

    map = 4;                        // block_face�� 4�� ���Դϴ�.
                                    // heart�� �����ϰ� �˰����� �ϳ��ϳ��� ������ϴ�.
    // ù��
    float aa;
    aa = block_x + 150;
    for (int i = 0; i < 4; i++) {
        block[i].x = aa;
        block[i].y = block_y - 30;
        aa += 50;
    }

    // ��° ��
    block[4].x = block_x + 100;
    block[4].y = block_y - 60;

    block[5].x = block_x + 350;
    block[5].y = block_y - 60;

    // ����° ��
    block[6].x = block_x + 50;
    block[6].y = block_y - 90;

    block[7].x = block_x + 400;
    block[7].y = block_y - 90;

    // �׹�° ��
    block[8].x = block_x + 50;
    block[8].y = block_y - 120;

    block[9].x = block_x + 150;
    block[9].y = block_y - 120;

    block[10].x = block_x + 300;
    block[10].y = block_y - 120;

    block[11].x = block_x + 400;
    block[11].y = block_y - 120;

    // �ټ���° ��
    block[12].x = block_x + 50;
    block[12].y = block_y - 150;

    block[13].x = block_x + 400;
    block[13].y = block_y - 150;

    // ������° ��
    float bb;
    bb = block_x + 50;
    for (int i = 14; i < 20; i++) {
        block[i].x = bb;

        if (i == 15 || i == 17) bb += 50;
        bb += 50;

        block[i].y = block_y - 180;
    }

    // �ϰ���° ��
    block[20].x = block_x + 100;
    block[20].y = block_y - 210;

    block[21].x = block_x + 350;
    block[21].y = block_y - 210;

    // ��������
    float cc;
    cc = block_x + 150;
    for (int i = 22; i < 26; i++) {
        block[i].x = cc;
        block[i].y = block_y - 240;
        cc += 50;
    }


    for (int i = 0; i < 26; i++) {      // ���� 26�� ������ �����ϰ� ����
        block[i].exist = true;
    }

    for (int i = 26; i < 50; i++) {     // ���� �� ó���� block_stair�� ����
        block[i].x = 5000;
        block[i].y = 5000;
        block[i].exist = false;
    }

}

// ��5 �� 39��
void block_fish(void) {

    map = 5;                            //  blcok_fish�� 5�� ���Դϴ�.

    // ù��° ��
    block[0].x = block_x + 150;
    block[0].y = block_y;

    block[1].x = block_x + 200;
    block[1].y = block_y;

    // �ι��� ��
    float aa;
    aa = block_x + 100;
    for (int i = 2; i < 6; i++) {
        block[i].x = aa;
        block[i].y = block_y - 30;

        if (i == 3) aa += 100;
        aa += 50;
    }

    // ����° ��
    float bb;
    bb = block_x + 50;
    for (int i = 6; i < 11; i++) {
        block[i].x = bb;
        block[i].y = block_y - 60;

        if (i == 7 || i == 9) bb += 100;
        bb += 50;
    }

    // �׹�° ��
    float cc;
    cc = block_x;
    for (int i = 11; i < 17; i++) {
        block[i].x = cc;
        block[i].y = block_y - 90;

        if (i == 12 || i == 14) cc += 100;
        cc += 50;
    }

    // �ټ���° ��
    float dd;
    dd = block_x;
    for (int i = 17; i < 22; i++) {
        block[i].x = dd;
        block[i].y = block_y - 120;

        if (i == 17 || i == 19) dd += 100;
        dd += 50;
    }

    // ������° ��
    float ee;
    ee = block_x;
    for (int i = 22; i < 28; i++) {
        block[i].x = ee;
        block[i].y = block_y - 150;

        if (i == 23 || i == 25) ee += 100;
        ee += 50;
    }

    // �ϰ���° ��
    float ff;
    ff = block_x + 50;
    for (int i = 28; i < 33; i++) {
        block[i].x = ff;
        block[i].y = block_y - 180;

        if (i == 29 || i == 31) ff += 100;
        ff += 50;
    }

    // ������° ��
    float gg;
    gg = block_x + 100;
    for (int i = 33; i < 37; i++) {
        block[i].x = gg;
        block[i].y = block_y - 210;

        if (i == 34) gg += 100;
        gg += 50;
    }

    //������ ��
    block[37].x = block_x + 150;
    block[37].y = block_y - 240;

    block[38].x = block_x + 200;
    block[38].y = block_y - 240;


    for (int i = 0; i < 39; i++) {          // ���� 39�� ������ �����ϰ� ����
        block[i].exist = true;
    }

    for (int i = 39; i < 50; i++) {         // ���� �� ó���� ���� ����
        block[i].x = 5000;
        block[i].y = 5000;
        block[i].exist = false;
    }

}

// ��6 �� 50��
void block_tomas(void) {
    
    map = 6;                            // block_tomas�� 6�� ���Դϴ�.

    // ù ��
    float aa = 0;
    for (int i = 0; i < 8; i++) {
        block[i].x = block_x + aa;
        block[i].y = block_y;
        aa += 50;
    }

    // �ι�° ��
    block[8].x = block_x;
    block[8].y = block_y - 30;

    // ����° ��
    aa = 0;
    for (int i = 9; i < 19; i++) {
        block[i].x = block_x + aa;
        block[i].y = block_y - 60;
        aa += 50;
    }

    // �׹�° ��
    block[19].x = block_x + 450;
    block[19].y = block_y - 90;

    // �ټ� ��° ��
    aa = 0;
    for (int i = 20; i < 30; i++) {
        block[i].x = block_x + aa;
        block[i].y = block_y - 120;
        aa += 50;
    }

    // ���� ��° ��
    block[30].x = block_x;
    block[30].y = block_y - 150;

    // �ϰ� ��° ��
    aa = 0;
    for (int i = 31; i < 41; i++) {
        block[i].x = block_x + aa;
        block[i].y = block_y - 180;
        aa += 50;
    }

    // ���� ��° ��
    block[41].x = block_x + 450;
    block[41].y = block_y - 210;

    // ������ ��
    aa = 100;
    for (int i = 42; i < 50; i++) {
        block[i].x = block_x + aa;
        block[i].y = block_y - 240;
        aa += 50;
    }

    for (int i = 0; i < 50; i++) { block[i].exist = true; }     // ��� ��(50��)�� �� ���
}

// ��7 �� 45��
void block_chess(void) {

    map = 7;              // block_chess �� 7�� ���Դϴ�.

    int row = 0;
    int aa = 0;
    for (int i = 0; i < 18; i++) {
        block[i].x = block_x + aa;
        block[i].y = block_y - 30*row;
        aa += 50;
        if (i == 2 || i == 8 || i == 14) { aa += 150; }
        if (i == 5 || i == 11 || i==17) { 
            row += 1;
            aa = 0;
        }
    }

    aa = 0;
    for (int i = 18; i < 27; i++) {
        block[i].x = block_x + 150 + aa;
        block[i].y = block_y - 30*row;
        aa += 50;
        if (i == 20 || i == 23 || i == 26) {
            row += 1; 
            aa = 0;
        }
    }

    aa = 0;
    for (int i = 27; i < 45; i++) {
        block[i].x = block_x + aa;
        block[i].y = block_y - 30 * row;
        aa += 50;
        if (i == 29 || i == 35 || i == 41) { aa += 150; }
        if (i == 32 || i == 38) { 
            row += 1;
            aa = 0;
        }
    }

    for (int i = 0; i < 45; i++) { block[i].exist = true; }     // ���� 45�� ������ �����ϰ� ����
    for (int i = 45; i < 50; i++) { block[i].exist = false; }   // ���� �� ó��
}

// ���콺 �̺�Ʈ
void mouse(int button, int state, int x, int y) {

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {     // ���콺 ���� ��ư�� ���� �����϶�

        gameOver = false;                                       // ���ӿ��� = false
        isStart = true;                                         // ���� ����
        print_one = true;                                       // Ÿ�̸� ����� �ѹ��� �ϱ� ���� �� ����

    }

    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {    // ���콺 ������ ��ư�� ���� �����϶�
        isStart = false;                                        // ���� ����X
        reset_Rotation();                                       // ��ġ �ʱ�ȭ �Լ�
    }

    glutPostRedisplay();                                        // �����츦 �ٽ� �׸����� ��û
}

// Ű���� �̺�Ʈ
void MyKey(unsigned char key, int x, int y) {   // Ű���� �̺�Ʈ - �� ��ȯ��

    switch (key) {
    case 'm':                                   // 'm' �϶� 1�� ��
        block_basic();
        reset_Rotation();                       // �� ��ġ �ʱ�ȭ

        break;

    case 'n':                                   // 'n' �϶� 2�� ��
        block_stair();                          
        reset_Rotation();                           
        break;

    case 'b':                                   // 'b' �϶� 3�� ��
        block_heart();
        reset_Rotation();
        break;

    case 'v':                                   // 'v' �϶� 4�� ��
        block_face();
        reset_Rotation();
        break;

    case 'c':                                   // 'c' �϶� 5�� ��
        block_fish();
        reset_Rotation();
        break;

    case 'x':                                   // 'x' �϶� 6�� ��
        block_tomas();
        reset_Rotation();
        break;

    case 'z':                                   // 'z' �϶� 7�� ��
        block_chess();
        reset_Rotation();
        break;
        
    case 'r':                                   // 'r' �϶� ��ġ �ʱ�ȭ
        reset_Rotation();
        break;

    case 27:                                    // '27' = esc key �϶�
        exit(0);                                // ����
        break;

    case 'q':                                   
        check[map-1] = true;                    // üũ�� ���� �� Ŭ����
        break;

    default:	break;
    }

    glutPostRedisplay();                        // �����츦 �ٽ� �׸����� ��û
}

// ��ƽ �׸���
void Draw_Stick(void) {
    glColor3f(1.0, 1.0, 1.0);                   // ��ƽ ���� => ���
    glBegin(GL_POLYGON);
    glVertex2f(stick_x, stick_y);               // (0, 30)�� ���������� �ݽð�������� �׸��ϴ�. 
    glVertex2f(stick_x + 95.0, stick_y);        // ��ƽ�� ���� 95
    glVertex2f(stick_x + 95.0, stick_y - 20.0); // ��ƽ�� ���̴� 20
    glVertex2f(stick_x, stick_y - 20.0);
    glEnd();
}

// ��ƽ ���� �Լ�
void Move_Stick(int key, int x, int y) {
    switch (key)
    {
    case GLUT_KEY_LEFT:             // Ű������ ���� ����Ű�� ������
        stick_x -= stick_move;      // ��ƽ x�� ��ǥ�� stick_move(10) ��ŭ ���� �̵�
        break;
    case GLUT_KEY_RIGHT:            // Ű������ ������ ����Ű�� ������
        stick_x += stick_move;      // ��ƽ x�� ��ǥ�� stick_move(10) ��ŭ ������ �̵�
        break;

    /*case GLUT_KEY_UP:             // ��ƽ�� �� �Ʒ��ε� ������ �� �ִ� ����� ������ �Ͽ�����
        stick_y += stick_move;      // ������ ��̿� ����� ���ٰ� �Ǵ��ؼ� �ּ�ó���Ͽ���
        break;
    case GLUT_KEY_DOWN:
        stick_y -= stick_move;
        break;*/
    }
    
    glutPostRedisplay();            // �����츦 �ٽ� �׸����� ��û
}


// ȭ�鿡 ���ڸ� ���� �Լ�
void drawBitmapText(char* string, float x, float y, float z)
{
    char* c;
    glRasterPos3f(x, y, z);                     // ���ڸ� ��� ���� ��ġ�� ����

    for (c = string; *c != '\0'; c++)           // �޾ƿ� ���ڸ� �ϳ��� ����
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
    }
}

// ���� ������ �����ִ� �Լ�
void show_info(void) {
    int total = 0;                      // ���� ����� ���� �� �� ����
    int score = 0;                      // ���� ����� ���� ����

    char point[50];                     // ���� ���� ���ھ� �޽���
    char max_score[50];                 // ���� ���� �ִ� ���ھ� �޽���
    char howToReset[100];               // �ʱ�ȭ ����� �˷��ִ� �޽���
    char clear[100];                    // Ŭ����� �޽���


    if (map == 1) total = 50;           // �� 1�϶� �� �� ���� 50
    else if (map == 2) total = 45;      // �� 2�϶� �� �� ���� 45
    else if (map == 3) total = 40;      // �� 3�϶� �� �� ���� 40
    else if (map == 4) total = 26;      // �� 4�϶� �� �� ���� 26
    else if (map == 5) total = 39;      // �� 5�϶� �� �� ���� 39
    else if (map == 6) total = 50;      // �� 6�϶� �� �� ���� 50
    else if (map == 7) total = 45;      // �� 7�϶� �� �� ���� 45


    for (int i = 0; i < total; i++) {   // �� �� ������ŭ �ݺ��� ���鼭
        if (block[i].exist == false) {  // ���� ���������(exist == false)
            score += 100;               // ���ھ� 100�� �߰�
        }
    }

    if (total * 100 == score) {         // �� ������ ���� ���� ������ ���� ��
        isClear = true;
        sprintf_s(clear, "Congratulations! try another map!");      // clear�̶� ������ Ŭ���� �޽��� ����
        drawBitmapText(clear, 100, 400, 0);                         // clear�� x�� 100, y�� 400 ��ġ�� ����

        char map_sec[50];                                           // Ŭ����ð��� ��� ���� ����
        sprintf_s(map_sec, "%.2f sec", duration);                   // map_sec�� duration ���� ����
        drawBitmapText(map_sec, 80, 300, 0);                        // x�� 80, y�� 300 ��ġ�� map_sec ���

    }
    else { isClear = false; }

    sprintf_s(max_score, "Max Score : %d", total * 100);            // max_score ������ total*100 �� ����
    sprintf_s(point, "Your Score : %d", score);                     // score ������ ���� ���� point�� ����
    sprintf_s(howToReset, "If your ball is stucked, Just press 'r' to reset ball");

    drawBitmapText(max_score, 80, 250, 0);          // max_score�� ȭ���� 80, 250 ��ġ�� ������
    drawBitmapText(point, 80, 200, 0);              // point�� ȭ���� 80, 200 ��ġ�� ������
    drawBitmapText(howToReset, 20, 100, 0);         // howToReset�� 20, 100 ��ġ�� ������

    int cnt = 0;
    for (int i = 0; i < 7; i++) {       
        if (check[i] == true) {                     // Ŭ����� �� �� Ȯ��
            cnt += 1;
        }
   }
    if (cnt == 7) { allClear = true; }              // 7���� ���� Ŭ���� ������ allClear = True 
}

// �ʱ� ���� �����ִ� �Լ�
void Draw_init_block(void) {

    for (int i = 0; i < 50; i++) {                              // �� 50���� ��

        if (i == random_num1) glColor3f(0.0, 0.0, 1.0);         // ���� ���ڿ��� ���� �÷� B
        else if (i == random_num2) glColor3f(0.0, 1.0, 0.0);    // ���� ���ڿ��� ���� �÷� G
        else if (i == random_num3) glColor3f(1.0, 0.0, 0.0);    // ���� ���ڿ��� ���� �÷� R
        else glColor3f(1.0, 1.0, 1.0);                          // ������ ������ �÷� White

        if (map == 3) {                     // 3���� (��Ʈ��) �϶�
            glColor3f(1.0, 0.0, 0.0);       // ��� ���� �÷� R
        }

        block[i].draw_block();              // ���� �׸�
    }

}

// �ʱ� ���� 
void MyReshape(int w, int h) {

    glViewport(0, 0, w, h);                                         // �� ��Ʈ ���� 0,0 ��ġ���� w(width), h(height) ����
    glMatrixMode(GL_PROJECTION);                                    // ���� ��ǥ�� ����
    glLoadIdentity();                                               // ��ǥ�� �ʱ�ȭ
    gluOrtho2D(_left, _left + width, _bottom, _bottom + height);    // ���� ���� ����
    playingBgm();                                                   // bgm��� 
}

// Ŭ���� üũ �Լ�
void checkClear(void) {

    if (isClear) { check[map - 1] = true; }             // Ŭ���� ������ �ش� ���� clear ���� true�� ��
    int cnt = 0;    
    for (int i = 0; i < 7; i++) {                       // Ŭ���� �� ���� ���� Ž���ϱ� ���� for��
        if (check[i] == true) { 
            cnt++;                                      // Ŭ��� true�� �� cnt �߰�
        }                
    }
    if (cnt == 7) {                                     // cnt�� 7�϶� = ��� ���� Ŭ���� ������
        allClear = true;                                // allClear�� true �� ��
        cout << "��� ���� Ŭ���� �Ͽ����ϴ�!" << endl;
    }
    else {                                              // ��� ���� Ŭ���� ���� �ʾ��� ��
        cout << "���� �������� " << 7 - cnt << " ���� ���� ���ҽ��ϴ�!" << endl;   // ���� ���� ���� ���� ������
        cout << "=====================================" << endl;
    }
}

// Ŭ���� �ð��� �����ϱ� ���� �Լ�
void checkTime(void) {

    if (isClear) {                                       // Ŭ���� ������
        finish = clock();                                // �ð��� ��ƿͼ�
        duration = (double)(finish) / CLOCKS_PER_SEC;    // milisec������ �ð��� �ʷ� ��ȯ�Ͽ� duration�� ��Ƶ�
        cout << "�� : " << map << endl;                  // ���� ���� �ܼ�â�� ������
        cout << duration << "��" << endl;                // ��� �ð��� �ܼ�â�� ������
        
        print_one = false;                               // �ѹ��� ����ϱ� ���� print_one ������ false�� ��
        checkClear();                                    // Ŭ���� üũ �Լ�
    }
}

// ������ ȭ��
void RenderScene(void) {

    glClear(GL_COLOR_BUFFER_BIT);       // ���� ���۸� ������

    if (isStart == false) {             // ���� ������ ������ ȭ��
        Draw_init_block();              // �ʱ� ���� ������

        char say[50] = "RGB Breaking Block";
        char say2[100] = "Press 'z', 'x', 'c', 'v', 'b', 'n', 'm' to select map";
        char say3[50] = "Click anywhere to start";
        drawBitmapText(say, 80, 200, 0);   // say�� 80, 200�� ��ġ�� ���
        drawBitmapText(say2, 20, 100, 0);  // say2�� 20, 100�� ���
        drawBitmapText(say3, 80, 50, 0);   // say3�� 80, 50�� ��� 
    }

  
    else if (gameOver) {                   // ���ӿ��� ������
        char gg[50] = "Game Over";         
        drawBitmapText(gg, 200, 300, 0);   // gg�� 200, 300�� ���
    }

    else if (allClear == true) {           // ��� ���� Ŭ���� ���� ��
        char ac[50] = "All Clear";         // �� Ŭ���� �޽��� ac
        drawBitmapText(ac, 200, 300, 0);   // ac�� 200, 300�� ���

        char map_sec[50];                                           // Ŭ����ð��� ��� ���� ����
        sprintf_s(map_sec, "%.2f sec", duration);                   // map_sec�� duration ���� ����
        drawBitmapText(map_sec, 200, 250, 0);                        // x�� 80, y�� 300 ��ġ�� map_sec ���
    }

    else if (isStart == true) {                                                     // ���� ���� ��
        glColor3f(1.0, 1.0, 1.0);                                                   // �÷� �� ������� ��
        show_info();                                                                // ���� ���� �Լ�
        moving_ball.Draw_ball(moving_ball.radius, moving_ball.x, moving_ball.y);    // ���� ���� �׸�

        if (!isClear) {                                                             // Ŭ���� ���� �ʾ��� ��
            moving_ball.Main_ball_Detection_to_Wall();                              // ���� ���� �浹 ���� => �Ʒ��� �浹�� ���� ����
        }
        else if (isClear) {                                                         // Ŭ���� ���� ��
            moving_ball.Collision_Detection_to_Walls();                             // ���� ���� �浹 ���� => �Ʒ��� �浹 ����
        }

        moving_ball.Collision_Detection_to_Stick();                                 // ��ƽ �浹
        moving_ball.Collision_Detection_to_Block();                                 // ��� �浹

        moving_ball.x += moving_ball.velocity_x;                                    // ���� ���� �ӵ�(x) ����
        moving_ball.y += moving_ball.velocity_y;                                    // ���� ���� �ӵ�(y) ����

        if (block[random_num1].exist == false) {                                    // ���� ���� 1�� �ش��ϴ� ���� ������� �� (�浹 �Ͽ�����)
            
            glColor3f(0.0, 0.0, 1.0);                                               // ���� ���� ������ Blue�� ��
            second_ball.Draw_ball(second_ball.radius, second_ball.x, second_ball.y);// ���� ���� �׸�

            second_ball.Collision_Detection_to_Walls();                             // ���� ���� �� �浹
            second_ball.Collision_Detection_to_Stick();                             // ���� ���� ��ƽ �浹
            second_ball.Collision_Detection_to_Block();                             // ���� ���� �� �浹

            second_ball.x += second_ball.velocity_x;                                // ���� ���� �ӵ� (x) ����
            second_ball.y += second_ball.velocity_y;                                // ���� ���� �ӵ� (y) ����

        }

        if (block[random_num2].exist == false) {                                    // ���� ���� 2�� �ش��ϴ� ���� ������� �� (�浹 �Ͽ�����)

            glColor3f(0.0, 1.0, 0.0);                                               // ���� ���� ������ Green���� ��
            third_ball.Draw_ball(third_ball.radius, third_ball.x, third_ball.y);    // ���� ���� �׸�
                                                                                    // �Ʒ��� ���� �� 1�� �����ϰ� �� ����
            third_ball.Collision_Detection_to_Walls();
            third_ball.Collision_Detection_to_Stick();
            third_ball.Collision_Detection_to_Block();

            third_ball.x += third_ball.velocity_x;
            third_ball.y += third_ball.velocity_y;

        }

        if (block[random_num3].exist == false) {                                    // ���� ���� 2�� �ش��ϴ� ���� ������� �� (�浹 �Ͽ�����)

            glColor3f(1.0, 0.0, 0.0);                                               // ���� ���� ������ Red���� ��
            fourth_ball.Draw_ball(fourth_ball.radius, fourth_ball.x, fourth_ball.y);// ���� ���� �׸�
                                                                                    // �Ʒ��� ���� �� 1�� �����ϰ� �� ����
            fourth_ball.Collision_Detection_to_Walls();
            fourth_ball.Collision_Detection_to_Stick();
            fourth_ball.Collision_Detection_to_Block();

            fourth_ball.x += fourth_ball.velocity_x;
            fourth_ball.y += fourth_ball.velocity_y;

        }

        Draw_Stick();                                                   // ��ƽ�� �׸�

        Draw_init_block();                                              // �ʱ� ���� �׸�

        if (stick_x <= 10) stick_x = 10;
        if (stick_x + 95 + 10 >= width) stick_x = width - 95 - 10;      // ��ƽ�� ���� �浹 (���� 10 ��ŭ ���� ) ��ƽ���� = 95
    }


    if (print_one) {
        checkTime();    // Ÿ�� üũ�� �ѹ��� ��� �� �� �ֵ��� ��
    }

    glutSwapBuffers();
    glFlush();
}

int main(int argc, char** argv) {

    cout << "=====================================" << endl;
    cout << " Welcome to RGB Block Breaking" << endl;
    cout << " RGB BLOCK Breaking�� ���Ű� ȯ���մϴ�!" << endl;
    cout << "=====================================" << endl;

    glutInit(&argc, argv);                          // glut �ʱ�ȭ
    glutInitWindowPosition(100, 100);               // ������ â ��ġ 100, 100
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);    // ���÷��̸�� - �������, ������ RGB
    glutInitWindowSize(width, height);              // ������ ũ�� width, height
    glutCreateWindow("RGB Breaking Block");         // ������ ���� ����
    glutReshapeFunc(MyReshape);                     // �������� �̺�Ʈ
    glutDisplayFunc(RenderScene);                   // RenderScene�� �ݹ��Լ��� ���
    glutIdleFunc(RenderScene);                      // ���ø����̼� �޸�(idle) �ð��� ȣ��� �ݹ��Լ� ���
    glutKeyboardFunc(MyKey);                        // Ű���� �̺�Ʈ�� ���ڴ�.
    glutSpecialFunc(Move_Stick);                    // Ű���� �̺�Ʈ (�����Ű)�� ���ڴ�.
    glutMouseFunc(mouse); 	                        // ���콺 �̺�Ʈ�� ���ڴ�.
    glutMainLoop();                                 

    return 0;
}
