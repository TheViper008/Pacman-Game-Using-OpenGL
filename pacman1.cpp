#include <GL/glut.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <cmath>

// Pac-Man's initial position
static double pacmanX = 0;
static double pacmanY = -1;
bool start = false;
bool eat = false;
bool left = false;
bool right = false;
bool up = false;
bool down = false;
bool pac = false;

// Points for Pac-Man to eat
float point[][3] = {
    {1, -0.6, 1}, {2, 1.2, 1}, {-1, -1, 1}, {-2, -1, 1}, {1, 1, 1}, {2, 1, 1},
    {-1, 1, 1}, {-2, 1, 1}, {-2, 2, 1}, {-1.2, 2, 1}, {0.8, 2, 1}, {0, 2, 1}, {2, 2, 1}
};

bool d[12];
bool gameOverFlag = false; // Flag to indicate if the game is over
bool winFlag = false; // Flag to indicate if all points are eaten

// Enemy position and state
const int numEnemies = 6; // Number of enemies
static double enemyX[numEnemies] = {2.0, -1.5, 0.0, -3.0, 1.0, -2.0}; // Initial positions
static double enemyY[numEnemies] = {1.0, -1.0, 2.0, 1.0, -2.0, 2.0}; // Initial positions
static double enemySpeed[numEnemies] = {0.02, 0.02, 0.02, 0.02, 0.03, 0.03}; // Speed

// Function to output text in stroke font
void stroke_output(GLfloat x, GLfloat y, const char *format, ...) {
    va_list args;
    char buffer[200], *p;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    glPushMatrix();
    glTranslatef(-2.5, y, 0);
    glScaled(0.003, 0.005, 0.005);
    for (p = buffer; *p; p++)
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
    glPopMatrix();
}

// Function to draw Pac-Man
void drawPacMan() {
    glPushMatrix();
    // Head
    glPushMatrix();
    glColor3f(1.0, 1.0, 0.0); // Yellow color
    glScalef(1, 1, 0.2);
    glutSolidSphere(0.5, 20, 20);
    glPopMatrix();

    // Eye
    glPushMatrix();
    glColor3f(1.0, 0, 1);
    glScaled(1, 1, 0.5);
    glTranslatef(0.15, 0.2, 0);
    glutSolidSphere(0.2, 20, 20);
    glPopMatrix();

    if (eat) {
        glPushMatrix();
        glColor3f(1.0, 0, 1);
        glTranslatef(0.3, -0.08, 0);
        glScaled(0.5, 0.2, 0.5);
        glutSolidSphere(0.4, 20, 20);
        glPopMatrix();
    }
    glPopMatrix();
}

// Function to draw walls
void wall(float y1) {
    glPushMatrix();
    glTranslatef(0, y1, 0);
    glBegin(GL_POLYGON);
    glColor3f(1.0, 1.0, 0.0);
    glVertex3f(-4, -0.3, -2);
    glVertex3f(4, -0.3, -2);
    glVertex3f(4, 0.3, -2);
    glVertex3f(-4, 0.3, -2);
    glEnd();
    glPopMatrix();
}

// Function to draw an enemy
void drawEnemy(double x, double y) {
    glPushMatrix();
    glTranslatef(x, y, 0);
    glColor3f(0.0, 0.0, 1.0); // Blue color
    glutSolidSphere(0.3, 20, 20);
    glPopMatrix();
}

// Function to check collision with points
void checkCollisionWithPoints() {
    for (int i = 0; i < 12; i++) {
        float dx = pacmanX - point[i][0];
        float dy = pacmanY - point[i][1];
        if (sqrt(dx * dx + dy * dy) < 0.5) { // Assuming Pac-Man radius is 0.5
            d[i] = true; // Mark point as eaten
        }
    }

    // Check if all points have been eaten
    winFlag = true;
    for (int i = 0; i < 12; i++) {
        if (!d[i]) {
            winFlag = false;
            break;
        }
    }
}

// Function to check collision with enemies
bool checkCollisionWithEnemies() {
    for (int i = 0; i < numEnemies; i++) {
        float dx = pacmanX - enemyX[i];
        float dy = pacmanY - enemyY[i];
        if (sqrt(dx * dx + dy * dy) < 0.5) { // Assuming Pac-Man radius is 0.5
            return true;
        }
    }
    return false;
}

// Function to display game over message
void gameOver(bool lost) {
    glClearColor(1.3, 1.3, 1.3, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -13.0f);

    glColor3f(1, 0, 0); // Red color for message
    if (lost) {
        stroke_output(0, 0, "YOU LOST!");
    } else {
        stroke_output(0, 0, "GAME OVER YOU LOST");
    }
    glutSwapBuffers();
}

