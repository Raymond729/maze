#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>   // 为了使用 strlen()

#define MAX_SIZE 100

// 定义结构体表示迷宫
typedef struct {
    char grid[MAX_SIZE][MAX_SIZE];  // 迷宫字符网格，行列上限均为100
    int width;                      // 迷宫宽度
    int height;                     // 迷宫高度
    int playerX;                    // 玩家当前位置：列索引
    int playerY;                    // 玩家当前位置：行索引
} Maze;

/**
 * @brief 从文件中加载迷宫到 Maze 结构
 * @param filename 迷宫文件名
 * @param maze Maze 结构指针
 * @return true 加载成功, false 加载失败
 */
bool loadMaze(const char* filename, Maze* maze) {
    if (!filename || !maze) {
        fprintf(stderr, "loadMaze: 参数无效\n");
        return false;
    }

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "无法打开文件: %s\n", filename);
        return false;
    }

    // 先将迷宫数组置空
    for (int i = 0; i < MAX_SIZE; i++) {
        for (int j = 0; j < MAX_SIZE; j++) {
            maze->grid[i][j] = '\0';
        }
    }

    int row = 0;
    int startCount = 0; // 检测 'S' 出现次数
    int endCount   = 0; // 检测 'E' 出现次数

    // 一行一行地读取文件
    while (fgets(maze->grid[row], MAX_SIZE, fp) != NULL) {
        // 去掉行末的换行符
        size_t len = strlen(maze->grid[row]);
        if (len > 0 && maze->grid[row][len - 1] == '\n') {
            maze->grid[row][len - 1] = '\0';
            len--;
        }

        // 如果行太长，超过 MAX_SIZE，就截断。一般不建议这样做，这里仅作示例
        if (len >= MAX_SIZE) {
            fprintf(stderr, "警告：第 %d 行长度超过 MAX_SIZE 已被截断\n", row);
            maze->grid[row][MAX_SIZE - 1] = '\0';
            len = MAX_SIZE - 1;
        }

        // 记录迷宫中 'S' 和 'E' 的数量（本示例只做计数，不做严格验证）
        for (size_t col = 0; col < len; col++) {
            if (maze->grid[row][col] == 'S') startCount++;
            if (maze->grid[row][col] == 'E') endCount++;
        }

        row++;
        if (row >= MAX_SIZE) {
            fprintf(stderr, "警告：行数超过了 MAX_SIZE，后续迷宫不再读取\n");
            break;
        }
    }
    fclose(fp);

    // 读取的行数即迷宫的高度
    maze->height = row;

    // 这里简单取第一行的长度为迷宫宽度（若所有行长度不同则不在此示例中严谨处理）
    maze->width = (int)strlen(maze->grid[0]);

    // 对高度和宽度进行范围检查
    if (maze->height < 5 || maze->height > 100 ||
        maze->width  < 5 || maze->width  > 100) {
        fprintf(stderr, "迷宫尺寸不合法: height=%d, width=%d\n",
                maze->height, maze->width);
        return false;
    }

    // 找到玩家初始位置 S
    bool foundStart = false;
    for (int y = 0; y < maze->height; y++) {
        for (int x = 0; x < maze->width; x++) {
            if (maze->grid[y][x] == 'S') {
                maze->playerY = y;
                maze->playerX = x;
                foundStart = true;
                break;
            }
        }
        if (foundStart) break;
    }

    if (!foundStart) {
        fprintf(stderr, "未能找到玩家起始位置 'S'\n");
        return false;
    }
    // (可选) 可以进一步验证至少有一个 'E'，但此处不强制

    return true;
}

/**
 * @brief 显示整个地图并将玩家当前坐标标记为 'X'
 * @param maze Maze 结构指针
 */
void displayMap(const Maze* maze) {
    if (!maze) return;

    for (int y = 0; y < maze->height; y++) {
        for (int x = 0; x < maze->width; x++) {
            if (y == maze->playerY && x == maze->playerX) {
                // 显示玩家当前位置
                printf("X");
            } else {
                printf("%c", maze->grid[y][x]);
            }
        }
        printf("\n");
    }
}

/**
 * @brief 判断玩家是否在出口位置
 * @param maze Maze 结构指针
 * @return true 若当前位置为 'E'
 */
bool isAtExit(const Maze* maze) {
    if (!maze) return false;
    return (maze->grid[maze->playerY][maze->playerX] == 'E');
}

/**
 * @brief 判断移动是否合法
 * @param maze Maze 结构指针
 * @param newX 新的列坐标
 * @param newY 新的行坐标
 * @return true 可以移动，false 不能移动
 */
bool isValidMove(const Maze* maze, int newX, int newY) {
    // 边界检查
    if (!maze) return false;
    if (newX < 0 || newX >= maze->width || newY < 0 || newY >= maze->height) {
        return false;
    }
    // 判断是否墙壁
    if (maze->grid[newY][newX] == '#') {
        return false;
    }
    // 若不是墙且坐标合法，即可移动
    return true;
}

/**
 * @brief 尝试移动玩家
 * @param maze Maze 结构指针
 * @param direction 'W'/'w' 上, 'S'/'s' 下, 'A'/'a' 左, 'D'/'d' 右
 */
void movePlayer(Maze* maze, char direction) {
    if (!maze) return;

    int dx = 0, dy = 0;
    switch (direction) {
        case 'W': case 'w': dy = -1; break;
        case 'S': case 's': dy =  1; break;
        case 'A': case 'a': dx = -1; break;
        case 'D': case 'd': dx =  1; break;
        default: /* 其它字符不做处理 */ return;
    }

    int newX = maze->playerX + dx;
    int newY = maze->playerY + dy;

    if (isValidMove(maze, newX, newY)) {
        maze->playerX = newX;
        maze->playerY = newY;
    } else {
        printf("无法移动：撞墙或越界！\n");
    }
}

/**
 * @brief 游戏主循环
 * @param maze Maze 结构指针
 */
void gameLoop(Maze* maze) {
    if (!maze) return;

    printf("欢迎来到迷宫游戏！使用 W A S D 来移动，M 查看地图，Q 退出。\n");

    while (true) {
        // 每次输入一个字符作为指令
        printf("请输入指令(W A S D / M / Q): ");
        char command;
        // 从标准输入获取字符
        if (scanf(" %c", &command) != 1) {
            printf("输入错误，请重试。\n");
            // 如果出错，可以清理缓冲等，这里做简化处理
            continue;
        }

        // 处理指令
        if (command == 'Q' || command == 'q') {
            printf("玩家选择退出游戏。\n");
            break;
        } else if (command == 'M' || command == 'm') {
            displayMap(maze);
        } else {
            // 移动玩家
            movePlayer(maze, command);
            // 判断是否到达出口
            if (isAtExit(maze)) {
                printf("恭喜，你找到了出口！游戏结束。\n");
                break;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("用法: %s <迷宫文件名>\n", argv[0]);
        return 1;
    }

    Maze maze;
    // 尝试加载迷宫
    if (!loadMaze(argv[1], &maze)) {
        printf("加载迷宫失败！\n");
        return 1;
    }

    // 进入游戏主循环
    gameLoop(&maze);

    return 0;
}

