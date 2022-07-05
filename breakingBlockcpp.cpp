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

// 맵 번호
int     map = 0;

// 랜덤 숫자 3가지
int     random_num1;
int     random_num2;
int     random_num3;

// 화면 왼쪽, 아래쪽
int     _left = 0;
int     _bottom = 0;

bool    isStart = false;
bool    gameOver = false;
bool    isClear = false;
bool    print_one = false;
bool    allClear = false;
bool    check[7] = { false, false, false, false, false, false, false };

// 스틱 이동 거리(10), 스틱 좌표 x, y
float    stick_move = 10;
float    stick_x = 0.0;
float    stick_y = 30.0;

// 블럭의 첫 x, y 좌표
float   block_x = 5.0;
float   block_y = 580.0;

clock_t finish;
double duration;

int dwID;

// 배경음악 작동 함수
void playingBgm(void) { 
    openBgm.lpstrElementName = TEXT(BGM); // 파일 경로 입력
    openBgm.lpstrDeviceType = TEXT("mpegvideo"); //mp3 형식
    mciSendCommand(0, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE, (DWORD)(LPVOID)&openBgm);
    dwID = openBgm.wDeviceID;
    mciSendCommand(dwID, MCI_PLAY, MCI_DGV_PLAY_REPEAT, (DWORD)(LPVOID)&openBgm); //음악 반복 재생
}

// 충돌 효과음 작동 함수
void playingHitSound(void) {
    openShuffleSound.lpstrElementName = TEXT(HIT); //파일 오픈
    openShuffleSound.lpstrDeviceType = TEXT("mpegvideo"); //mp3 형식
    mciSendCommand(0, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE, (DWORD)(LPVOID)&openShuffleSound);
    dwID = openShuffleSound.wDeviceID;
    mciSendCommand(dwID, MCI_PLAY, MCI_NOTIFY, (DWORD)(LPVOID)&openShuffleSound); //음악을 한 번 재생
    Sleep(50); //sleep()이기 때문에 잠깐 멈칫거림이 있음
    mciSendCommand(dwID, MCI_SEEK, MCI_SEEK_TO_START, (DWORD)(LPVOID)NULL); //재생위치를 초기위치로 돌림
}


// 블럭 클래스
class Block {

public:
    float x;                      // 블럭의 x 좌표 ( 좌측 하단)
    float y;                      // 블럭의 y 좌표 ( 좌측 하단)
    bool exist = false;           // 블럭의 존재 유무

    float block_width = 45;       // 블럭의 폭
    float block_height = 25;      // 블럭의 높이

    Block() {}                    // 기본 생성자

    Block(float b_x, float b_y) { // 생성자

        x = b_x;
        y = b_y;

    }

