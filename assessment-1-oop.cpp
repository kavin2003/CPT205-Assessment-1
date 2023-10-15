#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#define DEG2RAD 0.0174532925

GLuint loadPPMTexture(const char* filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open PPM file: " << filename << std::endl;
        return 0;
    }

    std::string line;
    std::getline(file, line);  // P6
    if (line != "P6") {
        std::cerr << "Not a valid PPM file: " << filename << std::endl;
        return 0;
    }

    // Skip comments
    while (std::getline(file, line) && line[0] == '#');

    std::stringstream dimensions(line);
    int width, height;
    dimensions >> width >> height;

    std::getline(file, line);  // Max color value, usually 255
    int maxColor = std::stoi(line);

    std::vector<unsigned char> data(width * height * 3);
    file.read(reinterpret_cast<char*>(data.data()), data.size());

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}
const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 800;
float skyColor = 0.4;  // Initial blue sky color
float bannerYOffset = -100;  // 初始牌子与气球的相对位置
int frameCounter = 0;  // 计数器来跟踪经过的帧数
bool windowsVisible = true;
bool windowsActivated = false;
//bool flowersDrawn = false;
bool balloonsFlying = false;
bool fireworksStarted = false;
bool timerStarted = false;


struct Particle {
    float x, y;  // 粒子的位置
    float vx, vy;  // 粒子的速度
    float life;  // 粒子的生命周期
    float r, g, b;  // 粒子的颜色
};
class Firework {
private:
    float x, y;  // 烟花的起始位置
    std::vector<Particle> particles;  // 烟花的粒子
    float alpha = 1;  // 烟花的透明度

public:
    Firework() {
        init();
    }

    void init() {
        x = static_cast<float>(rand() % WINDOW_WIDTH);
        y = static_cast<float>(500 + rand() % 300);
        alpha = 1.0;
        int numParticles = 100 + rand() % 100;  // 生成100到200个粒子
        for (int i = 0; i < numParticles; i++) {
            float speed = static_cast<float>(rand() % 100 + 100) / 100.0;  // 速度范围：1到2
            float angle = static_cast<float>(rand() % 360) * 3.142 / 180.0;  // 随机方向
            Particle particle = {
                    x, y,
                    speed * cos(angle), speed * sin(angle),
                    2.0,  // 初始生命周期为2
                    static_cast<float>(rand()) / RAND_MAX,
                    static_cast<float>(rand()) / RAND_MAX,
                    static_cast<float>(rand()) / RAND_MAX
            };
            particles.push_back(particle);
        }
    }

    void update() {
        for (auto& particle : particles) {
            particle.x += particle.vx;
            particle.y += particle.vy;
            particle.life -= 0.01;  // 减少生命周期
            if (particle.life < 0) particle.life = 0;
        }
        alpha -= 0.01;  // 减少烟花的透明度
        if (alpha <= 0) {
            particles.clear();
            init();
        }
    }

    void draw() const {
        glPointSize(3.0);
        for (const auto& particle : particles) {
            glColor4f(particle.r, particle.g, particle.b, alpha * particle.life);  // 使用透明度
            glBegin(GL_POINTS);
            glVertex2f(particle.x, particle.y);
            glEnd();
        }
    }

    bool isFadedOut() const {
        return alpha <= 0;
    }
};

struct Leaf {
    float x, y;  // 叶子的位置
    float width, height;  // 叶子的大小
    float r, g, b;  // 叶子的颜色
};
class Tree {
private:
    int x, y;
    std::vector<Leaf> leaves;

public:
    Tree(int x, int y) : x(x), y(y) {
        generateLeaves();
    }

    void generateLeaves() {
        int numLeaves = 100 + rand() % 10;  // 生成50到100片叶子
        for (int i = 0; i < numLeaves; i++) {
            float leafX = x + static_cast<float>(rand() % 100 - 50);  // 叶子的x坐标在树干的左右50像素内
            float leafY = y + static_cast<float>(rand() % 200);  // 叶子的y坐标在树干的上方200像素内
            float leafWidth = static_cast<float>(rand() % 10 + 5);  // 叶子的宽度在5到15像素之间
            float leafHeight = static_cast<float>(rand() % 10 + 5);  // 叶子的高度在5到15像素之间
            Leaf leaf = {
                    leafX, leafY,
                    leafWidth, leafHeight,
                    0.0, static_cast<float>(rand()) / RAND_MAX, 0.0  // 叶子的颜色为随机的绿色
            };
            leaves.push_back(leaf);
        }
    }

