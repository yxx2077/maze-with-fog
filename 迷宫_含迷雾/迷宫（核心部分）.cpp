// ==================== 迷宫探险家游戏架构设计 ====================
// 项目名称: 秘境奇遇  
// 开发团队: bug终结者
// ==============================================================

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600  // Windows Vista
#endif

static const int UI_PADDING = 20;
static const int TOP_BAR_H = 100;   // 顶部 HUD 区域高度
static const int BOTTOM_HINT_H = 36;


#include <graphics.h>      // EasyX图形库头文件 - 提供图形界面功能
#include <conio.h>         // 控制台输入输出 - 处理键盘输入
#include <iostream>        // C++标准输入输出 - 调试和信息输出
#include <cmath>           // 数学函数 - 距离计算等数学运算
#include <ctime>           // 时间函数 - 随机数种子和时间记录
#include <cstdlib>         // 随机数 - 迷宫生成和随机位置
#include <cstring>         // 字符串操作 - 排行榜姓名处理
#include <string>
#include <stack>
#include <vector>
#include <queue>
#include <algorithm> 
#include <random>
#include <windows.h> 
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include <cwchar>
#include <fcntl.h>
#include <io.h>

using namespace std;

typedef pair<int, int> PII;

// -------------------- 核心功能开始 --------------------

// ==================== 核心常量定义 ====================
#define MAZE_WIDTH 15          // 迷宫宽度（格子数）- 控制迷宫横向规模
#define MAZE_HEIGHT 15         // 迷宫高度（格子数）- 控制迷宫纵向规模
#define MAX_ENEMIES 3          // 最大敌人数 - 限制同时存在的敌人数量
#define MAX_HEALTH 100         // 最大生命值 - 玩家生命值上限
#define CELL_SIZE 40           // 每个格子的像素大小 - 控制渲染尺寸
#define SCREEN_WIDTH 1000      // 屏幕宽度 - 统一窗口尺寸
#define SCREEN_HEIGHT 760      // 屏幕高度 - 统一窗口尺寸

// ==================== 核心枚举类型 ====================

/*
 * 移动方向枚举 - 统一控制玩家和敌人的移动逻辑
 * 使用枚举确保方向值的唯一性和可读性
 */
typedef enum
{
    NONE,                  // 无方向 - 用于输入处理中的默认状态
    UP,                    // 上方向
    DOWN,                  // 下方向
    LEFT,                  // 左方向
    RIGHT                  // 右方向
} Direction;

/*
 * 格子类型枚举 - 定义迷宫中每个格子的功能和属性
 * 决定格子的通行性、交互效果和渲染方式
 */
typedef enum
{
    WALL,                 // 墙 - 不可穿越的障碍物，阻挡玩家和敌人移动
    PATH,                 // 路 - 玩家和敌人可通行的路径
    KEY,                  // 钥匙 - 通关必需收集品，收集后消失并增加钥匙计数
    HEALTH,               // 血包 - 恢复玩家生命值，使用后消失并恢复生命
    EXIT                  // 出口 - 关卡目标位置，满足条件可进入下一关
} CellType;

/*
 * 游戏状态枚举 - 统一管理游戏所有界面和状态流程
 * 控制游戏主循环中的界面切换和流程管理
 */
typedef enum
{
    MENU,                // 主菜单界面 - 显示游戏标题和功能选项
    LEVEL_SELECT,        // 关卡选择界面 - 选择不同难度关卡开始游戏
    PLAYING,             // 游戏进行中 - 核心游戏循环状态
    GAME_OVER,           // 游戏失败结束 - 玩家生命值归零或时间耗尽
    LEVEL_COMPLETE,      // 关卡胜利 - 玩家通过当前关卡
    EXIT_GAME            // 退出游戏 - 程序退出状态
} GameState;

// ==================== 核心数据结构 ====================

// 坐标位置结构体
struct Position
{
    int x = 0;
    int y = 0;
};

// 迷宫格子结构体
struct Cell
{
    CellType type = WALL;
    int visited = 0;
};

// 玩家信息结构体
struct Player
{
    Position pos = { 0, 0 };
    int health = MAX_HEALTH;
    int score = 0;
    int keys = 0;
    int steps = 0;
    Direction facing = RIGHT;
};

// 敌人信息结构体
struct Enemy
{
    Position pos = { 0, 0 };
    int moveSpeed = 0;
    int currentFrame = 0;
    Position patrolPath[5] = {};
    int pathLength = 0;
    int currentPathIndex = 0;
};

// 关卡数据结构
struct LevelData
{
    int enemyCount = 0;
    int mazeComplexity = 0;
    int timeLimit = 0;
    int keyCount = 0;
    int healthPackCount = 0;
    int unlocked = 0;
    int completed = 0;
    int bestScore = 0;
    int bestTime = 0;
    int currentScore = 0;
    int currentTime = 0;
    int attempts = 0;
};

// 游戏素材结构体
struct GameResources
{
    IMAGE wall;
    IMAGE path;
    IMAGE key;
    IMAGE health;
    IMAGE exit;
    IMAGE playerUp;
    IMAGE playerDown;
    IMAGE playerLeft;
    IMAGE playerRight;
    IMAGE enemy;
};

// 花伞新增的资源结构体
struct GameImages
{
    IMAGE img_wall;  //墙壁图片
    IMAGE img_path;  //路径图片
    IMAGE img_key_hs;   //钥匙图片
    IMAGE img_heart;  //血包图片
    IMAGE img_exit;   //出口图片
    IMAGE img_player_up;  //玩家图片
    IMAGE img_player_down;  //玩家图片
    IMAGE img_player_left;  //玩家图片
    IMAGE img_player_right;  //玩家图片
    IMAGE img_enemy;   //敌人图片
    IMAGE img_fog;
    //ui界面图片
    IMAGE img_key;
    IMAGE img_bkover;      // 游戏结束背景图片
};

struct GameSounds
{
    LPCWSTR collectKeySound;   // 收集钥匙音效
    LPCWSTR hurtSound;  // 受到攻击音效
    LPCWSTR bkSound;    // 新增背景音效
    LPCWSTR healSound;  // 新增治疗音效
    LPCWSTR wallSound;  // 新增撞墙音效
};

// 游戏主数据结构
struct GameData
{
    GameState gameState = MENU;
    Cell maze[MAZE_HEIGHT][MAZE_WIDTH];
    Player player;
    Enemy enemies[MAX_ENEMIES];
    int enemyCount = 0;
    LevelData levels[3];
    int currentLevel = 0;
    int totalBestTime = 0;
    int currentSessionTime = 0;
    int highestUnlockedLevel = 0;
    time_t levelStartTime = 0;
    GameImages gameImages;
    GameSounds gameSounds;
    Position crossroads[MAZE_HEIGHT * MAZE_WIDTH];
    int crossroadsCount = 0;
    bool fogMode = false;  // 新增：迷雾模式开关
};

// ==================== 全局核心数据 ====================
GameData game;
GameResources resources;

// ==================== 透明贴图类 ====================
class transbkpng_picture//可透明贴图的Png图像类
{
private:
    IMAGE m_img;
    const wchar_t* m_path;
public:
    transbkpng_picture(const wchar_t* path, int width, int height);
    void draw(int x, int y);
};

transbkpng_picture::transbkpng_picture(const wchar_t* path, int width, int height)
    : m_path(path)
{
    loadimage(&m_img, m_path, width, height);
}

void drawAlpha(int picture_x, int picture_y, IMAGE* picture) //显示带透明通道的png图像
{
    // 变量初始化
    DWORD* dst = GetImageBuffer();              // GetImageBuffer()函数，用于获取绘图设备的显存指针，EASYX自带
    DWORD* draw = GetImageBuffer();
    DWORD* src = GetImageBuffer(picture);       //获取picture的显存指针
    int picture_width = picture->getwidth();    //获取picture的宽度，EASYX自带
    int picture_height = picture->getheight();  //获取picture的高度，EASYX自带
    int graphWidth = getwidth();                //获取绘图区的宽度，EASYX自带
    int graphHeight = getheight();              //获取绘图区的高度，EASYX自带
    int dstX = 0;                               //在显存里像素的角标

    // 实现透明贴图 公式： Cp=αp*FP+(1-αp)*BP ， 贝叶斯定理来进行点颜色的概率计算
    for (int iy = 0; iy < picture_height; iy++)
    {
        for (int ix = 0; ix < picture_width; ix++)
        {
            int srcX = ix + iy * picture_width;        //在显存里像素的角标
            int sa = ((src[srcX] & 0xff000000) >> 24); //0xAArrggbb;AA是透明度
            int sr = ((src[srcX] & 0xff0000) >> 16);   //获取RGB里的R
            int sg = ((src[srcX] & 0xff00) >> 8);      //G
            int sb = src[srcX] & 0xff;                 //B
            if (ix >= 0 && ix <= graphWidth && iy >= 0 && iy <= graphHeight && dstX <= graphWidth * graphHeight)
            {
                dstX = (ix + picture_x) + (iy + picture_y) * graphWidth; //在显存里像素的角标
                int dr = ((dst[dstX] & 0xff0000) >> 16);
                int dg = ((dst[dstX] & 0xff00) >> 8);
                int db = dst[dstX] & 0xff;
                draw[dstX] = ((sr * sa / 255 + dr * (255 - sa) / 255) << 16)  //公式： Cp=αp*FP+(1-αp)*BP  ； αp=sa/255 , FP=sr , BP=dr
                    | ((sg * sa / 255 + dg * (255 - sa) / 255) << 8)          //αp=sa/255 , FP=sg , BP=dg
                    | (sb * sa / 255 + db * (255 - sa) / 255);                //αp=sa/255 , FP=sb , BP=db
            }
        }
    }
}

void transbkpng_picture::draw(int x, int y)
{
    drawAlpha(x, y, &m_img);
}

// ==================== 音频功能 ====================
void getmcierror(MCIERROR res, const wchar_t* stage)
{
    if (res)
    {
        _setmode(_fileno(stdout), _O_U16TEXT);
        wchar_t error[400] = { 0 };
        mciGetErrorString(res, error, _countof(error));
        wcout << L"[" << stage << L"]: " << error << endl;
    }
}

void playbackgroundmusic(const wchar_t* path, bool isrepeat, int volume = -1)
{
    wchar_t cmd[100] = { 0 };
    swprintf_s(cmd, L"open %ls alias bgm", path);
    MCIERROR res = mciSendString(cmd, nullptr, 0, nullptr);
    getmcierror(res, _T("打开"));
    swprintf_s(cmd, L"play bgm %ls", isrepeat ? L"repeat" : L"");
    res = mciSendString(cmd, nullptr, 0, nullptr);
    getmcierror(res, _T("播放"));
    if (volume != -1)
    {
        swprintf_s(cmd, L"setaudio bgm volume to %d", volume);
        res = mciSendString(cmd, nullptr, 0, nullptr);
        getmcierror(res, _T("调整音量"));
    }
}

void closebackgroundmusic()
{
    MCIERROR res = mciSendString(L"close bgm", nullptr, 0, nullptr);
    getmcierror(res, _T("关闭"));
}

// ==================== 核心功能函数声明 ====================

// ==================== 初始化函数 ====================