    void draw_block() {             // 블럭 그리기 함수
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

// 50개 블럭 선언
Block block[50];

// 볼 클래스
class Ball {

public:
    float x;            // 공의 x 좌표
    float y;            // 공의 y 좌표
    float radius;       // 공의 반지름
    float velocity_x;   // 공의 x축으로의 속도
    float velocity_y;   // 공의 y축으로의 속도

    Ball(float b_x, float b_y, float b_radius, float velocity) {    // Ball의 생성자
        x = b_x;
        y = b_y;
        radius = b_radius;
        velocity_x = velocity;
        velocity_y = velocity;
    }

    // 공을 그리는 함수
    void    Draw_ball(float radius, float x, float y) {             
        float    delta;
        delta = 2 * PI / polygon_num;               // polygon_num = 50
        glBegin(GL_POLYGON);
        for (int i = 0; i < polygon_num; i++)       // 점 (x,y) 를 기준으로 50개의 점을 찍음
            glVertex2f(x + radius * cos(delta * i), y + radius * sin(delta * i));
        glEnd();
    }

    // 공과 벽의 충돌 - 메인 볼
    void    Main_ball_Detection_to_Wall(void) {
        if (x - radius <= _left || x + radius >= _left + width) {  // 좌 우 충돌
            velocity_x *= -1.0;                                    // 공의속도(x) 반대방향
        }
        if (y + radius >= _bottom + height) {                      // 위쪽 충돌
            velocity_y *= -1.0;                                    // 공의속도(y) 반대방향
        }
        if (y - radius <= _bottom) {                               // 아래쪽 충돌
            velocity_y *= -1.0;
            gameOver = true;                                       // 게임 오버!
        }
    }

    // 공과 벽의 충돌 - 서브 볼
    void    Collision_Detection_to_Walls(void) {

        if (x - radius <= _left || x + radius >= _left + width) {       // 좌 우 충돌 
            velocity_x *= -1.0;                                         // 공의속도(x) 반대방향
        }
        if (y - radius <= _bottom || y + radius >= _bottom + height) {  // 상 하 충돌
            velocity_y *= -1.0;                                         // 공의속도(y) 반대방향
        }
                                                                        // 서브볼은 게임오버 X
    }


    // 공과 스틱의 충돌
    void    Collision_Detection_to_Stick(void) {

        // 위쪽 충돌
        if (stick_x < x && x < stick_x && stick_y > y - radius) {
            velocity_y *= -1.0;
        }

        // 아래쪽 충돌
        else if (stick_x < x && x < stick_x + 90 && stick_y - 20 < y + radius && y < stick_y + radius) {
            velocity_y *= -1.0;
        }

        // 오른쪽 충돌
        else if (stick_x + 90 > x - radius && x > stick_x - radius && stick_y > y && y > stick_y - 20) {
            velocity_y *= -1.0;
        }

        // 왼쪽 충돌
        else if (stick_x < x + radius && x < stick_x + radius && stick_y > y && y > stick_y - 20) {
            velocity_y *= -1.0;
        }


    }

    // 공과 블럭의 충돌
    void    Collision_Detection_to_Block(void) {

        bool hit = false;

        for (int i = 0; i < 50; i++) {

            // 아래쪽 충돌
            if (block[i].x < x && x < block[i].x + 45 && block[i].y - 25 < y + radius && y < block[i].y + radius) {
                velocity_y *= -1.0;         // 공의 속도(y) 반대 방향
                block[i].exist = false;     // 존재 true -> false
                hit = true;                 // 충돌! -> 음악 재생
                block[i].x -= 1000;         // 블럭의 exist가 false더라도, 그리지만 않을 뿐 블럭은 그곳에 존재
                block[i].y -= 1000;         // 따라서 블럭을 다른 곳으로 옮겨 버리면 되겠다라고 판단
                                            // + 이 방법으로 할경우 모서리 충돌에서 오류가 나지 않음
                                            // + 하지만 성능면에선 좋지 않다고 생각함
            }

            // 위쪽 충돌 (= 아래쪽과 동일)
            else if (block[i].x < x && x < block[i].x && block[i].y > y - radius) {
                velocity_y *= -1.0;
                block[i].exist = false;
                hit = true;
                block[i].x -= 1000;
                block[i].y -= 1000;
            }

            // 오른쪽 충돌
            else if (block[i].x + 45 > x - radius && x > block[i].x - radius && block[i].y > y && y > block[i].y - 25) {
                velocity_x *= -1.0;     // 공의 속도(x) 반대방향
                block[i].exist = false;
                hit = true;
                block[i].x -= 1000;     // 블럭을 멀리 날려버리자
                block[i].y -= 1000;
            }

            // 왼쪽 충돌 (= 오른쪽과 동일)
            else if (block[i].x < x + radius && x < block[i].x + radius && block[i].y > y && y > block[i].y - 25) {
                velocity_x *= -1.0;
                block[i].exist = false;
                hit = true;
                block[i].x -= 1000; 
                block[i].y -= 1000;
            }

        }

        if (hit) {
            playingHitSound();          // 공 충돌시 충돌 효과음 재생
        }
    }

};

// 4개의 볼 선언
Ball moving_ball(width / 2, height / 4, 10, 0.2);    // 공의 초기 위치 x, y, radius, 공의 속도(x,y 둘다)
Ball second_ball(width / 4, height / 4, 5, 0.22);
Ball third_ball(width / 6, height / 4, 5, 0.23);
Ball fourth_ball(width / 8, height / 4, 5, 0.24);

// 리셋 함수
void reset_Rotation(void) {

    srand(time(0));             // 랜덤 숫자 3개 값 초기화 (재설정)
    random_num1 = rand() % 50;
    random_num2 = rand() % 50;
    random_num3 = rand() % 50;
  

    moving_ball.x = width / 2;  // 메인 볼 위치 초기화
    moving_ball.y = height / 4;

    second_ball.x = width / 4;  // 서브 볼 1 위치 초기화
    second_ball.y = height / 4;

    third_ball.x = width / 6;   // 서브 볼 2 위치 초기화
    third_ball.y = height / 4;

    fourth_ball.x = width / 8;  // 서브 볼 3 위치 초기화
    fourth_ball.y = height / 4;

    if (moving_ball.velocity_y < 0) moving_ball.velocity_y *= -1.0; // 공이 아래쪽을 향한다면 위쪽으로 향하게 함

}

// 블럭 배치 어떻게 할지?
// 맵1 블럭 50개
void block_basic(void) {

    map = 1;                        // block_basic은 1번맵

    float   block_x = 5.0;          // 첫 블럭의 x좌표
    float   block_y = 580.0;        // 첫 블럭의 y좌표

    int row = 0;                    // 행 변환에 쓰기 위해 row 선언

    for (int i = 0; i < 50; i++) {  

        if (i % 10 == 0) {          // 10개의 블럭을 찍었을때
            row = 0;                // 행을 초기 위치로
            block_y -= 30;          // 블럭 y의 위치를 한칸(30) 내림
        }

        block[i].x = block_x + 50 * row;    // 블럭의 x 좌표를 한칸 띄울때마다 50씩 띄움
        block[i].y = block_y;               // 동일한 행에있는 블럭들의 y좌표는 똑같다.

        block[i].exist = true;              // 블럭을 그리면서 exist값을 true로 함

        row++;                              // 한 칸 그릴때마다 옆으로 한칸씩 옮김
    }

}

// 맵2 블럭 45개
void block_stair(void) {

    map = 2;                            // block_stair은 2번맵

    float   block_x = 5.0;              // 첫 블럭의 x 좌표
    float   block_y = 580.0;            // 첫 블럭의 y 좌표

    int frefix_sum[9] = { 0, };         // 행을 바꿀곳을 찾기 위한 방법으로 부분합을 이용하기로 함
    int sum = 0;
    for (int i = 1; i < 10; i++) {      // 부분합 계산
        frefix_sum[i - 1] = sum;
        sum += i;                       // 위 과정을 통해서
    }                                   // frefix_sum에는 [1, 3, 6, 10, 15, 21, 28, 36, 45]가 저장되어 있음

    int num = 0;
    for (int i = 0; i < 50; i++) {      // 총 블럭의 개수 50개 까지 반복
        for (int j = 0; j < 9; j++) {   // 부분합 배열인 frefix_sum을 탐색하기 위한 2중 for문
            if (i == frefix_sum[j]) {   // 50개를 쭉 찍으면서, i가 부분합 배열일때
                num = 0;                // 행의 첫번째로 돌아감
                block_y -= 30;          // 행을 한칸(30) 내림
            }
        }

        block[i].x = block_x + 50 * num;    // 한 블럭 그릴때마다 num * 50씩 옮김
        block[i].y = block_y;               // 동일한 행에 있는 블럭의 y좌표는 같음

        num++;                              // 한 블럭 그릴때 마다 옆으로 한칸씩 옮김
    }

    for (int i = 0; i < 45; i++) {          // 블럭이 45개 까지 존재하게 만듬
        block[i].exist = true;
    }
    for (int i = 45; i < 50; i++) {         // 남은 블럭 처리는 공 충돌에서 처리 했던 방식과 비슷하게
        block[i].x = 5000;                  // 블럭을 멀리 날려버림
        block[i].y = 5000;
        block[i].exist = false;             // 그리는 것도 false로 둠
    }

}

// 맵3 블럭 40개
void block_heart(void) {

    map = 3;                        // block_heart는 3번 맵입니다.

    float   block_x = 5.0;          // 첫 블럭의 x 좌표
    float   block_y = 550.0;        // 첫 블럭의 y 좌표

                                    // 알고리즘 문제(백준 별찍기)처럼 접근하려고 해보았지만
                                    // 결국에는 블럭을 하나씩 찍기로 결정

    // 첫줄
    block[0].x = block_x + 100;     
    block[1].x = block_x + 150;
    block[2].x = block_x + 300;
    block[3].x = block_x + 350;

    for (int i = 0; i < 4; i++) {
        block[i].y = block_y;
    }

    //2,3,4줄
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

    //5줄
    float cc;
    cc = block_x + 100;
    for (int i = 28; i < 34; i++) {
        block[i].x = cc;
        block[i].y = block_y - 120;
        cc += 50;
    }

    //6줄
    float dd;
    dd = block_x + 150;
    for (int i = 34; i < 38; i++) {
        block[i].x = dd;
        block[i].y = block_y - 150;
        dd += 50;
    }

    //7줄(끝)
    block[38].x = block_x + 200;
    block[38].y = block_y - 180;

    block[39].x = block_x + 250;
    block[39].y = block_y - 180;

    for (int i = 0; i < 40; i++) { // 블럭이 40개 까지만 존재하게 만듬
        block[i].exist = true;
    }


    for (int i = 40; i < 50; i++) { // 남은 블럭 처리는 blcok_stair과 동일
        block[i].x = 5000;
        block[i].y = 5000;
        block[i].exist = false;
    }
}

// 맵4 블럭 26개
void block_face(void) {

    map = 4;                        // block_face는 4번 맵입니다.
                                    // heart와 동일하게 알고리즘이 하나하나씩 찍었습니다.
    // 첫줄
    float aa;
    aa = block_x + 150;
    for (int i = 0; i < 4; i++) {
        block[i].x = aa;
        block[i].y = block_y - 30;
        aa += 50;
    }

    // 둘째 줄
    block[4].x = block_x + 100;
    block[4].y = block_y - 60;

    block[5].x = block_x + 350;
    block[5].y = block_y - 60;

    // 세번째 줄
    block[6].x = block_x + 50;
    block[6].y = block_y - 90;

    block[7].x = block_x + 400;
    block[7].y = block_y - 90;

    // 네번째 줄
    block[8].x = block_x + 50;
    block[8].y = block_y - 120;

    block[9].x = block_x + 150;
    block[9].y = block_y - 120;

    block[10].x = block_x + 300;
    block[10].y = block_y - 120;

    block[11].x = block_x + 400;
    block[11].y = block_y - 120;

    // 다섯번째 줄
    block[12].x = block_x + 50;
    block[12].y = block_y - 150;

    block[13].x = block_x + 400;
    block[13].y = block_y - 150;

    // 여섯번째 줄
    float bb;
    bb = block_x + 50;
    for (int i = 14; i < 20; i++) {
        block[i].x = bb;

        if (i == 15 || i == 17) bb += 50;
        bb += 50;

        block[i].y = block_y - 180;
    }

    // 일곱번째 줄
    block[20].x = block_x + 100;
    block[20].y = block_y - 210;

    block[21].x = block_x + 350;
    block[21].y = block_y - 210;

    // 마지막줄
    float cc;
    cc = block_x + 150;
    for (int i = 22; i < 26; i++) {
        block[i].x = cc;
        block[i].y = block_y - 240;
        cc += 50;
    }


    for (int i = 0; i < 26; i++) {      // 블럭이 26개 까지만 존재하게 만듬
        block[i].exist = true;
    }

    for (int i = 26; i < 50; i++) {     // 남은 블럭 처리는 block_stair과 동일
        block[i].x = 5000;
        block[i].y = 5000;
        block[i].exist = false;
    }

}

// 맵5 블럭 39개
void block_fish(void) {

    map = 5;                            //  blcok_fish는 5번 맵입니다.

    // 첫번째 줄
    block[0].x = block_x + 150;
    block[0].y = block_y;

    block[1].x = block_x + 200;
    block[1].y = block_y;

    // 두번쨰 줄
    float aa;
    aa = block_x + 100;
    for (int i = 2; i < 6; i++) {
        block[i].x = aa;
        block[i].y = block_y - 30;

        if (i == 3) aa += 100;
        aa += 50;
    }

    // 세번째 줄
    float bb;
    bb = block_x + 50;
    for (int i = 6; i < 11; i++) {
        block[i].x = bb;
        block[i].y = block_y - 60;

        if (i == 7 || i == 9) bb += 100;
        bb += 50;
    }

    // 네번째 줄
    float cc;
    cc = block_x;
    for (int i = 11; i < 17; i++) {
        block[i].x = cc;
        block[i].y = block_y - 90;

        if (i == 12 || i == 14) cc += 100;
        cc += 50;
    }

    // 다섯번째 줄
    float dd;
    dd = block_x;
    for (int i = 17; i < 22; i++) {
        block[i].x = dd;
        block[i].y = block_y - 120;

        if (i == 17 || i == 19) dd += 100;
        dd += 50;
    }

    // 여섯번째 줄
    float ee;
    ee = block_x;
    for (int i = 22; i < 28; i++) {
        block[i].x = ee;
        block[i].y = block_y - 150;

        if (i == 23 || i == 25) ee += 100;
        ee += 50;
    }

    // 일곱번째 줄
    float ff;
    ff = block_x + 50;
    for (int i = 28; i < 33; i++) {
        block[i].x = ff;
        block[i].y = block_y - 180;

        if (i == 29 || i == 31) ff += 100;
        ff += 50;
    }

    // 여덟번째 줄
    float gg;
    gg = block_x + 100;
    for (int i = 33; i < 37; i++) {
        block[i].x = gg;
        block[i].y = block_y - 210;

        if (i == 34) gg += 100;
        gg += 50;
    }

    //마지막 줄
    block[37].x = block_x + 150;
    block[37].y = block_y - 240;

    block[38].x = block_x + 200;
    block[38].y = block_y - 240;


    for (int i = 0; i < 39; i++) {          // 블럭이 39개 까지만 존재하게 만듬
        block[i].exist = true;
    }

    for (int i = 39; i < 50; i++) {         // 남은 블럭 처리는 위와 동일
        block[i].x = 5000;
        block[i].y = 5000;
        block[i].exist = false;
    }

}

// 맵6 블럭 50개
void block_tomas(void) {
    
    map = 6;                            // block_tomas는 6번 맵입니다.

    // 첫 줄
    float aa = 0;
    for (int i = 0; i < 8; i++) {
        block[i].x = block_x + aa;
        block[i].y = block_y;
        aa += 50;
    }

    // 두번째 줄
    block[8].x = block_x;
    block[8].y = block_y - 30;

    // 세번째 줄
    aa = 0;
    for (int i = 9; i < 19; i++) {
        block[i].x = block_x + aa;
        block[i].y = block_y - 60;
        aa += 50;
    }

    // 네번째 줄
    block[19].x = block_x + 450;
    block[19].y = block_y - 90;

    // 다섯 번째 줄
    aa = 0;
    for (int i = 20; i < 30; i++) {
        block[i].x = block_x + aa;
        block[i].y = block_y - 120;
        aa += 50;
    }

    // 여섯 번째 줄
    block[30].x = block_x;
    block[30].y = block_y - 150;

    // 일곱 번째 줄
    aa = 0;
    for (int i = 31; i < 41; i++) {
        block[i].x = block_x + aa;
        block[i].y = block_y - 180;
        aa += 50;
    }

    // 여덟 번째 줄
    block[41].x = block_x + 450;
    block[41].y = block_y - 210;

    // 마지막 줄
    aa = 100;
    for (int i = 42; i < 50; i++) {
        block[i].x = block_x + aa;
        block[i].y = block_y - 240;
        aa += 50;
    }

    for (int i = 0; i < 50; i++) { block[i].exist = true; }     // 모든 블럭(50개)을 다 사용
}

// 맵7 블럭 45개
void block_chess(void) {

    map = 7;              // block_chess 는 7번 맵입니다.

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

    for (int i = 0; i < 45; i++) { block[i].exist = true; }     // 블럭이 45개 까지만 존재하게 만듬
    for (int i = 45; i < 50; i++) { block[i].exist = false; }   // 남은 블럭 처리
}

// 마우스 이벤트
void mouse(int button, int state, int x, int y) {

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {     // 마우스 왼쪽 버튼이 누른 상태일때

        gameOver = false;                                       // 게임오버 = false
        isStart = true;                                         // 게임 시작
        print_one = true;                                       // 타이머 출력을 한번만 하기 위해 둔 변수

    }

    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {    // 마우스 오른쪽 버튼이 누른 상태일때
        isStart = false;                                        // 게임 시작X
        reset_Rotation();                                       // 위치 초기화 함수
    }

    glutPostRedisplay();                                        // 윈도우를 다시 그리도록 요청
}

// 키보드 이벤트
void MyKey(unsigned char key, int x, int y) {   // 키보드 이벤트 - 맵 전환용

    switch (key) {
    case 'm':                                   // 'm' 일때 1번 맵
        block_basic();
        reset_Rotation();                       // 공 위치 초기화

        break;

    case 'n':                                   // 'n' 일때 2번 맵
        block_stair();                          
        reset_Rotation();                           
        break;

    case 'b':                                   // 'b' 일때 3번 맵
        block_heart();
        reset_Rotation();
        break;

    case 'v':                                   // 'v' 일때 4번 맵
        block_face();
        reset_Rotation();
        break;

    case 'c':                                   // 'c' 일때 5번 맵
        block_fish();
        reset_Rotation();
        break;

    case 'x':                                   // 'x' 일때 6번 맵
        block_tomas();
        reset_Rotation();
        break;

    case 'z':                                   // 'z' 일때 7번 맵
        block_chess();
        reset_Rotation();
        break;
        
    case 'r':                                   // 'r' 일때 위치 초기화
        reset_Rotation();
        break;

    case 27:                                    // '27' = esc key 일때
        exit(0);                                // 종료
        break;

    case 'q':                                   
        check[map-1] = true;                    // 체크용 강제 맵 클리어
        break;

    default:	break;
    }

    glutPostRedisplay();                        // 윈도우를 다시 그리도록 요청
}

// 스틱 그리기
void Draw_Stick(void) {
    glColor3f(1.0, 1.0, 1.0);                   // 스틱 색상 => 흰색
    glBegin(GL_POLYGON);
    glVertex2f(stick_x, stick_y);               // (0, 30)을 시작점으로 반시계방향으로 그립니다. 
    glVertex2f(stick_x + 95.0, stick_y);        // 스틱의 폭은 95
    glVertex2f(stick_x + 95.0, stick_y - 20.0); // 스틱의 높이는 20
    glVertex2f(stick_x, stick_y - 20.0);
    glEnd();
}

// 스틱 조정 함수
void Move_Stick(int key, int x, int y) {
    switch (key)
    {
    case GLUT_KEY_LEFT:             // 키보드의 왼쪽 방향키를 누를시
        stick_x -= stick_move;      // 스틱 x의 좌표를 stick_move(10) 만큼 왼쪽 이동
        break;
    case GLUT_KEY_RIGHT:            // 키보드의 오른쪽 방향키를 누를시
        stick_x += stick_move;      // 스틱 x의 좌표를 stick_move(10) 만큼 오른쪽 이동
        break;

    /*case GLUT_KEY_UP:             // 스틱을 위 아래로도 움직일 수 있는 기능을 구현은 하였으나
        stick_y += stick_move;      // 게임의 재미와 상관이 없다고 판단해서 주석처리하였음
        break;
    case GLUT_KEY_DOWN:
        stick_y -= stick_move;
        break;*/
    }
    
    glutPostRedisplay();            // 윈도우를 다시 그리도록 요청
}


// 화면에 글자를 띄우는 함수
void drawBitmapText(char* string, float x, float y, float z)
{
    char* c;
    glRasterPos3f(x, y, z);                     // 글자를 띄울 곳의 위치를 정의

    for (c = string; *c != '\0'; c++)           // 받아온 문자를 하나씩 찍음
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
    }
}

// 현재 정보를 보여주는 함수
void show_info(void) {
    int total = 0;                      // 점수 계산을 위한 총 블럭 갯수
    int score = 0;                      // 점수 계산을 위한 변수

    char point[50];                     // 내가 얻은 스코어 메시지
    char max_score[50];                 // 현재 맵의 최대 스코어 메시지
    char howToReset[100];               // 초기화 방식을 알려주는 메시지
    char clear[100];                    // 클리어시 메시지


    if (map == 1) total = 50;           // 맵 1일때 블럭 총 갯수 50
    else if (map == 2) total = 45;      // 맵 2일때 블럭 총 갯수 45
    else if (map == 3) total = 40;      // 맵 3일때 블럭 총 갯수 40
    else if (map == 4) total = 26;      // 맵 4일때 블럭 총 갯수 26
    else if (map == 5) total = 39;      // 맵 5일때 블럭 총 갯수 39
    else if (map == 6) total = 50;      // 맵 6일때 블럭 총 갯수 50
    else if (map == 7) total = 45;      // 맵 7일때 블럭 총 갯수 45


    for (int i = 0; i < total; i++) {   // 블럭 총 갯수만큼 반복을 돌면서
        if (block[i].exist == false) {  // 블럭이 사라졌을때(exist == false)
            score += 100;               // 스코어 100점 추가
        }
    }

    if (total * 100 == score) {         // 총 점수와 내가 얻은 점수가 같을 때
        isClear = true;
        sprintf_s(clear, "Congratulations! try another map!");      // clear이란 변수에 클리어 메시지 넣음
        drawBitmapText(clear, 100, 400, 0);                         // clear을 x축 100, y축 400 위치에 놓음

        char map_sec[50];                                           // 클리어시간을 담기 위한 변수
        sprintf_s(map_sec, "%.2f sec", duration);                   // map_sec에 duration 값을 넣음
        drawBitmapText(map_sec, 80, 300, 0);                        // x축 80, y축 300 위치에 map_sec 출력

    }
    else { isClear = false; }

    sprintf_s(max_score, "Max Score : %d", total * 100);            // max_score 변수에 total*100 을 넣음
    sprintf_s(point, "Your Score : %d", score);                     // score 변수에 지금 얻은 point를 넣음
    sprintf_s(howToReset, "If your ball is stucked, Just press 'r' to reset ball");

    drawBitmapText(max_score, 80, 250, 0);          // max_score을 화면의 80, 250 위치에 보여줌
    drawBitmapText(point, 80, 200, 0);              // point를 화면의 80, 200 위치에 보여줌
    drawBitmapText(howToReset, 20, 100, 0);         // howToReset을 20, 100 위치에 보여줌

    int cnt = 0;
    for (int i = 0; i < 7; i++) {       
        if (check[i] == true) {                     // 클리어된 맵 수 확인
            cnt += 1;
        }
   }
    if (cnt == 7) { allClear = true; }              // 7개의 맵을 클리어 했을때 allClear = True 
}

// 초기 블럭을 보여주는 함수
void Draw_init_block(void) {

    for (int i = 0; i < 50; i++) {                              // 총 50개의 블럭

        if (i == random_num1) glColor3f(0.0, 0.0, 1.0);         // 랜덤 숫자에서 블럭의 컬러 B
        else if (i == random_num2) glColor3f(0.0, 1.0, 0.0);    // 랜덤 숫자에서 블럭의 컬러 G
        else if (i == random_num3) glColor3f(1.0, 0.0, 0.0);    // 랜덤 숫자에서 블럭의 컬러 R
        else glColor3f(1.0, 1.0, 1.0);                          // 나머지 블럭들의 컬러 White

        if (map == 3) {                     // 3번맵 (하트맵) 일때
            glColor3f(1.0, 0.0, 0.0);       // 모든 블럭의 컬러 R
        }

        block[i].draw_block();              // 블럭을 그림
    }

}

// 초기 설정 
void MyReshape(int w, int h) {

    glViewport(0, 0, w, h);                                         // 뷰 포트 설정 0,0 위치부터 w(width), h(height) 까지
    glMatrixMode(GL_PROJECTION);                                    // 투상 좌표계 설정
    glLoadIdentity();                                               // 좌표계 초기화
    gluOrtho2D(_left, _left + width, _bottom, _bottom + height);    // 관측 영역 지정
    playingBgm();                                                   // bgm재생 
}

// 클리어 체크 함수
void checkClear(void) {

    if (isClear) { check[map - 1] = true; }             // 클리어 했을때 해당 맵의 clear 값을 true로 함
    int cnt = 0;    
    for (int i = 0; i < 7; i++) {                       // 클리어 한 맵의 수를 탐색하기 위한 for문
        if (check[i] == true) { 
            cnt++;                                      // 클리어가 true일 때 cnt 추가
        }                
    }
    if (cnt == 7) {                                     // cnt가 7일때 = 모든 맵을 클리어 했을때
        allClear = true;                                // allClear을 true 로 함
        cout << "모든 맵을 클리어 하였습니다!" << endl;
    }
    else {                                              // 모든 맵을 클리어 하지 않았을 때
        cout << "아직 깨지못한 " << 7 - cnt << " 개의 맵이 남았습니다!" << endl;   // 깨지 않은 맵의 수를 보여줌
        cout << "=====================================" << endl;
    }
}

// 클리어 시간을 측정하기 위한 함수
void checkTime(void) {

    if (isClear) {                                       // 클리어 했을시
        finish = clock();                                // 시간을 담아와서
        duration = (double)(finish) / CLOCKS_PER_SEC;    // milisec단위의 시간을 초로 변환하여 duration에 담아둠
        cout << "맵 : " << map << endl;                  // 현재 맵을 콘솔창에 보여줌
        cout << duration << "초" << endl;                // 경과 시간을 콘솔창에 보여줌
        
        print_one = false;                               // 한번만 출력하기 위한 print_one 변수를 false로 둠
        checkClear();                                    // 클리어 체크 함수
    }
}

// 렌더링 화면
void RenderScene(void) {

    glClear(GL_COLOR_BUFFER_BIT);       // 색상 버퍼를 지워줌

    if (isStart == false) {             // 게임 시작전 보여줄 화면
        Draw_init_block();              // 초기 블럭을 보여줌

        char say[50] = "RGB Breaking Block";
        char say2[100] = "Press 'z', 'x', 'c', 'v', 'b', 'n', 'm' to select map";
        char say3[50] = "Click anywhere to start";
        drawBitmapText(say, 80, 200, 0);   // say를 80, 200의 위치에 띄움
        drawBitmapText(say2, 20, 100, 0);  // say2를 20, 100에 띄움
        drawBitmapText(say3, 80, 50, 0);   // say3를 80, 50에 띄움 
    }

  
    else if (gameOver) {                   // 게임오버 했을때
        char gg[50] = "Game Over";         
        drawBitmapText(gg, 200, 300, 0);   // gg를 200, 300에 띄움
    }

    else if (allClear == true) {           // 모든 맵을 클리어 했을 때
        char ac[50] = "All Clear";         // 올 클리어 메시지 ac
        drawBitmapText(ac, 200, 300, 0);   // ac를 200, 300에 띄움

        char map_sec[50];                                           // 클리어시간을 담기 위한 변수
        sprintf_s(map_sec, "%.2f sec", duration);                   // map_sec에 duration 값을 넣음
        drawBitmapText(map_sec, 200, 250, 0);                        // x축 80, y축 300 위치에 map_sec 출력
    }

    else if (isStart == true) {                                                     // 시작 했을 때
        glColor3f(1.0, 1.0, 1.0);                                                   // 컬러 값 흰색으로 둠
        show_info();                                                                // 현재 정보 함수
        moving_ball.Draw_ball(moving_ball.radius, moving_ball.x, moving_ball.y);    // 메인 볼을 그림

        if (!isClear) {                                                             // 클리어 하지 않았을 시
            moving_ball.Main_ball_Detection_to_Wall();                              // 메인 볼의 충돌 판정 => 아래쪽 충돌시 게임 오버
        }
        else if (isClear) {                                                         // 클리어 했을 시
            moving_ball.Collision_Detection_to_Walls();                             // 메인 볼의 충돌 판정 => 아래쪽 충돌 가능
        }

        moving_ball.Collision_Detection_to_Stick();                                 // 스틱 충돌
        moving_ball.Collision_Detection_to_Block();                                 // 블록 충돌

        moving_ball.x += moving_ball.velocity_x;                                    // 메인 볼의 속도(x) 적용
        moving_ball.y += moving_ball.velocity_y;                                    // 메인 볼의 속도(y) 적용

        if (block[random_num1].exist == false) {                                    // 랜덤 숫자 1에 해당하는 블럭이 사라졌을 때 (충돌 하였을때)
            
            glColor3f(0.0, 0.0, 1.0);                                               // 서브 볼의 색깔을 Blue로 둠
            second_ball.Draw_ball(second_ball.radius, second_ball.x, second_ball.y);// 서브 볼을 그림

            second_ball.Collision_Detection_to_Walls();                             // 서브 볼의 벽 충돌
            second_ball.Collision_Detection_to_Stick();                             // 서브 볼의 스틱 충돌
            second_ball.Collision_Detection_to_Block();                             // 서브 볼의 블럭 충돌

            second_ball.x += second_ball.velocity_x;                                // 서브 볼의 속도 (x) 적용
            second_ball.y += second_ball.velocity_y;                                // 서브 볼의 속도 (y) 적용

        }

        if (block[random_num2].exist == false) {                                    // 랜덤 숫자 2에 해당하는 블럭이 사라졌을 때 (충돌 하였을때)

            glColor3f(0.0, 1.0, 0.0);                                               // 서브 볼의 색깔을 Green으로 둠
            third_ball.Draw_ball(third_ball.radius, third_ball.x, third_ball.y);    // 서브 볼을 그림
                                                                                    // 아래는 서브 볼 1과 동일하게 다 적용
            third_ball.Collision_Detection_to_Walls();
            third_ball.Collision_Detection_to_Stick();
            third_ball.Collision_Detection_to_Block();

            third_ball.x += third_ball.velocity_x;
            third_ball.y += third_ball.velocity_y;

        }

        if (block[random_num3].exist == false) {                                    // 랜덤 숫자 2에 해당하는 블럭이 사라졌을 때 (충돌 하였을때)

            glColor3f(1.0, 0.0, 0.0);                                               // 서브 볼의 색깔을 Red으로 둠
            fourth_ball.Draw_ball(fourth_ball.radius, fourth_ball.x, fourth_ball.y);// 서브 볼을 그림
                                                                                    // 아래는 서브 볼 1과 동일하게 다 적용
            fourth_ball.Collision_Detection_to_Walls();
            fourth_ball.Collision_Detection_to_Stick();
            fourth_ball.Collision_Detection_to_Block();

            fourth_ball.x += fourth_ball.velocity_x;
            fourth_ball.y += fourth_ball.velocity_y;

        }

        Draw_Stick();                                                   // 스틱을 그림

        Draw_init_block();                                              // 초기 블럭을 그림

        if (stick_x <= 10) stick_x = 10;
        if (stick_x + 95 + 10 >= width) stick_x = width - 95 - 10;      // 스틱과 벽의 충돌 (벽과 10 만큼 띄우고 ) 스틱길이 = 95
    }


    if (print_one) {
        checkTime();    // 타임 체크를 한번만 출력 할 수 있도록 함
    }

    glutSwapBuffers();
    glFlush();
}

int main(int argc, char** argv) {

    cout << "=====================================" << endl;
    cout << " Welcome to RGB Block Breaking" << endl;
    cout << " RGB BLOCK Breaking에 오신걸 환영합니다!" << endl;
    cout << "=====================================" << endl;

    glutInit(&argc, argv);                          // glut 초기화
    glutInitWindowPosition(100, 100);               // 윈도우 창 위치 100, 100
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);    // 디스플레이모드 - 더블버퍼, 색상모드 RGB
    glutInitWindowSize(width, height);              // 윈도우 크기 width, height
    glutCreateWindow("RGB Breaking Block");         // 윈도우 제목 설정
    glutReshapeFunc(MyReshape);                     // 리쉐이프 이벤트
    glutDisplayFunc(RenderScene);                   // RenderScene을 콜백함수로 등록
    glutIdleFunc(RenderScene);                      // 애플리케이션 휴먼(idle) 시간에 호출될 콜백함수 등록
    glutKeyboardFunc(MyKey);                        // 키보드 이벤트를 쓰겠다.
    glutSpecialFunc(Move_Stick);                    // 키보드 이벤트 (스페셜키)를 쓰겠다.
    glutMouseFunc(mouse); 	                        // 마우스 이벤트를 쓰겠다.
    glutMainLoop();                                 

    return 0;
}
