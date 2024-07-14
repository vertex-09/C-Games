#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

#define new snake[length]
#define header snake[0]
#define tail snake[length - 1]

typedef struct part {
    char direction;
    int row;
    int column;
} snakePart;

typedef struct p {
    int row;
    int column;
} position;

snakePart snake[1200];
position list[1200];
FILE *rule, *rank;
int closed;     // 跟踪rank是否被fclose
int map[100][100];  // 记录蛇和食物是否存在
int foodMap[100][100];  // 记录食物是否存在
int foodAmount = 0;
char up = 'u', down = 'd', left = 'l', right = 'r';
int point = 0;
int length = 5;
int width = 21, height = 15;
char head = '@', body = 'O', edge = 'x', food = '$';
int highest = 0;

// 设置光标位置
void gotoXY(int x, int y) {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD p = {y, x};
    SetConsoleCursorPosition(out, p);
}

// 隐藏光标
void hideCursor() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursor;
    cursor.bVisible = FALSE;
    cursor.dwSize = sizeof(cursor);
    SetConsoleCursorInfo(h, &cursor);
}

void showCursor() {
    CONSOLE_CURSOR_INFO cursor;
    cursor.bVisible = TRUE;
    cursor.dwSize = sizeof(cursor);
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor);
}

void hello() {
    printf("贪吃蛇游戏\n");
    printf("献礼北航入学一周年！\n");
    printf("做出选择:\n");
    printf("1: 开始游戏\n");
    printf("2: 游戏规则\n");
    printf("3: 最高纪录\n");
    printf("4: 退出游戏\n");
    fflush(stdin);
}

void makeMap() {
    for (int i = 0; i < width + 2; i++) {
        printf("%c", edge);
    }
    printf("\n");
    for (int i = 1; i <= height; i++) {
        printf("%c", edge);
        for (int j = 1; j <= width; j++) {
            printf(" ");
        }
        printf("%c\n", edge);
    }
    for (int i = 0; i < width + 2; i++) {
        printf("%c", edge);
    }
}

void initSnake() {
    for (int i = 0; i < length; i++) {
        snake[i].row = (1 + height) / 2;
        snake[i].column = (1 + width) / 2 + i;
        snake[i].direction = left;
    }

    for (int i = 0; i < length; i++) {
        map[snake[i].row][snake[i].column] = 1;
    }
}

void add() {
    int upAdd = 1;
    int downAdd = 2;
    int leftAdd = 3;
    int rightAdd = 4;

    int flag;
    if ((tail.row == snake[length - 2].row) && (tail.column < snake[length - 2].column)) {
        flag = leftAdd;
    } else if ((tail.row == snake[length - 2].row) && (tail.column > snake[length - 2].column)) {
        flag = rightAdd;
    } else if ((tail.column == snake[length - 2].column) && (tail.row < snake[length - 2].row)) {
        flag = upAdd;
    } else {
        flag = downAdd;
    }

    if (flag == upAdd) {
        snake[length].column = snake[length - 1].column;
        new.row = tail.row - 1;
        if (new.row == 0) {
            new.row = height;
        }
    } else if (flag == downAdd) {
        new.column = tail.column;
        new.row = tail.row % height + 1;
    } else if (flag == leftAdd) {
        new.row = tail.row;
        new.column = tail.column - 1;
        if (new.column == 0) {
            new.column = width;
        }
    } else {
        new.row = tail.row;
        new.column = tail.column % width + 1;
    }
    map[new.row][new.column] = 1;
    gotoXY(new.row, new.column);
    printf("%c", body);
    length++;
}

void makeFood() {
    if (foodAmount >= 6) {
        return;
    }
    srand((unsigned int) time(NULL));

    int row = rand() % height + 1;
    int column = rand() % width + 1;

    if (map[row][column] == 0) {
        gotoXY(row, column);
        printf("%c", food);
        foodMap[row][column] = 1;
        map[row][column] = 1;
        foodAmount++;
    }
}