/*
 * 功能: 初始化游戏全局数据
 * 负责人: 我菜我多练
 * 参数: 无
 * 返回值: 无
 * 详细流程:
 *   1. 初始化所有关卡的配置参数
 *   2. 重置所有关卡的动态状态（解锁状态、分数、时间等）
 *   3. 设置第一关为解锁状态
 *   4. 初始化玩家总数据
 *   5. 设置游戏状态为菜单界面
 * 设计说明:
 *   此函数只初始化全局数据，不初始化具体关卡数据
 *   具体关卡初始化由initLevel()函数负责
 */
void initGame();

/*
 * 功能: 初始化指定关卡
 * 负责人: 我菜我多练
 * 参数: level - 关卡编号(0-2)
 * 返回值: 无
 * 详细流程:
 *   1. 设置当前关卡编号，更新游戏进度记录
 *   2. 初始化玩家状态: 位置设为(1,1)，生命值100，钥匙0，步数0，面向右方向
 *   3. 根据关卡配置设置敌人数量，初始化敌人数组
 *   4. 调用 genMaze() 生成随机迷宫，根据复杂度参数调整生成算法
 *   5. 调用 setExit() 设置出口位置，确保与起点有足够距离
 *   6. 调用 placeCoreItems() 放置钥匙和血包，数量由关卡配置决定
 *   7. 调用 genPatrolPaths() 生成敌人巡逻路径，路径复杂度随关卡递增
 *   8. 重置关卡计时器，记录关卡开始时间
 *   9. 初始化敌人状态和位置，设置移动速度和巡逻路径
 *   10. 设置游戏状态为PLAYING，进入游戏主循环
 * 设计说明:
 *   - 此函数负责具体关卡的初始化，可被多次调用用于切换不同关卡
 *   - 每次调用都会完全重置当前关卡状态，确保游戏可重玩性
 *   - 关卡难度通过配置参数控制，支持渐进式难度设计
 */
void initLevel(int level);

// ==================== 游戏逻辑函数 ====================

/*
 * 功能: 处理玩家移动和碰撞
 * 负责人: Ricardoo
 * 参数: dir - 移动方向(上下左右)
 * 返回值: 无
 * 详细流程:
 *   1. 根据方向计算目标位置坐标
 *   2. 检查目标位置是否在迷宫边界内(0 到 MAZE_WIDTH-1, 0 到 MAZE_HEIGHT-1)
 *   3. 检测目标位置的碰撞类型
 *   4. 根据碰撞类型执行相应处理:
 *      - 墙壁/边界外: 阻止移动，播放碰撞音效
 *      - 通路: 更新玩家位置，步数+1
 *      - 敌人: 更新玩家位置，生命值减20，播放受伤音效
 *      - 钥匙: 更新玩家位置，钥匙数+1，移除钥匙，播放收集音效
 *      - 血包: 更新玩家位置，生命值+10，移除血包，播放治疗音效
 *      - 出口: 更新玩家位置，检查通关条件
 *   5. 更新玩家面向方向
 *   6. 刷新UI显示状态变化
 */
void playerMove(Direction dir);

/*
 * 功能: 获取玩家输入
 * 负责人: Ricardoo
 * 参数: 无
 * 返回值: 移动方向
 * 详细流程:
 *   1. 监听键盘按键事件
 *   2. 将物理按键映射为游戏操作:
 *      - W: UP
 *      - S: DOWN
 *      - A: LEFT
 *      - D: RIGHT
 *   3. 处理按键按下和释放状态
 *   4. 支持按键连按检测
 *   5. 处理输入缓冲，避免输入丢失
 */
Direction getInput();

/*
 * 功能: 更新所有敌人行为
 * 负责人: christa
 * 参数: 无
 * 返回值: 无
 * 详细流程:
 *   1. 遍历敌人数组中每个有效敌人
 *   2. 更新敌人帧计数，控制移动速度
 *   3. 当帧计数达到移动速度时，按预设巡逻路径移动到下一个路径点
 *   4. 更新敌人位置和方向状态
 *   5. 处理敌人与玩家的碰撞检测
 *   6. 重置满足移动条件的敌人帧计数
 */
void updateEnemies();

/*
 * 功能: 生成敌人巡逻路径
 * 负责人: christa
 * 参数: 无
 * 返回值: 无
 * 详细流程:
 *   1. 为每个敌人初始化巡逻路径
 *   2. 以敌人当前位置为中心，使用BFS探索周围可达区域
 *   3. 根据关卡难度决定路径点数量: 简单2个，普通3个，困难4-5个
 *   4. 从可达位置中随机选择路径点，确保不重复且距离适中
 *   5. 验证路径点间的连通性，形成巡逻回路
 *   6. 初始化当前路径索引为0
 */
void genPatrolPaths();

/*
 * 功能: 检查并处理游戏状态
 * 负责人: 浮生如梦
 * 参数: 无
 * 返回值: 游戏状态代码
 * 详细流程:
 *   1. 检查失败条件:
 *      - 玩家生命值 ≤ 0 → 返回 GAME_OVER 状态
 *      - 关卡时间超时 → 返回 GAME_OVER 状态
 *   2. 检查胜利条件:
 *      - 玩家到达出口且钥匙足够 → 返回 LEVEL_COMPLETE 状态
 *   3. 默认返回游戏进行中状态
 */
GameState checkGameState();

/*
 * 功能: 处理关卡通关
 * 负责人: 浮生如梦
 * 参数: 无
 * 返回值: 无
 * 详细流程:
 *   1. 记录当前关卡的本次成绩：得分、用时、步数等数据
 *   2. 更新最佳成绩：比较本次成绩与历史最佳，更新最佳得分和最佳用时
 *   3. 重新计算总最佳时间：累加各关卡最佳用时，更新总进度
 *   4. 解锁下一关：如果当前不是最后一关，解锁下一关卡
 *   5. 更新最高解锁关卡记录，确保进度正确保存
 *   6. 设置游戏状态为 LEVEL_COMPLETE，进入通关界面
 * 设计说明:
 *   - 通关条件验证在 checkGameState() 中完成，此函数处理通关后的逻辑
 *   - 成绩比较采用保守策略，只更新更优记录，避免覆盖玩家最佳表现
 *   - 关卡解锁采用递进式，必须完成当前关卡才能解锁下一关卡
 */
void levelComplete();

/*
 * 功能: 生成随机迷宫
 * 负责人: HFSCR7
 * 参数: 无
 * 返回值: 无
 * 详细流程:
 *   1. 初始化所有格子为墙壁，visited标记为false
 *   2. 选择固定起点(1,1)开始DFS算法
 *   3. 使用栈结构实现DFS遍历，随机选择方向打通路径
 *   4. 根据当前关卡复杂度控制分支数量
 *   5. 确保迷宫连通性，避免孤立区域
 *   6. 保证起点和终点间至少有一条通路
 */
void genMaze();

/*
 * 功能: 设置出口位置
 * 负责人: HFSCR7
 * 参数: 无
 * 返回值: 无
 * 详细流程:
 *   1. 从起点(1,1)开始BFS遍历迷宫
 *   2. 只遍历通路格子，记录每个位置到起点的距离
 *   3. 选择距离起点最远的位置作为出口
 *   4. 确保出口位置不在起点附近(最小距离阈值)
 *   5. 验证出口与起点的连通性
 *   6. 将出口位置格子类型设置为EXIT
 */
void setExit();

/*
 * 功能: 放置核心道具
 * 负责人: HFSCR7
 * 参数: 无
 * 返回值: 无
 * 详细流程:
 *   1. 根据当前关卡配置决定钥匙和血包数量
 *   2. 在迷宫中随机选择通路位置放置道具，确保道具位置不重叠
 *   3. 钥匙: 放置 keyCount 个，确保分布在不同区域
 *   4. 血包: 放置 healthPackCount 个，优先放在危险区域附近
 *   5. 确保道具位置不在起点/出口位置
 *   6. 验证所有道具位置可达且合理分布
 */
void placeCoreItems();

// ==================== 核心界面函数 ====================

/*
 * 功能: 显示主菜单界面
 * 负责人: wzf
 * 参数: 无
 * 返回值: 无
 * 详细流程:
 *   1. 显示游戏标题和Logo
 *   2. 提供主要选项:
 *      - "开始游戏": 调用 initLevel(0) 初始化第一关并开始游戏
 *      - "关卡选择": 进入关卡选择界面
 *      - "退出游戏": 退出程序
 *   3. 处理键盘输入和选项导航
 *   4. 添加菜单选项高亮效果
 *   5. 根据用户选择切换到对应状态
 */
void showMenu();

/*
 * 功能: 显示关卡选择界面
 * 负责人: wzf
 * 参数: 无
 * 返回值: 无
 * 详细流程:
 *   1. 显示已解锁关卡选项和锁定关卡提示
 *   2. 展示每个关卡的特色和挑战
 *   3. 处理关卡选择输入
 *   4. 选择后调用 initLevel(选择的关卡) 初始化对应关卡
 *   5. 自动切换到PLAYING状态进入游戏
 *   6. 提供返回主菜单选项
 */
void showLevelSelect();

/*
 * 功能: 显示游戏主界面
 * 负责人: 浮生如梦
 * 参数: 无
 * 返回值: 无
 * 详细流程:
 *   1. 进入游戏主循环，持续运行直到状态改变
 *   2. 每帧处理用户输入(WASD控制)
 *   3. 根据输入调用 playerMove() 处理玩家移动
 *   4. 调用 updateEnemies() 更新敌人行为
 *   5. 调用 checkGameState() 检查游戏状态
 *   6. 根据游戏状态切换到对应界面:
 *      - GAME_OVER: 调用 showOver() 显示失败界面
 *      - LEVEL_COMPLETE: 调用 showLevelComplete() 显示胜利界面
 *   7. 渲染游戏画面:
 *      - 调用 renderMaze() 绘制迷宫地图
 *      - 调用 renderUI() 绘制UI界面
 *   8. 处理 ESC 键暂停功能
 *   9. 控制帧率(60FPS)，保持流畅的游戏体验
 *   10. 处理窗口关闭事件
 */
void showGame();

/*
 * 功能: 显示游戏结束界面
 * 负责人: chen
 * 参数: 无
 * 返回值: 无
 * 详细流程:
 *   1. 显示玩家失败信息和原因分析：生命值耗尽或时间超时
 *   2. 展示本次关卡数据：得分、用时、步数，并与最佳记录对比显示
 *   3. 更新当前关卡尝试次数，记录玩家挑战数据
 *   4. 提供两个操作选项供玩家选择:
 *      - "重新挑战": 调用 initLevel(game.currentLevel) 重新开始当前关卡
 *      - "返回关卡选择": 回到关卡选择界面，保留其他关卡进度
 *   5. 处理用户输入选择，根据选择执行相应操作
 *   6. 添加失败音效，增强游戏体验
 * 设计说明:
 *   - 失败只重置当前关卡，保留其他关卡进度和总分数数据
 *   - 增加当前关卡的尝试次数统计
 *   - 不清除关卡最佳记录
 */
void showOver();

/*
 * 功能: 显示关卡胜利界面
 * 负责人: wzf
 * 参数: 无
 * 返回值: 无
 * 详细流程:
 *   1. 显示关卡通关成功信息
 *   2. 展示本关游戏数据: 得分、用时、步数
 *   3. 根据当前关卡决定下一步:
 *      - 非最终关卡: 提供"继续下一关"（调用 initLevel(nextLevel) ）和"返回关卡选择"选项
 *      - 最终关卡: 显示"恭喜通关！"，提供 "返回主菜单" 选项
 *   4. 播放胜利音效
 *   5. 处理用户输入选择
 */