    void draw() const {
//        std::cout << "Drawing a tree at position (" << x << ", " << y << ")" << std::endl;
        // 绘制树干
        glColor3f(0.5, 0.35, 0.05);  // 棕色
        glBegin(GL_QUADS);
        glVertex2i(x - 10, y);
        glVertex2i(x + 10, y);
        glVertex2i(x + 10, y + 200);
        glVertex2i(x - 10, y + 200);
        glEnd();

        // 绘制叶子
        for (const Leaf& leaf : leaves) {
            glColor3f(leaf.r, leaf.g, leaf.b);
            glBegin(GL_QUADS);
            glVertex2f(leaf.x - leaf.width / 2, leaf.y - leaf.height / 2);
            glVertex2f(leaf.x + leaf.width / 2, leaf.y - leaf.height / 2);
            glVertex2f(leaf.x + leaf.width / 2, leaf.y + leaf.height / 2);
            glVertex2f(leaf.x - leaf.width / 2, leaf.y + leaf.height / 2);
            glEnd();
        }
    }
};

class Flower {
private:
    float x, y;
    float bloomFactor;  // 0表示完全闭合的花苞，1表示完全开放的花
    bool isBlooming;

public:
    Flower(float x, float y) : x(x), y(y), bloomFactor(0.0f), isBlooming(false) {}

    void startBlooming() {
        isBlooming = true;
    }

    void update() {
        if (isBlooming && bloomFactor < 1.0f) {
            bloomFactor += 0.005f;  // 调整这个值以改变花朵开放的速度
        }
    }

    void draw() const {
        // 绘制茎
        glColor3f(0.0, 0.5, 0.0);  // 绿色茎
        glBegin(GL_LINES);
        glVertex2f(x, y - 5);
        glVertex2f(x, y - 25);
        glEnd();

        // 绘制叶子
        glColor3f(0.0, 0.5, 0.0);  // 绿色叶子
        glBegin(GL_POLYGON);
        glVertex2f(x - 5, y - 20);
        glVertex2f(x + 5, y - 20);
        glVertex2f(x, y - 30);
        glEnd();
        glBegin(GL_POLYGON);
        glVertex2f(x - 5, y - 10);
        glVertex2f(x + 5, y - 10);
        glVertex2f(x, y - 20);
        glEnd();

        // 绘制花蕊
        glColor3f(1.0, 1.0, 0.0);  // 黄色花蕊
        glBegin(GL_POLYGON);
        for (int i = 0; i < 360; i += 10) {
            float theta = i * 3.14159 / 180;
            float xOffset = bloomFactor * 10 * cos(theta);
            float yOffset = bloomFactor * 10 * sin(theta);
            glVertex2f(x + xOffset, y + yOffset);
        }
        glEnd();

        if (isBlooming)
        {
            // 绘制花蕊
            glColor3f(1.0, 1.0, 0.0);  // 黄色花蕊
            glBegin(GL_POLYGON);
            for (int i = 0; i < 360; i += 10) {
                float theta = i * 3.14159 / 180;
                float xOffset = bloomFactor * 10 * cos(theta);
                float yOffset = bloomFactor * 10 * sin(theta);
                glVertex2f(x + xOffset, y + yOffset);
            }
            glEnd();
            // 绘制花瓣
            glColor3f(1.0, 0.5, 1.0);  // 粉红色花瓣
            for (int petal = 0; petal < 8; petal++) {
                float angleOffset = petal * 45 * 3.14159 / 180;
                glBegin(GL_POLYGON);
                for (int i = 0; i < 360; i += 45) {
                    float theta = i * 3.14159 / 180 + angleOffset;
                    float xOffset = bloomFactor * 20 * cos(theta);
                    float yOffset = bloomFactor * 20 * sin(theta);
                    glVertex2f(x + xOffset, y + yOffset);
                }
                glEnd();
            }

        }
    }
};