void move() {
    // 先把当前位置记录在list
    for (int i = 0; i < length; i++) {
        list[i].row = snake[i].row;
        list[i].column = snake[i].column;
    }
    // 更新蛇的位置
    // 先把除了蛇头的部分前移
    for (int i = length - 1; i >= 1; i--) {
        snake[i].row = snake[i - 1].row;
        snake[i].column = snake[i - 1].column;
    }
    // 单独处理蛇头
    if (header.direction == up) {
        header.row--;
    } else if (header.direction == down) {
        header.row++;
    } else if (header.direction == left) {
        header.column--;
    } else {
        header.column++;
    }

    for (int i = 0; i < length; i++) {
        map[snake[i].row][snake[i].column] = 1;
    }
    map[list[length - 1].row][list[length - 1].column] = 0;
    // 优化
    map[header.row][header.column] = 1;
    map[list[length - 1].row][list[length - 1].column] = 0;

    // 擦除蛇尾
    gotoXY(list[length - 1].row, list[length - 1].column);
    printf(" ");
    // 把蛇头@改成O
    gotoXY(list[0].row, list[0].column);
    printf("%c", body);
    // 添加新蛇头
    gotoXY(header.row, header.column);
    printf("%c", head);

    // 判断一下吃到食物没
    if (foodMap[header.row][header.column] == 1) {
        point++;
        add();
        foodAmount--;
        foodMap[header.row][header.column] = 0;
    }

}

int isCollided() {
    // 先检查蛇头撞墙没
    if ((header.row == 0 || header.row == height + 1) || (header.column == 0 || header.column == width + 1)) {
        return 1;
    }

    // 检查撞自己没
    for (int i = 3; i < length; i++) {
        if (snake[i].row == header.row && snake[i].column == header.column) {
            return 1;
        }
    }

    return 0;
}

void reset() {
    point = 0;
    for (int i = 0; i < length; i++) {
        snake[i].row = 0;
        snake[i].column = 0;
        snake[i].direction = left;
    }
    length = 5;
    foodAmount = 0;
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            map[i][j] = 0;
            foodMap[i][j] = 0;
        }
    }
}

void setup() {
    system("cls");
    reset();
    hideCursor();
    initSnake();
    makeMap();
    fflush(stdin);
}

void loop() {
    rank = fopen("rank.txt", "r");
    if (rank == NULL) {
        rank = fopen("D:\\GluttonousSnake\\SourceCode\\rank.txt", "r");
    }

    while (1) {
        if (_kbhit()) {
            int ch = _getch();
            if (ch == 'w' || ch == 'W') {
                if (header.direction == left || header.direction == right) {
                    header.direction = up;
                }
            } else if (ch == 's' || ch == 'S') {
                if (header.direction == left || header.direction == right) {
                    header.direction = down;
                }
            } else if (ch == 'a' || ch == 'A') {
                if (header.direction == up || header.direction == down) {
                    header.direction = left;
                }
            } else if (ch == 'd' || ch == 'D') {
                if (header.direction == up || header.direction == down) {
                    header.direction = right;
                }
            }
        }

        makeFood();
        move();
        if (isCollided()) {
            Sleep(1000);
            gotoXY(height + 3, 0);
            printf("游戏结束，分数：%d\n", point);
            showCursor();

            if (point > highest) {
                printf("恭喜你，打破纪录！\n");
                highest = point;
                fclose(rank);
                rank = fopen("rank.txt", "w");
                if (rank == NULL) {
                    rank = fopen("D:\\GluttonousSnake\\SourceCode\\rank.txt", "w");
                }
                fprintf(rank, "%d", point);
                fclose(rank);
                closed = 1;
            }

            fflush(stdin);
            printf("按回车键返回主菜单：\n");
            getchar();
            return;
        }
        Sleep(200);
    }
}

int main() {
    rule = fopen("rules.txt", "r");

    if (rule == NULL) {
        rule = fopen("D:\\GluttonousSnake\\SourceCode\\rules.txt", "r");
    }

    while (1) {
        a:
        system("cls");
        fflush(stdin);
        showCursor();
        hello();
        fflush(stdin);
        int choice;
        scanf("%d", &choice);
        if (choice == 1) {
            setup();
            loop();
            goto a;
        } else if (choice == 2) {
            // 规则
            system("cls");
            char line[111];
            while (fgets(line, 110, rule) != NULL) {
                printf("%s", line);
            }
            printf("按1返回：");
            getchar();
            getchar();
            system("cls");
            goto a;
        } else if (choice == 3) {
            // 最高纪录
            system("cls");
            printf("最高纪录：%d\n", highest);
            printf("按1返回：");
            getchar();
            getchar();
            system("cls");
            goto a;
        } else if (choice == 4) {
            break;
        } else {
            printf("请重新输入\n");
            goto a;
        }
    }

    fclose(rule);
    if (closed == 0) {
        fclose(rank);
    }

    return 0;
}