void showLevelComplete();

/*
 * 功能: 渲染迷宫地图
 * 负责人: 花伞
 * 参数:  maze - 迷宫二维数组数据，包含所有格子类型信息
 *        player - 玩家实例数据，包含位置、方向等状态
 *        enemies - 敌人数组，包含所有敌人的位置和状态
 *        enemyCount - 当前敌人数量，控制渲染的敌人数目
 * 返回值: 无
 * 详细流程:
 *   1. 遍历迷宫二维数组每个格子，根据格子类型选择对应图片渲染
 *   2. 计算玩家位置坐标，根据面向方向渲染对应的玩家图片
 *   3. 遍历敌人数组，在各自位置渲染敌人图片
 * 设计说明:
 *   - 图片资源从全局resources结构体获取，确保资源统一管理
 *   - 渲染顺序：先背景格子，再道具，最后实体(玩家和敌人)
 */
void renderMaze(Cell maze[MAZE_HEIGHT][MAZE_WIDTH],
    const Player& player,
    const Enemy enemies[],
    int enemyCount);

void toggleFogMode();

// ==================== 迷雾模式功能 ====================

/*
 * 功能: 迷宫渲染（迷雾效果）
 * 负责人: 花伞
 * 参数:  maze - 迷宫二维数组数据，包含所有格子类型信息
 *        player - 玩家实例数据，包含位置、方向等状态
 *        enemies - 敌人数组，包含所有敌人的位置和状态
 *        enemyCount - 当前敌人数量，控制渲染的敌人数目
 * 返回值: 无
 * 详细流程:
 *   1. 遍历迷宫二维数组每个格子，计算与玩家的距离
 *   2. 根据距离判断格子是否在玩家可视范围内(周围2格)
 *   3. 在可视范围内的格子正常渲染
 *   4. 不在可视范围内的格子叠加迷雾效果
 *   5. 根据玩家面向方向渲染对应的玩家图片
 *   6. 只在可视范围内渲染敌人图片
 * 设计说明:
 *   - 迷雾效果增强游戏探索感和挑战性
 *   - 可视范围可调节，当前设置为玩家周围2格
 *   - 确保重要元素(玩家、可视范围内的敌人)始终可见
 */
void renderMaze_MIWU(Cell maze[MAZE_HEIGHT][MAZE_WIDTH],
    const Player& player,
    const Enemy enemies[],
    int enemyCount);



/*
 * 功能: 渲染游戏UI界面
 * 负责人: 我菜我多练
 * 参数:  player - 玩家状态数据，包含生命值、钥匙数、分数、步数等信息
 *        currentLevel - 当前关卡编号，用于显示关卡信息
 *        levelData - 当前关卡数据，包含时间限制和最佳记录等信息
 *        remainingTime - 剩余时间，用于显示倒计时
 * 返回值: 无
 * 详细流程:
 *   1. 在屏幕指定位置绘制生命值条：红色背景表示最大值，绿色填充表示当前生命值
 *   2. 显示钥匙数量图标和计数：使用钥匙图标和数字显示当前持有钥匙数
 *   3. 显示当前分数和关卡信息：包括当前得分、关卡编号和关卡名称
 *   4. 显示剩余时间和移动步数：时间显示采用倒计时格式，步数实时更新
 *   5. 提供操作提示：显示WASD移动控制说明和ESC暂停功能
 *   6. 使用清晰易读的字体和合理的布局，确保信息显示直观明了
 * 设计说明:
 *   - UI布局适应1000×760的窗口尺寸，合理利用屏幕空间
 *   - 颜色搭配协调，重要信息使用醒目颜色突出显示
 *   - 支持实时更新，确保UI状态与游戏状态同步
 */
void renderUI(const Player& player,
    int currentLevel,
    const LevelData& levelData,
    int remainingTime);

// ==================== 资源管理函数 ====================
/*
 * 功能: 加载所有游戏资源
 * 负责人: 花伞
 * 参数: 无
 * 返回值: 成功加载返回1，失败返回0
 * 详细流程:
 *   1. 初始化图形窗口，设置窗口大小为 1000×760 统一尺寸
 *   2. 调用loadGameImages()加载所有图片资源，验证加载结果
 *   3. 如果图片加载失败，立即返回0并关闭图形窗口，避免资源泄漏
 *   4. 初始化音频系统，建立音频设备连接和混音通道
 *   5. 加载游戏音效和背景音乐：收集、受伤、胜利、失败等音效
 *   6. 验证音频资源加载完整性，测试音频播放功能
 *   7. 设置资源加载状态标志，标记所有资源已就绪可用
 * 设计说明:
 *   - 此函数作为资源加载的总入口，统一管理所有资源初始化过程
 *   - 采用分层加载策略，图片加载失败则不继续加载音频，避免部分资源可用
 *   - 严格的错误处理机制，确保任一关键资源失败都能正确报告并处理
 */
int loadGameResources(GameImages& img, GameSounds& sound);


/*
 * 功能: 释放所有游戏资源
 * 负责人: chen
 * 参数: 无
 * 返回值: 无
 * 详细流程:
 *   1. 释放所有音频资源，停止所有正在播放的音效和背景音乐
 *   2. 关闭音频设备连接，释放音频系统占用的系统资源
 *   3. 关闭图形窗口，自动释放所有图片资源内存和图形设备上下文
 *   4. 重置资源加载状态标志为未加载状态，便于重新初始化
 *   5. 清理临时生成的资源文件和使用过程中产生的缓存数据
 *   6. 输出资源释放完成信息，确认资源清理工作正常结束
 * 设计说明:
 *   - 遵循"谁申请谁释放"原则，确保无内存泄漏和资源残留
 *   - 释放顺序与加载顺序相反，先音频后图形，符合资源依赖关系
 */
void freeGameResources();

// -------------------- 核心功能结束 --------------------

// ==================== 程序主入口 ====================

/*
 * 功能: 程序主入口点
 * 负责人: Ricardoo
 * 参数: 无
 * 返回值: 程序退出代码
 * 详细流程:
 *   1. 调用loadGameResources()加载所有游戏资源
 *   2. 检查资源加载状态，失败则退出程序
 *   3. 调用initGame()初始化全局游戏数据
 *   4. 进入游戏主循环，根据状态显示对应界面
 *   5. 管理程序生命周期和资源释放
 *   6. 程序退出时调用freeGameResources()释放资源
 */
int main() {
    // 程序初始化
    if (!loadGameResources(game.gameImages, game.gameSounds))
    {
        cout << "游戏资源加载失败, 程序退出!" << endl;
        return -1;
    }

    // 初始化全局游戏数据（不初始化具体关卡）
    initGame();

    // 游戏主循环
    while (true) {
        switch (game.gameState) {
        case MENU:
            showMenu();
            break;
        case LEVEL_SELECT:
            showLevelSelect();  
            break;
        case PLAYING:
            showGame();
            break;
        case GAME_OVER:
            showOver();
            break;
        case LEVEL_COMPLETE:
            levelComplete();    
            showLevelComplete();
            break;
        case EXIT_GAME:
            goto exit_program;
        default:
            break;
        }
    }

exit_program:
    freeGameResources();
    return 0;
}

// ==================== 函数实现框架 ====================

// ========== 初始化函数实现 ==========

void initGame()
{
    // 玩家全局默认
    game.player.pos = { 0, 0 };
    game.player.health = MAX_HEALTH;
    game.player.score = 0;
    game.player.keys = 0;
    game.player.steps = 0;
    game.player.facing = DOWN;

    //初始化三个关卡的静态配置参数
    game.levels[0].enemyCount = 1;
    game.levels[0].mazeComplexity = 1;   // 简单
    game.levels[0].timeLimit = 180;      // 时间限制180秒
    game.levels[0].keyCount = 1;
    game.levels[0].healthPackCount = 2;

    game.levels[1].enemyCount = 2;
    game.levels[1].mazeComplexity = 2;   // 普通
    game.levels[1].timeLimit = 210;
    game.levels[1].keyCount = 2;
    game.levels[1].healthPackCount = 2;

    game.levels[2].enemyCount = 3;
    game.levels[2].mazeComplexity = 3;   // 困难
    game.levels[2].timeLimit = 240;
    game.levels[2].keyCount = 2;
    game.levels[2].healthPackCount = 1;

    for (int i = 0; i < 3; ++i) {
        game.levels[i].completed = 0;
        game.levels[i].bestScore = 0;   // 尚无最佳分数记录
        game.levels[i].bestTime = 0;    // 尚无最佳用时记录（为0表示未记录）
        game.levels[i].currentScore = 0;
        game.levels[i].currentTime = 0;
        game.levels[i].attempts = 0;
        game.levels[i].unlocked = 0;    // 先统一锁定，稍后单独解锁第0关
    }

    // 其它参数设置
    game.gameState = MENU;               // 初始进主菜单
    game.currentLevel = 0;               // 逻辑上从第0关开始（未真正进入关卡）
    game.highestUnlockedLevel = 0;       // 最高解锁关：第0关
    game.totalBestTime = 0;              // 各关最佳用时之和（暂无记录为0）
    game.currentSessionTime = 0;         // 本次会话累计用时
    game.levelStartTime = 0;             // 尚未开始任何关卡
    game.enemyCount = 0;                 // 未在关卡中，敌人数量为0
    game.fogMode = 0;                    // 新增：初始化迷雾模式


    game.levels[0].unlocked = 1;

}


void initLevel(int level)
{
    // 设置当前关卡编号，更新尝试次数等动态记录
    game.currentLevel = level;          // 当前关卡索引
    LevelData& L = game.levels[level];  // 本关数据配置
    L.currentScore = 0;
    L.currentTime = 0;
    L.attempts++;   // 进入关卡视为一次新尝试

    // 重置岔路数组
    game.crossroadsCount = 0;

    // 重置玩家状态到初始值
    game.player.pos = { 1, 1 };       // 约定起点(1,1)；DFS生成也以此为起点
    game.player.health = MAX_HEALTH;  // 100生命值上限
    game.player.keys = 0;
    game.player.steps = 0;
    game.player.facing = NONE;
    game.player.score = 0;            // 新增：重置玩家分数

    //依据关卡复杂度生成迷宫
    genMaze();          // 生成随机迷宫
    setExit();          // 生成出口位置
    placeCoreItems();   // 按关卡配置放置钥匙与血包

    //初始化敌人数量与初始状态
    game.enemyCount = min(L.enemyCount, MAX_ENEMIES);

    // 敌人的移动速度随难度缩放：越难越快（数值越小越快，因其是"帧数/格"）
    // 另：由于不能在函数里面定义新函数，所以使用lambda表达式
    auto calcMoveSpeed = [&](int complexity) -> int {
        switch (complexity) {
        case 1: return 25;  // 简单关卡：25帧/格
        case 2: return 20;  // 普通关卡：20帧/格
        case 3: return 18;  // 困难关卡：15帧/格
        default: return 20;
        }
        };

    // 将敌人随机放在可通行区（PATH），避开起点(1,1)与出口(EXIT)
    int placed = 0;
    for (int i = 0; i < game.enemyCount; ++i)
    {
        // 为了避免死循环，限制尝试次数
        for (int tries = 0; tries < 200; tries++)
        {
            int rx = 2 + rand() % (MAZE_WIDTH - 3);
            int ry = 2 + rand() % (MAZE_HEIGHT - 3);

            // 只放在通路格；避开玩家与出口
            if (game.maze[ry][rx].type != PATH) continue;
            if (rx == game.player.pos.x && ry == game.player.pos.y) continue;

            // 避开出口：简单检查邻接是否是出口
            bool nearExit = false;
            for (int dy = -1; dy <= 1; ++dy) for (int dx = -1; dx <= 1; ++dx)
            {
                int nx = rx + dx, ny = ry + dy;
                if (nx < 0 || nx >= MAZE_WIDTH || ny < 0 || ny >= MAZE_HEIGHT) continue;
                if (game.maze[ny][nx].type == EXIT) { nearExit = true; break; }
            }
            if (nearExit) continue;

            // 放置敌人
            game.enemies[i].pos = { rx, ry };
            game.enemies[i].moveSpeed = calcMoveSpeed(L.mazeComplexity);
            game.enemies[i].currentFrame = 0;
            game.enemies[i].pathLength = 0;
            game.enemies[i].currentPathIndex = 0;
            placed++;
            break;
        }
    }

    //生成敌人巡逻路径
    genPatrolPaths();

    //计时器与游戏状态
    game.levelStartTime = time(nullptr);                              // 记录关卡开始时间
    cout << "initLevel: 设置关卡开始时间 = " << game.levelStartTime << endl;
    game.gameState = PLAYING;                                         // 进入游戏状态
}