class Balloon {
private:
    float x,y;
    float speed;
    float r, g, b;
    float controlPointOffset;
    bool isHoldingText;
    float windTime = 0.0f; // 是否拉着字上升

public:
    Balloon(float x, float y, float r, float g, float b, bool isHoldingText = false)
            : x(x), y(y), r(r), g(g), b(b), isHoldingText(isHoldingText) {
        speed = isHoldingText ? 2.0f : 1.0f + static_cast<float>(rand() % 3);  // 如果拉着字，速度固定为2.0，否则随机速度
    }

    void update() {
        y += speed;
        windTime += 0.05f;  // 偏移程度
        controlPointOffset = sin(windTime) * 5.0f;  // 调
    }

    void setSpeed(float newSpeed) {
        speed = newSpeed;
    }


    bool holdingText() const {
        return isHoldingText;
    }

    void setColor(float newR, float newG, float newB) {
        r = newR;
        g = newG;
        b = newB;
    }


    float getY() const {
        return y;
    }

    void setY(float newY) {
        y = newY;
    }

    void draw() {
        if (!isHoldingText == true) {
            //绘制弯曲的绳子
            glColor3f(0.5, 0.5, 0.5);  // 灰色
            float controlX = x + controlPointOffset;
            float controlY = y - 40;  // 控制点

            glBegin(GL_LINE_STRIP);  // 使用GL_LINE_STRIP来绘制连续的线段
            glVertex2f(x, y);  // 起点
            // 使用贝塞尔曲线的公式来绘制曲线
            for (float t = 0; t <= 1; t += 0.01) {
                float pointX = (1 - t) * (1 - t) * x + 2 * (1 - t) * t * controlX + t * t * x;
                float pointY = (1 - t) * (1 - t) * y + 2 * (1 - t) * t * controlY + t * t * (y - 80);
                glVertex2f(pointX, pointY);
            }
            glEnd();
        }
        else
        {
            // 绘制绳子
            glColor3f(0.5, 0.5, 0.5);  // 灰色
            glBegin(GL_LINES);
            glVertex2f(x, y - 30);  // 气球底部
            glVertex2f(x, y - 80);  // 绳子的末端
            glEnd();
        }
        // 绘制气球
        glColor3f(r, g, b);  // 红色
        glBegin(GL_POLYGON);
        for (int i = 0; i < 360; i += 10) {
            float degInRad = i * 3.14159 / 180;
            glVertex2f(x + cos(degInRad) * 20, y + sin(degInRad) * 30);  // 椭圆形的气球
        }
        glEnd();

    // 绘制高光
        float highlightWidth = 10.0f;  // 高光的宽度
        float highlightHeight = 5.0f;  // 高光的高度
        float highlightX = x;  // 高光X
        float highlightY = y + 15.0f;  // 高光Y

        glBegin(GL_TRIANGLE_FAN);
        glColor4f(1.0f, 1.0f, 1.0f, 0.6f);  // 高光颜色
        glVertex2f(highlightX, highlightY);  // 高光中心点
        for (int i = 0; i <= 360; i += 10) {  // 高光的边缘
            glColor4f(1.0f, 1.0f, 1.0f, 0.0f);  // 高光的边缘颜色
            float degInRad = i * DEG2RAD;
            glVertex2f(highlightX + cos(degInRad) * highlightWidth, highlightY + sin(degInRad) * highlightHeight);
        }
        glEnd();




    }
};

struct Star {
    float x, y;  // 星星的位置
    float brightness;  // 星星的亮度
};
struct Cloud {
    float x, y;  // 云朵的位置
    float width, height;  // 云朵的大小
};
class Sky {
private:
    float red, green, blue;
    std::vector<Star> stars;
    std::vector<Cloud> clouds;

public:
    Sky() : red(0.0), green(0.0), blue(0.5) {
        initStars();
        initClouds();
    }

