#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include <iostream>

const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 800;
float skyColor = 0.8;  // Initial blue sky color
float bannerYOffset = -100;  // 初始牌子与气球的相对位置
int frameCounter = 0;  // 计数器来跟踪经过的帧数
bool windowsVisible = true;
bool windowsActivated = false;
//bool flowersDrawn = false;
bool balloonsFlying = false;
bool fireworksStarted = false;

struct Balloon {
    float x, y;
    float r, g, b;
};

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

public:
    Flower(float x, float y) : x(x), y(y), bloomFactor(0.0f) {}

    void update() {
        // 假设花朵会随时间慢慢绽放
        bloomFactor += 0.01f;
        if (bloomFactor > 1.0f) {
            bloomFactor = 1.0f;
        }
    }

    void draw() const {
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

void drawBalloon(Balloon balloon) {
    glColor3f(balloon.r, balloon.g, balloon.b);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; i++) {
        float theta = i * 3.142 / 180;
        glVertex2f(balloon.x + 20 * cos(theta), balloon.y + 30 * sin(theta));
    }
    glEnd();

    // Balloon string
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2i(balloon.x, balloon.y - 10);
    glVertex2i(balloon.x, balloon.y - 70);
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

void init() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);

    trees.push_back(Tree(100, 100));
    trees.push_back(Tree(500, 100));
    // Initialize balloons with random positions and bright colors
    balloons.push_back({250, -100, 1.0, 0.0, 0.0});  // Left balloon (bright red)
    balloons.push_back({350, -100, 1.0, 0.0, 0.0});  // Right balloon (bright red)
    for (int i = 0; i < 3; i++){
        balloons.push_back({static_cast<float>(rand() % WINDOW_WIDTH), -100,
                            static_cast<float>(rand()) / RAND_MAX,
                            0.0,
                            0.0});
    }
    for (int i = 0; i < 5; i++) {  // 例如，初始化5个烟花
        fireworks.push_back(Firework());
    }
}

void display() {
    if (fireworksStarted) {
        skyColor -= 0.01;  // Gradually darken the sky
        if (skyColor < 0.0) skyColor = 0.0;
    }
    glClearColor(0.5, skyColor, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
//    std::cout << "Number of trees: " << trees.size() << std::endl;
    drawGround();
    drawBuilding();

    // 绘制花朵
    for (Flower& flower : flowers) {
        flower.update();  // 更新花朵的状态
        flower.draw();
    }
    // 绘制树
    for (const Tree& tree : trees) {
        tree.draw();
    }

    if (balloonsFlying) {
        for (int i = 0; i < balloons.size(); i++) {
            auto& balloon = balloons[i];
            drawBalloon(balloon);
            if (i < 2) {  // Only the first two balloons (attached to the banner) have fixed speed
                balloon.y += 2;
            } else {
                balloon.y += 1 + (rand() % 3);  // Random speed between 1 and 3
            }
        }
        if (balloons[0].y + bannerYOffset < 500) {
            drawCenteredText(balloons[0].y + bannerYOffset + 15, "2024 XJTLU Graduation Ceremony");
        } else {
            drawCenteredText(515, "2024 XJTLU Graduation Ceremony");  // Centered on the building top
            // updateFlowers();  // 这个函数已经被Flower类的update方法替代了
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


void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        balloonsFlying = true;
        windowsActivated = true;
    }
    glutPostRedisplay();
}

void timer(int) {
    frameCounter++;

    // 更新烟花
    for (Firework& firework : fireworks) {
        firework.update();  // 使用Firework类的update方法更新烟花状态

        // 如果烟花完全淡出，重新初始化
        if (firework.isFadedOut()) {  // 假设我们在Firework类中定义了一个isFadedOut方法来检查烟花是否已经完全淡出
            firework.init();  // 使用Firework类的init方法重新初始化烟花
        }
    }

    // 每30帧（即0.5秒）切换一次窗户的可见性
    if (frameCounter >= 30) {
        windowsVisible = !windowsVisible;
        frameCounter = 0;
    }

    glutPostRedisplay();
    glutTimerFunc(1000/60, timer, 0);  // 60 FPS
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

    // 初始化花朵
    for (int i = 0; i < 50; i++) {
        flowers.push_back(Flower(static_cast<float>(rand() % WINDOW_WIDTH), static_cast<float>(rand() % 100)));
    }

    glutMainLoop();  // 进入主循环
    return 0;
}