// ========== 游戏逻辑函数实现 ==========
ULONGLONG g_lastHurtTime = 0;
const ULONGLONG INVINCIBLE_TIME = 500; // 500毫秒无敌时间

void playerMove(Direction dir)
{
    // 1. 根据方向计算目标位置坐标
    int targetX = game.player.pos.x;
    int targetY = game.player.pos.y;

    switch (dir) {
    case UP:    targetY -= 1; break;
    case DOWN:  targetY += 1; break;
    case LEFT:  targetX -= 1; break;
    case RIGHT: targetX += 1; break;
    case NONE:  return; // 无移动
    }

    // 2. 检查目标位置是否在迷宫边界内
    if (targetX < 0 || targetX >= MAZE_WIDTH || targetY < 0 || targetY >= MAZE_HEIGHT) {
        // 目标位置超出边界，阻止移动
        return;
    }

    // 3. 检测目标位置的碰撞类型
    CellType targetCell = game.maze[targetY][targetX].type;

    switch (targetCell) {
    case WALL:
        // 墙壁，阻止移动
        // 播放撞墙音效
        PlaySound(game.gameSounds.wallSound, NULL, SND_FILENAME | SND_ASYNC);
        return;

    case PATH:
        // 通路，允许移动
        game.player.pos.x = targetX;
        game.player.pos.y = targetY;
        game.player.steps += 1;
        break;

    case KEY:
        // 钥匙，允许移动并收集
        game.player.pos.x = targetX;
        game.player.pos.y = targetY;
        game.player.keys += 1;
        game.player.score += 50; // 收集钥匙加分
        game.maze[targetY][targetX].type = PATH; // 移除钥匙

        //播放收集钥匙音效
        PlaySound(game.gameSounds.collectKeySound, NULL, SND_FILENAME | SND_ASYNC);
        game.player.steps += 1;
        break;

    case HEALTH:
        // 血包，允许移动并治疗
        game.player.pos.x = targetX;
        game.player.pos.y = targetY;
        game.player.health += 30;
        if (game.player.health > MAX_HEALTH) game.player.health = MAX_HEALTH;
        game.player.score += 20; // 收集血包加分
        game.maze[targetY][targetX].type = PATH; // 移除血包

        //播放恢复音效
        PlaySound(game.gameSounds.healSound, NULL, SND_FILENAME | SND_ASYNC);

        game.player.steps += 1;
        break;

    case EXIT:
        // 出口，允许移动并检查通关
        game.player.pos.x = targetX;
        game.player.pos.y = targetY;
        game.player.steps += 1;
        // 检查是否有足够钥匙通关
        {
            LevelData& currentLevel = game.levels[game.currentLevel];
            if (game.player.keys >= currentLevel.keyCount) {
                // 通关成功
                game.gameState = LEVEL_COMPLETE;
            }
            else {
                // 钥匙不足，阻止通关
                return;
            }
        }
        break;

    default:
        return; // 未知类型，阻止移动
    }

    // ========== 敌人碰撞检测（带无敌帧） ==========
    ULONGLONG currentTime = GetTickCount64();
    bool isInvincible = (currentTime - g_lastHurtTime < INVINCIBLE_TIME);

    for (int i = 0; i < game.enemyCount; i++) {
        if (game.player.pos.x == game.enemies[i].pos.x &&
            game.player.pos.y == game.enemies[i].pos.y) {

            if (!isInvincible) {
                // 可以受伤
                Sleep(10);
                game.player.health -= 20;
                
                g_lastHurtTime = currentTime; // 记录受伤时间
                
                PlaySound(game.gameSounds.hurtSound, NULL, SND_FILENAME | SND_ASYNC | SND_NOSTOP);

                cout << "玩家受伤！生命值: " << game.player.health
                    << ", 进入无敌状态" << endl;

                if (game.player.health <= 0) {
                    game.player.health = 0;
                    game.gameState = GAME_OVER;
                }
            }
            else {
                // 无敌状态中，不受伤
                cout << "无敌状态中，免疫伤害" << endl;
            }
            break;
        }
    }

    // 5. 更新玩家面向方向
    game.player.facing = dir;
}


Direction getInput()
{
    static Direction lastDir = NONE;
    static ULONGLONG lastInputTime = 0;
    const ULONGLONG INITIAL_DELAY = 200;   // 首次响应延迟
    const ULONGLONG REPEAT_DELAY = 100;    // 连按延迟

    ExMessage msg;
    Direction newDir = NONE;

    // 检查按键消息
    while (peekmessage(&msg, EX_KEY)) {
        if (msg.message == WM_KEYDOWN) {
            switch (msg.vkcode) {
            case 'W': newDir = UP; break;
            case 'S': newDir = DOWN; break;
            case 'A': newDir = LEFT; break;
            case 'D': newDir = RIGHT; break;
            }
        }
    }

    ULONGLONG currentTime = GetTickCount64();

    // 如果有新按键
    if (newDir != NONE && newDir != lastDir) {
        lastDir = newDir;
        lastInputTime = currentTime;
        return newDir;
    }

    // 如果是持续按键，检查连按延迟
    if (lastDir != NONE && (GetAsyncKeyState('W') & 0x8000 ||
        GetAsyncKeyState('S') & 0x8000 ||
        GetAsyncKeyState('A') & 0x8000 ||
        GetAsyncKeyState('D') & 0x8000)) {
        ULONGLONG delay = (lastInputTime == 0) ? INITIAL_DELAY : REPEAT_DELAY;

        if (currentTime - lastInputTime >= delay) {
            lastInputTime = currentTime;
            return lastDir;
        }
    }
    else {
        lastDir = NONE; // 没有按键按下，重置
    }

    return NONE;
}

// 全局变量声明
int pathCount = 2;  // 默认路径点数量

int Distance(Position a, Position b)
{
    return abs(a.x - b.x) + abs(a.y - b.y);
}

void updateEnemies()
{
    int dx[4] = { 0, 1, 0, -1 };
    int dy[4] = { -1, 0, 1, 0 };

    for (int i = 0; i < game.enemyCount; i++)
    {
        Enemy& enemy = game.enemies[i];
        enemy.currentFrame++;

        if (enemy.currentFrame >= enemy.moveSpeed)
        {
            Position tryPos;
            bool isvalid = false;
            int attempts = 0;
            const int maxAttempts = 20; // 限制尝试次数，避免死循环

            do {
                int index = rand() % 4;
                tryPos.x = enemy.pos.x + dx[index];
                tryPos.y = enemy.pos.y + dy[index];

                // 简化条件：只要在边界内且不是墙就可以移动
                if (tryPos.x >= 0 && tryPos.x < MAZE_WIDTH &&
                    tryPos.y >= 0 && tryPos.y < MAZE_HEIGHT &&
                    game.maze[tryPos.y][tryPos.x].type != WALL) {
                    isvalid = true;
                    break; // 找到有效位置就退出
                }

                attempts++;
            } while (!isvalid && attempts < maxAttempts);

            // 如果找到有效位置就移动，否则停留在原地
            if (isvalid) {
                enemy.pos = tryPos;
            }

            enemy.currentFrame = 0;
        }

        // 碰撞检测
        if (enemy.pos.x == game.player.pos.x && enemy.pos.y == game.player.pos.y)
        {
            ULONGLONG currentTime = GetTickCount64();
            bool isInvincible = (currentTime - g_lastHurtTime < INVINCIBLE_TIME);

            if (!isInvincible)
            {
                game.player.health -= 20;
                g_lastHurtTime = currentTime; // 记录受伤时间
                PlaySound(game.gameSounds.hurtSound, NULL, SND_FILENAME | SND_ASYNC);

                if (game.player.health <= 0)
                {
                    game.player.health = 0;
                    game.gameState = GAME_OVER;
                }
            }
        }
    }
}

void genPatrolPaths()
{
    //存储生成的下标
    vector <int> index;
    //遍历每一个敌人
    for (int i = 0; i < game.enemyCount; i++)
    {
        Enemy& enemy = game.enemies[i];    //引用简化敌人书写代码
        //默认路径点数量为2
        pathCount = 2;
        if (game.levels[game.currentLevel].mazeComplexity == 3) //困难
        {
            pathCount = 3;
        }
        else if (game.levels[game.currentLevel].mazeComplexity == 4)
        {
            pathCount = 4;
        }
        else
        {
            pathCount = 5;
        }
        //随机选择一个岔路口
        Position randomcPos;
        int randomIndex;
        do
        {
            randomIndex = rand() % game.crossroadsCount;
        } while (find(index.begin(), index.end(), randomIndex) != index.end());

        index.push_back(randomIndex);
        randomcPos = game.crossroads[randomIndex];
        //以岔路口为起点放入巡逻数组
        enemy.patrolPath[0] = randomcPos;
        enemy.currentPathIndex = 0;
    }
}

GameState checkGameState()
{
    // 更新时间
    time_t currentTime = time(nullptr);
    game.levels[game.currentLevel].currentTime = (int)(currentTime - game.levelStartTime);

    if (game.player.health <= 0 || game.levels[game.currentLevel].currentTime > game.levels[game.currentLevel].timeLimit)//判断玩家是否失败
    {
        return GAME_OVER;//更改游戏状态为游戏结束
    }
    else if (game.maze[game.player.pos.y][game.player.pos.x].type == EXIT) //判断玩家是否成功走到出口
    {
        if (game.player.keys >= game.levels[game.currentLevel].keyCount)   //判断玩家是否达到关卡要求的钥匙数量
        {
            return LEVEL_COMPLETE;//更改游戏状态为关卡通关
        }
    }
    return PLAYING;//即未失败也未成功，则处于playing状态
}