    void darken() {
        blue -= 0.005;  // 每次减少的量，可以根据需要调整
        if (blue < 0.2) blue = 0.2;  // 设置最小值为偏蓝黑的黑色
    }

    float getColor() const {
        return skyColor;
    }

    void draw() const {
        for (const Star& star : stars) {
            drawStar(star);
        }

        for (const Cloud& cloud : clouds) {
            drawCloud(cloud);
        }
    }

    float getRed() const {
        return red;
    }

    float getGreen() const {
        return green;
    }

    float getBlue() const {
        return blue;
    }

    void updateClouds() {
        for (auto& cloud : clouds) {
            cloud.x -= 0.5;  // 平移速度，可以根据需要调整

            // 如果云朵完全移出屏幕，生成一个新的云朵
            if (cloud.x + cloud.width < 0) {
                cloud.x = WINDOW_WIDTH;
                cloud.y = static_cast<float>(rand() % (WINDOW_HEIGHT / 2) + (WINDOW_HEIGHT / 4));
                cloud.width = 50 + static_cast<float>(rand() % 100);
                cloud.height = 20 + static_cast<float>(rand() % 40);
            }
        }
    }

    void updateStars() {
        for (auto& star : stars) {
            star.brightness += (rand() % 3 - 1) * 0.05;  // 随机增加或减少亮度
            if (star.brightness < 0) star.brightness = 0;
            if (star.brightness > 1) star.brightness = 1;
        }
    }

private:
    void initStars() {
        for (int i = 0; i < 100; i++) {  // 创建100颗星星
            Star star = {
                    static_cast<float>(rand() % WINDOW_WIDTH),
                    static_cast<float>(rand() % (WINDOW_HEIGHT / 4) + (WINDOW_HEIGHT / 2)),  // 在屏幕的上四分之一到上四分之三之间创建星星
                    static_cast<float>(rand()) / RAND_MAX  // 随机亮度
            };
            stars.push_back(star);
        }
    }

    void initClouds() {
        for (int i = 0; i < 5; i++) {  // 创建5朵云
            Cloud cloud = {
                    static_cast<float>(rand() % WINDOW_WIDTH),
                    static_cast<float>(rand() % (WINDOW_HEIGHT / 2) + (WINDOW_HEIGHT / 4)),  // 在屏幕的上四分之一到上二分之一之间创建云朵
                    50 + static_cast<float>(rand() % 100),  // 随机宽度
                    20 + static_cast<float>(rand() % 40)  // 随机高度
            };
            clouds.push_back(cloud);
        }
    }

    void drawStar(const Star& star) const {
        glColor3f(star.brightness, star.brightness, star.brightness);
        glBegin(GL_LINES);
        glVertex2f(star.x, star.y - 2);
        glVertex2f(star.x, star.y + 2);
        glVertex2f(star.x - 2, star.y);
        glVertex2f(star.x + 2, star.y);
        glEnd();
    }

    void drawCloud(const Cloud& cloud) const{
        glColor3f(0.9, 0.9, 0.9);  // 云朵的颜色
        glBegin(GL_QUADS);
        glVertex2f(cloud.x, cloud.y);
        glVertex2f(cloud.x + cloud.width, cloud.y);
        glVertex2f(cloud.x + cloud.width, cloud.y + cloud.height);
        glVertex2f(cloud.x, cloud.y + cloud.height);
        glEnd();
    }

};



void drawBuilding() {
    // Main building
    glColor3f(0.6, 0.6, 0.6);
    glBegin(GL_QUADS);
    glVertex2i(150, 100);
    glVertex2i(450, 100);
    glVertex2i(450, 500);
    glVertex2i(150, 500);
    glEnd();

    glColor3f(0.3, 0.3, 0.3);
    for (int i = 160; i < 440; i += 60) {
        for (int j = 120; j < 480; j += 60) {
            if (windowsActivated && i == 280 && j != 180 && windowsVisible) {  // 仅当windowsVisible为true时绘制黄色窗户
                glColor3f(1.0, 1.0, 0.0);  // Yellow for activated windows
            } else {
                glColor3f(0.3, 0.3, 0.3);
            }
            glBegin(GL_QUADS);
            glVertex2i(i, j);
            glVertex2i(i + 40, j);
            glVertex2i(i + 40, j + 40);
            glVertex2i(i, j + 40);
            glEnd();
            if (windowsActivated && i == 280 && j != 180) {  // 绘制感叹号
                glColor3f(0.0, 0.0, 0.0);
                glBegin(GL_LINES);
                glVertex2i(i + 20, j + 10);
                glVertex2i(i + 20, j + 25);
                glEnd();
                glBegin(GL_POINTS);
                glVertex2i(i + 20, j + 5);
                glEnd();
            }
        }
    }
}