// Function to display win message
void win() {
    glClearColor(0.0, 1.0, 0.0, 0.0); // Green background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -13.0f);

    glColor3f(1, 1, 0); // Yellow color for message
    stroke_output(0, 0, "CONGRATULATIONS!");
    stroke_output(0, -1, "YOU WON!");

    glutSwapBuffers();
}

// Function to update the game state and render
void pacMan() {
    if (gameOverFlag) {
        gameOver(false); // Display game over message
        return;
    }
    if (winFlag) {
        win(); // Display win message
        return;
    }

    glClearColor(1.0, 1.0, 1.0, 0.0); // Set background to white
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -13.0f);

    // Check boundaries and adjust direction if needed
    if (pacmanX <= -4.2) {
        pacmanX = -4.2; // Prevent moving out of bounds
        left = false;
    }
    if (pacmanX >= 4.2) {
        pacmanX = 4.2; // Prevent moving out of bounds
        right = false;
    }
    if (pacmanY <= -3.4) {
        pacmanY = -3.4; // Prevent moving out of bounds
        down = false;
    }
    if (pacmanY >= 3.4) {
        pacmanY = 3.4; // Prevent moving out of bounds
        up = false;
    }

    // Move Pac-Man
    if (left) {
        pacmanX -= 0.1; // Movement speed
    }
    if (right) {
        pacmanX += 0.1; // Movement speed
    }
    if (up) {
        pacmanY += 0.1; // Movement speed
    }
    if (down) {
        pacmanY -= 0.1; // Movement speed
    }

    // Draw background
    glBegin(GL_QUADS);
    glColor3f(0.0, 0.0, 0.0); // Black color for background
    glVertex3f(-4.8, -3.8, -2);
    glVertex3f(4.8, -3.8, -2);
    glVertex3f(4.8, 3.8, -2);
    glVertex3f(-4.8, 3.8, -2);
    glEnd();

    // Draw walls
    wall(2.8);
    wall(1.5);

    // Draw vertical walls
    glPushMatrix();
    glTranslatef(-3.6, -0.8, 0);
    glRotatef(90, 0, 0, 1);
    glScalef(0.28, 1, 1);
    wall(0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(3.6, -0.8, 0);
    glRotatef(90, 0, 0, 1);
    glScalef(0.28, 1, 1);
    wall(0);
    glPopMatrix();

    // Draw horizontal walls
    glPushMatrix();
    glTranslatef(0.7, 0, 0);
    glScalef(0.8, 1, 1);
    wall(0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.7, 0, 0);
    glScalef(0.8, 1, 1);
    wall(-1.5);
    glPopMatrix();

    wall(-2.8);

    // Move enemies and check collisions
    for (int i = 0; i < numEnemies; i++) {
        if (i % 2 == 0) { // Even-indexed enemies move horizontally
            enemyX[i] += enemySpeed[i];
            if (enemyX[i] > 4.0 || enemyX[i] < -4.0) {
                enemySpeed[i] = -enemySpeed[i]; // Reverse direction
            }
        } else { // Odd-indexed enemies move vertically
            enemyY[i] += enemySpeed[i];
            if (enemyY[i] > 3.0 || enemyY[i] < -3.0) {
                enemySpeed[i] = -enemySpeed[i]; // Reverse direction
            }
        }
        drawEnemy(enemyX[i], enemyY[i]);
    }

    if (checkCollisionWithEnemies()) {
        gameOverFlag = true; // Set game over flag
    }

    // Create Pac-Man and assign movement
    glPushMatrix();
    glTranslatef(pacmanX, pacmanY, 0);
    if (left) {
        glRotatef(180, 0, 1, 0);
    } else if (right) {
        // No rotation needed
    } else if (down) {
        glRotatef(-90, 0, 0, 1);
    } else if (up) {
        glRotatef(90, 0, 0, 1);
    }
    drawPacMan();
    glPopMatrix();

    // Draw points
    for (int i = 0; i < 12; i++) {
        glPushMatrix();
        if (d[i]) {
            glTranslatef(10, 10, 10); // Hide eaten points
        } else {
            glTranslatef(point[i][0], point[i][1], point[i][2]);
        }
        glutSolidSphere(0.1, 10, 10);
        glPopMatrix();
    }

    // Check collision with points
    checkCollisionWithPoints();

    glutSwapBuffers();
}

// Display function
void display() {
    pacMan();
}

// Keyboard input function
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'a': left = true; right = false; up = false; down = false; break;
        case 'd': right = true; left = false; up = false; down = false; break;
        case 'w': up = true; down = false; left = false; right = false; break;
        case 's': down = true; up = false; left = false; right = false; break;
        case 27: exit(0); break; // ESC key to exit
    }
    glutPostRedisplay(); // Redraw the scene
}

// Initialization function
void init() {
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 1.0, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Main function
int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Pac-Man");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}