void levelComplete()
{
    // 记录当前关卡的通关时间
    time_t currentTime = time(nullptr);
    int timeUsed = (int)(currentTime - game.levelStartTime);

    // 记录当前关卡的分数和时间
    game.levels[game.currentLevel].currentTime = timeUsed;
    game.levels[game.currentLevel].currentScore = game.player.score;

    // 更新最佳记录
    if (timeUsed < game.levels[game.currentLevel].bestTime || game.levels[game.currentLevel].bestTime == 0)
    {
        game.levels[game.currentLevel].bestTime = timeUsed;
    }

    // 更新最佳分数
    if (game.player.score > game.levels[game.currentLevel].bestScore)
    {
        game.levels[game.currentLevel].bestScore = game.player.score;
    }

    // 标记关卡完成
    game.levels[game.currentLevel].completed = 1;

    // 解锁下一关 - 修复逻辑
    if (game.currentLevel < 2) {
        int nextLevel = game.currentLevel + 1;

        // 解锁下一关
        game.levels[nextLevel].unlocked = 1;

        // 更新最高解锁关卡
        if (nextLevel > game.highestUnlockedLevel) {
            game.highestUnlockedLevel = nextLevel;
        }
    }
    else {
        cout << "已经是最后一关" << endl;
    }

    game.gameState = LEVEL_COMPLETE;
}


void genMaze()
{
    // 1. 初始化所有格子为墙壁，visited标记为false
    for (int i = 0; i < MAZE_HEIGHT; i++)
    {
        for (int j = 0; j < MAZE_WIDTH; j++)
        {
            game.maze[i][j].type = WALL;
            game.maze[i][j].visited = 0;
        }
    }

    // 2. 选择固定起点(1,1)开始DFS算法
    int startX = 1, startY = 1;
    game.maze[startY][startX].type = PATH;
    game.maze[startY][startX].visited = 1;

    // 使用栈结构实现DFS
    stack<PII> st;
    st.push({ startX, startY });

    // 方向数组：上、右、下、左
    vector<PII> dirc = { {0, -1}, {1, 0}, {0, 1}, {-1, 0} };

    // 创建随机数引擎
    random_device rd;
    default_random_engine rng(rd());

    // 添加最大迭代次数保护，防止死循环
    int maxIterations = MAZE_WIDTH * MAZE_HEIGHT * 10;
    int iterationCount = 0;

    // 3. 使用栈结构实现DFS遍历，随机选择方向打通路径
    while (!st.empty() && iterationCount < maxIterations)
    {
        iterationCount++;
        auto current = st.top();
        st.pop();
        int x = current.first;
        int y = current.second;

        // 随机打乱方向顺序 
        vector<PII> undirc = dirc;
        shuffle(undirc.begin(), undirc.end(), rng);

        bool foundNewPath = false;  // 标记是否找到新路径

        // 尝试四个方向
        for (const auto& dir : undirc)
        {
            int nx = x + dir.first * 2;
            int ny = y + dir.second * 2;

            // 检查边界和是否未访问
            if (nx >= 1 && nx <= MAZE_WIDTH - 1 &&
                ny >= 1 && ny <= MAZE_HEIGHT - 1 &&
                !game.maze[ny][nx].visited)
            {
                // 打通路径
                game.maze[ny][nx].type = PATH;
                game.maze[ny][nx].visited = 1;

                // 打通中间的墙
                game.maze[y + dir.second][x + dir.first].type = PATH;
                game.maze[y + dir.second][x + dir.first].visited = 1;

                // 4. 根据当前关卡复杂度控制分支数量
                int cplx = game.levels[game.currentLevel].mazeComplexity;
                if (st.size() < cplx * 10)
                {
                    st.push({ nx, ny });
                }

                // 将当前节点重新压栈以便继续探索其他方向
                st.push({ x, y });

                foundNewPath = true;
                break;  // 每次只打通一个方向
            }
        }

        // 如果没有找到新路径，就不重新压栈，让算法自然回溯
    }

    // 安全警告
    if (iterationCount >= maxIterations) {
        cout << "警告：迷宫生成达到最大迭代次数，可能不完整" << endl;
    }

    //计算岔路格子
    game.crossroadsCount = 0; // 重置岔路计数
    for (int i = 1; i < MAZE_HEIGHT - 1; i++)
    {
        for (int j = 1; j < MAZE_WIDTH - 1; j++)
        {
            // 只检查路径格子
            if (game.maze[i][j].type != PATH) continue;

            int pathCount = 0;

            // 检查四个方向的连通性
            if (game.maze[i - 1][j].type != WALL) pathCount++; // 上
            if (game.maze[i + 1][j].type != WALL) pathCount++; // 下  
            if (game.maze[i][j - 1].type != WALL) pathCount++; // 左
            if (game.maze[i][j + 1].type != WALL) pathCount++; // 右

            // 如果有3个或4个方向连通，说明是岔路
            if (pathCount >= 3 && game.crossroadsCount < MAZE_HEIGHT * MAZE_WIDTH)
            {
                // 存储岔路坐标到一维数组中
                game.crossroads[game.crossroadsCount].x = j;
                game.crossroads[game.crossroadsCount].y = i;
                game.crossroadsCount++;
            }
        }
    }

    cout << "迷宫生成完成，岔路数量: " << game.crossroadsCount << endl;
}

void setExit()
{
    // 1. 从起点(1,1)开始BFS遍历迷宫
    int startX = 1, startY = 1;

    // 记录距离和访问标记
    vector<vector<int>> dist(MAZE_HEIGHT, vector<int>(MAZE_WIDTH, -1));
    vector<vector<bool>> visited(MAZE_HEIGHT, vector<bool>(MAZE_WIDTH, 0));

    // 使用队列实现BFS
    queue<PII> q;

    // 2. 只遍历通路格子，记录每个位置到起点的距离
    q.push({ startX, startY });
    dist[startY][startX] = 0;
    visited[startY][startX] = 1;

    int maxDist = 0;
    int exitX = startX, exitY = startY;

    // BFS方向数组
    vector<PII> dirc = { {0, -1}, {1, 0}, {0, 1}, {-1, 0} };

    while (!q.empty())
    {
        auto current = q.front();
        q.pop();
        int x = current.first;
        int y = current.second;

        // 更新最远距离和位置
        if (dist[y][x] > maxDist)
        {
            maxDist = dist[y][x];
            exitX = x;
            exitY = y;
        }

        // 遍历四个方向
        for (const auto& dir : dirc)
        {
            int nx = x + dir.first;
            int ny = y + dir.second;

            // 检查边界和通路
            if (nx >= 0 && nx < MAZE_WIDTH && ny >= 0 && ny < MAZE_HEIGHT)
            {
                if (!visited[ny][nx] && game.maze[ny][nx].type == PATH)
                {
                    visited[ny][nx] = 1;
                    dist[ny][nx] = dist[y][x] + 1;
                    q.push({ nx, ny });
                }
            }
        }
    }

    // 4. 确保出口位置不在起点附近(最小距离阈值)
    int minDist = 8;
    if (maxDist < minDist)
    {
        int rex = startX, rey = startY;
        for (int i = 0; i < MAZE_HEIGHT; i++)
        {
            for (int j = 0; j < MAZE_WIDTH; j++)
            {
                if (dist[i][j] >= minDist / 2 && dist[i][j] > maxDist)
                {
                    maxDist = dist[i][j];
                    rex = j;
                    rey = i;
                }
            }
        }
        exitX = rex;
        exitY = rey;
    }

    // 5. 将出口位置格子类型设置为EXIT
    game.maze[exitY][exitX].type = EXIT;
}