void drawGround() {
    glColor3f(0.0, 0.6, 0.0);  // 深绿色
    glBegin(GL_QUADS);
    glVertex2i(0, 0);
    glVertex2i(WINDOW_WIDTH, 0);
    glVertex2i(WINDOW_WIDTH, 100);  // 地面的高度，可以根据需要调整
    glVertex2i(0, 100);
    glEnd();
}

void drawArtisticText(float x, float y, const char* text) {
    // Blue bold text with white outline
    glColor3f(1.0, 1.0, 1.0);
    for (int dx = -2; dx <= 2; dx++) {
        for (int dy = -2; dy <= 2; dy++) {
            glRasterPos2i(x + dx, y + dy);
            while (*text) {
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text++);
            }
        }
    }
    glColor3f(0.0, 0.0, 1.0);
    glRasterPos2i(x, y);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text++);
    }
}

void drawCenteredText(float y, const char* text) {
    float scaleFactor = 0.2;
    float textWidth = glutStrokeLength(GLUT_STROKE_ROMAN, (unsigned char*)text) * scaleFactor;

    // 描边 (白色)
    glColor3f(1.0, 1.0, 1.0);
    for (int dx = -2; dx <= 2; dx++) {
        for (int dy = -2; dy <= 2; dy++) {
            glPushMatrix();
            glTranslatef((WINDOW_WIDTH - textWidth) / 2 + dx, y + dy, 0);
            glScalef(scaleFactor, scaleFactor, 1);
            for (const char* c = text; *c != '\0'; c++) {
                glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
            }
            glPopMatrix();
        }
    }

    glColor3f(0.0, 0.0, 1.0);
    glPushMatrix();
    glTranslatef((WINDOW_WIDTH - textWidth) / 2, y, 0);
    glScalef(scaleFactor, scaleFactor, 1);
    for (const char* c = text; *c != '\0'; c++) {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
    }
    glPopMatrix();
}

void drawInvitationButton() {
    // Red carpet
    glColor3f(0.8, 0.2, 0.2);
    glBegin(GL_QUADS);
    glVertex2i(200, 0);
    glVertex2i(400, 0);
    glVertex2i(400, 100);
    glVertex2i(200, 100);
    glEnd();

    // Vertical "Invitation" text
    const char* text = "You Have an";
    int yStart = 70;
    drawArtisticText(250, yStart, text);
    text = "Invitation!";
    drawArtisticText(260, yStart - 20, text);
}

std::vector<Balloon> balloons;
std::vector<Tree> trees;
std::vector<Firework> fireworks;
std::vector<Flower> flowers;
Sky sky;
void init() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);

    trees.push_back(Tree(100, 100));
    trees.push_back(Tree(500, 100));
    // Initialize balloons with random positions and bright colors
    balloons.push_back(Balloon(250, -100, 1.0, 0.0, 0.0, true));  // Left balloon (bright red)
    balloons.push_back(Balloon(350, -100, 1.0, 0.0, 0.0, true));  // Right balloon (bright red)
    for (int i = 0; i < 20; i++){
        balloons.push_back({static_cast<float>(rand() % WINDOW_WIDTH), -100,
                            static_cast<float>(rand()) / RAND_MAX,
                            static_cast<float>(rand()) / RAND_MAX,
                            static_cast<float>(rand()) / RAND_MAX});
    }
    //初始化烟花
    for (int i = 0; i < 5; i++) {
        fireworks.push_back(Firework());
    }
    // 初始化花朵
    for (int i = 0; i < 50; i++) {
        flowers.push_back(Flower(static_cast<float>(rand() % WINDOW_WIDTH), static_cast<float>(rand() % 100)));
    }
}