void placeCoreItems()
{
    // 1. 根据当前关卡配置决定钥匙和血包数量
    int keyCount = game.levels[game.currentLevel].keyCount;
    int healthPackCount = game.levels[game.currentLevel].healthPackCount;

    // 使用vector收集所有可用的通路位置
    vector<PII> availablePositions;

    // 找到所有通路位置，排除起点和出口
    for (int i = 0; i < MAZE_HEIGHT; i++)
    {
        for (int j = 0; j < MAZE_WIDTH; j++)
        {
            if (game.maze[i][j].type == PATH && !(j == 1 && i == 1) && game.maze[i][j].type != EXIT)
            {
                availablePositions.push_back({ j, i });
            }
        }
    }

    // 创建随机数引擎
    random_device rd;
    default_random_engine rng(rd());

    // 随机打乱可用位置
    shuffle(availablePositions.begin(), availablePositions.end(), rng);

    // 方向数组
    vector<PII> dirc = { {0, -1}, {1, 0}, {0, 1}, {-1, 0} };

    // 2. 放置钥匙，确保分布合理
    vector<PII> placedKeys;
    for (const auto& pos : availablePositions)
    {
        if (placedKeys.size() >= keyCount)
            break;

        // 检查这个位置是否已经被其他道具占用
        if (game.maze[pos.second][pos.first].type != PATH)
            continue;

        // 检查与已放置钥匙的距离
        bool tooClose = 0;
        for (const auto& keyPos : placedKeys)
        {
            int dist = abs(pos.first - keyPos.first) + abs(pos.second - keyPos.second);
            if (dist < 3)
            {
                tooClose = 1;
                break;
            }
        }

        if (!tooClose)
        {
            game.maze[pos.second][pos.first].type = KEY;
            placedKeys.push_back(pos);
        }
    }

    //3. 血包: 放置 healthPackCount 个
    int healthPacksPlaced = 0;

    // 记录已放置的血包位置
    vector<PII> placedHealthPacks;

    // 在出口附近放置1个血包
    bool exitFound = 0;
    for (int i = 0; i < MAZE_HEIGHT && !exitFound; i++)
    {
        for (int j = 0; j < MAZE_WIDTH && !exitFound; j++)
        {
            if (game.maze[i][j].type == EXIT)
            {
                exitFound = 1;
                // 在出口周围寻找可用位置放置血包
                for (const auto& dir : dirc)
                {
                    int nx = j + dir.first;
                    int ny = i + dir.second;

                    // 判断边界
                    if (nx >= 0 && nx < MAZE_WIDTH && ny >= 0 && ny < MAZE_HEIGHT)
                    {
                        // 判断放置条件
                        if (game.maze[ny][nx].type == PATH && healthPacksPlaced < healthPackCount)
                        {
                            // 检查与已放置钥匙的距离
                            bool tooCloseToKey = 0;
                            for (const auto& keyPos : placedKeys)
                            {
                                int dist = abs(nx - keyPos.first) + abs(ny - keyPos.second);
                                if (dist < 2) // 血包与钥匙的距离检查可以稍微宽松些
                                {
                                    tooCloseToKey = 1;
                                    break;
                                }
                            }

                            if (!tooCloseToKey)
                            {
                                game.maze[ny][nx].type = HEALTH;
                                healthPacksPlaced++;
                                placedHealthPacks.push_back({ nx, ny });
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    // 其余血包在迷宫中随机分布
    for (const auto& pos : availablePositions)
    {
        if (healthPacksPlaced >= healthPackCount)
            break;

        // 检查位置是否可用（是通路且没有被占用）
        if (game.maze[pos.second][pos.first].type != PATH)
            continue;

        // 检查与已放置钥匙的距离
        bool tooCloseToKey = 0;
        for (const auto& keyPos : placedKeys)
        {
            int dist = abs(pos.first - keyPos.first) + abs(pos.second - keyPos.second);
            if (dist < 2)   // 血包与钥匙的最小距离
            {
                tooCloseToKey = 1;
                break;
            }
        }

        if (tooCloseToKey)
            continue;

        // 检查与已放置血包的距离
        bool tooCloseToHealth = 0;
        for (const auto& healthPos : placedHealthPacks)
        {
            int dist = abs(pos.first - healthPos.first) + abs(pos.second - healthPos.second);
            if (dist < 3)   // 血包与血包的最小距离
            {
                tooCloseToHealth = 1;
                break;
            }
        }

        if (!tooCloseToHealth)
        {
            game.maze[pos.second][pos.first].type = HEALTH;
            healthPacksPlaced++;
            placedHealthPacks.push_back(pos);
        }
    }
}

// ========== 界面函数实现 ==========

void showMenu()
{
    ExMessage msg = { 0 };
    playbackgroundmusic(L"assets/menubkmusic_loop.mp3", true);
    IMAGE img;
    loadimage(&img, _T("assets/menubackground.png"), 1200, 960);
    transbkpng_picture title(_T("assets/gametitle.png"), 354, 134);
    setbkcolor(WHITE);
    setbkmode(TRANSPARENT);
    settextstyle(39, 0, _T("方正舒体"));
    settextcolor(WHITE);
    setfillcolor(RGB(227, 225, 76));
    int highlight_y = 310;
    int highlight_x = 113;
    int offset_x, offset_y;
    int mouse_x = 0;
    int mouse_y = 0;

    // 迷雾模式显示文本
    wchar_t fogModeText[64];
    swprintf_s(fogModeText, L"迷雾模式: %s", game.fogMode ? L"开" : L"关");

    while (true)
    {
        peekmessage(&msg, EX_KEY | EX_MOUSE);
        if (msg.message == WM_KEYDOWN)
        {
            switch (msg.vkcode)
            {
            case 'W':
                if (highlight_x > 113)
                {
                    highlight_y -= 79;
                    highlight_x -= 16;
                }
                break;
            case 'S':
                if (highlight_x < 161)  // 修改：因为现在有4个选项
                {
                    highlight_y += 79;
                    highlight_x += 16;
                }
                break;
            case VK_RETURN:
                switch (highlight_x)
                {
                case 113:
                    game.gameState = PLAYING;
                    closebackgroundmusic();
                    initLevel(0);
                    msg.message = 0;
                    return;
                case 129:
                    game.gameState = LEVEL_SELECT;
                    closebackgroundmusic();
                    msg.message = 0;
                    return;
                case 145:
                    // 迷雾模式切换
                    toggleFogMode();
                    swprintf_s(fogModeText, L"迷雾模式: %s", game.fogMode ? L"开" : L"关");
                    break;
                case 161:
                    game.gameState = EXIT_GAME;
                    closebackgroundmusic();
                    msg.message = 0;
                    return;
                }
            }
        }
        else if (msg.message == WM_MOUSEMOVE)
        {
            mouse_x = msg.x;
            mouse_y = msg.y;
        }
        else if (msg.message == WM_LBUTTONDOWN)
        {
            // 鼠标点击迷雾模式选项
            if (mouse_x >= 145 && mouse_x <= 295 && mouse_y >= 468 && mouse_y <= 507)
            {
                toggleFogMode();
                swprintf_s(fogModeText, L"迷雾模式: %s", game.fogMode ? L"开" : L"关");
            }
        }

        offset_x = (int)((double)(mouse_x - 500) / 500 * 100);
        offset_y = (int)((double)(mouse_y - 380) / 380 * 100);
        BeginBatchDraw();
        cleardevice();
        putimage(0, 0, 1000, 760, &img, 100 + offset_x, 100 + offset_y);
        title.draw(425, 29);
        solidrectangle(highlight_x, highlight_y, highlight_x + 150, highlight_y + 39);
        outtextxy(113, 310, _T("开始游戏"));
        outtextxy(129, 389, _T("关卡选择"));
        outtextxy(145, 468, fogModeText); ;  // 使用动态文本
        outtextxy(161, 547, _T("退出游戏"));
        EndBatchDraw();
        msg.message = 0;
        Sleep(5);
    }
}


void showLevelSelect()
{
    // 调试信息
    cout << "进入关卡选择界面" << endl;
    cout << "关卡解锁状态: 0=" << game.levels[0].unlocked
        << ", 1=" << game.levels[1].unlocked
        << ", 2=" << game.levels[2].unlocked << endl;

    ExMessage msg;
    playbackgroundmusic(L"assets/selectbgm_loop.mp3", true, 400);
    IMAGE img;
    transbkpng_picture easy(_T("assets/feather.png"), 200, 200);
    transbkpng_picture middle(_T("assets/sword.png"), 200, 200);
    transbkpng_picture difficult(_T("assets/skull.png"), 200, 200);
    transbkpng_picture home(_T("assets/home.png"), 47, 51);
    loadimage(&img, _T("assets/select_background.jpg"), 1000, 760);
    transbkpng_picture lock(_T("assets/lock2.png"), 200, 200);

    settextstyle(30, 0, _T("方正舒体"));
    settextcolor(WHITE);

    while (true)
    {
        peekmessage(&msg, EX_MOUSE);
        BeginBatchDraw();
        cleardevice();
        putimage(0, 0, &img);
        home.draw(0, 0);

        // 绘制关卡图标
        easy.draw(100, 100);
        if (game.levels[0].unlocked == 0)
            lock.draw(100, 100);

        middle.draw(400, 100);
        if (game.levels[1].unlocked == 0)
            lock.draw(400, 100);

        difficult.draw(700, 100);
        if (game.levels[2].unlocked == 0)
            lock.draw(700, 100);

        // 关卡1点击检测 (矩形区域: 100,100 到 300,300)
        if (msg.x >= 100 && msg.x <= 300 && msg.y >= 100 && msg.y <= 300)
        {
            PlaySound(L"assets/UIselection.wav", nullptr, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
            outtextxy(24, 500, _T("迷宫简单，敌人少，慢，时间多，需要钥匙数量少,血包多"));
            if (msg.message == WM_LBUTTONDOWN)
            {
                cout << "点击关卡1，解锁状态: " << game.levels[0].unlocked << endl;
                if (game.levels[0].unlocked)
                {
                    cout << "正在进入关卡1..." << endl;
                    closebackgroundmusic();
                    initLevel(0);
                    msg.message = 0;
                    EndBatchDraw();
                    return;
                }
                else
                {
                    cout << "关卡1未解锁!" << endl;
                    PlaySound(L"assets/hurtSound.wav", NULL, SND_FILENAME | SND_ASYNC);
                }
            }
        }
        // 关卡2点击检测 (矩形区域: 400,100 到 600,300)
        else if (msg.x >= 400 && msg.x <= 600 && msg.y >= 100 && msg.y <= 300)
        {
            PlaySound(L"assets/UIselection.wav", nullptr, SND_FILENAME | SND_ASYNC);
            outtextxy(24, 500, _T("迷宫复杂度适中,敌人数量移速，时间限制适中，需要钥匙数量较多,血包少"));
            if (msg.message == WM_LBUTTONDOWN)
            {
                cout << "点击关卡2，解锁状态: " << game.levels[1].unlocked << endl;
                if (game.levels[1].unlocked)
                {
                    cout << "正在进入关卡2..." << endl;
                    closebackgroundmusic();
                    initLevel(1);
                    msg.message = 0;
                    EndBatchDraw();
                    return;
                }
                else
                {
                    cout << "关卡2未解锁!" << endl;
                    PlaySound(L"assets/hurtSound.wav", NULL, SND_FILENAME | SND_ASYNC);
                }
            }
        }
        // 关卡3点击检测 (矩形区域: 700,100 到 900,300)
        else if (msg.x >= 700 && msg.x <= 900 && msg.y >= 100 && msg.y <= 300)
        {
            PlaySound(L"assets/UIselection.wav", nullptr, SND_FILENAME | SND_ASYNC);
            outtextxy(24, 500, _T("迷宫复杂,敌人多,移动块，需要钥匙数量多,血包极少"));
            if (msg.message == WM_LBUTTONDOWN)
            {
                cout << "点击关卡3，解锁状态: " << game.levels[2].unlocked << endl;
                if (game.levels[2].unlocked)
                {
                    cout << "正在进入关卡3..." << endl;
                    closebackgroundmusic();
                    initLevel(2);
                    msg.message = 0;
                    EndBatchDraw();
                    return;
                }
                else
                {
                    cout << "关卡3未解锁!" << endl;
                    PlaySound(L"assets/hurtSound.wav", NULL, SND_FILENAME | SND_ASYNC);
                }
            }
        }
        // 返回主菜单检测
        else if (msg.x <= 47 && msg.y <= 51)
        {
            if (msg.message == WM_LBUTTONDOWN)
            {
                cout << "返回主菜单" << endl;
                closebackgroundmusic();
                PlaySound(L"assets/UIselection.wav", nullptr, SND_FILENAME | SND_ASYNC);
                game.gameState = MENU;
                msg.message = 0;
                EndBatchDraw();
                return;
            }
        }

        EndBatchDraw();
        Sleep(10);
    }
}


bool isPaused = false;
void showGame()
{
    cleardevice();
    Direction dir;
    GameState currentState;
    static bool isPaused = false;
    bool escPressed = false;      // 记录ESC键状态

    playbackgroundmusic(game.gameSounds.bkSound, true);
    // 帧率控制
    ULONGLONG lastTime = GetTickCount64();
    const ULONGLONG TARGET_FPS = 60;
    const ULONGLONG FRAME_TIME = 1000 / TARGET_FPS;

    BeginBatchDraw();
    while (game.gameState == PLAYING)
    {
        ULONGLONG currentTime = GetTickCount64();
        ULONGLONG deltaTime = currentTime - lastTime;

        if (deltaTime < FRAME_TIME) {
            Sleep(FRAME_TIME - deltaTime);
            continue;
        }

        // 暂停状态检测
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
        {
            if (!escPressed)
            {
                isPaused = !isPaused;
                escPressed = true;
            }
        }
        else
            escPressed = false;

        // 暂停状态下：只渲染，不更新游戏逻辑
        if (isPaused)
        {
            // 渲染当前游戏画面
            cleardevice();

            // ========== 修改部分开始：根据迷雾模式选择渲染函数 ==========
            if (game.fogMode) {
                renderMaze_MIWU(game.maze, game.player, game.enemies, game.enemyCount);
            }
            else {
                renderMaze(game.maze, game.player, game.enemies, game.enemyCount);
            }
            // ========== 修改部分结束 ==========

            int remainingTime = game.levels[game.currentLevel].timeLimit -
                game.levels[game.currentLevel].currentTime;
            if (remainingTime < 0) remainingTime = 0;

            renderUI(game.player, game.currentLevel, game.levels[game.currentLevel], remainingTime);

            // 显示暂停提示（覆盖在游戏画面上）
            // 使用深色半透明背景
            setfillcolor(RGB(0, 0, 0));
            solidrectangle(SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 - 80,
                SCREEN_WIDTH / 2 + 200, SCREEN_HEIGHT / 2 + 80);

            settextcolor(WHITE);
            settextstyle(36, 0, _T("Microsoft YaHei"));
            setbkmode(TRANSPARENT);
            outtextxy(SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT / 2 - 40, _T("游戏暂停"));
            settextstyle(24, 0, _T("Microsoft YaHei"));
            outtextxy(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 10, _T("按 ESC 继续游戏"));

            FlushBatchDraw();
            lastTime = currentTime;
            continue; // 跳过后续游戏逻辑更新
        }

        // 更新时间
        time_t now = time(nullptr);
        game.levels[game.currentLevel].currentTime = (int)(now - game.levelStartTime);

        // 处理输入和逻辑
        dir = getInput();
        if (dir != NONE) {
            playerMove(dir);
        }

        updateEnemies();
        currentState = checkGameState();
        if (currentState != PLAYING) {
            game.gameState = currentState;
            closebackgroundmusic();
            break;
        }

        // 渲染
        cleardevice();

        // ========== 修改部分开始：根据迷雾模式选择渲染函数 ==========
        if (game.fogMode) {
            renderMaze_MIWU(game.maze, game.player, game.enemies, game.enemyCount);
        }
        else {
            renderMaze(game.maze, game.player, game.enemies, game.enemyCount);
        }
        // ========== 修改部分结束 ==========

        int remainingTime = game.levels[game.currentLevel].timeLimit -
            game.levels[game.currentLevel].currentTime;
        if (remainingTime < 0) remainingTime = 0;

        renderUI(game.player, game.currentLevel, game.levels[game.currentLevel], remainingTime);
        FlushBatchDraw();

        lastTime = currentTime;
    }
    EndBatchDraw();
}


void showOver()
{
    cleardevice();

    // 使用关卡记录的时间
    int currentLevelTime = game.levels[game.currentLevel].currentTime;
    LevelData& current = game.levels[game.currentLevel];
    current.attempts++;

    //放置背景图片
    IMAGE bkover;
    loadimage(&bkover, L"assets/bkover.png", 1000, 780);
    putimage(0, 0, &bkover);

    //播放音效
    mciSendString(L"close lose_sound", NULL, 0, NULL);
    wchar_t templose[256];
    swprintf_s(templose, L"open assets\\game_over_bad.wav alias lose_sound");
    mciSendString(templose, NULL, 0, NULL);
    mciSendString(L"play lose_sound", NULL, 0, NULL);

    //大标题"游戏结束"
    settextcolor(RED);
    settextstyle(50, 0, L"黑体");
    setbkmode(TRANSPARENT);
    outtextxy(400, 125, L"游戏结束");

    //失败原因 
    settextcolor(WHITE);
    settextstyle(24, 0, L"宋体");
    setbkmode(TRANSPARENT);
    wchar_t reason[64];
    if (game.player.health <= 0)
    {
        wcscpy_s(reason, L"生命值耗尽!");
        settextcolor(RED);
    }
    else if (currentLevelTime >= current.timeLimit)
    {
        wcscpy_s(reason, L"时间耗尽!");
        settextcolor(YELLOW);
    }
    else {
        wcscpy_s(reason, L"游戏结束！");
        settextcolor(LIGHTGRAY);
    }
    outtextxy(430, 208, reason);

    //游戏数据展示
    settextcolor(WHITE);
    settextstyle(24, 0, L"宋体");
    wchar_t buffer[3][64];
    swprintf_s(buffer[0], 64, L"得分：%d", game.player.score);
    swprintf_s(buffer[1], 64, L"用时：%d秒", currentLevelTime);
    swprintf_s(buffer[2], 64, L"步数：%d", game.player.steps);

    outtextxy(430, 296, buffer[0]);
    outtextxy(430, 336, buffer[1]);
    outtextxy(430, 376, buffer[2]);

    //显示最佳记录 
    if (current.bestScore >= 0 || current.bestTime >= 0)
    {
        settextcolor(LIGHTGRAY);
        settextstyle(24, 0, L"宋体");
        setbkmode(TRANSPARENT);
        outtextxy(400, 448, L"--- 最佳记录 ---");

        wchar_t temp[64];
        if (current.bestScore > 0)
        {
            swprintf_s(temp, 64, L"最佳得分：%d", current.bestScore);
            outtextxy(430, 472, temp);
        }
        if (current.bestTime > 0)
        {
            swprintf_s(temp, 64, L"最佳用时：%d秒", current.bestTime);
            outtextxy(430, 496, temp);
        }
    }
    //选项按钮
    settextstyle(28, 0, L"黑体");
    settextcolor(WHITE);
    setbkmode(TRANSPARENT);
    // 按钮文字位置
    outtextxy(350, 586, L"重新开始");
    outtextxy(550, 586, L"返回主菜单");

    ExMessage msg;
    while (1)
    {
        getmessage(&msg, EX_MOUSE);
        int curx = msg.x;
        int cury = msg.y;
        if (msg.message == WM_LBUTTONDOWN)
        {
            mciSendString(L"close lose_sound", NULL, 0, NULL);
            if (curx >= 350 && curx <= 470 && cury >= 570 && cury <= 620)//重新开始
            {
                initLevel(game.currentLevel);
                return;
            }
            else if (curx >= 550 && curx <= 690 && cury >= 570 && cury <= 620)//返回主菜单
            {
                initGame();
                return;
            }
        }
    }
}


bool w_inarea(const ExMessage& ref2, int x, int y, int width, int height)
{
    if (ref2.x > x && ref2.x < x + width && ref2.y > y && ref2.y < y + height)
    {
        return true;
    }
    return false;
    
}

bool w_button(const wchar_t* text, int x, int y, const ExMessage& ref)
{
    int text_width = textwidth(text);
    int text_height = textheight(text);
    if (w_inarea(ref, x, y, text_width + 40, text_height + 40))
    {
        setfillcolor(RGB(255, 253, 85));
        if (ref.message == WM_LBUTTONDOWN)
            return true;
    }
    else
    {
        setfillcolor(RGB(158, 157, 53));

    }
    solidroundrect(x, y, x + text_width + 40, y + text_height + 40, 10, 10);
    settextcolor(WHITE);
    setbkmode(TRANSPARENT);
    outtextxy(x + 20, y + 20, text);
    return false;
}

void showLevelComplete()
{
    IMAGE last_frame;
    getimage(&last_frame, 0, 0, 1000, 760);
    transbkpng_picture white(_T("assets/trans_white.png"), 1000, 760);
    vector<transbkpng_picture*> title;
    wstring m_path;
    transbkpng_picture home(L"assets/home.png", 47, 51);
    int frame_start;
    int frame_middle;
    int index;
    int w_max_frames;
    bool islastlevel;
    ExMessage msg = { 0 };
    if (game.currentLevel < 2)
    {
        islastlevel = false;
        m_path = L"assets/LevelUp/LevelUp-n.png";
        for (int i = 0; i < 15; i++)
        {
            m_path.replace(m_path.size() - 5, 1, to_wstring(i + 1));
            if (i > 9)
            {
                m_path.erase(m_path.size() - 7, 1);
            }
            title.push_back(new transbkpng_picture(m_path.c_str(), 513, 513));
        }
        w_max_frames = 14;
    }
    else
    {
        islastlevel = true;
        m_path = L"assets/YouWin/YouWin-n.png";
        for (int i = 0; i < 27; i++)
        {
            m_path.replace(m_path.size() - 5, 1, to_wstring(i + 1));
            if (i > 9)
            {
                m_path.erase(m_path.size() - 7, 1);
            }
            title.push_back(new transbkpng_picture(m_path.c_str(), 513, 256));
        }
        w_max_frames = 26;
    }
    settextstyle(39, 0, _T("方正舒体"));
    settextcolor(WHITE);
    setbkmode(TRANSPARENT);

    wchar_t score[100] = { 0 };
    wchar_t time[100] = { 0 };
    wchar_t step[100] = { 0 };

    swprintf_s(score, L"本关得分： %d 分", game.levels[game.currentLevel].currentScore);
    swprintf_s(time, L"本关用时： %d s", game.levels[game.currentLevel].currentTime);
    swprintf_s(step, L"本关步数： %d 步", game.player.steps);

    PlaySound(L"assets/Win_sound.wav", nullptr, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
    frame_start = clock() % 2147483674;
    while (true)
    {
        peekmessage(&msg, EX_MOUSE);
        BeginBatchDraw();
        cleardevice();
        putimage(0, 0, &last_frame);
        white.draw(0, 0);
        frame_middle = clock() - frame_start;
        index = (frame_middle / (1000 / 6));
        if (index <= w_max_frames)
        {
            title[index]->draw(200, 200);
        }
        else
        {
            title[w_max_frames]->draw(200, 200);
        }
        outtextxy(100, 50, score);
        outtextxy(100, 100, time);
        outtextxy(100, 150, step);
        if (!islastlevel)
        {
            if (w_button(L"继续下一关", 100, 650, msg))
            {
                initLevel(game.currentLevel + 1);
                EndBatchDraw();
                // 释放内存
                for (auto& ptr : title) {
                    delete ptr;
                }
                return;
            }
            if (w_button(L"返回关卡选择", 650, 650, msg))
            {
                game.gameState = LEVEL_SELECT;
                EndBatchDraw();
                // 释放内存
                for (auto& ptr : title) {
                    delete ptr;
                }
                return;
            }
        }
        home.draw(0, 0);
        if (msg.x < 47 && msg.y < 51)
        {
            if (msg.message == WM_LBUTTONDOWN)
            {
                game.gameState = MENU;
                EndBatchDraw();
                // 释放内存
                for (auto& ptr : title) {
                    delete ptr;
                }
                return;
            }
        }
        EndBatchDraw();
        Sleep(5);
    }
}



void putimageAlpha(int x, int y, IMAGE* img)
{
    drawAlpha(x, y, img);
}


void toggleFogMode()
{
    game.fogMode = !game.fogMode;
    cout << "迷雾模式: " << (game.fogMode ? "开启" : "关闭") << endl;
    PlaySound(L"assets/UIselection.wav", NULL, SND_FILENAME | SND_ASYNC);
}


void renderMaze_MIWU(Cell maze[MAZE_HEIGHT][MAZE_WIDTH],
    const Player& player,
    const Enemy enemies[],
    int enemyCount)
{
    // 使用全局的game.gameImages
    GameImages& img = game.gameImages;

    for (int i = 0; i < MAZE_HEIGHT; i++)
    {
        for (int j = 0; j < MAZE_WIDTH; j++)
        {
            int x = 200 + j * CELL_SIZE;
            int y = 80 + i * CELL_SIZE;

            // ---------- 判断是否在玩家可视范围 ----------
            bool visible = abs(j - player.pos.x) <= 2 && abs(i - player.pos.y) <= 2;

            // ---------- 1. 绘制地板 ----------
            putimageAlpha(x, y, &img.img_path);

            // ---------- 2. 绘制道具 ----------
            switch (maze[i][j].type) {
            case CellType::KEY:
                putimageAlpha(x, y, &img.img_key_hs);
                break;
            case CellType::HEALTH:
                putimageAlpha(x, y, &img.img_heart);
                break;
            case CellType::EXIT:
                putimageAlpha(x, y, &img.img_exit);
                break;
            case CellType::WALL:
                putimageAlpha(x, y, &img.img_wall);
                break;
            default:
                break;
            }

            // ---------- 3. 若不在可视范围，叠加迷雾 ----------
            if (!visible)
            {
                putimageAlpha(x, y, &img.img_fog);
            }
        }
    }

    // ---------- 4. 绘制玩家 ----------
    int px = 200 + player.pos.x * CELL_SIZE;
    int py = 80 + player.pos.y * CELL_SIZE;
    switch (player.facing)
    {
    case UP:
        putimageAlpha(px, py, &img.img_player_up);
        break;
    case DOWN:
        putimageAlpha(px, py, &img.img_player_down);
        break;
    case LEFT:
        putimageAlpha(px, py, &img.img_player_left);
        break;
    case RIGHT:
        putimageAlpha(px, py, &img.img_player_right);
        break;
    default:
        putimageAlpha(px, py, &img.img_player_down);
    }

    // ---------- 5. 绘制敌人 ----------
    for (int i = 0; i < enemyCount; i++)
    {
        int ex = 200 + enemies[i].pos.x * CELL_SIZE;
        int ey = 80 + enemies[i].pos.y * CELL_SIZE;

        // 只有敌人在玩家可视范围内才绘制
        if (abs(enemies[i].pos.x - player.pos.x) <= 2 && abs(enemies[i].pos.y - player.pos.y) <= 2)
            putimageAlpha(ex, ey, &img.img_enemy);
    }
}

void renderMaze(Cell maze[MAZE_HEIGHT][MAZE_WIDTH],
    const Player& player,
    const Enemy enemies[],
    int enemyCount)
{
    // 使用全局的game.gameImages
    GameImages& img = game.gameImages;

    // 双层循环遍历
    for (int i = 0; i < MAZE_HEIGHT; i++)
    {
        for (int j = 0; j < MAZE_WIDTH; j++)
        {
            int x = 200 + j * CELL_SIZE;   // 格子的左上角X坐标
            int y = 80 + i * CELL_SIZE;    // 格子左上角的Y坐标

            // 绘制底层（路）
            putimageAlpha(x, y, &img.img_path);

            // 绘制道具（叠在地板上）
            switch (maze[i][j].type) {
            case CellType::KEY:
                putimageAlpha(x, y, &img.img_key_hs);
                break;
            case CellType::HEALTH:
                putimageAlpha(x, y, &img.img_heart);
                break;
            case CellType::EXIT:
                putimageAlpha(x, y, &img.img_exit);
                break;
            case CellType::WALL:
                putimageAlpha(x, y, &img.img_wall);
                break;
            default:
                break;
            }
        }
    }

    // 绘制玩家
    switch (player.facing)
    {
    case UP:
        putimageAlpha(200 + player.pos.x * CELL_SIZE, 80 + player.pos.y * CELL_SIZE, &img.img_player_up);
        break;
    case DOWN:
        putimageAlpha(200 + player.pos.x * CELL_SIZE, 80 + player.pos.y * CELL_SIZE, &img.img_player_down);
        break;
    case LEFT:
        putimageAlpha(200 + player.pos.x * CELL_SIZE, 80 + player.pos.y * CELL_SIZE, &img.img_player_left);
        break;
    case RIGHT:
        putimageAlpha(200 + player.pos.x * CELL_SIZE, 80 + player.pos.y * CELL_SIZE, &img.img_player_right);
        break;
    default:
        putimageAlpha(200 + player.pos.x * CELL_SIZE, 80 + player.pos.y * CELL_SIZE, &img.img_player_down);
        break;
    }

    // 绘制敌人
    for (int i = 0; i < enemyCount; i++)
    {
        putimageAlpha(200 + enemies[i].pos.x * CELL_SIZE, 80 + enemies[i].pos.y * CELL_SIZE, &img.img_enemy);
    }
}

// --- 工具：把秒转成 mm:ss ---
void formatTime(int seconds, TCHAR* buf, int n)
{
    if (seconds < 0) seconds = 0;
    int mm = seconds / 60;
    int ss = seconds % 60;
    _stprintf_s(buf, n, _T("%02d:%02d"), mm, ss);
}


void renderUI(const Player& player,
    int currentLevel,
    const LevelData& levelData,
    int remainingTime)
{
    // —— 基础文本样式 ——
    setbkmode(TRANSPARENT);
    settextcolor(RGB(20, 20, 20));
    settextstyle(24, 0, _T("Microsoft YaHei"));


    // ========== 生命值条 ==========
    const int hpX = UI_PADDING + 16;
    const int hpY = UI_PADDING + 18;
    const int hpW = 320;
    const int hpH = 22;

    // 背景条
    setlinecolor(RGB(180, 0, 0));
    setfillcolor(RGB(200, 40, 40));
    solidroundrect(hpX, hpY, hpX + hpW, hpY + hpH, 8, 8);

    // 当前值填充
    double ratio = 1.0 * player.health / MAX_HEALTH;
    ratio = max(0.0, min(1.0, ratio));
    int curW = static_cast<int>(hpW * ratio + 0.5);
    setfillcolor(RGB(60, 180, 75));
    solidroundrect(hpX, hpY, hpX + curW, hpY + hpH, 8, 8);

    // HP 数字
    TCHAR hpbuf[64];
    _stprintf_s(hpbuf, _T("HP: %d/%d"), player.health, MAX_HEALTH);
    settextcolor(WHITE);
    outtextxy(hpX + 8, hpY - 2, hpbuf);

    // ========== 钥匙数量 ==========
    // 使用 game.gameImages 而不是 resources
    int keyW = game.gameImages.img_key.getwidth();
    int keyH = game.gameImages.img_key.getheight();
    bool keyLoaded = (keyW > 0 && keyH > 0);

    // 若未加载成功则默认尺寸
    if (!keyLoaded) { keyW = keyH = 28; }

    const int keyX = hpX + hpW + 40;
    const int keyY = hpY - 3;

    // 显示钥匙图片
    putimageAlpha(keyX, keyY, &game.gameImages.img_key);

    settextcolor(RGB(20, 20, 20));
    TCHAR keybuf[64];
    _stprintf_s(keybuf, _T("x %d"), player.keys);
    outtextxy(keyX + keyW + 10, keyY + 2, keybuf);

    // ========== 分数 + 关卡信息 ==========
    const int infoX = UI_PADDING + 16;
    const int infoY = hpY + hpH + 18;

    TCHAR infobuf[256];
    _stprintf_s(infobuf, _T("Score: %d    Level: %d"), player.score, currentLevel + 1);
    settextcolor(RGB(20, 20, 20));
    outtextxy(infoX, infoY, infobuf);

    // ========== 剩余时间 + 步数 ==========
    TCHAR tbuf[64];
    formatTime(remainingTime, tbuf, 64);

    int timeBoxW = 180;
    int timeBoxH = 42;
    int timeBoxX = SCREEN_WIDTH - UI_PADDING - timeBoxW;
    int timeBoxY = UI_PADDING + 16;

    setlinecolor(RGB(50, 50, 50));
    setfillcolor(RGB(240, 240, 240));
    solidroundrect(timeBoxX, timeBoxY, timeBoxX + timeBoxW, timeBoxY + timeBoxH, 10, 10);

    settextstyle(28, 0, _T("Microsoft YaHei"));
    if (remainingTime <= 10) settextcolor(RGB(210, 30, 30));
    else                     settextcolor(RGB(20, 20, 20));
    int tx = timeBoxX + 18;
    int ty = timeBoxY + 6;
    outtextxy(tx, ty, _T("Time  "));
    outtextxy(tx + 78, ty, tbuf);

    // 步数
    settextstyle(24, 0, _T("Microsoft YaHei"));
    settextcolor(RGB(20, 20, 20));
    TCHAR stepbuf[64];
    _stprintf_s(stepbuf, _T("Steps: %d"), player.steps);
    outtextxy(timeBoxX + 6, timeBoxY + timeBoxH + 10, stepbuf);

    // ========== 操作提示 ==========
    int hintY = SCREEN_HEIGHT - UI_PADDING - BOTTOM_HINT_H;
    setlinecolor(RGB(220, 220, 220));
    rectangle(UI_PADDING, hintY, SCREEN_WIDTH - UI_PADDING, hintY + BOTTOM_HINT_H);

    settextstyle(22, 0, _T("Microsoft YaHei"));
    settextcolor(RGB(70, 70, 70));
    outtextxy(UI_PADDING + 16, hintY + 6, _T("WASD 移动   |   ESC 暂停"));

    // ========== 修改部分开始：添加迷雾模式状态显示 ==========
    // 右下角提示 - 调整位置为迷雾模式状态显示留出空间
    settextstyle(18, 0, _T("Microsoft YaHei"));
    settextcolor(RGB(140, 140, 140));

    // 显示当前游戏模式
    TCHAR modeText[32];
    _stprintf_s(modeText, _T("模式: %s"), game.fogMode ? _T("迷雾") : _T("普通"));
    outtextxy(SCREEN_WIDTH - UI_PADDING - 120, hintY + 8, modeText);

    // 调整原有提示位置
    outtextxy(SCREEN_WIDTH - UI_PADDING - 260, hintY + 8, _T("拾取钥匙 -> 找到出口"));
    // ========== 修改部分结束 ==========
}


// ========== 资源管理函数实现 ==========

int loadGameResources(GameImages& img, GameSounds& sound)   //此处应当传入一个全局的存储图片的（g_img），和音效的变量(g_sound)，
{
    //  初始化图形窗口
    initgraph(1000, 760);
    setbkcolor(WHITE);
    cleardevice();

    bool success = true;  //记录加载是否成功
    try
    {
        //2 人物与道具图片加载
        loadimage(&img.img_wall, _T("assets/wall.png"));
        loadimage(&img.img_path, _T("assets/path.jpg"));
        loadimage(&img.img_key_hs, _T("assets/key_hs.png"));
        loadimage(&img.img_heart, _T("assets/heart.png"));
        loadimage(&img.img_exit, _T("assets/door.png"));
        loadimage(&img.img_player_up, _T("assets/player_up.png"));
        loadimage(&img.img_player_down, _T("assets/player_down.png"));
        loadimage(&img.img_player_left, _T("assets/player_left.png"));
        loadimage(&img.img_player_right, _T("assets/player_right.png"));
        loadimage(&img.img_enemy, _T("assets/enemy.png"));
        loadimage(&img.img_fog, _T("assets/fog.png"));
        //ui界面的key
        loadimage(&img.img_key, _T("assets/key.png"),CELL_SIZE,CELL_SIZE);
        loadimage(&img.img_bkover, _T("assets/bkover.png"));  
    }
    catch (...)
    {
        cout << "图片加载中出现错误" << endl;
        success = false;
    }

    //  加载音频资源（只记录路径，播放时用 mciSendString）
    sound.collectKeySound = L"assets/collectKeySound.wav";
    sound.hurtSound = L"assets/hurtSound1.wav";
    sound.bkSound = L"assets/bkSound.mp3";
    sound.healSound = L"assets/healSound.wav";
    sound.wallSound = L"assets/wallSound.wav";

    // 4 输出加载结果
    if (success) {
        cout << "[系统] 所有资源加载成功！" << endl;
        return 1;
    }
    else
    {
        cout << "[警告] 资源加载失败！请检查资源文件路径。" << endl;
        return 0;
    }
}


void freeGameResources()
{
    mciSendString(L"close all", NULL, 0, NULL);  // 关闭所有MCI音频设备
    closegraph();  // 关闭图形窗口
}