void display() {
    if (fireworksStarted) {
        sky.darken();
    }
    glClearColor(sky.getRed(), sky.getGreen(), sky.getBlue(), 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    sky.draw();

    drawGround();
    drawBuilding();

    // 绘制花朵
    for (Flower& flower : flowers) {
        flower.update();
        flower.draw();
    }
    // 绘制树
    for (const Tree& tree : trees)
    {
        tree.draw();
    }

    for (Balloon& balloon : balloons) {
        balloon.update();
    }
    if (balloonsFlying) {
        for (int i = 0; i < balloons.size(); i++) {
            auto& balloon = balloons[i];
            balloon.draw();  // 使用Balloon类的draw方法绘制气球

            if (balloon.holdingText()) {  // 如果气球拉着字，使用固定速度
                balloon.setY(balloon.getY() + 2);
            } else {
                balloon.setY(balloon.getY() + 1 + (rand() % 3));  // Random speed between 1 and 3
            }
        }

        if (balloons[0].getY() + bannerYOffset < 500) {
            drawCenteredText(balloons[0].getY() + bannerYOffset + 15, "2024 XJTLU Graduation Ceremony");
        } else {
            drawCenteredText(515, "2024 XJTLU Graduation Ceremony");  // Centered on the building top

            if (!fireworksStarted) {
                fireworksStarted = true;
            }
            if (fireworksStarted) {
                glEnable(GL_BLEND);  // 启用混合
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // 设置混合模式
                for (Firework& firework : fireworks) {
                    firework.update();  // 更新烟花的状态
                    firework.draw();
                }
            }
        }
    }

    drawInvitationButton();
    glutSwapBuffers();
}

void timer(int) {
    if (!timerStarted) {
        return;
    }
    frameCounter++;

    // 更新气球
    for (auto& balloon : balloons) {
        if (balloon.getY() > WINDOW_HEIGHT) {
            if (!balloon.holdingText()) {  // 只有不拉着字的气球才重新初始化
                // 重新初始化气球的位置和颜色
                balloon.setY(-100);  // 使气球从屏幕底部重新出现
                balloon.setColor(static_cast<float>(rand()) / RAND_MAX,
                                 static_cast<float>(rand()) / RAND_MAX,
                                 static_cast<float>(rand()) / RAND_MAX);  // 设置随机颜色
            } else {
                // 如果气球拉着字并且到达屋顶，停止上升
                balloon.setSpeed(0);
            }
        }
    }

    // 更新云朵的位置
    sky.updateClouds();

    // 更新星星的闪烁效果
    sky.updateStars();

    // 更新烟花
    for (Firework& firework : fireworks) {
        firework.update();  // 使用Firework类的update方法更新烟花状态

        // 如果烟花完全淡出，重新初始化
        if (firework.isFadedOut()) {  //使用Firework类的isFadedOut方法来检查烟花是否已经完全淡出
            firework.init();  // 使用Firework类的init方法重新初始化烟花
        }
    }

    // 每30帧切换一次窗户的可见性
    if (frameCounter >= 30) {
        windowsVisible = !windowsVisible;
        frameCounter = 0;
    }

    glutPostRedisplay();
    glutTimerFunc(1000/60, timer, 0);  // 60 FPS
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && !timerStarted == true) {
        timerStarted = true;
        glutTimerFunc(0, timer, 0);
        balloonsFlying = true;
        windowsActivated = true;
        for (Flower& flower : flowers) {
            flower.startBlooming();
        }
    }
    glutPostRedisplay();
}




int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("XJTLU Graduation Ceremony Invitation Card");

    init();  // 初始化OpenGL和场景
    glutDisplayFunc(display);  // 设置显示回调函数
    glutMouseFunc(mouse);  // 设置鼠标回调函数
    glutTimerFunc(0, timer, 0);  // 设置定时器回调函数

    glutMainLoop();  // 进入主循环
    return 0;
}
